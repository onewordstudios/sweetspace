//
//  CUAudioMixer.cpp
//  Cornell University Game Library (CUGL)
//
//  Thie module provides an audio graph node for mixing together several input
//  streams into a single output stream. The input nodes must all have the same
//  same number of channels and sampling rate.
//
//  Mixing works by adding together all of the streams.  This means that the
//  results may exceed the range [-1,1], causing clipping.  The mixer provides
//  a "soft-knee" option for confining the results to the range [-1,1].
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
//  Version: 11/7/18
//
#include <cugl/audio/graph/CUAudioMixer.h>
#include <cugl/audio/CUAudioManager.h>
#include <cugl/math/dsp/CUDSPMath.h>
#include <cugl/util/CUDebug.h>
#include <atomic>

using namespace cugl;
using namespace cugl::audio;

/** The default number of inputs supported (typically 8) */
const Uint8 AudioMixer::DEFAULT_WIDTH = 8;
/** The standard knee value for preventing clipping */
const float AudioMixer::DEFAULT_KNEE  = 0.9;


#pragma mark -
#pragma mark Constructors
/**
 * Creates a degenerate mizer that takes no inputs
 *
 * The mixer has no width and therefore cannot accept any inputs. The mixer
 * must be initialized to be used.
 */
AudioMixer::AudioMixer() :
_width(0),
_knee(-1),
_capacity(0),
_inputs(nullptr),
_buffer(nullptr) {
    _classname = "AudioScheduler";
#if CU_PLATFORM == CU_PLATFORM_ANDROID
	// Android handles clipping very badly.
	_knee = AudioMixer::DEFAULT_KNEE;
#endif
}

/**
 * Initializes the mixer with default stereo settings
 *
 * The number of channels is two, for stereo output.  The sample rate is
 * the modern standard of 48000 HZ.
 *
 * These values determine the buffer the structure for all {@link read}
 * operations.  In addition, they also detemine exactly which audio nodes
 * are supported by this mixer.  A mixer can only mix nodes that agree
 * on both sample rate and frequency.
 *
 * @param width     The number of audio nodes that may be attached to this mixer
 *
 * @return true if initialization was successful
 */
bool AudioMixer::init() {
    return init(DEFAULT_WIDTH,DEFAULT_CHANNELS,DEFAULT_SAMPLING);
}

/**
 * Initializes the mixer with default stereo settings
 *
 * The number of channels is two, for stereo output.  The sample rate is
 * the modern standard of 48000 HZ.
 *
 * These values determine the buffer the structure for all {@link read}
 * operations.  In addition, they also detemine exactly which audio nodes
 * are supported by this mixer.  A mixer can only mix nodes that agree
 * on both sample rate and frequency.
 *
 * @param width     The number of audio nodes that may be attached to this mixer
 *
 * @return true if initialization was successful
 */
bool AudioMixer::init(Uint8 width) {
    return init(width,DEFAULT_CHANNELS,DEFAULT_SAMPLING);
}

/**
 * Initializes the mixer with the given number of channels and sample rate
 *
 * These values determine the buffer the structure for all {@link read}
 * operations.  In addition, they also detemine exactly which audio nodes
 * are supported by this mixer.  A mixer can only mix nodes that agree
 * on both sample rate and frequency.
 *
 * @param width     The number of audio nodes that may be attached to this mixer
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in HZ
 *
 * @return true if initialization was successful
 */
bool AudioMixer::init(Uint8 channels, Uint32 rate) {
    return init(DEFAULT_WIDTH,channels,rate);
}

/**
 * Initializes the mixer with the given number of channels and sample rate
 *
 * These values determine the buffer the structure for all {@link read}
 * operations.  In addition, they also detemine exactly which audio nodes
 * are supported by this mixer.  A mixer can only mix nodes that agree
 * on both sample rate and frequency.
 *
 * @param width     The number of audio nodes that may be attached to this mixer
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in HZ
 *
 * @return true if initialization was successful
 */
bool AudioMixer::init(Uint8 width, Uint8 channels, Uint32 rate) {
    if (AudioNode::init(channels,rate)) {
        CUAssertLog(width,"Mixer width is 0");
        _width = width;
        _knee  = -1;
        _capacity = AudioManager::get()->getReadSize();
        _inputs = new std::shared_ptr<AudioNode>[_width];
        _buffer = (float*)malloc(_capacity*_channels*sizeof(float));
        return true;
    }
    return false;
}

