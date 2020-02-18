//
//  CUAudioEngine.h
//  Cornell University Game Library (CUGL)
//
//  This module is a singleton providing a legacy (2000-era) audio engine.  Like
//  all engines of this era, it provides a flat channel structure for playing
//  sounds as well as a single channel for background music.  This is much
//  more primitive than modern sound engines, with the advantage that it is
//  simpler to use.
//
//  Because this is a singleton, there are no publicly accessible constructors
//  or intializers.  Use the static methods instead.  This singleton should be
//  used instead of AudioManager, and not used at the same time as it.
//
//  This engine has been refactored to take advantage of our more modern audio
//  graph backend.  However, we have kept the legacy API for backwards
//  compatibility with older versions of CUGL.  The mixer graph behind the
//  scenes is a little complicated because we make heavy use of AudioFader.
//  This is to prevent the audible "clicking" that comes when sound is stopped
//  or paused, and was a major problem with OpenAL.
//
//  CUGL MIT License:
//
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 12/20/18
//
#ifndef __CU_AUDIO_CHANNELS_H__
#define __CU_AUDIO_CHANNELS_H__
#include <cugl/audio/CUAudioManager.h>
#include <cugl/audio/CUSound.h>
#include <cugl/util/CUTimestamp.h>
#include <unordered_map>
#include <functional>
#include <vector>
#include <deque>
#include <queue>

/** The default fade setting for stopping and pausing */
#define DEFAULT_FADE     0.015

namespace cugl {
    /** The mixer graph classes assigned to this class */
    namespace audio {
        class AudioOutput;
        class AudioScheduler;
        class AudioMixer;
        class AudioFader;
        class AudioPanner;
    }
/**
 * Class provides a singleton audio engine
 *
 * This class is a simple (e.g. 2000-era) audio engine.  It exposes a flat
 * channel structure and there is no (publicly available) mixer graph for
 * advanced effects such as 3D audio.  If you need these features, use
 * {@link AudioManager} instead.  In fact, you should only use one of either
 * this class or AudioManager, not both (since this class built on top of
 * AudioManager).
 *
 * This class separates sound into two categories: music and effects. In theory,
 * streaming assets should be used as music and non-streaming assets should
 * be used for effects.  However, this difference is not enforced and any
 * {@link Sound} asset may be used either as a music asset or an effect asset
 * freely at any time.
 *
 * Music is treated separately because we assume that only one music asset is
 * played at a time.  Music can be queued up and there a methods to manage
 * this queue, to allow you to play a continuous, uninterrupted stream of
 * music that reacts to the player's actions.
 *
 * Effects on the other hand are intended for short sound effects that are
 * often happening in parallel.  The engine has a fixed number of "channels"
 * or slots for these sounds (historically 24) and it can only play as many
 * sounds simultaneously as it has slots. Slots are assigned automatically
 * by the engine.  However, when you play an effect, you must assign it a
 * unique key so that you can access it later (for volume changes, panning,
 * early termination, etc.).  This key eliminates any need for tracking the
 * channel assigned to an effect.
 *
 * Sound assets may either by mono or stereo, but must be encoded at 48000
 * samples per second.  The output of the audio engine is always stereo,
 * regardless of the sounds asset.  Mono assets are played half volume to
 * each (left/right) speaker.
 *
 * You cannot create new instances of this class.  Instead, you should access
 * the singleton through the three static methods: {@link start()}, {@link stop()},
 * and {@link get()}.
 *
 * IMPORTANT: Like the OpenGL context, this class is not thread-safe.  It is
 * only safe to access this class in the main application thread.  This means
 * it should never be called in a call-back function as those are typically
 * executed in the host thread.  If you need to access the AudioEngine in a
 * callback function, you should use the {@link Application#schedule} method
 * to delay until the main thread is next available.
 */
class AudioChannels {
#pragma mark Sound State
public:
    /**
     * This enumeration provides a way to determine the state of a channel
     */
    enum class State {
        /** This sound channel is not actually active */
        INACTIVE,
        /** This sound is active and currently playing */
        PLAYING,
        /** This sound is active but is currently paused */
        PAUSED
    };

private:
    /** Reference to the audio engine singleton */
    static AudioChannels* _gEngine;

    /** The number of supported audio channels */
    unsigned int _capacity;
    /** Map keys to identifiers */
    std::unordered_map<std::string,std::shared_ptr<audio::AudioFader>> _effects;
    /** The queue for subsequent sound loops */
    std::deque<std::string> _equeue;
    /** The queue for subsequent sound loops */
    //std::deque<std::string,> _equeue;

    /** The audio graph output device */
    std::shared_ptr<audio::AudioOutput> _output;
    /** The audio graph mixer (which determines the number of channels) */
    std::shared_ptr<audio::AudioMixer>  _mixer;
    /** The channel objects for sheduling sounds */
    std::vector<std::shared_ptr<audio::AudioScheduler>> _channel;
    /** The channel wrappers for fading (pausing/stopping) channels */
    std::vector<std::shared_ptr<audio::AudioFader>>     _chfader;

    /** An object pool of faders for individual sound instances */
    std::deque<std::shared_ptr<audio::AudioFader>>  _fadePool;
    /** An object pool of panners for adapting mono sound assets */
    std::deque<std::shared_ptr<audio::AudioPanner>> _pan1Pool;
    /** An object pool of panners for adapting stereo sound assets */
    std::deque<std::shared_ptr<audio::AudioPanner>> _pan2Pool;

