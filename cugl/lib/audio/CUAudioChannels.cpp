//
//  CUAudioChannels.cpp
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
#include <cugl/cugl.h>
#include <algorithm>

using namespace cugl;
using namespace cugl::audio;

/** The default number of slots */
#define DEFAULT_SLOTSIZE    24

/** Reference to the sound engine singleton */
AudioChannels* AudioChannels::_gEngine = nullptr;


#pragma mark -
#pragma mark Constructors
/**
 * Creates, but does not initialize the singleton audio engine
 *
 * The engine must be initialized before is can be used.
 */
AudioChannels::AudioChannels() : _capacity(0) {
    _output = nullptr;
    _mixer  = nullptr;
}

/**
 * Initializes the audio engine.
 *
 * This method initializes the audio engine and constructs the mixer graph
 * for the sound effect channels.  This initializer provides the historical
 * standard of 24 sound effect channels.
 *
 * @return true if the audio engine was successfully initialized.
 */
bool AudioChannels::init() {
    return init(DEFAULT_SLOTSIZE);
}

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
bool AudioChannels::init(Uint32 channels) {
    CUAssertLog(channels, "The number of channels must be non-zero");
    _capacity = channels;

    // Assume that manager has already started
    _output = AudioManager::get()->openOutput();
    _mixer  = AudioMixer::alloc(_capacity+1,_output->getChannels(),_output->getRate());
    
    for(int ii = 0; ii <= _capacity; ii++) {
        std::shared_ptr<AudioScheduler> slot;
        slot = audio::AudioScheduler::alloc(_mixer->getChannels(),_mixer->getRate());
        slot->setTag(ii);
        _channel.push_back(slot);
        std::shared_ptr<AudioFader> cover;
        cover = audio::AudioFader::alloc(slot);
        cover->setTag(ii);
        _chfader.push_back(cover);
        _mixer->attach(ii,cover);
        
        if (ii == 0) {
            slot->setCallback([=](const std::shared_ptr<cugl::audio::AudioNode>& node,
                                     cugl::audio::AudioNode::Action action) {
                if (action != cugl::audio::AudioNode::Action::LOOPBACK) {
                    bool success = (action == cugl::audio::AudioNode::Action::COMPLETE);
                    this->gcMusic(node,success);
                }
            });
        } else {
            slot->setCallback([=](const std::shared_ptr<cugl::audio::AudioNode>& node,
                                  cugl::audio::AudioNode::Action action) {
                if (action != cugl::audio::AudioNode::Action::LOOPBACK) {
                    bool success = (action == cugl::audio::AudioNode::Action::COMPLETE);
                    this->gcEffect(node,success);
                }
            });
        }
    }
    
    // Pool needs a fader and panner for 2 times the number of slots
    for(int ii = 0; ii <= 2*_capacity; ii++) {
        _fadePool.push_back(AudioFader::alloc(_mixer->getChannels(),_mixer->getRate()));
        _pan1Pool.push_back(AudioPanner::alloc(_mixer->getChannels(),1,_mixer->getRate()));
        _pan2Pool.push_back(AudioPanner::alloc(_mixer->getChannels(),2,_mixer->getRate()));
    }
    
    // Launch and go
    _output->attach(_mixer);
    AudioManager::get()->activate();
    return true;
}

/**
 * Releases all resources for this singleton audio engine.
 *
 * Sounds and music assets can no longer be loaded. If you need to use the
 * engine again, you must call init().
 */
void AudioChannels::dispose() {
    if (_capacity) {
        AudioManager::get()->closeOutput(_output);
		AudioManager::get()->deactivate();

        _channel.clear();
        _chfader.clear();
        
        _fadePool.clear();
        _pan1Pool.clear();
        _pan2Pool.clear();
        _capacity = 0;
        
		_output->detach();
		_output = nullptr;
        _mixer = nullptr;
        
		_equeue.clear();
		_effects.clear();
	}
}