/**
 * Disposes any resources allocated for this mixer
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioMixer::dispose() {
    if (_booted) {
        AudioNode::dispose();
        delete[] _inputs;
        free(_buffer);
        _inputs = nullptr;
        _buffer = nullptr;
        _width = 0;
        _knee  = -1;
        _capacity = 0;
    }
}


#pragma mark -
#pragma mark Audio Graph Methods
/**
 * Attaches an input node to this mixer.
 *
 * The input is attached at the given slot. Any input node previously at
 * that slot is removed (and returned by this method).
 *
 * @param slot  The slot for the input node
 * @param input The input node to attach
 *
 * @return the input node previously at the given slot
 */
std::shared_ptr<AudioNode> AudioMixer::attach(Uint8 slot, const std::shared_ptr<AudioNode>& input) {
    CUAssertLog(slot < _width, "Slot %d is out of range",slot);
    if (input == nullptr) {
        return detach(slot);
    } else if (input->getChannels() != _channels) {
        CUAssertLog(false,"AudioNode has wrong number of channels: %d vs %d",
                    input->getChannels(),_channels);
        return nullptr;
    } else if (input->getRate() != _sampling) {
        CUAssertLog(false,"AudioNode has wrong sample rate: %d vs %d",
                    input->getRate(),_sampling);
        return nullptr;
    }
    return std::atomic_exchange_explicit(_inputs+slot,input,std::memory_order_relaxed);
}

/**
 * Detaches the input node at the given slot.
 *
 * The input node detached is returned by this method.
 *
 * @param slot  The slot for the input node
 *
 * @return the input node detached from the slot
 */
std::shared_ptr<AudioNode> AudioMixer::detach(Uint8 slot) {
    CUAssertLog(slot < _width, "Slot %d is out of range",slot);
    return std::atomic_exchange_explicit(_inputs+slot,{},std::memory_order_relaxed);
}

/**
 * Reads up to the specified number of frames into the given buffer
 *
 * AUDIO THREAD ONLY: Users should never access this method directly, unless
 * part of a custom audio graph node.
 *
 * The buffer should have enough room to store frames * channels elements.
 * The channels are interleaved into the output buffer.
 *
 * Reading the buffer has no affect on the read position.  You must manually
 * move the frame position forward.  This is to allow for a frame window to
 * be reread if necessary.
 *
 * @param buffer    The read buffer to store the results
 * @param frames    The maximum number of frames to read
 *
 * @return the actual number of frames read
 */
Uint32 AudioMixer::read(float* buffer, Uint32 frames) {
    dsp::DSPMath::VECTORIZE = true;
    std::memset(buffer,0,frames*_channels*sizeof(float));
    frames = std::min(frames,_capacity);
    if (!_paused.load(std::memory_order_relaxed)) {
        std::shared_ptr<AudioNode> temp;
        for(int ii = 0; ii < _width; ii++) {
            temp = std::atomic_load_explicit(_inputs+ii,std::memory_order_relaxed);
            if (temp) {
                Uint32 amt = temp->read(_buffer,frames);
                if (amt < frames) {
                    std::memset(_buffer+amt,0,(frames-amt)*_channels*sizeof(float));
                }
                dsp::DSPMath::add(_buffer,buffer,buffer,frames*_channels);
            }
        }
        //dsp::DSPMath::scale(buffer,_ndgain.load(std::memory_order_relaxed),buffer,frames*_channels);
        float knee = _knee.load(std::memory_order_relaxed);
        if (knee == 1) {
            //dsp::DSPMath::clamp(buffer,-1,1,frames*_channels);
        } else if (knee > 0) {
            //dsp::DSPMath::ease(buffer,1,knee,frames*_channels);
        }
    }
    return frames;
}


#pragma mark -
#pragma mark Audio Graph Methods
/**
 * Returns the "soft knee" of this mixer, or -1 if not set
 *
 * The soft knee is used to ensure that the results fit in the range [-1,1].
 * If the knee is k, then values in the range [-k,k] are unaffected, but
 * values outside of this range are asymptotically clamped to the range
 * [-1,1], using the formula (x-k+k*k)/x.
 *
 * @return the "soft knee" of this mixer, or -1 if not set
 */
float AudioMixer::getKnee() const {
    return _knee.load(std::memory_order_relaxed);
}

/**
 * Sets the "soft knee" of this mixer.
 *
 * The soft knee is used to ensure that the results fit in the range [-1,1].
 * If the knee is k, then values in the range [-k,k] are unaffected, but
 * values outside of this range are asymptotically clamped to the range
 * [-1,1], using the formula (x-k+k*k)/x
 *
 * Setting this value to negative will disable the soft knee.  All inputs
 * will be mixed exactly with no distortion.
 *
 * @param knee  the "soft knee" of this mixer
 */
void AudioMixer::setKnee(float knee) {
    if (knee <= 0 || knee >= 1) {
        knee = -1;
    }
    _knee.store(knee,std::memory_order_relaxed);
}