    /**
     * Callback function for background music
     *
     * This function is called whenever a background music track completes.
     * It is called whether or not the music completed normally or if it
     * was terminated manually.  However, the second parameter can be used
     * to distinguish the two cases.
     *
     * @param asset     The music asset that just completed
     * @param status    True if the music terminated normally, false otherwise.
     */
    std::function<void(Sound* asset, bool status)> _musicCB;

    /**
     * Callback function for the sound effects
     *
     * This function is called whenever a sound effect completes. It is called
     * whether or not the sound completed normally or if it was terminated
     * manually.  However, the second parameter can be used to distinguish the
     * two cases.
     *
     * @param key       The key identifying this sound effect
     * @param status    True if the music terminated normally, false otherwise.
     */
    std::function<void(const std::string& key , bool status)> _soundCB;

#pragma mark -
#pragma mark Constructors (Private)
    /**
     * Creates, but does not initialize the singleton audio engine
     *
     * The engine must be initialized before is can be used.
     */
    AudioChannels();

    /**
     * Disposes of the singleton audio engine.
     *
     * This destructor releases all of the resources associated with this
     * audio engine.  Sounds and music assets can no longer be loaded.
     */
    ~AudioChannels() { dispose(); }

    /**
     * Initializes the audio engine.
     *
     * This method initializes the audio engine and constructs the mixer graph
     * for the sound effect channels.  This initializer provides the historical
     * standard of 24 sound effect channels.
     *
     * @return true if the audio engine was successfully initialized.
     */
    bool init();

    /**
     * Initializes the audio engine.
     *
     * This method initializes the audio engine and constructs the mixer graph
     * for the sound effect channels.  The provided parameter indicates the
     * number of simultaneously supported sounds.
     *
     * @param slots The maximum number of sound effect channels to support
     *
     * @return true if the audio engine was successfully initialized.
     */
    bool init(Uint32 slots);

    /**
     * Releases all resources for this singleton audio engine.
     *
     * Sounds and music assets can no longer be loaded. If you need to use the
     * engine again, you must call init().
     */
    void dispose();


#pragma mark -
#pragma mark Internal Helpers
    /**
     * Purges this key from the list of active effects.
     *
     * This method is not the same as stopping the channel. A channel may play a
     * little longer after the key is removed.  This is simply a clean-up method.
     *
     * @remove key  The key to purge from the list of active effects.
     */
    void removeKey(std::string key);

    /**
     * Returns a playable audio node for a given a sound instance.
     *
     * Each sound asset needs a panner (for pan support, and to guarantee the
     * correct number of output channels) and a fader before it can be plugged
     * in to the mixer graph.  This method uses the object pools to simplify
     * this process
     *
     * @param asset The sound asset
     *
     * @return a playable audio node for a given a sound instance.
     */
    std::shared_ptr<audio::AudioFader> wrapInstance(const std::shared_ptr<Sound>& asset);

    /**
     * Returns the sound asset for the given playable audio node.
     *
     * Each sound asset needs a panner (for pan support, and to guarantee the
     * correct number of output channels) and a fader before it can be plugged
     * in to the mixer graph.  This method is the reverse of {@link wrapInstance},
     * allowing access to the sound asset previously wrapped as an audio node.
     *
     * @param node  The audio node wrapping the sound asset
     *
     * @return the sound asset for the given playable audio node.
     */
    std::shared_ptr<Sound> accessInstance(const std::shared_ptr<audio::AudioNode>& node) const;

    /**
     * Disposes of the audio nodes wrapping a previously wrapped audio asset.
     *
     * Each sound asset needs a panner (for pan support, and to guarantee the
     * correct number of output channels) and a fader before it can be plugged
     * in to the mixer graph.  This method is the reverse of {@link wrapInstance},
     * disposing (and recycling) those previously allocated nodes.
     *
     * @param node  The audio node wrapping the sound asset
     *
     * @return the sound asset for the given playable audio node.
     */
    std::shared_ptr<Sound> disposeInstance(const std::shared_ptr<audio::AudioNode>& node);

    /**
     * Callback function for when a music asset finishes
     *
     * This method is called when the active music completes. It disposes
     * any audio nodes (faders, panners), recycling them for later.  It also
     * invokes any callback functions associated with the music queue.
     *
     * This method is never intended to be accessed by general users.
     *
     * @param status    True if the music terminated normally, false otherwise.
     */
    void gcMusic(const std::shared_ptr<audio::AudioNode>& sound, bool status);

    /**
     * Callback function for when a sound effect channel finishes
     *
     * This method is called when the active sound effect completes. It disposes
     * any audio nodes (faders, panners), recycling them for later.  It also
     * allows the key to be reused for later effects.  Finally, it invokes any
     * callback functions associated with the sound effect channels.
     *
     * This method is never intended to be accessed by general users.
     *
     * @param node      The playback instance for the sound asset
     * @param status    True if the music terminated normally, false otherwise.
     */
    void gcEffect(const std::shared_ptr<audio::AudioNode>& sound, bool status);

#pragma mark -
#pragma mark Static Accessors
public:
    /**
     * Returns the singleton instance of the audio engine.
     *
     * If the audio engine has not been started, then this method will return
     * nullptr.
     *
     * @return the singleton instance of the audio engine.
     */
    static AudioChannels* get() { return _gEngine; }