#pragma mark -
#pragma mark Internal Helpers

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
void AudioChannels::gcMusic(const std::shared_ptr<AudioNode>& node, bool status) {
    std::shared_ptr<Sound> sound = disposeInstance(node);
    if (_musicCB) {
        _musicCB(sound.get(),status);
    }

}

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
void AudioChannels::gcEffect(const std::shared_ptr<AudioNode>& node, bool status) {
    std::string key = node->getName();
    std::shared_ptr<Sound> sound = disposeInstance(node);
    removeKey(key);
    if (_soundCB) {
        _soundCB(key,status);
    }
}

/**
 * Purges this key from the list of active effects.
 *
 * This method is not the same as stopping the channel. A channel may play a
 * little longer after the key is removed.  This is simply a clean-up method.
 *
 * @remove key  The key to purge from the list of active effects.
 */
void AudioChannels::removeKey(std::string key) {
    _effects.erase(key);
    for(auto it = _equeue.begin(); it != _equeue.end(); ) {
        if (*it == key) {
            it = _equeue.erase(it);
            break;
        } else {
            it++;
        }
    }
}

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
std::shared_ptr<AudioFader> AudioChannels::wrapInstance(const std::shared_ptr<Sound>& asset) {
    CUAssertLog(asset->getChannels() <= 2, "Sound asset has more than 2 channels");
    CUAssertLog(asset->getRate() == _mixer->getRate(), "Sound asset is not encoded at %d Hz",_mixer->getRate());
    
    std::shared_ptr<AudioFader> fader = nullptr;
    if (_fadePool.empty()) {
        fader = AudioFader::alloc(_mixer->getChannels(),_mixer->getRate());
    } else {
        fader = _fadePool.front();
        _fadePool.pop_front();
    }
    std::shared_ptr<AudioPanner> panner = nullptr;
    if (asset->getChannels() == 1) {
        if (_pan1Pool.empty()) {
            panner = AudioPanner::alloc(_mixer->getChannels(),1,_mixer->getRate());
        } else {
            panner = _pan1Pool.front();
            _pan1Pool.pop_front();
        }
    } else {
        if (_pan2Pool.empty()) {
            panner = AudioPanner::alloc(_mixer->getChannels(),2,_mixer->getRate());
        } else {
            panner = _pan2Pool.front();
            _pan2Pool.pop_front();
        }
    }
    fader->attach(panner);
    panner->attach(asset->createNode());
    return fader;
}

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
std::shared_ptr<Sound> AudioChannels::accessInstance(const std::shared_ptr<AudioNode>& node) const {
    if (node) {
        AudioFader* fader = dynamic_cast<AudioFader*>(node.get());
        AudioPanner* panner = dynamic_cast<AudioPanner*>(fader->getInput().get());
        AudioPlayer* player = dynamic_cast<AudioPlayer*>(panner->getInput().get());
        if (player) {
            return player->getSource();
        }
    }
    return nullptr;
}

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
std::shared_ptr<Sound> AudioChannels::disposeInstance(const std::shared_ptr<audio::AudioNode>& node) {
    if (node) {
        std::shared_ptr<AudioFader> fader   = std::dynamic_pointer_cast<AudioFader>(node);
        std::shared_ptr<AudioPanner> panner = std::dynamic_pointer_cast<AudioPanner>(fader->getInput());
        if (panner) {
            std::shared_ptr<AudioNode>   source = panner->getInput();
            AudioPlayer* player = dynamic_cast<AudioPlayer*>(source.get());
            fader->detach();
            fader->fadeOut(-1);
            fader->reset();
            panner->detach();
            panner->reset();
            _fadePool.push_back(fader);
            if (panner->getField() == 1) {
                _pan1Pool.push_back(panner);
            } else {
                _pan2Pool.push_back(panner);
            }
            if (player) {
                return player->getSource();
            }
        }
    }
    return nullptr;
}


#pragma mark -
#pragma mark Static Accessors

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
 * must wait 46 milliseconds before it can play.  A value of 512 is the
 * perferred value for 60 fps framerate. With that said, many devices
 * cannot handle this rate and need a buffer size of 1024 instead.
 *
 * @param slots  The maximum number of sound effect channels to support
 */
