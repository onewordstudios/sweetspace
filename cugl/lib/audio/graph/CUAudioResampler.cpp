//
//  CUAudioResampler.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a graph node for converting from one sample rate to
//  another.  It uses SDL_AudioStream to perform continuous resampling on a
//  potentially infinite audio stream.  This is is necessary for cross-platform
//  reasons as iPhones are very stubborn about delivering any requested sampling
//  rates other than 48000.
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
//  Version: 1/26/19
//
#include <cugl/audio/graph/CUAudioResampler.h>
#include <cugl/audio/CUAudioManager.h>
#include <cugl/math/dsp/CUDSPMath.h>
#include <cugl/util/CUDebug.h>
#include <cmath>

using namespace cugl::audio;

/**
 * Creates a degenerate audio resampler.
 *
 * The node has not been initialized, so it is not active.  The node
 * must be initialized to be used.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on
 * the heap, use the factory in {@link AudioManager}.
 */
AudioResampler::AudioResampler() : AudioNode(),
_inputrate(0),
_capacity(0),
_cvtratio(1.0f),
_cvtbuffer(nullptr),
_resampler(nullptr) {
    _input = nullptr;
    _classname = "AudioResampler";
}

/**
 * Initializes a resampler with 2 channels at 48000 Hz.
 *
 * This sample rate of the output of this node is 48000 Hz, but the input
 * sample rate depends on the input node, which can change over time. However,
 * the input node must agree with number of channels, which is fixed.
 *
 * @return true if initialization was successful
 */
bool AudioResampler::init() {
    return init(DEFAULT_CHANNELS,DEFAULT_SAMPLING);
}

/**
 * Returns a newly allocated resampler with the given channels and sample rate.
 *
 * This sample rate is the output rate of this node.  The input same rate
 * depends on the input node, which can change over time. However, the
 * input node must agree with number of channels, which is fixed.
 *
 * @param channels  The number of audio channels
 * @param rate      The output sample rate (frequency) in HZ
 *
 * @return a newly allocated resampler with the given channels and sample rate.
 */
bool AudioResampler::init(Uint8 channels, Uint32 rate) {
    if (AudioNode::init(channels,rate)) {
        _capacity = sizeof(float)*_channels*2*AudioManager::get()->getReadSize();
        _cvtbuffer = (float*)malloc(_capacity);
        std::memset(_cvtbuffer,0,_capacity);
        
        _inputrate = rate;
        return true;
    }
    return false;
}

/**
 * Disposes any resources allocated for this resampler.
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioResampler::dispose() {
    if (_booted) {
        _input = nullptr;
        if (_resampler != NULL) {
            SDL_AudioStreamClear(_resampler);
            SDL_FreeAudioStream(_resampler);
            free(_cvtbuffer);
            _resampler = NULL;
            _cvtbuffer = nullptr;
        }
        _capacity  = 0;
        _cvtratio  = 1.0f;
        _inputrate = 0;
    }
}

#pragma mark -
#pragma mark Audio Graph
/**
 * Attaches an audio node to this resampler.
 *
 * This method will reset the resampler stream if the input has a different
 * rate than the previous input value (and is not the same rate as the
 * output).  It will fail if the input does not have the same number of
 * channels as this resampler.
 *
 * @param node  The audio node to resample
 *
 * @return true if the attachment was successful
 */