    /**
     * Starts the singleton audio engine.
     *
     * Once this method is called, the method get() will no longer return
     * nullptr.  Calling the method multiple times (without calling stop) will
     * have no effect.
     *
     * The parameter `slots` indicates the number of simultaneously supported
     * sounds.  Attempting to play more than this number of sounds may fail,
     * it may eject a previously playing sound, depending on the settings.
     *
     * This method creates an audio buffer size of 512. This is the number of
     * samples collected at each poll. Smaller buffers clearly tax the CPU, as
     * the device is collecting data at a higher rate. Furthermore, if the
     * value is too small, the time to collect the data may be larger than the
     * time to play it. This will result in pops and crackles in the audio.
     *
     * However, larger values increase the audio lag.  For example, a buffer
     * of 1024 for a sample rate of 48000 Hz corresponds to 21 milliseconds.
     * This is the delay between when sound is gathered and it is played.
     * But this gathering process is also buffered, so this means that any
     * sound effect generated at the same time that the audio device executes
     * must wait 42 milliseconds before it can play.  A value of 512 is the
     * perferred value for 60 fps framerate. With that said, many devices
     * cannot handle this rate and need a buffer size of 1024 instead.
     *
     * @param slots  The maximum number of sound effect channels to support
     */
    static void start(Uint32 slots);

    /**
     * Starts the singleton audio engine.
     *
     * Once this method is called, the method get() will no longer return
     * nullptr.  Calling the method multiple times (without calling stop) will
     * have no effect.
     *
     * The parameter `slots` indicates the number of simultaneously supported
     * sounds.  Attempting to play more than this number of sounds may fail,
     * it may eject a previously playing sound, depending on the settings.
     *
     * This buffer size is the number of samples collected at each poll. Smaller
     * buffers clearly tax the CPU, as the device is collecting data at a higher
     * rate. Furthermore, if the value is too small, the time to collect the data
     * may be larger than the time to play it. This will result in pops and
     * crackles in the audio.
     *
     * However, larger values increase the audio lag.  For example, a buffer
     * of 1024 for a sample rate of 48000 Hz corresponds to 21 milliseconds.
     * This is the delay between when sound is gathered and it is played.
     * But this gathering process is also buffered, so this means that any
     * sound effect generated at the same time that the audio device executes
     * must wait 42 milliseconds before it can play.  A value of 512 is the
     * perfered value for 60 fps framerate.  With that said, many devices
     * cannot handle this rate and need a buffer size of 1024 instead.
     *
     * @param channels	The maximum number of sound effect channels to support
     * @param buffer 	The number of samples collected at each audio poll
     */
    static void start(Uint32 channels, Uint32 buffer);

    /**
     * Stops the singleton audio engine, releasing all resources.
     *
     * Once this method is called, the method get() will return nullptr.
     * Calling the method multiple times (without calling stop) will have
     * no effect.
     */
    static void stop();


#pragma mark -
#pragma mark Music Playback
    /**
     * Plays given music asset as a background track.
     *
     * Music is handled differently from sound effects. You can only play one
     * music asset at a time. However, it is possible to queue music assets
     * for immediate playback once the active asset is finished. Proper
     * queue management is the keep for smooth, uninterrupted playback that
     * responds to the user's actions.
     *
     * This method immediately plays the provided asset. Hence it overrides
     * and clears the music queue. To safely play an asset without affecting
     * the music queue, use the method {@link queueMusic} instead.
     *
     * When it begins playing, the music will start at full volume unless you
     * provide a number of seconds to fade in. Note that looping a song will
     * cause it to block the queue indefinitely until you turn off looping for
     * that asset {@see setLoop}. This can be desired behavior, as it gives you
     * a way to control the speed of the queue processing.
     *
     * @param music     The music asset to play
     * @param loop      Whether to loop the music continuously
     * @param volume    The music volume (< 0 to use asset default volume)
     * @param fade      The number of seconds to fade in
     */
    void playMusic(const std::shared_ptr<Sound>& music, bool loop=false,
                   float volume=-1.0f, float fade=0.0f);

    /**
     * Returns the music asset currently playing
     *
     * If there is no active background music, this method returns nullptr.
     *
     * @return the music asset currently playing
     */
    const Sound* currentMusic() const;

    /**
     * Returns the current state of the background music
     *
     * @return the current state of the background music
     */
    State getMusicState() const;

    /**
     * Returns true if the background music is in a continuous loop.
     *
     * If there is no active background music, this method will return false.
     *
     * @return true if the background music is in a continuous loop.
     */
    bool isMusicLoop() const;

    /**
     * Sets whether the background music is on a continuous loop.
     *
     * If loop is true, this will block the queue until it is set to false
     * again. This can be desired behavior, as it gives you a way to control
     * the speed of the queue processing.
     *
     * If there is no active background music, this method will do nothing.
     *
     * @param  loop  whether the background music should be on a continuous loop
     */
    void setMusicLoop(bool loop);

    /**
     * Returns the volume of the background music
     *
     * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
     * silence. If there is no active background music, this method will return 0.
     *
     * @return the volume of the background music
     */
    float getMusicVolume() const;

    /**
     * Sets the volume of the background music
     *
     * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
     * silence. If there is no active background music, this method will have no effect.
     *
     * @param  volume   the volume of the background music
     */
    void setMusicVolume(float volume);

    /**
     * Returns the stereo pan of the background music
     *
     * This audio engine provides limited (e.g. not full 3D) stereo panning
     * for simple effects. The pan value is a float from -1 to 1.  A value
     * of 0 (default) plays to both channels (regardless of whether the current
     * music is mono or stereo).  A value of -1 will play to the left channel
     * only, while the right will play to the right channel only.
     *
     * In the case of stereo assets, panning to the left or right will mix
     * the audio feed; this process will never lose audio.
     *
     * @return the stereo pan of the background music
     */
    float getMusicPan() const;