void AudioChannels::start(Uint32 channels) {
    start(channels,AudioManager::DEFAULT_OUTPUT_BUFFER);
}

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
 * must wait 46 milliseconds before it can play.  A value of 512 is the
 * perferred value for 60 fps framerate.  With that said, many devices
 * cannot handle this rate and need a buffer size of 1024 instead.
 *
 * @param slots  The maximum number of sound effect channels to support
 * @param buffer The number of samples collected at each audio poll
 */
void AudioChannels::start(Uint32 channels, Uint32 buffer) {
    if (_gEngine != nullptr) {
        return;
    }
    AudioManager::start(buffer);
    _gEngine = new AudioChannels();
    if (!_gEngine->init(channels)) {
        delete _gEngine;
        _gEngine = nullptr;
        AudioManager::stop();
        CUAssertLog(false,"Audio engine failed to initialize");
    }
}

/**
 * Stops the singleton audio engine, releasing all resources.
 *
 * Once this method is called, the method get() will return nullptr.
 * Calling the method multiple times (without calling stop) will have
 * no effect.
 */
void AudioChannels::stop() {
    if (_gEngine == nullptr) {
        return;
    }
    delete _gEngine;
    _gEngine = nullptr;
    AudioManager::stop();
}


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
void AudioChannels::playMusic(const std::shared_ptr<Sound>& music, bool loop, float volume, float fade) {
    CUAssertLog(volume >= 0 && volume <= 1, "Volume %f is out of range",volume);
    CUAssertLog(fade >= 0, "Fade-in cannot be negative");
    std::shared_ptr<audio::AudioScheduler> slot = _channel[0];

    // Wrap up the music
    std::shared_ptr<AudioFader> fader = wrapInstance(music);
    if (volume >= 0) {
        fader->setGain(volume);
    } else {
        fader->setGain(music->getVolume());
    }
    if (fade > 0) {
        fader->fadeIn(fade);
    }
    
    slot->play(fader, loop ? -1 : 0);
}

/**
 * Returns the music asset currently playing
 *
 * If there is no active background music, this method returns nullptr.
 *
 * @return the music asset currently playing
 */
const Sound* AudioChannels::currentMusic() const {
    std::shared_ptr<audio::AudioScheduler> slot = _channel[0];
    std::shared_ptr<Sound> sound = accessInstance(slot->getCurrent());
    return sound.get();
}

/**
 * Returns the current state of the background music
 *
 * @return the current state of the background music
 */
AudioChannels::State AudioChannels::getMusicState() const {
    std::shared_ptr<audio::AudioScheduler> slot = _channel[0];
    if (!slot->isPlaying()) {
        return State::INACTIVE;
    }
    std::shared_ptr<AudioNode> node = slot->getCurrent();
    if (node->isPaused() || slot->isPaused()) {
        return State::PAUSED;
    }
    
    return State::PLAYING;
}

/**
 * Returns true if the background music is in a continuous loop.
 *
 * If there is no active background music, this method will return false.
 *
 * @return true if the background music is in a continuous loop.
 */
bool AudioChannels::isMusicLoop() const {
    return _channel[0]->getLoops() != 0;
}

/**
 * Sets whether the background music is on a continuous loop.
 *
 * If loop is true, this will block the queue until it is set to false
 * again. This can be desired behavior, as it gives you a way to control
 * the speed of the queue processing.
 *
 * If there is no active background music, this method will raise an error.
 *
 * @param  loop  whether the background music should be on a continuous loop
 */
void AudioChannels::setMusicLoop(bool loop) {
    _channel[0]->setLoops(loop ? -1 : 0);
}

/**
 * Returns the volume of the background music
 *
 * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
 * silence. If there is no active background music, this method will return 0.
 *
 * @return the volume of the background music
 */
float AudioChannels::getMusicVolume() const {
    std::shared_ptr<AudioNode> node = _channel[0]->getCurrent();
    if (node) {
        return node->getGain();
    }
    return 0;
}

/**
 * Sets the volume of the background music
 *
 * The volume is a value 0 to 1, where 1 is maximum volume and 0 is complete
 * silence. If there is no active background music, this method will have no effect.
 *
 * @param  volume   the volume of the background music
 */
