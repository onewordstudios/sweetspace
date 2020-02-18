//
//  CUAudioMixer.h
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
#ifndef __CU_AUDIO_MIXER_H__
#define __CU_AUDIO_MIXER_H__
#include "CUAudioNode.h"

namespace cugl {
    namespace audio {
/**
 * This class represents an audio mixer.
 *
 * This mixer can take (a fixed number of) input streams and combine them
 * together into a single output stream.  The input streams must all have the
 * same number of channels and sample rate as this node.
 *
 * Mixing works by adding together all of the streams.  This means that the
 * results may exceed the range [-1,1], causing clipping.  The mixer provides
 * a "soft-knee" option (disabled by default) for confining the results to the
 * range [-1,1]. When a knee k is specified, all values [-k,k] will not be
 * effected, but values outside of this range will asymptotically bend to
 * the range [-1,1].
 *
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the user.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioMixer : public AudioNode {
private:
    /** The input nodes to be mixed */
    std::shared_ptr<AudioNode>* _inputs;
    /** The number of input nodes supported by this mixer */
    Uint8 _width;

    /** The intermediate buffer for the mixed result */
    float* _buffer;
    /** The capacity of the intermediate buffer */
    Uint32 _capacity;
    
    /** The knee value for clamping */
    std::atomic<float>  _knee;

public:
#pragma mark Constructors
    /** The default number of inputs supported (typically 8) */
    static const Uint8 DEFAULT_WIDTH;
    /** The standard knee value for preventing clipping */
    static const float DEFAULT_KNEE;

    /**
     * Creates a degenerate mizer that takes no inputs
     *
     * The mixer has no width and therefore cannot accept any inputs. The mixer
     * must be initialized to be used.
     */
    AudioMixer();
    
    /**
     * Deletes this mixer, disposing of all resources.
     */
    ~AudioMixer() { dispose(); }
    
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
     * @return true if initialization was successful
     */
    virtual bool init() override;

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
    bool init(Uint8 width);

    /**
     * Initializes the mixer with the given number of channels and sample rate
     *
     * These values determine the buffer the structure for all {@link read}
     * operations.  In addition, they also detemine exactly which audio nodes
     * are supported by this mixer.  A mixer can only mix nodes that agree
     * on both sample rate and frequency.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
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
    bool init(Uint8 width, Uint8 channels, Uint32 rate);
    
    /**
     * Disposes any resources allocated for this mixer
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;

#pragma mark Static Constructors
    /**
     * Returns a newly allocated mixer with default stereo settings
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
     * @return a newly allocated mixer with default stereo settings
     */
    static std::shared_ptr<AudioMixer> alloc(Uint8 width=8) {
        std::shared_ptr<AudioMixer> result = std::make_shared<AudioMixer>();
        return (result->init(width) ? result : nullptr);
    }

    /**
     * Returns a newly allocated mixer with the given number of channels and sample rate
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
     * @return a newly allocated mixer with the given number of channels and sample rate
     */
    static std::shared_ptr<AudioMixer> alloc(Uint8 width, Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioMixer> result = std::make_shared<AudioMixer>();
        return (result->init(width,channels, rate) ? result : nullptr);
    }
    
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
    std::shared_ptr<AudioNode> attach(Uint8 slot, const std::shared_ptr<AudioNode>& input);
    
    /**
     * Detaches the input node at the given slot.
     *
     * The input node detached is returned by this method.
     *
     * @param slot  The slot for the input node
     *
     * @return the input node detached from the slot
     */
    std::shared_ptr<AudioNode> detach(Uint8 slot);
    
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
    virtual Uint32 read(float* buffer, Uint32 frames) override;
    
#pragma mark Anticlipping Methods
    /**
     * Returns the "soft knee" of this mixer, or -1 if not set
     *
     * The soft knee is used to ensure that the results fit in the range [-1,1].
     * If the knee is k, then values in the range [-k,k] are unaffected, but
     * values outside of this range are asymptotically clamped to the range
     * [-1,1], using the formula (x-k+k*k)/x.
     *
     * If the value is 0, then this mixer will hard clamp to [-1,1]. If it is
     * negative, all inputs will be mixed exactly with no distortion.
     *
     * @return the "soft knee" of this mixer, or -1 if not set
     */
    float getKnee() const;
    
    /**
     * Sets the "soft knee" of this mixer.
     *
     * The soft knee is used to ensure that the results fit in the range [-1,1].
     * If the knee is k, then values in the range [-k,k] are unaffected, but
     * values outside of this range are asymptotically clamped to the range
     * [-1,1], using the formula (x-k+k*k)/x
     *
     * If the value is 0, then this mixer will hard clamp to [-1,1]. If it is
     * negative, all inputs will be mixed exactly with no distortion.
     *
     * @param knee  the "soft knee" of this mixer
     */
    void setKnee(float knee);

};
    }
}

#endif /* __CU_AUDIO_MIXER_H__ */