    /**
     * Sets the stereo pan of the background music
     *
     * This audio engine provides limited (e.g. not full 3D) stereo panning
     * for simple effects. The pan value is a float from -1 to 1.  A value
     * of 0 (default) plays to both channels (regardless of whether the current
     * music is mono or stereo).  A value of -1 will play to the left channel
     * only, while the right will play to the right channel only.
     *
     * In the case of stereo assets, panning to the left or right will mix
     * the audio feed; this process will never lose audio.
     *
     * @param pan   The stereo pan of the background music
     */
    void setMusicPan(float pan);

    /**
     * Returns the length of background music, in seconds.
     *
     * This is only the duration of the active background music.  All other
     * music in the queue is ignored. If there is no active background music,
     * this method will return 0.
     *
     * This information is retrieved from the decoder. As the file is completely
     * decoded at load time, the result of this method is reasonably accurate.
     *
     * @return the length of background music, in seconds.
     */
    float getMusicDuration() const;

    /**
     * Returns the elapsed time of the background music, in seconds
     *
     * The elapsed time is the current position of the music from the beginning.
     * It does not include any time spent on a continuous loop. If there is no
     * active background music, this method will return 0.
     *
     * This information is not guaranteed to be accurate.  Attempting to time
     * the playback of streaming data (as opposed to a fully in-memory PCM
     * buffer) is very difficult and not cross-platform.  We have tried to be
     * reasonably accurate, but from our tests we can only guarantee accuracy
     * within a 10th of a second.
     *
     * @return the elapsed time of the background music, in seconds
     */
    float getMusicElapsed() const;

    /**
     * Returns the time remaining for the background music, in seconds
     *
     * The time remaining is just duration-elapsed.  This method does not take
     * into account whether the music is on a loop. It also does not include
     * the duration of any music waiting in the queue. If there is no active
     * background music, this method will return 0.
     *
     * This information is not guaranteed to be accurate.  Attempting to time
     * the playback of streaming data (as opposed to a fully in-memory PCM
     * buffer) is very difficult and not cross-platform.  We have tried to be
     * reasonably accurate, but from our tests we can only guarantee accuracy
     * within a 10th of a second.
     *
     * @return the time remaining for the background music, in seconds
     */
    float getMusicRemaining() const;

    /**
     * Sets the elapsed time of the background music, in seconds
     *
     * The elapsed time is the current position of the music from the beginning.
     * It does not include any time spent on a continuous loop.
     *
     * This adjustment is not guaranteed to be accurate.  Attempting to time
     * the playback of streaming data (as opposed to a fully in-memory PCM
     * buffer) is very difficult and not cross-platform.  We have tried to be
     * reasonably accurate, but from our tests we can only guarantee accuracy
     * within a 10th of a second.
     *
     * If there is no active background music, this method will have no effect.
     *
     * @param  time  the new position of the background music
     */
    void setMusicElapsed(float time);

    /**
     * Sets the time remaining for the background music, in seconds
     *
     * The time remaining is just duration-elapsed.  It does not take into
     * account whether the music is on a loop. It also does not include the
     * duration of any music waiting in the queue.
     *
     * This adjustment is not guaranteed to be accurate.  Attempting to time
     * the playback of streaming data (as opposed to a fully in-memory PCM
     * buffer) is very difficult and not cross-platform.  We have tried to be
     * reasonably accurate, but from our tests we can only guarantee accuracy
     * within a 10th of a second.
     *
     * If there is no active background music, this method will have no effect.
     *
     * @param  time  the new time remaining of the background music
     */
    void setMusicRemaining(float time);

    /**
     * Stops the background music and clears the entire queue.
     *
     * Before the music is stopped, this method gives the user an option to
     * fade out the music.  If the argument is 0, it will halt the music
     * immediately. Otherwise it will fade to completion over the given number
     * of seconds (or until the end of the song).  Only by fading can you
     * guarantee no audible clicks.
     *
     * This method also clears the queue of any further music.
     *
     * @param fade  The number of seconds to fade out
     */
    void stopMusic(float fade=DEFAULT_FADE);

    /**
     * Pauses the background music, allowing it to be resumed later.
     *
     * Before the music is stopped, this method gives the user an option to
     * fade out the music.  If the argument is 0, it will pause the music
     * immediately. Otherwise it will fade to completion over the given number
     * of seconds (or until the end of the song).  Only by fading can you
     * guarantee no audible clicks.
     *
     * This method has no effect on the music queue.
     *
     * @param fade  The number of seconds to fade out
     */
    void pauseMusic(float fade=DEFAULT_FADE);

    /**
     * Resumes the background music assuming that it was paused previously.
     *
     * This method has no effect on the music queue.
     */
    void resumeMusic();

    /**
     * Sets the callback for background music
     *
     * This callback function is called whenever a background music track
     * completes. It is called whether or not the music completed normally or
     * if it was terminated manually.  However, the second parameter can be
     * used to distinguish the two cases.
     *
     * @param callback The callback for background music
     */
    void setMusicListener(std::function<void(Sound*,bool)> callback) {
        _musicCB = callback;
    }