void AudioChannels::setMusicVolume(float volume) {
    CUAssertLog(volume >= 0 && volume <= 1, "Volume %f is out of range",volume);
    std::shared_ptr<AudioNode> node = _channel[0]->getCurrent();
    if (node) {
        return node->setGain(volume);
    }
}

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
float AudioChannels::getMusicPan() const {
    std::shared_ptr<AudioFader> fader = std::dynamic_pointer_cast<AudioFader>(_channel[0]->getCurrent());
    if (fader) {
        std::shared_ptr<AudioPanner> panner = std::dynamic_pointer_cast<AudioPanner>(fader->getInput());
        if (panner->getField() == 1) {
            return panner->getPan(0,1)-panner->getPan(0,0);
        } else {
            return panner->getPan(1,1)-panner->getPan(0,0);
        }
    }
    return 0;
}

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
void AudioChannels::setMusicPan(float pan) {
    CUAssertLog(pan >= -1 && pan <= 1, "Pan value %f is out of range",pan);
    std::shared_ptr<AudioFader> fader = std::dynamic_pointer_cast<AudioFader>(_channel[0]->getCurrent());
    if (fader) {
        std::shared_ptr<AudioPanner> panner = std::dynamic_pointer_cast<AudioPanner>(fader->getInput());
        if (panner->getField() == 1) {
            panner->setPan(0,0,0.5-pan/2.0);
            panner->setPan(0,1,0.5+pan/2.0);
        } else {
            if (pan <= 0) {
                panner->setPan(0,0,1);
                panner->setPan(0,1,0);
                panner->setPan(1,0,-pan);
                panner->setPan(1,1,1+pan);
            } else {
                panner->setPan(1,1,1);
                panner->setPan(1,0,0);
                panner->setPan(0,0,1-pan);
                panner->setPan(0,1,pan);
            }
        }
    }
}

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
float AudioChannels::getMusicDuration() const {
    Sound* sound = accessInstance(_channel[0]->getCurrent()).get();
    if (sound) {
        return sound->getDuration();
    }
    return 0;
}

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
float AudioChannels::getMusicElapsed() const {
    std::shared_ptr<AudioNode> node = _channel[0]->getCurrent();
    if (node) {
        return (float)node->getElapsed();
    }
    return 0;
}

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
float AudioChannels::getMusicRemaining() const {
    std::shared_ptr<AudioNode> node = _channel[0]->getCurrent();
    if (node) {
        return (float)node->getRemaining();
    }
    return 0;
}

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
void AudioChannels::setMusicElapsed(float time) {
    std::shared_ptr<AudioNode> node = _channel[0]->getCurrent();
    if (node) {
        node->setElapsed(time);
    }
}

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
void AudioChannels::setMusicRemaining(float time) {
    std::shared_ptr<AudioNode> node = _channel[0]->getCurrent();
    if (node) {
        node->setRemaining(time);
    }
}

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
void AudioChannels::stopMusic(float fade) {
    std::shared_ptr<audio::AudioScheduler> slot = _channel[0];
    std::shared_ptr<AudioFader> fader = std::dynamic_pointer_cast<AudioFader>(slot->getCurrent());
    if (fader) {
        if (fade >= 0) {
            slot->setLoops(0);
            slot->trim();
            fader->fadeOut(fade);
        } else {
            slot->clear();
        }
    }
}

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
void AudioChannels::pauseMusic(float fade) {
    std::shared_ptr<audio::AudioFader> slot = _chfader[0];
    if (fade > 0) {
        slot->fadePause(fade);
    } else {
        slot->pause();
    }
}

/**
 * Resumes the background music assuming that it was paused previously.
 *
 * This method has no effect on the music queue.
 */
