//
//  CUAudioPanner.h
//  Cornell University Game Library (CUGL)
//
//  This module is a general purpose audio panner.  It can convert an audio node
//  with any given number of channels to one with a different number of channels
//  (but the same sampling rate).  It does this via a panning matrix.  This
//  matrix specifies the contribution (in a range of 0 to 1) of each input
//  channel to each output channel.
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
//  Version: 12/5/18
//
#ifndef __CU_AUDIO_PANNER_H__
#define __CU_AUDIO_PANNER_H__
#include "CUAudioNode.h"
#include <atomic>

namespace cugl {
    namespace audio {
/**
 * A class representing a general purpose audio panner.
 *
 * This audio node takes another audio node as input. That node must agree with
 * the sample rate of this node, but need not have the same number of channels.
 * In fact, the input node must instead have {@link getField()} number of
 * channels.  It then maps the data from this input channels to the output
 * channels.
 *
 * This mapping happens via a panning matrix.  This matrix specifies the
 * contribution (in a range of 0 to 1) of each input channel to each output
 * channel.  By default, each input channel maps fully (value 1) to the same
 * output channel (or is dropped if that output channel does not exist).
 * The values of this matrix may be changed at any time.
 *
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the user.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioPanner : public AudioNode {
private:
    /** The channel size of the input node */
    Uint8 _field;
    
    /** The intermediate read buffer */
    float* _buffer;
    /** The capacity of the intermediate buffer */
    Uint32 _capacity;
    
    /** The audio input node */
    std::shared_ptr<AudioNode> _input;
    /** The panning matrix */
    std::atomic<float>* _mapper;

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degenerate audio panner
     *
     * The node has no channels, so read options will do nothing. The node must
     * be initialized to be used.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
     * the heap, use one of the static constructors instead.
     */
    AudioPanner();
    
    /**
     * Deletes the audio panner, disposing of all resources
     */
    ~AudioPanner() { dispose(); }
    
    /**
     * Initializes the node with default stereo settings
     *
     * The number of input channels (the field) and the number of output
     * channels is two, for stereo output.  The sample rate is the modern
     * standard of 48000 HZ.
     *
     * This initializer will create a default stereo panner.  The initial
     * panning matrix will map left to left and right to right.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
    /**
     * Initializes the node with the given number of channels and sample rate
     *
     * The number of input channels (the field) and the number of output
     * channels will be the same.  The initial panning matrix will map each
     * channel to itself.  This is a generalization of a default stereo panner.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
    /**
     * Initializes the node with the given number of input/output channels.
     *
     * The number of input channels is given by `field`, while `channels` is
     * the number of output channels. The initial panning matrix will map each
     * channel to itself, and drop those input channels that do not have a
     * corresponding output channel.
     *
     * @param channels  The number of output channels
     * @param field     The number of input channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return true if initialization was successful
     */
    bool init(Uint8 channels, Uint8 field, Uint32 rate);
    
    /**
     * Disposes any resources allocated for this panner
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated panner with default stereo settings
     *
     * The number of input channels (the field) and the number of output
     * channels is two, for stereo output.  The sample rate is the modern
     * standard of 48000 HZ.
     *
     * This initializer will create a default stereo panner.  The initial
     * panning matrix will map left to left and right to right.
     *
     * @return a newly allocated panner with default stereo settings
     */
    static std::shared_ptr<AudioPanner> alloc() {
        std::shared_ptr<AudioPanner> result = std::make_shared<AudioPanner>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated panner with the given number of channels and sample rate
     *
     * The number of input channels (the field) and the number of output
     * channels will be the same.  The initial panning matrix will map each
     * channel to itself.  This is a generalization of a default stereo panner.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return a newly allocated panner with the given number of channels and sample rate
     */
    static std::shared_ptr<AudioPanner> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioPanner> result = std::make_shared<AudioPanner>();
        return (result->init(channels,rate) ? result : nullptr);
    }