    /**
     * Returns the callback for background music
     *
     * This callback function is called whenever a background music track
     * completes. It is called whether or not the music completed normally or
     * if it was terminated manually.  However, the second parameter can be
     * used to distinguish the two cases.
     *
     * @return the callback for background music
     */
    std::function<void(Sound*,bool)> getMusicListener() const {
        return _musicCB;
    }

#pragma mark -
#pragma mark Music Queue Management
    /**
     * Adds the given music asset to the background music queue
     *
     * Music is handled differently from sound effects. You can only play one
     * music asset at a time. However, it is possible to queue music assets
     * for immediate playback once the active asset is finished. Proper
     * queue management is the keep for smooth, uninterrupted playback that
     * responds to the user's actions.
     *
     * If the queue is empty and there is no active music, this method will
     * play the music immediately.  Otherwise, it will add the music to the
     * queue, and it will play as soon as it is removed from the queue.
     *
     * When it begins playing, the music will start at full volume unless you
     * provide a number of seconds to fade in. Note that looping a song will
     * cause it to block the queue indefinitely until you turn off looping for
     * that asset {@see setLoop}. This can be desired behavior, as it gives you
     * a way to control the speed of the queue processing.
     *
     * @param music     The music asset to queue
     * @param loop      Whether to loop the music continuously
     * @param volume    The music volume (< 0 to use asset default volume)
     * @param fade      The number of seconds to fade in
     */
    void queueMusic(const std::shared_ptr<Sound>& music, bool loop=false,
                    float volume=-1.0f, float fade=0.0f);

    /**
     * Returns the list of assets for the music queue
     *
     * @return the list of assets for the music queue
     */
    const std::vector<Sound*> getMusicQueue() const;

    /**
     * Returns the size of the music queue
     *
     * @return the size of the music queue
     */
    size_t getMusicPending() const;

    /**
     * Returns the overlap time in seconds.
     *
     * The overlap time is the amount of time to cross-fade between a music
     * asset and the next. It does not apply to looped music; music assets
     * can never cross-fade with themselves.
     *
     * By default, this value is 0.  Assets play sequentially but do not
     * overlap.  However, you may get smoother transitions between musical
     * segments if you adjust this value. The overlap should be chosen with
     * care.  If the play length of an asset is less than the overlap, the
     * results are undefined.
     *
     * @return the overlap time in seconds.
     */
    float getOverlap() const;

    /**
     * Sets the overlap time in seconds.
     *
     * The overlap time is the amount of time to cross-fade between a music
     * asset and the next. It does not apply to looped music; music assets
     * can never cross-fade with themselves.
     *
     * By default, this value is 0.  Assets play sequentially but do not
     * overlap.  However, you may get smoother transitions between musical
     * segments if you adjust this value. The overlap should be chosen with
     * care.  If the play length of an asset is less than the overlap, the
     * results are undefined.
     *
     * @param time  The overlap time in seconds.
     */
    void setOverlap(double time);

    /**
     * Advances ahead in the music queue.
     *
     * The value `fade` is the number of seconds to fade out the currently
     * playing music assets (if any).  This is to ensure a smooth transition
     * to the next song.  If the music ends naturally, before this time, the
     * fadeout will not carry over to later entries in the queue.
     *
     * The value `step` is the number of songs to skip over. A value of 0 will
     * simply skip over the active music to the next element of the queue. Each
     * value above 0 will skip over one more element in the queue.  If this
     * skipping empties the queue, no music will play.
     *
     * @param  fade     The number of seconds to fade out the current asset
     * @param  steps    The number of elements to skip in the queue
     */
    void advanceMusicQueue(float fade=DEFAULT_FADE,unsigned int steps=0);

    /**
     * Clears the music queue, but does not release any other resources.
     *
     * This method does not stop the current background music from playing. It
     * only clears pending music assets from the queue.
     */
    void clearMusicQueue();


#pragma mark -
#pragma mark Sound Effect Management
    /**
     * Plays the given sound effect, and associates it with the specified key.
     *
     * Sound effects are associated with a reference key.  This allows the
     * application to easily reference the sound state without having to
     * internally manage pointers to the audio channel.
     *
     * If the key is already associated with an active sound effect, this
     * method will stop the existing sound and replace it with this one.  It
     * is the responsibility of the application layer to manage key usage.
     *
     * There are a limited number of slots ("channels") available for sound
     * effects.  If you go over the number available, the sound will not play
     * unless `force` is true. In that case, it will grab the channel from the
     * longest playing sound effect.
     *
     * @param  key      The reference key for the sound effect
     * @param  sound    The sound effect to play
     * @param  loop     Whether to loop the sound effect continuously
     * @param  volume   The sound effect (< 0 to use asset default volume)
     * @param  force    Whether to force another sound to stop.
     *
     * @return true if there was an available channel for the sound
     */
    bool playEffect(const std::string& key, const std::shared_ptr<Sound>& sound,
                    bool loop=false, float volume=-1.0f, bool force=false);

    /**
     * Plays the given sound effect, and associates it with the specified key.
     *
     * Sound effects are associated with a reference key.  This allows the
     * application to easily reference the sound state without having to
     * internally manage pointers to the audio channel.
     *
     * If the key is already associated with an active sound effect, this
     * method will stop the existing sound and replace it with this one.  It
     * is the responsibility of the application layer to manage key usage.
     *
     * There are a limited number of slots ("channels") available for sound
     * effects.  If you go over the number available, the sound will not play
     * unless `force` is true. In that case, it will grab the channel from the
     * longest playing sound effect.
     *
     * @param  key      The reference key for the sound effect
     * @param  sound    The sound effect to play
     * @param  loop     Whether to loop the sound effect continuously
     * @param  volume   The sound effect (< 0 to use asset default volume)
     * @param  force    Whether to force another sound to stop.
     *
     * @return true if there was an available channel for the sound
     */
    bool playEffect(const char* key, const std::shared_ptr<Sound>& sound,
                    bool loop=false, float volume=1.0f, bool force=false) {
        return playEffect(std::string(key),sound,loop,volume,force);
    }

