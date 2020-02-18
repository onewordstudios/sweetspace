//
//  CUAudioSpinnner.h
//  Cornell University Game Library (CUGL)
//
//  This module is a spatial audio audio panner.  It is used to rotate or
//  or "spin" a sound input about a sound field.  Doing this requires
//  specification of the audio channels angles about a circle.  There are
//  several default sound set-ups, but the user can specify any configuration
//  that they want.  This module is also useful for directing sound to a
//  subwoofer.
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
#ifndef __CU_AUDIO_SPINNER_H__
#define __CU_AUDIO_SPINNER_H__
#include "CUAudioNode.h"
#include <cugl/math/dsp/cu_dsp.h>
#include <atomic>

namespace cugl {
    namespace audio {

/**
 * A class representing a spatial audio panner.
 *
 * This audio node takes another audio node as input. That node must agree with
 * the sample rate of this node, but need not have the same number of channels.
 * In fact, the input node must instead have {@link getFieldPlan()} number of
 * channels.  It then maps the data from this input channels to the output
 * channels.
 *
 * This mapping happens according to an angle, which specifies the direction
 * of the sound source (not the listener).  An angle of 0 means the sound is
 * coming from straight ahead, and the sound should map to the natural output
 * channels.  An angle of PI/2 means the sound is centered directly to the left,
 * and the output channels should be adjusted accordingly.
 *
 * To properly pan the audio, this node needs to know the location of all of
 * the output channels in the room, specified as an angle (with 0 being straight
 * ahead -- the traditional center channel).  There are several built-in options
 * for specifying the channels.  These are taken from
 *
 *     http://www.wendycarlos.com/surround/
 *
 * However, the user can specify the channel locations manually using the
 * {@link setChannelOrientation()} method.
 *
 * There are separate plans for both to the audio input and the ouput.  This
 * is how the node knows how to handle rotation of non-monaural sounds.
 *
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the user.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioSpinner : public AudioNode {
public:
    /**
     * This enum represents the possible surround-sound layouts.
     *
     * Most of these layouts are taken from personal experience or from
     *
     *     http://www.wendycarlos.com/surround/
     *
     * They should not be taken as exhaustive.  This list may be modified
     * at any time.  For that reason, you should never refer to a layout plan
     * by its raw number.
     */
    enum Plan : int {
        /**
         * Single channel monaural sound.
         *
         * The only channel is straight ahead at 0 degrees.
         */
        MONAURAL     = 0,
        /**
         * Stereo sound in front of the listener.
         *
         * The left and right channels are separated by 60 degrees (so at +30
         * and -30 degrees in front of the listener).  This is typical for a
         * stereo set-up associated with a display (e.g. a TV).
         */
        FRONT_STEREO = 1,
        /**
         * Stereo sound to the sides of the listener.
         *
         * The left and right channels are separated by 180 degrees (so at
         * +90 and -90 degrees).  This is the classic headphones set-up.
         */
        SIDE_STEREO  = 2,
        /**
         * Three channel sound in front of the listener.
         *
         * The left and right channels are separated by 90 degrees (so at +45
         * and -45 degrees in front of the listener), while the center channel
         * is at 0 degrees.  This is typical for a three channels set-up
         * associated with a display (e.g. a TV).
         */
        FRONT_CENTER = 3,
        /**
         * Three channel sound to the sides of the listener.
         *
         * The left and right channels are separated by 180 degrees (so at
         * +90 and -90 degrees), while the center channel is at 0 degrees.
         * This is a less typical display setup.
         */
        SIDE_CENTER  = 4,
        /**
         * Four channel sound favoring the font and back.
         *
         * The front left and right channels are separated by 60 degrees, just
         * as with FRONT_STEREO.  The back left an right channels are also
         * separated by 60 degrees, centered at 180 degrees behind the listener.
         * This is a quad set-up that minimizes "black holes" in the sound
         * field at the expense of sound on the sides.
         */
        FRONT_QUADS  = 5,
        /**
         * Four channel sound equi-spaced.
         *
         * All four channes are separated by 90 degrees about the circle,
         * with 0 degrees equidistant between the left and right channel.
         * This creates a uniform sound field, but can create "black holes"
         * between output channels.
         */
        CORNER_QUADS = 6,
        /**
         * Five channel sound (with subwoofer) with surrounds at the back.
         *
         * The first three channels are arranged as in FRONT_CENTER.  The
         * rears are separated by 60 degrees, centered at 180 degrees behind
         * the listener.  In this case, the surround gives rear information
         * only.
         */
        BACK_5_1     = 7,
        /**
         * Five channel sound (with subwoofer) with surrounds at the sides.
         *
         * The first three channels are arranged as in FRONT_CENTER.  The
         * rears are separated by 180 degrees, with left rear at 90 degrees
         * and right rear at 180 degrees. In this case, the surround gives
         * side sound information.
         */
        SIDE_5_1     = 8,
        /**
         * Five channel sound (with subwoofer) with surrounds in the corner.
         *
         * The first three channels are arranged as in FRONT_CENTER.  The
         * rears are separated by 90 degrees, centered at 180 degrees behind
         * the listener.  This is an attempted compromise between back and
         * side 5.1 set-ups.
         */
        CORNER_5_1   = 9,
        /**
         * Seven channel sound (with subwoofer) with surrounds at the back.
         *
         * The first three channels are arranged as in FRONT_CENTER, and the
         * last two channels are arranged at the sides separated by 180 degrees.
         * The two intermediate surround channels are separated by 60 degrees,
         * centered at 180 degrees behind the listener. This is an attempt to
         * give a tight surround experience.
         */
        BACK_7_1     = 10,
        /**
         * Seven channel sound (with subwoofer) with surrounds at the back.
         *
         * The first three channels are arranged as in FRONT_CENTER, and the
         * last two channels are arranged at the sides separated by 180 degrees.
         * The two intermediate surround channels are separated by 90 degrees,
         * centered at 180 degrees behind the listener. This is an attempt to
         * give a more diffuse surround experience.
         */
        CORNER_7_1   = 11,
        /**
         * A custom layout plan.
         *s
         * This type is used whenever the user expects to set their own
         * layout orientations.
         */
        CUSTOM       = 12,
    };
    
private:
    /** The channel size of the input node */
    Uint8 _field;
    /** The layout plan for the audio input */
    std::atomic<Plan>   _inplan;
    /** The layout plan for the audio output */
    std::atomic<Plan>   _outplan;
    /** The orientation angles for the audio input */
    std::atomic<float>* _inlines;
    /** The orientation angles for the audio output */
    std::atomic<float>* _outlines;
    /** The angle of the sound source */
    std::atomic<float>  _angle;
    