bool AudioResampler::attach(const std::shared_ptr<AudioNode>& node) {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot attach to an uninitialized audio node");
        return false;
    } else if (node == nullptr) {
        detach();
        return true;
    } else if (node->getChannels() != _channels) {
        CUAssertLog(false,"Input node has wrong number of channels: %d", node->getChannels());
        return false;
    }
    
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        detach();
    }

    {
        std::unique_lock<std::mutex> lk(_buffmtex);
        if (node->getRate() != _inputrate) {
            _inputrate = node->getRate();
            _cvtratio  = ((float)_inputrate)/getRate();
            size_t bsize = sizeof(float)*_channels*std::ceil(_cvtratio*AudioManager::get()->getReadSize());
            if (bsize > _capacity) {
                free(_cvtbuffer);
                _capacity  = bsize;
                _cvtbuffer = (float*)malloc(_capacity);
            }
            if (_resampler != NULL) {
                SDL_AudioStreamClear(_resampler);
                SDL_FreeAudioStream(_resampler);
            }
            if (_inputrate != getRate()) {
                _resampler = SDL_NewAudioStream(AUDIO_F32SYS, _channels, _inputrate,
                                                AUDIO_F32SYS, _channels, getRate());
                if (_resampler == NULL) {
                    CULogError("[AUDIO] Could not create a resampler.");
                    return false;
                }
            }
        }
        
        // Initial 0s (else it will pop)
        std::memset(_cvtbuffer,0,_capacity);
        SDL_AudioStreamPut(_resampler, _cvtbuffer, sizeof(float)*_channels);
        
        std::atomic_store_explicit(&_input,node,std::memory_order_relaxed);
        return true;
    }
    return false;
}

/**
 * Detaches an audio node from this resampler.
 *
 * If the method succeeds, it returns the audio node that was removed.
 * This method will not automatically reset the sampling stream.
 *
 * @return  The audio node to detach (or null if failed)
 */
std::shared_ptr<AudioNode> AudioResampler::detach() {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot detach from an uninitialized output device");
        return nullptr;
    }
    
    std::shared_ptr<AudioNode> result = std::atomic_exchange_explicit(&_input,{},std::memory_order_relaxed);
    return result;
}

#pragma mark -
#pragma mark Playback Control
/**
 * Returns true if this resampler has no more data.
 *
 * An audio node is typically completed if it return 0 (no frames read) on
 * subsequent calls to {@link read()}.  However, for infinite-running
 * audio threads, it is possible for this method to return true even when
 * data can still be read; in that case the node is notifying that it
 * should be shut down.
 *
 * @return true if this audio node has no more data.
 */
bool AudioResampler::completed() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    return (input == nullptr || input->completed());
}

/**
 * Reads up to the specified number of frames into the given buffer
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioOutput.
 *
 * The buffer should have enough room to store frames * channels elements.
 * The channels are interleaved into the output buffer.
 *
 * This method will always forward the read position.
 *
 * @param buffer    The read buffer to store the results
 * @param frames    The maximum number of frames to read
 *
 * @return the actual number of frames read
 */
Uint32 AudioResampler::read(float* buffer, Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input == nullptr || _paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,frames*_channels*sizeof(float));
    } else {
        std::unique_lock<std::mutex> lk(_buffmtex);
        Sint32 take = 0;
        if (_resampler != NULL) {
            bool search = true;
            while (take < frames && search) {
                Sint32 amt = std::ceil(frames*_cvtratio);
                amt = input->read(_cvtbuffer, amt);
                SDL_AudioStreamPut(_resampler, _cvtbuffer, amt*sizeof(float)*_channels);
                amt  = SDL_AudioStreamGet(_resampler, buffer+take*_channels,
                                          (frames-take)*_channels*sizeof(float));
                if (amt == -1) {
                    CULogError("[AUDIO] Resampling error.");
                    std::memset(buffer+take*_channels,0,(frames-take)*_channels*sizeof(float));
                    take = frames;
                } else if (amt == 0) {
                    search = false;
                } else {
                    take += amt/sizeof(float)*_channels;
                }
            }
        } else {
            take = input->read(buffer, frames);
        }
        
        dsp::DSPMath::scale(buffer,_ndgain.load(std::memory_order_relaxed),buffer,take*_channels);
        return take;
    }
    return frames;
}