    /**
     * Returns a newly allocated panner with the given number of input/output channels.
     *
     * The number of input channels is given by `field`, while `channels` is
     * the number of output channels. The initial panning matrix will map each
     * channel to itself, and drop those input channels that do not have a
     * corresponding output channel.
     *
     * @param channels  The number of output channels
     * @param field     The number of input channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return a newly allocated panner with the given number of input/output channels.
     */
    static std::shared_ptr<AudioPanner> alloc(Uint8 channels, Uint8 field, Uint32 rate) {
        std::shared_ptr<AudioPanner> result = std::make_shared<AudioPanner>();
        return (result->init(channels,field,rate) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Audio Graph
    /**
     * Attaches an audio node to this panner.
     *
     * This method will fail if the channels of the audio node do not agree
     * with the field size of this panner
     *
     * @param node  The audio node to pan
     *
     * @return true if the attachment was successful
     */
    bool attach(const std::shared_ptr<AudioNode>& node);
    
    /**
     * Detaches an audio node from this panner.
     *
     * If the method succeeds, it returns the audio node that was removed.
     *
     * @return  The audio node to detach (or null if failed)
     */
    std::shared_ptr<AudioNode> detach();
    
    /**
     * Returns the input node of this panner.
     *
     * @return the input node of this panner.
     */
    std::shared_ptr<AudioNode> getInput() const { return _input; }
    
    /**
     * Returns the input field size of this panner.
     *
     * @return the input field size of this panner.
     */
    Uint32 getField() const { return _field; }
    
    /**
     * Returns the matrix pan value for input field and output channel.
     *
     * The pan value is the percentage (gain) of the input channel (field)
     * that is sent to the given output channel.  Technically, this value
     * can be more than 1, but it cannot be negative.
     *
     * @return the matrix pan value for input field and output channel.
     */
    float getPan(Uint32 field, Uint32 channel) const;

    /**
     * Awra the matrix pan value for input field and output channel.
     *
     * The pan value is the percentage (gain) of the input channel (field)
     * that is sent to the given output channel.  Technically, this value
     * can be more than 1, but it cannot be negative.
     *
     * @param field     The input channel
     * @param channel   The output channel
     * @param value     The percentage gain
     */
    void setPan(Uint32 field, Uint32 channel, float value);

#pragma mark -
#pragma mark Playback Control
    /**
     * Returns true if this audio node has no more data.
     *
     * An audio node is typically completed if it return 0 (no frames read) on
     * subsequent calls to {@link read()}.  However, for infinite-running
     * audio threads, it is possible for this method to return true even when
     * data can still be read; in that case the node is notifying that it
     * should be shut down.
     *
     * @return true if this audio node has no more data.
     */
    virtual bool completed() override;
    
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
    virtual Uint32 read(float* buffer, Uint32 frames) override;
    
#pragma mark -
#pragma mark Optional Methods
    /**
     * Marks the current read position in the audio steam.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns false if there is no input node or if this method is unsupported
     * in that node
     *
     * This method is typically used by {@link reset()} to determine where to
     * restore the read position. For some nodes (like {@link AudioInput}),
     * this method may start recording data to a buffer, which will continue
     * until {@link reset()} is called.
     *
     * It is possible for {@link reset()} to be supported even if this method
     * is not.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns false if there is no input node or if this method is unsupported
     * in that node
     *
     * If the method {@link mark()} started recording to a buffer (such as
     * with {@link AudioInput}), this method will stop recording and release
     * the buffer.  When the mark is cleared, {@link reset()} may or may not
     * work depending upon the specific node.
     *
     * @return true if the read position was marked.
     */
    virtual bool unmark() override;
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns false if there is no input node or if this method is unsupported
     * in that node
     *
     * When no {@link mark()} is set, the result of this method is node
     * dependent.  Some nodes (such as {@link AudioPlayer}) will reset to the
     * beginning of the stream, while others (like {@link AudioInput}) only
     * support a rest when a mark is set. Pay attention to the return value of
     * this method to see if the call is successful.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;
    
    /**
     * Advances the stream by the given number of frames.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
     *
     * This method only advances the read position, it does not actually
     * read data into a buffer. This method is generally not supported
     * for nodes with real-time input like {@link AudioInput}.
     *
     * @param frames    The number of frames to advace
     *
     * @return the actual number of frames advanced; -1 if not supported
     */
    virtual Sint64 advance(Uint32 frames) override;
    
    /**
     * Returns the current frame position of this audio node
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
     *
     * In some nodes like {@link AudioInput}, this method is only supported
     * if {@link mark()} is set.  In that case, the position will be the
     * number of frames since the mark. Other nodes like {@link AudioPlayer}
     * measure from the start of the stream.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
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
    virtual Sint64 setPosition(Uint32 position) override;
    
    /**
     * Returns the elapsed time in seconds.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
     *
     * In some nodes like {@link AudioInput}, this method is only supported
     * if {@link mark()} is set.  In that case, the times will be the
     * number of seconds since the mark. Other nodes like {@link AudioPlayer}
     * measure from the start of the stream.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
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
    virtual double setElapsed(double time) override;
    
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
    virtual double getRemaining() const override;
    
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
    virtual double setRemaining(double time) override;
};
    }
}
#endif /* __CU_AUDIO_SWITCHER_H__ */
