//
//  CUAudioFader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an audio node wrapper for fade-in and fade-out support.
//  We have decoupled fade-in and out from the player since we want to apply it
//  to arbitrary audio patches. We have also decoupled it from the scheduler,
//  as the schedule API is complicated enough as it is.  By factoring this
//  out into its own node, it is easier for us to support nonlinear fades (such
//  as with an easing function.
//
//  NOTE: Easing functions are not yet supported.  They are on the milestone
//  for the next release.
//
//  CUGL MIT License:
//
//     This software is provided 'as-is', without any express or implied
//     warranty.  In no event will the authors be held liable for any damages
//     arising from the use of this software.
//
//     Permission is granted to anyone to use this software for any purpose,
//     including commercial applications, and to alter it and redistribute it
//     freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 11/20/18
//
#ifndef __CU_AUDIO_FADER_H__
#define __CU_AUDIO_FADER_H__
#include <SDL/SDL.h>
#include "CUAudioNode.h"
#include <mutex>

namespace cugl {
    namespace audio {

/**
 * This class adds fade-in and fade-out support to an audio node.
 *
 * Fading is an important part of any audio engine.  Without fading, an audio
 * source will click when paused or stopped.  This node should be inserted into
 * an audio graph anywhere the user might need to pause/resume audio on demand.
 *
 * We have decoupled fade-in and out from {@link AudioPlayer} since we want to
 * apply it to arbitrary audio graphs. We have also decoupled it from
 * {@link AudioScheduler}, since we do not want fade support to require a
 * scheduler, and the scheduler API is complicated enough as it is. By factoring
 * this out into its own node, it is easier for us to support nonlinear fades
 * (such as with an easing function).
 *
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the user.
 *
 * This audio node supports the callback functions in {@link AudioNode#setCallback}.
 * This function function is called whenever a fade-in or fade-out has completed
 * successfully (without interruption).
 */
class AudioFader : public AudioNode {
protected:
    /** The audio input node */
    std::shared_ptr<AudioNode> _input;

    /** This class needs a proper lock guard; too many race conditions */
    std::mutex _mutex;
    
    // Fade-in: For softer starts
    /** The final frame of the current fade-in; -1 if no active fade-in */
    Sint64 _inmark;
    /** The current fade-in in frames; 0 if no active fade-in */
    Uint64 _fadein;
    
    // Fade-out: For smooth stopping
    /** The final frame of the current fade-out; -1 if no active fade-out */
    Sint64 _outmark;
    /** The current fade-out in frames; 0 if no active fade-out */
    Uint64 _fadeout;
    /** Whether we have completed this node due to a fadeout */
    bool   _outdone;
    /** Whether to persist fade-out on a reset */
    bool   _outkeep;
    
    // Fade-dip: For smooth pausing
    /** The current fade-dip in frames; 0 if no active fade-dip */
    Uint64 _fadedip;
    /** The middle (pause) frame of the fade-dip; -1 if no active fade-dip */
    Sint64 _dipmark;
    /** The final (resume) frame of the fade-dip; 0 if no active fade-dip */
    Uint64 _dipstop;
    /** Whether we have completed the first half of a fade-dip */
    bool   _diphalf;
    /** To prevent a race condition on pausing */
    bool   _dipstart;

    
    /**
     * Performs a fade-in.
     *
     * This method is called by {@link read} to adjust the gain for a fade-in.
     * Depending on where the fade completes, it may not process all of the
     * elements in the buffer.
     *
     * If this method reaches the end of a fade-in, it will execute a callback
     * function if one is provided.
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * @param buffer    The read buffer to store the results
     * @param frames    The maximum number of frames to process
     *
     * @return the actual number of frames processed
     */
    Uint32 doFadeIn(float* buffer, Uint32 frames);
    
    /**
     * Performs a fade-out.
     *
     * This method is called by {@link read} to adjust the gain for a fade-in.
     * Depending on where the fade completes, it may not process all of the
     * elements in the buffer.
     *
     * If this method reaches the end of a fade-out, it will execute a callback
     * function if one is provided.  It will also mark the node as completed.
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * @param buffer    The read buffer to store the results
     * @param frames    The maximum number of frames to process
     *
     * @return the actual number of frames processed
     */
    Uint32 doFadeOut(float* buffer, Uint32 frames);
    
    /**
     * Performs a fade-pause.
     *
     * This method is called by {@link read} to adjust the gain for a fade-in.
     * Depending on where the fade completes, it may not process all of the
     * elements in the buffer.
     *
     * If this method reaches the MIDDLE of a fade-pause, it will execute a
     * callback function if one is provided.  Hence the callback is to indicate
     * when the pause has gone into effect.  It will also pause the node at
     * that time.  When the node is resumed, this method will call another
     * callback (if applicable) when the fade-in is completed.
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * @param buffer    The read buffer to store the results
     * @param frames    The maximum number of frames to process
     *
     * @return the actual number of frames processed
     */
    Uint32 doFadePause(float* buffer, Uint32 frames);

public:
#pragma mark Constructors
    /**
     * Creates a degenerate audio player with no associated source.
     *
     * The player has no channels or source file, so read options will do nothing.
     * The player must be initialized to be used.
     */
    AudioFader();
    