    /**
     * Returns the number of slots or "channels" available for sound effects.
     *
     * There are a limited number of channels available for sound effects.  If
     * all channels are in use, this method will return 0. If you go over the
     * number available, you cannot play another sound unless you force it. In
     * that case, it will grab the channel from the longest playing sound effect.
     *
     * @return the number of channels available for sound effects.
     */
    size_t getAvailableChannels() const {
        return (size_t)_capacity-_effects.size();
    }

    /**
     * Returns the current state of the sound effect for the given key.
     *
     * If there is no sound effect for the given key, it returns
     * State::INACTIVE.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the current state of the sound effect for the given key.
     */
    State getEffectState(const std::string& key) const;

    /**
     * Returns the current state of the sound effect for the given key.
     *
     * If there is no sound effect for the given key, it returns
     * State::INACTIVE.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the current state of the sound effect for the given key.
     */
    State getEffectState(const char* key) const {
        return getEffectState(std::string(key));
    }

    /**
     * Returns true if the key is associated with an active channel.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return true if the key is associated with an active channel.
     */
    bool isActiveEffect(const std::string& key) const {
        return _effects.find(key) != _effects.end();
    }

    /**
     * Returns true if the key is associated with an active channel.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return true if the key is associated with an active channel.
     */
    bool isActiveEffect(const char* key) const {
        return isActiveEffect(std::string(key));
    }

    /**
     * Returns the sound asset attached to the given key.
     *
     * If there is no active sound effect for the given key, this method
     * returns nullptr.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the sound asset attached to the given key.
     */
    const Sound* currentEffect(const std::string& key) const;

    /**
     * Returns the sound asset attached to the given key.
     *
     * If there is no active sound effect for the given key, this method
     * returns nullptr.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the sound asset attached to the given key.
     */
    const Sound* currentEffect(const char* key) const {
        return currentEffect(std::string(key));
    }

    /**
     * Returns true if the sound effect is in a continuous loop.
     *
     * If the key does not correspond to a channel, this method returns false.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return true if the sound effect is in a continuous loop.
     */
    bool isEffectLoop(const std::string& key) const;

    /**
     * Returns true if the sound effect is in a continuous loop.
     *
     * If the key does not correspond to a channel, this method returns false.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return true if the sound effect is in a continuous loop.
     */
    bool isEffectLoop(const char* key) const {
        return isEffectLoop(std::string(key));
    }

    /**
     * Sets whether the sound effect is in a continuous loop.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  loop     whether the sound effect is in a continuous loop
     */
    void setEffectLoop(const std::string& key, bool loop);

    /**
     * Sets whether the sound effect is in a continuous loop.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  loop     whether the sound effect is in a continuous loop
     */
    void setEffectLoop(const char* key, bool loop) {
        setEffectLoop(std::string(key),loop);
    }

    /**
     * Returns the current volume of the sound effect.
     *
     * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
     * silence. If the key does not correspond to a channel, this method returns 0.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the current volume of the sound effect
     */
    float getEffectVolume(const std::string& key) const;

    /**
     * Returns the current volume of the sound effect.
     *
     * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
     * silence. If the key does not correspond to a channel, this method returns 0.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the current volume of the sound effect
     */
    float getEffectVolume(const char* key) const {
        return getEffectVolume(std::string(key));
    }

    /**
     * Sets the current volume of the sound effect.
     *
     * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
     * silence. If the key does not correspond to a channel, this method does
     * nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  volume   the current volume of the sound effect
     */
    void setEffectVolume(const std::string& key, float volume);

    /**
     * Sets the current volume of the sound effect.
     *
     * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
     * silence. If the key does not correspond to a channel, this method does
     * nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  volume   the current volume of the sound effect
     */
    void setEffectVolume(const char* key, float volume) {
        setEffectVolume(std::string(key),volume);
    }

    /**
     * Returns the stereo pan of the sound effect.
     *
     * This audio engine provides limited (e.g. not full 3D) stereo panning
     * for simple effects. The pan value is a float from -1 to 1.  A value
     * of 0 (default) plays to both channels (regardless of whether the current
     * effect is mono or stereo).  A value of -1 will play to the left channel
     * only, while the right will play to the right channel only.
     *
     * In the case of stereo assets, panning to the left or right will mix
     * the audio feed; this process will never lose audio.
     *
     * If the key does not correspond to a channel, this method returns 0.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the stereo pan of the sound effect
     */
    float getEffectPan(const std::string& key) const;

    /**
     * Returns the stereo pan of the sound effect.
     *
     * This audio engine provides limited (e.g. not full 3D) stereo panning
     * for simple effects. The pan value is a float from -1 to 1.  A value
     * of 0 (default) plays to both channels (regardless of whether the current
     * effect is mono or stereo).  A value of -1 will play to the left channel
     * only, while the right will play to the right channel only.
     *
     * In the case of stereo assets, panning to the left or right will mix
     * the audio feed; this process will never lose audio.
     *
     * If the key does not correspond to a channel, this method returns 0.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the stereo pan of the sound effect
     */
    float getEffectPan(const char* key) const {
        return getEffectPan(std::string(key));
    }

