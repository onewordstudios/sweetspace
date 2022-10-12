//
//  CUAudioFader.cpp
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
#include <cugl/audio/graph/CUAudioFader.h>
#include <cugl/math/dsp/CUDSPMath.h>
#include <cugl/util/CUDebug.h>
#include <algorithm>
#include <thread>

using namespace cugl::audio;

/**
 * Creates a degenerate audio player with no associated source.
 *
 * The player has no channels or source file, so read options will do nothing.
 * The player must be initialized to be used.
 */
AudioFader::AudioFader() :
_fadein(0),
_fadeout(0),
_fadedip(0),
_inmark(-1),
_outmark(-1),
_dipmark(-1),
_dipstop(0),
_outdone(false),
_outkeep(false),
_diphalf(false) {
    _classname = "AudioFader";
}

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
bool AudioFader::init() {
    if (AudioNode::init()) {
        _input = nullptr;
        return true;
    }
    return false;
}

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
bool AudioFader::init(Uint8 channels, Uint32 rate) {
    if (AudioNode::init(channels,rate)) {
        _input = nullptr;
        return true;
    }
    return false;

}

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
bool AudioFader::init(const std::shared_ptr<AudioNode>& input) {
    if (input && AudioNode::init(input->getChannels(),input->getRate())) {
        _input = input;
        return true;
    }
    return false;

}

/**
 * Disposes any resources allocated for this player
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioFader::dispose() {
    if (_booted) {
        AudioNode::dispose();
        _fadein = 0;
        _inmark = -1;
        _fadeout = 0;
        _outmark = -1;
        _outkeep = false;
        _fadedip = 0;
        _dipmark = -1;
        _dipstop = 0;
        _diphalf = false;
    }
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
bool AudioFader::attach(const std::shared_ptr<AudioNode>& node) {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot attach to an uninitialized audio node");
        return false;
    } else if (node == nullptr) {
        detach();
        return true;
    } else if (node->getChannels() != _channels) {
        CUAssertLog(false,"Input node has wrong number of channels: %d", node->getChannels());
        return false;
    } else if (node->getRate() != _sampling) {
        CUAssertLog(false,"Input node has wrong sample rate: %d", node->getRate());
        return false;
    }
    
    std::atomic_exchange_explicit(&_input,node,std::memory_order_relaxed);
    return true;
}

/**
 * Detaches an audio node from this fader.
 *
 * If the method succeeds, it returns the audio node that was removed.
 *
 * @return  The audio node to detach (or null if failed)
 */
std::shared_ptr<AudioNode> AudioFader::detach() {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot detach from an uninitialized audio node");
        return nullptr;
    }
    
    std::shared_ptr<AudioNode> result = std::atomic_exchange_explicit(&_input,{},std::memory_order_relaxed);
    return result;
}

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
void AudioFader::fadeIn(double duration) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (duration <= 0) {
        _inmark = -1;
        _fadein = 0;
    } else {
        _inmark = (Sint64)(duration*getRate());
        _fadein = 0;
    }
}

/**
 * Returns true if this node is in an active fade-in.
 *
 * @return true if this node is in an active fade-in.
 */
bool AudioFader::isFadeIn() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _inmark >= 0;
}

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
void AudioFader::fadeOut(double duration, bool wrap) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (duration <= 0) {
        _outmark = -1;
        _fadeout = 0;
    } else {
        _outmark = (Sint64)(duration*getRate());
        _fadeout = 0;
    }
    _outkeep = wrap;
    _outdone = false;
}

/**
 * Returns true if this node is in an active fade-out.
 *
 * @return true if this node is in an active fade-out.
 */
bool AudioFader::isFadeOut() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _outmark >= 0;
}

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
void AudioFader::fadePause(double duration) {
    fadePause(duration,duration);
}

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
void AudioFader::fadePause(double fadein, double fadeout) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (fadein < 0 || fadeout < 0) {
        _dipmark = -1;
        _fadedip = 0;
        _dipstop = 0;
    } else {
        _dipmark = (Sint64)(fadein*getRate());
        _dipstop = (Uint64)(fadeout*getRate());
        _fadedip = 0;
    }
    _diphalf = false;
}

/**
 * Returns true if this node is in an active fade-pause.
 *
 * This method will not distinguish if the node is before or after the
 * pause point.
 *
 * @return true if this node is in an active fade-pause.
 */