    /**
     * Deletes this audio player, disposing of all resources.
     */
    ~AudioFader() { dispose(); }
    
    /**
     * Initializes the node with default stereo settings
     *
     * The number of channels is two, for stereo output.  The sample rate is
     * the modern standard of 48000 HZ.
     *
     * These values determine the buffer the structure for all {@link read}
     * operations.  In addition, they also detemine whether this node can
     * serve as an input to other nodes in the audio graph.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
    /**
     * Initializes the node with the given number of channels and sample rate
     *
     * These values determine the buffer the structure for all {@link read}
     * operations.  In addition, they also detemine whether this node can
     * serve as an input to other nodes in the audio graph.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
    /**
     * Initializes a fader for the given input node.
     *
     * This node acquires the channels and sample rate of the input.  If
     * input is nullptr, this method will fail.
     *
     * @param input     The audio node to fade
     *
     * @return true if initialization was successful
     */
    bool init(const std::shared_ptr<AudioNode>& input);
    
    /**
     * Disposes any resources allocated for this player
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated fader with the default stereo settings
     *
     * The number of channels is two, for stereo output.  The sample rate is
     * the modern standard of 48000 HZ. Any input node must agree with these
     * settings.
     *
     * @return a newly allocated fader with the default stereo settings
     */
    static std::shared_ptr<AudioFader> alloc() {
        std::shared_ptr<AudioFader> result = std::make_shared<AudioFader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated fader with the given number of channels and sample rate
     *
     * Any input node must agree with these settings.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return a newly allocated fader with the given number of channels and sample rate
     */
    static std::shared_ptr<AudioFader> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioFader> result = std::make_shared<AudioFader>();
        return (result->init(channels,rate) ? result : nullptr);
    }

    /**
     * Returns a newly allocated fader for the given input node.
     *
     * This node acquires the channels and sample rate of the input.  If
     * input is nullptr, this method will fail.
     *
     * @param input     The audio node to fade
     *
     * @return a newly allocated fader for the given input node.
     */
    static std::shared_ptr<AudioFader> alloc(const std::shared_ptr<AudioNode>& input) {
        std::shared_ptr<AudioFader> result = std::make_shared<AudioFader>();
        return (result->init(input) ? result : nullptr);
    }

#pragma mark -
#pragma mark Fade In/Out Support
    /**
     * Attaches an audio node to this fader.
     *
     * This method will fail if the channels of the audio node do not agree
     * with this fader.
     *
     * @param node  The audio node to fade
     *
     * @return true if the attachment was successful
     */
    bool attach(const std::shared_ptr<AudioNode>& node);
    
    /**
     * Detaches an audio node from this fader.
     *
     * If the method succeeds, it returns the audio node that was removed.
     *
     * @return  The audio node to detach (or null if failed)
     */
    std::shared_ptr<AudioNode> detach();
    
    /**
     * Returns the input node of this fader.
     *
     * @return the input node of this fader.
     */
    std::shared_ptr<AudioNode> getInput() { return _input; }

    // TODO: Add easing functions for fade-in/fade-out
    /**
     * Starts a fade-in from the current position.
     *
     * This temporarily sets the gain to 0 and then ramps up to the correct
     * gain over the time period duration.  The effect is a linear fade-in.
     * If this node has a callback function, it will be called when the
     * fade-in is complete (e.g. volume has returned to normal).
     *
     * Fade-in is emphemeral and it lost when the read position is moved or the
     * fade-in is complete.
     *
     * @param duration  The fade-in time in seconds
     */
    void fadeIn(double duration);
    
    /**
     * Returns true if this node is in an active fade-in.
     *
     * @return true if this node is in an active fade-in.
     */
    bool isFadeIn();
    
    /**
     * Starts a fade-out from the current position.
     *
     * This will reduce the gain to 0 over the time period duration.  In
     * addition, it will mark the node as completed once it reaches 0
     * gain. The effect is a linear fade-out. If this node has a callback
     * function, it will be called when the fade-out is complete (e.g. volume
     * has reduced to 0).
     *
     * If the duration is longer than the length of the audio, the volume
     * will not drop to 0 (and the callback will not be invoked).  However,
     * if wrap is set to true, the fade-in will carry over on a reset,
     * fadeing in from the beginning.
     *
     * Moving the read position will cancel a fade out.
     *
     * @param duration  The fade-in time in seconds
     * @param wrap      Whether to support a fade-out after reset
     */
    void fadeOut(double duration, bool wrap=false);

    /**
     * Returns true if this node is in an active fade-out.
     *
     * @return true if this node is in an active fade-out.
     */
    bool isFadeOut();