    /**
     * Sets the stereo pan of the sound effect.
     *
     * This audio engine provides limited (e.g. not full 3D) stereo panning
     * for simple effects. The pan value is a float from -1 to 1.  A value
     * of 0 (default) plays to both channels (regardless of whether the current
     * effect is mono or stereo).  A value of -1 will play to the left channel
     * only, while the right will play to the right channel only.
     *
     * In the case of stereo assets, panning to the left or right will mix
     * the audio feed; this process will never lose audio.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  pan      the stereo pan of the sound effect
     */
    void setEffectPan(const std::string& key, float pan);

    /**
     * Sets the stereo pan of the sound effect.
     *
     * This audio engine provides limited (e.g. not full 3D) stereo panning
     * for simple effects. The pan value is a float from -1 to 1.  A value
     * of 0 (default) plays to both channels (regardless of whether the current
     * effect is mono or stereo).  A value of -1 will play to the left channel
     * only, while the right will play to the right channel only.
     *
     * In the case of stereo assets, panning to the left or right will mix
     * the audio feed; this process will never lose audio.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  pan      the stereo pan of the sound effect
     */
    void setEffectPan(const char* key, float pan) {
        setEffectPan(std::string(key),pan);
    }

    /**
     * Returns the duration of the sound effect, in seconds.
     *
     * Because the asset is fully decompressed at load time, the result of this
     * method is reasonably accurate.
     *
     * If the key does not correspond to a channel, this method returns -1.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the duration of the sound effect, in seconds.
     */
    float getEffectDuration(const std::string& key) const;

    /**
     * Returns the duration of the sound effect, in seconds.
     *
     * Because the asset is fully decompressed at load time, the result of this
     * method is reasonably accurate.
     *
     * If the key does not correspond to a channel, this method returns -1.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the duration of the sound effect, in seconds.
     */
    float getEffectDuration(const char* key) const {
        return getEffectDuration(std::string(key));
    }

    /**
     * Returns the elapsed time of the sound effect, in seconds
     *
     * The elapsed time is the current position of the sound from the beginning.
     * It does not include any time spent on a continuous loop. Because most
     * sound effects are fully decompressed at load time, the result of this
     * method is reasonably accurate.
     *
     * If the key does not correspond to a channel, this method returns -1.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the elapsed time of the sound effect, in seconds
     */
    float getEffectElapsed(const std::string& key) const;

    /**
     * Returns the elapsed time of the sound effect, in seconds
     *
     * The elapsed time is the current position of the sound from the beginning.
     * It does not include any time spent on a continuous loop. Because most
     * sound effects are fully decompressed at load time, the result of this
     * method is reasonably accurate.
     *
     * If the key does not correspond to a channel, this method returns -1.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the elapsed time of the sound effect, in seconds
     */
    float getEffectElapsed(const char* key) const {
        return getEffectElapsed(std::string(key));
    }

    /**
     * Sets the elapsed time of the sound effect, in seconds
     *
     * The elapsed time is the current position of the sound from the beginning.
     * It does not include any time spent on a continuous loop.  Because most
     * sound effects are fully decompressed at load time, the result of this
     * method is reasonably accurate.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  time     the new position of the sound effect
     */
    void setEffectElapsed(const std::string& key, float time);

    /**
     * Sets the elapsed time of the sound effect, in seconds
     *
     * The elapsed time is the current position of the sound from the beginning.
     * It does not include any time spent on a continuous loop.  Because most
     * sound effects are fully decompressed at load time, the result of this
     * method is reasonably accurate.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  time     the new position of the sound effect
     */
    void setEffectElapsed(const char* key, float time) {
        setEffectElapsed(std::string(key),time);
    }

    /**
     * Returns the time remaining for the sound effect, in seconds
     *
     * The time remaining is just duration-elapsed.  This method does not take
     * into account whether the sound is on a loop. Because most sound effects
     * are fully decompressed at load time, the result of this method is
     * reasonably accurate.
     *
     * If the key does not correspond to a channel, this method returns -1.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the time remaining for the sound effect, in seconds
     */
    float getEffectRemaining(const std::string& key) const;

    /**
     * Returns the time remaining for the sound effect, in seconds
     *
     * The time remaining is just duration-elapsed.  This method does not take
     * into account whether the sound is on a loop. Because most sound effects
     * are fully decompressed at load time, the result of this method is
     * reasonably accurate.
     *
     * If the key does not correspond to a channel, this method returns -1.
     *
     * @param  key      the reference key for the sound effect
     *
     * @return the time remaining for the sound effect, in seconds
     */
    float getEffectRemaining(const char* key) const {
        return getEffectRemaining(std::string(key));
    }

    /**
     * Sets the time remaining for the sound effect, in seconds
     *
     * The time remaining is just duration-elapsed.  This method does not take
     * into account whether the sound is on a loop. Because most sound effects
     * are fully decompressed at load time, the result of this method is
     * reasonably accurate.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  time     the new time remaining for the sound effect
     */
    void setEffectRemaining(const std::string& key, float time);

    /**
     * Sets the time remaining for the sound effect, in seconds
     *
     * The time remaining is just duration-elapsed.  This method does not take
     * into account whether the sound is on a loop. Because most sound effects
     * are fully decompressed at load time, the result of this method is
     * reasonably accurate.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param  time     the new time remaining for the sound effect
     */
    void setEffectRemaining(const char* key, float time) {
        setEffectRemaining(std::string(key),time);
    }

    /**
     * Stops the sound effect for the given key, removing it.
     *
     * The effect will be removed from the audio engine entirely. You will need
     * to add it again if you wish to replay it.
     *
     * Before the effect is stopped, this method gives the user an option to
     * fade out the effect.  If the argument is 0, it will halt the sound
     * immediately. Otherwise it will fade to completion over the given number
     * of seconds (or until the end of the effect).  Only by fading can you
     * guarantee no audible clicks.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param fade      the number of seconds to fade out
     */
    void stopEffect(const std::string& key,float fade=DEFAULT_FADE);