    /** The crossover frequency of the subwoofer */
    std::atomic<float> _crossover;
    /** To mark if the filter needs to be reset */
    std::atomic<bool>  _dirtycross;
    /** A filter for subwoofer processing */
    cugl::dsp::BiquadIIR* _filter;
    
    /** The intermediate read buffer */
    float* _buffer;
    /** The capacity of the intermediate buffer */
    Uint32 _capacity;
    
    /** The audio input node */
    std::shared_ptr<AudioNode> _input;

    /**
     * Returns the default plan for the given number of channels.
     *
     * This is used for initializing this node.
     *
     * @param channels  The number of channels in the sound field
     *
     * @return the default plan for the given number of channels.
     */
    Plan getDefaultPlan(Uint8 channels);

    /**
     * Returns true if the plan is valid for the given number of channels.
     *
     * This is used to check user settings.
     *
     * @param plan      The layout plan
     * @param channels  The number of channels in the sound field
     *
     * @return true if the plan is valid for the given number of channels.
     */
    bool isValidPlan(Plan plan, Uint8 channels);

    /**
     * Initializes the given array with the specified plan.
     *
     * This method assumes lines is an arra of the right length.  It initializes
     * lines with the right angles for the given plan.
     *
     * @param plan  The layout plan
     * @param lines The array with the channel orientations
     */
    void initPlan(Plan plan, std::atomic<float>* lines);

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degenerate audio spinner.
     *
     * The node has no channels, so read options will do nothing. The node must
     * be initialized to be used.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
     * the heap, use one of the static constructors instead.
     */
    AudioSpinner();
    
    /**
     * Deletes the audio spinner, disposing of all resources
     */
    ~AudioSpinner() { dispose(); }
    
    /**
     * Initializes the node with default stereo settings
     *
     * The number of channels is two, for stereo output.  The sample rate is
     * the modern standard of 48000 HZ.  The spinner will start with left and
     * right mapped to the appropriate locations.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
    /**
     * Initializes the node with the given number of channels and sample rate
     *
     * The field will be the same as the number of channels.  By default, each
     * input channel will map to itself as an output channel (until the angle
     * changes).  Both the input and output will share the same (default)
     * layout plan.
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
     * the number of output channels. The input and output will each have the
     * default layout plan for the given size.
     *
     * @param channels  The number of output channels
     * @param field     The number of input channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return true if initialization was successful
     */
    bool init(Uint8 channels, Uint8 field, Uint32 rate);
    