    /**
     * Starts a fade-pause from the current position.
     *
     * This method will reduce the gain to 0 over the time period duration. In
     * addition, it will pause once reaches 0 gain.  When the node is resumed,
     * it will ramp back up to the correct gain over the time period duration.
     * Both fade effects are linear.
     *
     * If this node has a callback function, it will be called twice during the
     * pause.  It will be called when the first fade-out is complete (e.g. the
     * volume is reduced to 0) and then again when the fade-in has completed
     * after resuming.
     *
     * A fade-pause is emphemeral and it lost when the read position is moved
     * or the fade-pause is complete.
     *
     * @param duration  The fade-in/out time in seconds
     */
    void fadePause(double duration);
    
    /**
     * Returns true if this node is in an active fade-pause.
     *
     * This method will not distinguish if the node is before or after the
     * pause point.
     *
     * @return true if this node is in an active fade-pause.
     */
    bool isFadePause();

    /**
     * Starts a fade-pause from the current position.
     *
     * This method will reduce the gain to 0 over the time period fadein. In
     * addition, it will pause once reaches 0 gain.  When the node is resumed,
     * it will ramp back up to the correct gain over the time period fadeout.
     * Both fade effects are linear.
     *
     * If this node has a callback function, it will be called twice during the
     * pause.  It will be called when the first fade-out is complete (e.g. the
     * volume is reduced to 0) and then again when the fade-in has completed
     * after resuming.
     *
     * A fade-pause is emphemeral and it lost when the read position is moved
     * or the fade-pause is complete.
     *
     * @param fadeout  The fade-out time in seconds
     * @param fadein   The fade-in time in seconds
     */
    void fadePause(double fadein, double fadeout);

#pragma mark -
#pragma mark Overriden Methods
    /**
     * Returns true if this node is currently paused
     *
     * @return true if this node is currently paused
     */
    virtual bool isPaused() override;
    
    /**
     * Pauses this node, preventing any data from being read.
     *
     * If the node is already paused, this method has no effect. Pausing will
     * not go into effect until the next render call in the audio thread.
     *
     * @return true if the node was successfully paused
     */
    virtual bool pause() override;
    
    /**
     * Resumes this previously paused node, allowing data to be read.
     *
     * If the node is not paused, this method has no effect.
     *
     * @return true if the node was successfully resumed
     */
    virtual bool resume() override;

    /**
     * Reads up to the specified number of frames into the given buffer
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * The buffer should have enough room to store frames * channels elements.
     * The channels are interleaved into the output buffer.
     *
     * This method will always forward the read position after reading. Reading
     * again may return different data.
     *
     * @param buffer    The read buffer to store the results
     * @param frames    The maximum number of frames to read
     *
     * @return the actual number of frames read
     */
    virtual Uint32 read(float* buffer, Uint32 frames) override;
    
    /**
     * Returns true if this audio node has no more data.
     *
     * A completed audio node is one that will return 0 (no frames read) on
     * subsequent threads read.
     *
     * @return true if this audio node has no more data.
     */
    virtual bool completed() override;
    
    /**
     * Marks the current read position in the audio steam.
     *
     * This method is used by {@link reset()} to determine where to restore
     * the read position.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * Clearing the mark in a player is equivelent to setting the mark at
     * the beginning of the audio asset.  Future calls to {@link reset()}
     * will return to the start of the audio stream.
     *
     * @return true if the read position was cleared.
     */
    virtual bool unmark() override;
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * If no mark is set, this will reset to the player to the beginning of
     * the audio sample.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;
    
    /**
     * Advances the stream by the given number of frames.
     *
     * This method only advances the read position, it does not actually
     * read data into a buffer.
     *
     * @param frames    The number of frames to advace
     *
     * @return the actual number of frames advanced; -1 if not supported
     */
    virtual Sint64 advance(Uint32 frames) override;
    
    /**
     * Returns the current frame position of this audio node
     *
     * The value returned will always be the absolute frame position regardless
     * of the presence of any marks.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * The value set will always be the absolute frame position regardless
     * of the presence of any marks.
     *
     * @param position  the current frame position of this audio node.
     *
     * @return the new frame position of this audio node.
     */
    virtual Sint64 setPosition(Uint32 position) override;
    
    /**
     * Returns the elapsed time in seconds.
     *
     * The value returned is always measured from the start of the steam,
     * regardless of the presence of any marks.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * The value returned is always measured from the start of the steam,
     * regardless of the presence of any marks.
     *
     * @param time  The elapsed time in seconds.
     *
     * @return the new elapsed time in seconds.
     */
    virtual double setElapsed(double time) override;
    
    /**
     * Returns the remaining time in seconds.
     *
     * The remaining time is duration from the current read position to the
     * end of the sample.  It is not effected by any fade-out.
     *
     * @return the remaining time in seconds.
     */
    virtual double getRemaining() const override;
    
    /**
     * Sets the remaining time in seconds.
     *
     * This method will move the read position so that the distance between
     * it and the end of the same is the given number of seconds.
     *
     * @param time  The remaining time in seconds.
     *
     * @return the new remaining time in seconds.
     */
    virtual double setRemaining(double time) override;
};
    }
}

#endif /* __CU_AUDIO_FADER_H__ */