#pragma mark -
#pragma mark Optional Methods
/**
 * Marks the current read position in the audio steam.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * This method is typically used by {@link reset()} to determine where to
 * restore the read position. For some nodes (like {@link AudioInput}),
 * this method may start recording data to a buffer, which will continue
 * until {@link clear()} is called.
 *
 * It is possible for {@link reset()} to be supported even if this method
 * is not.
 *
 * @return true if the read position was marked.
 */
bool AudioResampler::mark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->mark();
    }
    return false;
}

/**
 * Clears the current marked position.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * If the method {@link mark()} started recording to a buffer (such as
 * with {@link AudioInput}), this method will stop recording and release
 * the buffer.  When the mark is cleared, {@link reset()} may or may not
 * work depending upon the specific node.
 *
 * @return true if the read position was marked.
 */
bool AudioResampler::unmark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->unmark();
    }
    return false;
}

/**
 * Resets the read position to the marked position of the audio stream.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * When no {@link mark()} is set, the result of this method is node
 * dependent.  Some nodes (such as {@link AudioPlayer}) will reset to the
 * beginning of the stream, while others (like {@link AudioInput}) only
 * support a rest when a mark is set. Pay attention to the return value of
 * this method to see if the call is successful.
 *
 * @return true if the read position was moved.
 */
bool AudioResampler::reset() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->reset();
    }
    return false;
}

/**
 * Advances the stream by the given number of frames.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * This method only advances the read position, it does not actually
 * read data into a buffer. This method is generally not supported
 * for nodes with real-time input like {@link AudioInput}.
 *
 * @param frames    The number of frames to advace
 *
 * @return the actual number of frames advanced; -1 if not supported
 */
Sint64 AudioResampler::advance(Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        std::unique_lock<std::mutex> lk(_buffmtex);
        return input->advance(std::ceil(frames*_cvtratio));
    }
    return -1;
}

/**
 * Returns the current frame position of this audio node
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the position will be the
 * number of frames since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @return the current frame position of this audio node.
 */
Sint64 AudioResampler::getPosition() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return std::ceil(input->getPosition()*_cvtratio.load(std::memory_order_relaxed));
    }
    return -1;
}

/**
 * Sets the current frame position of this audio node.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the position will be the
 * number of frames since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @param position  the current frame position of this audio node.
 *
 * @return the new frame position of this audio node.
 */
Sint64 AudioResampler::setPosition(Uint32 position) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setPosition(std::ceil(position*_cvtratio));
    }
    return -1;
}

/**
 * Returns the elapsed time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the times will be the
 * number of seconds since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @return the elapsed time in seconds.
 */
double AudioResampler::getElapsed() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getElapsed();
    }
    return -1;
}

/**
 * Sets the read position to the elapsed time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the new time will be meaured
 * from the mark. Other nodes like {@link AudioPlayer} measure from the
 * start of the stream.
 *
 * @param time  The elapsed time in seconds.
 *
 * @return the new elapsed time in seconds.
 */
double AudioResampler::setElapsed(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setElapsed(time);
    }
    return -1;
}

/**
 * Returns the remaining time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node or if this method is unsupported
 * in that node
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link setRemaining()} has been called.  In that case, the node will
 * be marked as completed after the given number of seconds.  This may or may
 * not actually move the read head.  For example, in {@link AudioPlayer} it
 * will skip to the end of the sample.  However, in {@link AudioInput} it
 * will simply time out after the given time.
 *
 * @return the remaining time in seconds.
 */
double AudioResampler::getRemaining() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getRemaining();
    }
    return -1;
}

/**
 * Sets the remaining time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node or if this method is unsupported
 * in that node
 *
 * If this method is supported, then the node will be marked as completed
 * after the given number of seconds.  This may or may not actually move
 * the read head.  For example, in {@link AudioPlayer} it will skip to the
 * end of the sample.  However, in {@link AudioInput} it will simply time
 * out after the given time.
 *
 * @param time  The remaining time in seconds.
 *
 * @return the new remaining time in seconds.
 */
double AudioResampler::setRemaining(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setRemaining(time);
    }
    return -1;
}