void AudioChannels::resumeMusic() {
    std::shared_ptr<audio::AudioFader> slot = _chfader[0];
    if (slot->isPaused()) {
        slot->resume();
    }
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
void AudioChannels::queueMusic(const std::shared_ptr<Sound>& music, bool loop, float volume, float fade) {
    std::shared_ptr<audio::AudioScheduler> slot = _channel[0];
    
    // Wrap up the music
    std::shared_ptr<AudioFader> fader = wrapInstance(music);
    if (volume >= 0) {
        fader->setGain(volume);
    } else {
        fader->setGain(music->getVolume());
    }
    if (fade > 0) {
        fader->fadeIn(fade);
    }
    
    slot->append(fader, loop ? -1 : 0);
}

/**
 * Returns the list of assets for the music queue
 *
 * @return the list of assets for the music queue
 */
const std::vector<Sound*> AudioChannels::getMusicQueue() const {
    std::vector<Sound*> result;
    std::deque<std::shared_ptr<AudioNode>> list = _channel[0]->getTail();
    for(auto it = list.begin(); it != list.end(); ++it) {
        std::shared_ptr<Sound> sample = accessInstance(*it);
        result.push_back(sample.get());
    }
    return result;
}

/**
 * Returns the size of the music queue
 *
 * @return the size of the music queue
 */
size_t AudioChannels::getMusicPending() const {
    return _channel[0]->getTailSize();
}

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
float AudioChannels::getOverlap() const {
    return _channel[0]->getOverlap();
}

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
void AudioChannels::setOverlap(double time) {
    _channel[0]->setOverlap(time);
}

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
void AudioChannels::advanceMusicQueue(float fade, unsigned int steps) {
    _channel[0]->setLoops(0);
    std::shared_ptr<AudioFader> fader = std::dynamic_pointer_cast<AudioFader>(_channel[0]->getCurrent());
    if (fader) {
        fader->fadeOut(fade);
    }
    if (steps) {
        _channel[0]->trim(steps);
    }
}

/**
 * Clears the music queue, but does not release any other resources.
 *
 * This method does not stop the current background music from playing. It
 * only clears pending music assets from the queue.
 */
void AudioChannels::clearMusicQueue() {
    _channel[0]->trim();
}


#pragma mark -
#pragma mark Sound Effect Management
/**
 * Plays the given sound effect, and associates it with the specified key.
 *
 * Sound effects are associated with a reference key.  This allows the
 * application to easily reference the sound state without having to
 * internally manage pointers to the audio channel.
 *
 * If the key is already associated with an active sound channel, this
 * method will stop the existing sound and replace it with this one.  It
 * is the responsibility of the application layer to manage key usage.
 *
 * There are a limited number of channels available for sound effects.  If
 * you go over the number available, the sound will not play unless force
 * is true. In that case, it will grab the channel from the longest playing
 * sound effect.
 *
 * @param  key      The reference key for the sound effect
 * @param  sound    The sound effect to play
 * @param  loop     Whether to loop the sound effect continuously
 * @param  volume   The sound effect (< 0 to use asset default volume)
 * @param  force    Whether to force another sound to stop.
 *
 * @return true if there was an available channel for the sound
 */
bool AudioChannels::playEffect(const std::string& key, const std::shared_ptr<Sound>& sound,
                             bool loop, float volume, bool force) {
    if (isActiveEffect(key)) {
        if (force) {
            stopEffect(key,0);
            removeKey(key);
        } else {
            CULogError("Sound effect key is in use");
            return false;
        }
    }
    
    // Find an empty scheduler
    int audioID = -1;
    for(auto it = _channel.begin()+1; audioID == -1 && it != _channel.end(); ++it) {
        if (!(*it)->isPlaying()) {
            audioID = (*it)->getTag();
        }
    }
    
    // Try again for soon to be deleted.
    for(auto it = _effects.begin(); audioID == -1 && it != _effects.end(); ++it) {
        if (it->second->isFadeOut()) {
            Uint32 tag = it->second->getTag();
            if (!_channel[tag]->getTailSize()) {
                audioID = tag;
            }
        }
    }
    
    if (audioID == -1) {
        if (force) {
            std::string altkey = _equeue.front();
            audioID = _effects[altkey]->getTag();
            stopEffect(altkey);
        } else {
            // Fail if nothing available
            CULogError("No available sound channels");
            return false;
        }
    }
    
    std::shared_ptr<AudioFader> fader = wrapInstance(sound);
    if (volume >= 0) {
        fader->setGain(volume);
    } else {
        fader->setGain(sound->getVolume());
    }
    fader->setTag(audioID);
    fader->setName(key);
    _channel[audioID]->play(fader, loop ? -1 : 0);
    _effects.emplace(key,fader);
    _equeue.push_back(key);
    return true;
}

/**
 * Returns the current state of the sound effect for the given key.
 *
 * If there is no sound effect for the given key, it returns State::INACTIVE.
 *
 * @param  key      the reference key for the sound effect
 *
 * @return the current state of the sound effect for the given key.
 */
AudioChannels::State AudioChannels::getEffectState(const std::string& key) const {
    if (_effects.find(key) == _effects.end()) {
        return State::INACTIVE;
    }
    
    std::shared_ptr<AudioNode> node = _effects.at(key);
    std::shared_ptr<audio::AudioScheduler> slot = _channel.at(node->getTag());
    if (!slot->isPlaying()) {
        return State::INACTIVE;
    } else if (node->isPaused() || slot->isPaused()) {
        return State::PAUSED;
    }
    
    return State::PLAYING;
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
const Sound* AudioChannels::currentEffect(const std::string& key) const {
    if (_effects.find(key) == _effects.end()) {
        return nullptr;
    }
    std::shared_ptr<audio::AudioNode> node = _effects.at(key);
    return accessInstance(node).get();
}

/**
 * Returns true if the sound effect is in a continuous loop.
 *
 * If the key does not correspond to a channel, this method returns false
 *
 * @param  key      the reference key for the sound effect
 *
 * @return true if the sound effect is in a continuous loop.
 */
bool AudioChannels::isEffectLoop(const std::string& key) const {
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<AudioNode> node = _effects.at(key);
        return _channel.at(node->getTag())->getLoops() != 0;
    }
    return false;
}

/**
 * Sets whether the sound effect is in a continuous loop.
 *
 * If the key does not correspond to a channel, this method does nothing.
 *
 * @param  key      the reference key for the sound effect
 * @param  loop     whether the sound effect is in a continuous loop
 */
void AudioChannels::setEffectLoop(const std::string& key, bool loop) {
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<AudioNode> node = _effects.at(key);
        _channel[node->getTag()]->setLoops(loop ? -1 : 0);
    }
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
float AudioChannels::getEffectVolume(const std::string& key) const {
    if (_effects.find(key) != _effects.end()) {
        return _effects.at(key)->getGain();
    }
    return 0;
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
void AudioChannels::setEffectVolume(const std::string& key, float volume) {
    CUAssertLog(volume >= 0 && volume <= 1, "Volume %f is out of range",volume);
    if (_effects.find(key) != _effects.end()) {
        _effects.at(key)->setGain(volume);
    }
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
float AudioChannels::getEffectPan(const std::string& key) const {
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<AudioFader> fader = std::dynamic_pointer_cast<AudioFader>(_effects.at(key));
        if (fader) {
            std::shared_ptr<AudioPanner> panner = std::dynamic_pointer_cast<AudioPanner>(fader->getInput());
            if (panner->getField() == 1) {
                return panner->getPan(0,1)-panner->getPan(0,0);
            } else {
                return panner->getPan(1,1)-panner->getPan(0,0);
            }
        }
    }
    return 0;
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
void AudioChannels::setEffectPan(const std::string& key, float pan) {
    CUAssertLog(pan >= -1 && pan <= 1, "Pan value %f is out of range",pan);
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<AudioFader> fader = std::dynamic_pointer_cast<AudioFader>(_effects.at(key));
        if (fader) {
            std::shared_ptr<AudioPanner> panner = std::dynamic_pointer_cast<AudioPanner>(fader->getInput());
            if (panner->getField() == 1) {
                panner->setPan(0,0,0.5-pan/2.0);
                panner->setPan(0,1,0.5+pan/2.0);
            } else {
                if (pan <= 0) {
                    panner->setPan(0,0,1);
                    panner->setPan(0,1,0);
                    panner->setPan(1,0,-pan);
                    panner->setPan(1,1,1+pan);
                } else {
                    panner->setPan(1,1,1);
                    panner->setPan(1,0,0);
                    panner->setPan(0,0,1-pan);
                    panner->setPan(0,1,pan);
                }
            }
        }
    }
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
float AudioChannels::getEffectDuration(const std::string& key) const {
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<Sound> sound = accessInstance(_effects.at(key));
        return sound->getDuration();
    }
    return -1;
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
float AudioChannels::getEffectElapsed(const std::string& key) const {
    if (_effects.find(key) != _effects.end()) {
        return _effects.at(key)->getElapsed();
    }
    return -1;
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
void AudioChannels::setEffectElapsed(const std::string& key, float time) {
    if (_effects.find(key) != _effects.end()) {
        _effects.at(key)->setElapsed(time);
    }
}

/**
 * Returns the time remaining for the sound effect, in seconds
 *
 * The time remaining is just duration-elapsed.  This method does not take
 * into account whether the sound is on a loop. Because mos sound effects
 * are fully decompressed at load time, the result of this method is
 * reasonably accurate.
 *
 * If the key does not correspond to a channel, this method returns -1.
 *
 * @param  key      the reference key for the sound effect
 *
 * @return the time remaining for the sound effect, in seconds
 */
float AudioChannels::getEffectRemaining(const std::string& key) const {
    if (_effects.find(key) != _effects.end()) {
        return _effects.at(key)->getRemaining();
    }
    return -1;
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
void AudioChannels::setEffectRemaining(const std::string& key, float time) {
    if (_effects.find(key) != _effects.end()) {
        _effects.at(key)->setRemaining(time);
    }
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
void AudioChannels::stopEffect(const std::string& key,float fade) {
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<AudioNode> node = _effects.at(key);
        _channel[node->getTag()]->setLoops(0);
        ((AudioFader*)node.get())->fadeOut(fade);
    }
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
void AudioChannels::pauseEffect(const std::string& key, float fade) {
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<AudioNode> node = _effects.at(key);
        Uint32 tag = _effects.at(key)->getTag();
        CUAssertLog(!_chfader.at(tag)->isPaused(), "The sound for that effect is already paused");
        if (fade) {
            _chfader.at(tag)->fadePause(fade);
        } else {
            _chfader.at(tag)->pause();
        }
    }
}

/**
 * Resumes the sound effect for the given key.
 *
 * If the key does not correspond to a channel, this method does nothing.
 *
 * @param  key      the reference key for the sound effect
 */
void AudioChannels::resumeEffect(std::string key) {
    if (_effects.find(key) != _effects.end()) {
        std::shared_ptr<AudioNode> node = _effects.at(key);
        Uint32 tag = _effects.at(key)->getTag();
        CUAssertLog(_chfader.at(tag)->isPaused(), "The sound for that effect is not paused");
        _chfader.at(tag)->resume();
    }
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
void AudioChannels::stopAllEffects(float fade) {
    for(auto it = _effects.begin(); it != _effects.end(); ++it) {
        it->second->fadeOut(fade);
    }
    _effects.clear();
    _equeue.clear();
}

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
void AudioChannels::pauseAllEffects(float fade) {
    for(auto it = _chfader.begin()+1; it != _chfader.end(); ++it) {
        if (!(*it)->isPaused()) {
            if (fade) {
                (*it)->fadePause(fade);
            } else {
                (*it)->pause();
            }
        }
    }
}

/**
 * Resumes all paused sound effects.
 */
void AudioChannels::resumeAllEffects() {
    for(auto it = _chfader.begin()+1; it != _chfader.end(); ++it) {
        if ((*it)->isPaused()) {
            (*it)->resume();
        }
    }
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
void AudioChannels::stopAll(float fade) {
    stopAllEffects(fade);
    stopMusic(fade);
}

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
void AudioChannels::pauseAll(float fade) {
    pauseAllEffects(fade);
    pauseMusic(fade);
}

/**
 * Resumes all paused sounds, both music and sound effects.
 *
 * You should generally call this method right after the app returns
 * from the background.
 */
void AudioChannels::resumeAll() {
    resumeAllEffects();
    resumeMusic();
}