bool AudioFader::isFadePause() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _dipmark >= 0;
}

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
Uint32 AudioFader::doFadeIn(float* buffer, Uint32 frames) {
    if (_inmark >= 0) {
        Uint32 left = std::min(frames,(Uint32)(_inmark-_fadein));
        float start = (float)_fadein/(float)_inmark;
        float ends  = (float)(left+_fadein)/(float)_inmark;
        dsp::DSPMath::slide(buffer,start,ends,buffer,left*_channels);
        _fadein += left;
        if (_fadein >= _inmark) {
            _inmark = -1;
            _fadein = 0;
            if (_calling.load(std::memory_order_relaxed)) {
                notify(shared_from_this(),Action::FADE_IN);
            }
        }
    }
    return frames;
}

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
Uint32 AudioFader::doFadeOut(float* buffer, Uint32 frames) {
    Sint32 amt = frames;
    if (_outmark >= 0) {
        Sint32 left = std::max(std::min(amt,(Sint32)(_outmark-_fadeout)),0);
        float start = (float)(_outmark-_fadeout)/(float)_outmark;
        float ends  = (float)(_outmark-left-_fadeout)/(float)_outmark;
        dsp::DSPMath::slide(buffer,start,ends,buffer,left*_channels);
        _fadeout += left;
        if (_fadeout >= _outmark) {
            _outmark = -1;
            _fadeout = 0;
            _outkeep = false;
            _outdone = true;
            if (_calling.load(std::memory_order_relaxed)) {
                notify(shared_from_this(),Action::FADE_OUT);
            }
        }
        amt = left;
    }
    return amt;
}

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
Uint32 AudioFader::doFadePause(float* buffer, Uint32 frames) {
    Uint32 amt = frames;
    if (_dipmark >= 0) {
        if (_diphalf) {
            Uint32 left = std::min(amt,(Uint32)std::max((Sint32)(_dipmark+_dipstop-_fadedip),(Sint32)0));
            float start = (float)(_fadedip-_dipmark)/(float)_dipstop;
            float ends  = (float)(left+_fadedip-_dipmark)/(float)_dipstop;
            dsp::DSPMath::slide(buffer,start,ends,buffer,left*_channels);
            _fadedip += left;
            if (_fadedip >= _dipmark+_dipstop) {
                _dipmark = -1;
                _dipstop = 0;
                _fadedip = 0;
                _diphalf = false;
            }
        } else {
            Uint32 left = std::min(amt,(Uint32)std::max((Sint32)(_dipmark-_fadedip),(Sint32)0));
            float start = (float)(_dipmark-_fadedip)/(float)_dipmark;
            float ends  = (float)(_dipmark-left-_fadedip)/(float)_dipmark;
            dsp::DSPMath::slide(buffer,start,ends,buffer,left*_channels);
            _fadedip += left;
            if (_fadedip >= _dipmark) {
                _paused.store(true,std::memory_order_relaxed);
                std::memset(buffer+left*_channels,0,(amt-left)*_channels*sizeof(float));
                _diphalf = true;
                if (_calling.load(std::memory_order_relaxed)) {
                    notify(shared_from_this(),Action::FADE_DIP);
                }
            }
        }
    }
    return amt;
}


#pragma mark -
#pragma mark Overriden Methods
/**
 * Returns true if this node is currently paused
 *
 * @return true if this node is currently paused
 */
bool AudioFader::isPaused() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _paused.load(std::memory_order_relaxed) || (_dipmark >= 0 && !_diphalf);
}

/**
 * Pauses this node, preventing any data from being read.
 *
 * If the node is already paused, this method has no effect. Pausing will
 * not go into effect until the next render call in the audio thread.
 *
 * @return true if the node was successfully paused
 */
bool AudioFader::pause() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_dipmark < 0 || _diphalf) {
        return !_paused.exchange(true);
    }
    return false;
}

/**
 * Resumes this previously paused node, allowing data to be read.
 *
 * If the node is not paused, this method has no effect.
 *
 * @return true if the node was successfully resumed
 */
bool AudioFader::resume() {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_dipmark >= 0 && !_diphalf) {
        _dipmark = -1;
        _fadedip = 0;
        _dipstart = 0;
        _paused.store(false,std::memory_order_relaxed);
        return true;
    }
    return _paused.exchange(false);
}

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
Uint32 AudioFader::read(float* buffer, Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input == nullptr || _paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,frames*_channels*sizeof(float));
        return frames;
    } else {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_outdone) {
            Uint32 amt = input->read(buffer, frames);
            float gain = _ndgain.load(std::memory_order_relaxed);
            if (gain != 1) {
                dsp::DSPMath::scale(buffer,gain,buffer,amt*_channels);
            }
            amt = doFadeIn(buffer,amt);
            amt = doFadeOut(buffer,amt);
            amt = doFadePause(buffer,amt);
            return amt;
        }
    }
    return 0;
}

/**
 * Returns true if this audio node has no more data.
 *
 * A completed audio node is one that will return 0 (no frames read) on
 * subsequent threads read.
 *
 * @return true if this audio node has no more data.
 */
bool AudioFader::completed() {
    bool outdone = false;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        outdone = _outdone;
    }
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    return (input == nullptr || input->completed() || outdone);
}

/**
 * Marks the current read position in the audio steam.
 *
 * This method is used by {@link reset()} to determine where to restore
 * the read position.
 *
 * @return true if the read position was marked.
 */
bool AudioFader::mark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->mark();
    }
    return false;

}

/**
 * Clears the current marked position.
 *
 * Clearing the mark in a player is equivelent to setting the mark at
 * the beginning of the audio asset.  Future calls to {@link reset()}
 * will return to the start of the audio stream.
 *
 * @return true if the read position was cleared.
 */