    /**
     * Disposes any resources allocated for this spinner.
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated spinner with the default stereo settings
     *
     * The number of channels is two, for stereo output.  The sample rate is
     * the modern standard of 48000 HZ.  The spinner will start with left and
     * right mapped to the appropriate locations.
     *
     * @return a newly allocated spinner with the default stereo settings
     */
    static std::shared_ptr<AudioSpinner> alloc() {
        std::shared_ptr<AudioSpinner> result = std::make_shared<AudioSpinner>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated spinner with the given number of channels and sample rate
     *
     * The field will be the same as the number of channels.  By default, each
     * input channel will map to itself as an output channel (until the angle
     * changes).  Both the input and output will share the same (default)
     * layout plan.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return a newly allocated spinner with the given number of channels and sample rate
     */
    static std::shared_ptr<AudioSpinner> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioSpinner> result = std::make_shared<AudioSpinner>();
        return (result->init(channels,rate) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated spinner with the given number of input/output channels.
     *
     * The number of input channels is given by `field`, while `channels` is
     * the number of output channels. The input and output will each have the
     * default layout plan for the given size.
     *
     * @param channels  The number of output channels
     * @param field     The number of input channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return a newly allocated spinner with the given number of input/output channels.
     */
    static std::shared_ptr<AudioSpinner> alloc(Uint8 channels, Uint8 field, Uint32 rate) {
        std::shared_ptr<AudioSpinner> result = std::make_shared<AudioSpinner>();
        return (result->init(channels,field,rate) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Audio Graph
    /**
     * Attaches an audio node to this spinner.
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
     * Detaches an audio node from this spinner.
     *
     * If the method succeeds, it returns the audio node that was removed.
     *
     * @return  The audio node to detach (or null if failed)
     */
    std::shared_ptr<AudioNode> detach();
    
    /**
     * Returns the input node of this spinner.
     *
     * @return the input node of this spinner.
     */
    std::shared_ptr<AudioNode> getInput() { return _input; }

#pragma mark -
#pragma mark Sound Field
    /**
     * Returns the layout plan for the audio input.
     *
     * This is the intended layout of the sound source, unrotated.
     *
     * @return the layout plan for the audio input.
     */
    Plan  getFieldPlan() const;
    
    /**
     * Sets the layout plan for the audio input.
     *
     * This is the intended layout of the sound source, unrotated.  If the
     * value is CUSTOM, the current orientations will not be affected. Instead,
     * the user should set the orientations manually.
     *
     * @param plan  The layout plan for the audio input.
     */
    void  setFieldPlan(Plan plan);
    
    /**
     * Returns the layout plan for the audio output.
     *
     * This is the layout of the output channels.
     *
     * @return the layout plan for the audio output.
     */
    Plan  getChannelPlan() const;

    /**
     * Sets the layout plan for the audio output.
     *
     * This is the layout of the output channels.  If the value is CUSTOM, the
     * current orientations will not be affected. Instead, the user should set
     * the orientations manually.
     *
     * @param plan  The layout plan for the audio output.
     */
    void  setChannelPlan(Plan plan);
    
    /**
     * Returns the orientation of an input channel.
     *
     * This is the intended layout of the sound source, unrotated.
     *
     * @param channel   The input channel
     *
     * @return the orientation of an input channel.
     */
    float getFieldOrientation(Uint32 channel) const;

    /**
     * Sets the orientation of an input channel.
     *
     * This is the intended layout of the sound source, unrotated.
     *
     * @param channel   The input channel
     * @param angle     The angle from the listener's forward position.
     *
     * @return the orientation of an input channel.
     */
    void  setFieldOrientation(Uint32 channel,float angle);
    
    /**
     * Returns the orientation of an output channel.
     *
     * @param channel   The output channel
     *
     * @return the orientation of an output channel.
     */
    float getChannelOrientation(Uint32 channel) const;
    
    /**
     * Sets the orientation of an output channel.
     *
     * @param channel   The output channel
     * @param angle     The angle from the listener's forward position.
     *
     * @return the orientation of an output channel.
     */
    void  setChannelOrientation(Uint32 channel,float angle);
    
    /**
     * Returns the crossover frequency (in Hz) for the subwoofer.
     *
     * Sounds below this frequency will be sent to the subwoofer, regardless
     * of the input channel.
     *
     * @return the crossover frequency (in Hz) for the subwoofer.
     */
    float getSubwoofer() const;

    /**
     * Sets the crossover frequency (in Hz) for the subwoofer.
     *
     * Sounds below this frequency will be sent to the subwoofer, regardless
     * of the input channel.
     *
     * @param frequency The crossover frequency (in Hz) for the subwoofer.
     */
    void  setSubwoofer(float frequency);
    
#pragma mark -
#pragma mark Playback Control
    /**
     * Returns the angle of the sound source.
     *
     * If this angle is not 0, the input orientation will be rotated by
     * the given angle to align it with the output orientation.  Input
     * channels that are between two output channels will be interpolated.
     *
     * @return the angle of the sound source.
     */
    float getAngle() const;
    
    /**
     * Sets the angle of the sound source.
     *
     * If this angle is not 0, the input orientation will be rotated by
     * the given angle to align it with the output orientation.  Input
     * channels that are between two output channels will be interpolated.
     *
     * @param angle The angle of the sound source.
     */
    void  setAngle(float angle);

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


#endif /* __CU_AUDIO_SPINNER_H__ */