    /**
     * Stops the sound effect for the given key, removing it.
     *
     * The effect will be removed from the audio engine entirely. You will need
     * to add it again if you wish to replay it.
     *
     * Before the effect is stopped, this method gives the user an option to
     * fade out the effect.  If the argument is 0, it will halt the sound
     * immediately. Otherwise it will fade to completion over the given number
     * of seconds (or until the end of the effect).  Only by fading can you
     * guarantee no audible clicks.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param fade      the number of seconds to fade out
     */
    void stopEffect(const char* key,float fade=DEFAULT_FADE) {
        stopEffect(std::string(key));
    }

    /**
     * Pauses the sound effect for the given key.
     *
     * Before the effect is paused, this method gives the user an option to
     * fade out the effect.  If the argument is 0, it will pause the sound
     * immediately. Otherwise it will fade to completion over the given number
     * of seconds (or until the end of the effect).  Only by fading can you
     * guarantee no audible clicks.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param fade      the number of seconds to fade out
     */
    void pauseEffect(const std::string& key,float fade=DEFAULT_FADE);

    /**
     * Pauses the sound effect for the given key.
     *
     * Before the effect is paused, this method gives the user an option to
     * fade out the effect.  If the argument is 0, it will pause the sound
     * immediately. Otherwise it will fade to completion over the given number
     * of seconds (or until the end of the effect).  Only by fading can you
     * guarantee no audible clicks.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     * @param fade      the number of seconds to fade out
     */
    void pauseEffect(const char* key,float fade=DEFAULT_FADE) {
        pauseEffect(std::string(key));
    }

    /**
     * Resumes the sound effect for the given key.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     */
    void resumeEffect(std::string key);

    /**
     * Resumes the sound effect for the given key.
     *
     * If the key does not correspond to a channel, this method does nothing.
     *
     * @param  key      the reference key for the sound effect
     */
    void resumeEffect(const char* key) {
        resumeEffect(std::string(key));
    }

    /**
     * Stops all sound effects, removing them from the engine.
     *
     * Before the effects are stopped, this method gives the user an option to
     * fade out the effect.  If the argument is 0, it will halt all effects
     * immediately. Otherwise it will fade the to completion over the given
     * number of seconds (or until the end of the effect).  Only by fading can
     * you guarantee no audible clicks.
     *
     * You will need to add the effects again if you wish to replay them.
     *
     * @param fade      the number of seconds to fade out
     */
    void stopAllEffects(float fade=DEFAULT_FADE);

    /**
     * Pauses all sound effects, allowing them to be resumed later.
     *
     * Before the effects are paused, this method gives the user an option to
     * fade out the effect.  If the argument is 0, it will pause all effects
     * immediately. Otherwise it will fade the to completion over the given
     * number of seconds (or until the end of the effect).  Only by fading can
     * you guarantee no audible clicks.
     *
     * Sound effects already paused will remain paused.
     *
     * @param fade      the number of seconds to fade out
     */
    void pauseAllEffects(float fade=DEFAULT_FADE);

    /**
     * Resumes all paused sound effects.
     */
    void resumeAllEffects();

    /**
     * Sets the callback for sound effects
     *
     * This callback function is called whenever a sound effect completes. It
     * is called whether or not the sound completed normally or if it was
     * terminated manually.  However, the second parameter can be used to
     * distinguish the two cases.
     *
     * @param callback  The callback for sound effects
     */
    void setEffectListener(std::function<void(const std::string& key,bool)> callback) {
        _soundCB = callback;
    }

    /**
     * Returns the callback for sound effects
     *
     * This callback function is called whenever a sound effect completes. It
     * is called whether or not the sound completed normally or if it was
     * terminated manually.  However, the second parameter can be used to
     * distinguish the two cases.
     *
     * @return the callback for sound effects
     */
    std::function<void(const std::string& key,bool)> getEffectListener() const {
        return _soundCB;
    }

#pragma mark -
#pragma mark Global Management
    /**
     * Stops all sounds, both music and sound effects.
     *
     * Before the sounds are stopped, this method gives the user an option to
     * fade out everything.  If the argument is 0, it will halt the sounds
     * immediately. Otherwise it will fade everythign to completion over the
     * given number of seconds (or until the end of each sound).  Only by
     * fading can you guarantee no audible clicks.
     *
     * This method effectively clears the sound engine.
     *
     * @param fade      the number of seconds to fade out
     */
    void stopAll(float fade=DEFAULT_FADE);

    /**
     * Pauses all sounds, both music and sound effects.
     *
     * Before the sounds are paused, this method gives the user an option to
     * fade out everything.  If the argument is 0, it will pause the sounds
     * immediately. Otherwise it will fade everythign to completion over the
     * given number of seconds (or until the end of each sound).  Only by
     * fading can you guarantee no audible clicks.
     *
     * This method allows them to be resumed later. You should generally
     * call this method just before the app pages to the background.
     *
     * @param fade      the number of seconds to fade out
     */
    void pauseAll(float fade=DEFAULT_FADE);

    /**
     * Resumes all paused sounds, both music and sound effects.
     *
     * You should generally call this method right after the app returns
     * from the background.
     */
    void resumeAll();
};

}


#endif /* __CU_AUDIO_CHANNELS_H__ */