bool AudioFader::unmark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->unmark();
    }
    return false;
}

/**
 * Resets the read position to the marked position of the audio stream.
 *
 * If no mark is set, this will reset to the player to the beginning of
 * the audio sample.
 *
 * @return true if the read position was moved.
 */
bool AudioFader::reset() {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_inmark >= 0) {
            _inmark = -1;
            _fadein = 0;
        }
        if (_outmark >= 0) {
            if (!_outkeep) {
                _outmark = -1;
                _fadeout = 0;
            }
        }
        _outdone = false;
        if (_dipmark >= 0) {
            _dipmark = -1;
            _fadedip = 0;
            _dipstop = 0;
        }
        _diphalf = false;
    }
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->reset();
    }
    return false;
}

/**
 * Advances the stream by the given number of frames.
 *
 * This method only advances the read position, it does not actually
 * read data into a buffer.
 *
 * This method will cancel any active fade-in or fade-out.
 *
 * @param frames    The number of frames to advace
 *
 * @return the actual number of frames advanced; -1 if not supported
 */
Sint64 AudioFader::advance(Uint32 frames) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_inmark >= 0) {
            _inmark = -1;
            _fadein = 0;
        }
        if (_outmark >= 0) {
            _outmark = -1;
            _fadeout = 0;
        }
        _outdone = false;
        _outkeep = false;
        if (_dipmark >= 0) {
            _dipmark = -1;
            _fadedip = 0;
            _dipstop = 0;
        }
        _diphalf = false;
    }
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->advance(frames);
    }
    return -1;
}

/**
 * Returns the current frame position of this audio node
 *
 * The value returned will always be the absolute frame position regardless
 * of the presence of any marks.
 *
 * @return the current frame position of this audio node.
 */
Sint64 AudioFader::getPosition() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getPosition();
    }
    return -1;
}

/**
 * Sets the current frame position of this audio node.
 *
 * The value set will always be the absolute frame position regardless
 * of the presence of any marks.
 *
 * This method will cancel any active fade-in or fade-out.
 *
 * @param position  the current frame position of this audio node.
 *
 * @return the new frame position of this audio node.
 */
Sint64 AudioFader::setPosition(Uint32 position)  {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_inmark >= 0) {
            _inmark = -1;
            _fadein = 0;
        }
        if (_outmark >= 0) {
            _outmark = -1;
            _fadeout = 0;
        }
        _outdone = false;
        _outkeep = false;
        if (_dipmark >= 0) {
            _dipmark = -1;
            _fadedip = 0;
            _dipstop = 0;
        }
        _diphalf = false;
    }
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setPosition(position);
    }
    return -1;
}

/**
 * Returns the elapsed time in seconds.
 *
 * The value returned is always measured from the start of the steam,
 * regardless of the presence of any marks.
 *
 * @return the elapsed time in seconds.
 */
double AudioFader::getElapsed() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getElapsed();
    }
    return -1;
}

/**
 * Sets the read position to the elapsed time in seconds.
 *
 * The value returned is always measured from the start of the steam,
 * regardless of the presence of any marks.
 *
 * This method will cancel any active fade-in or fade-out.
 *
 * @param time  The elapsed time in seconds.
 *
 * @return the new elapsed time in seconds.
 */
double AudioFader::setElapsed(double time) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_inmark >= 0) {
            _inmark = -1;
            _fadein = 0;
        }
        if (_outmark >= 0) {
            _outmark = -1;
            _fadeout = 0;
        }
        _outdone = false;
        _outkeep = false;
        if (_dipmark >= 0) {
            _dipmark = -1;
            _fadedip = 0;
            _dipstop = 0;
        }
        _diphalf = false;
    }
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setElapsed(time);
    }
    return -1;
}

/**
 * Returns the remaining time in seconds.
 *
 * The remaining time is duration from the current read position to the
 * end of the sample.  It is not effected by any fade-out.
 *
 * @return the remaining time in seconds.
 */
double AudioFader::getRemaining() const  {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (_outmark >= 0) {
        Sint64 temp =  std::max((Sint64)0,_outmark-(Sint64)_fadeout);
        return ((double)temp)/_sampling;
    }
    if (input) {
        return input->getRemaining();
    }
    return -1;
}

/**
 * Sets the remaining time in seconds.
 *
 * This method will move the read position so that the distance between
 * it and the end of the same is the given number of seconds.
 *
 * This method will cancel any active fade-in or fade-out.
 *
 * @param time  The remaining time in seconds.
 *
 * @return the new remaining time in seconds.
 */
double AudioFader::setRemaining(double time) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_inmark >= 0) {
            _inmark = -1;
            _fadein = 0;
        }
        if (_outmark >= 0) {
            _outmark = -1;
            _fadeout = 0;
        }
        _outdone = false;
        _outkeep = false;
        if (_dipmark >= 0) {
            _dipmark = -1;
            _fadedip = 0;
            _dipstop = 0;
        }
        _diphalf = false;
    }
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setRemaining(time);
    }
    return -1;
}

