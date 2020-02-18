//
//  CUAudioOutput.h
//  Cornell University Game Library (CUGL)
//
//  This module provides interface to an audio output device. As such, it is
//  often the final node in an audio stream DAG. It is analogous to AVAudioEngine
//  in Apple's AVFoundation API. The main differences is that it does not have
//  a dedicated mixer node.  Instead, you attach the single terminal node of the
//  audio graph.  In addition, it is possible to have a distinct audio graph for
//  each output device.
//
//  The audio graph and its nodes will always be accessed by two threads: the
//  main thread and the audio thread.  The audio graph is designed to safely
//  coordinate between these two threads.  However, it is minimizes locking
//  and instead relies on a fail-fast model.  If part of the audio graph is
//  not in a state to be used by the audio thread, it will skip over that part
//  of the graph until the next render frame.  Hence some changes should only
//  be made if the graph is paused.  When there is some question about the
//  thread safety, the methods are clearly marked.
//
//  It is NEVER safe to access the audio graph outside of the main thread. The
//  coordination algorithms only assume coordination between two threads.
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
//  Version: 8/20/18
//
#ifndef __CU_AUDIO_OUTPUT_H__
#define __CU_AUDIO_OUTPUT_H__
#include <SDL/SDL.h>
#include "CUAudioNode.h"
#include <cugl/math/dsp/CUBiquadIIR.h>
#include <unordered_set>
#include <string>
#include <memory>
#include <vector>

namespace cugl {
    
    /** Forward reference to the audio manager */
    class AudioManager;
    
    namespace audio {
/**
 * This class provides a graph node interface for an audio playback device.
 *
 * This audio node provides a modern, graph-based approach to sound design.
 * Unlike other audio engines, this output node does not have a dedicated mixer.
 * Instead, you attach the single terminal node of the audio graph to this
 * output device node. The output channels of that node must match with those
 * of this output device.
 *
 * This method has no public initializers or non-degenerate constructors. That
 * is because all output nodes should be created by the factory methods in
 * {@link AudioManager}.  That way, the AudioManager can properly handle device
 * conflicts as they may arise.
 *
 * Audio devices in SDL are identified by name. If you have two devices with
 * the same name, SDL will add a distinguishing index to the name. You can
 * see the list of all available devices with the {@link AudioManager#devices()}
 * method.  In addition, the empty string may be used to refer to the default
 * devices.  Instances of AudioOutput attached to a default device will roll
 * over (if possible) whenever the default device changes.
 *
 * When deciding on the number of channels, SDL supports 1 (mono), 2 (stereo),
 * 4 (quadrophonic), 6 (5.1 surround), or 8 (7.1 surround) channels for
 * playback. Stereo and quadraphonic are arranged left-right, front-back.
 * For 5.1 surround, they are arranged in the following order.
 *
 * 1. front-left
 * 2. front-right
 * 3. center
 * 4. subwoofer/low-frequency
 * 5. rear left
 * 6. rear right
 *
 * For 7.1 surround, they are arranged in the same order with the following
 * additional channels.
 *
 * 7. side left
 * 8. side right
 *
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the 
 * user.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioOutput : public AudioNode {
private:
    /** The device name for this output node.  Empty string for default */
    std::string _dvname;
    
    /** The processing time required for this device */
    std::atomic<Uint64> _overhd;

    /** The audio device in use */
    SDL_AudioDeviceID _device;
    /** The audio specification */
    SDL_AudioSpec  _audiospec;
    
    /** Whether or not the device is currently active */
    std::atomic<bool> _active;

    /** The terminal node of the audio graph. This pulls data from the sources */
    std::shared_ptr<AudioNode> _input;
    
    /** Conversion resampler (if needed) */
    SDL_AudioStream* _resampler;
    /** The intermediate sampling buffer */
    float* _cvtbuffer;
    /** The conversion ratio */
    float  _cvtratio;
    /** The native bitrate for this output device */
    size_t _bitrate;

#pragma mark -
#pragma mark AudioManager Methods
    /**
     * Initializes the default output device with 2 channels at 48000 Hz.
     *
     * This device node will have a buffer (e.g. the number of samples that
     * the node plays at a time) of {@link AudioManager#getReadSize()} samples.
     * By default, this is 512 samples. At 48000 Hz, this means that the node
     * has a potential lag of 11 ms, which is a single animation frame at 60
     * fps. Since audio is double buffered, this means a play request may be
     * delayed by two frames.
     *
     * An output device is initialized with both active and paused as false.
     * That means it will begin playback as soon as {@link AudioManager} sets
     * this device to active.
     *
     * This node is always logically attached to the default output device.
     * That means it will switch devices whenever the default output changes.
     * This method may fail if the default device is in use.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;

    /**
     * Initializes the default output device with the given channels and sample rate.
     *
     * This device node will have a buffer (e.g. the number of samples that
     * the node plays at a time) of {@link AudioManager#getReadSize()} samples.
     * By default, this is 512 samples. At 48000 Hz, this means that the node
     * has a potential lag of 11 ms, which is a single animation frame at 60
     * fps. Since audio is double buffered, this means a play request may be
     * delayed by two frames.
     *
     * An output device is initialized with both active and paused as false.
     * That means it will begin playback as soon as {@link AudioManager} sets
     * this device to active.
     *
     * This node is always logically attached to the default output device.
     * That means it will switch devices whenever the default output changes.
     * This method may fail if the default output device is in use.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
    /**
     * Initializes the default output device with the given channels and sample rate.
     *
     * The buffer value is the number of samples collected at each poll. Smaller
     * buffers clearly tax the CPU, as the device is collecting data at a higher
     * rate. Furthermore, if the value is too small, the time to collect the
     * data may be larger than the time to play it. This will result in pops
     * and crackles in the audio.
     *
     * However, larger values increase the audio lag.  For example, a buffer
     * of 1024 for a sample rate of 48000 Hz corresponds to 21 milliseconds.
     * This is the delay between when sound is gathered and it is played.
     * But this gathering process is also buffered, so this means that any
     * sound effect generated at the same time that the audio device executes
     * must wait 46 milliseconds before it can play. A value of 512 is the
     * perferred value for 60 fps framerate.  With that said, many devices
     * cannot handle this rate and need a buffer size of 1024 instead.
     *
     * An output device is initialized with both active and paused as false.
     * That means it will begin playback as soon as {@link AudioManager} sets
     * this device to active.
     *
     * This node is always logically attached to the default output device.
     * That means it will switch devices whenever the default output changes.
     * This method may fail if the default output device is in use.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param buffer    The size of the buffer to play audio
     *
     * @return true if initialization was successful
     */
    bool init(Uint8 channels, Uint32 rate, Uint32 buffer);
    
    /**
     * Initializes the given output device with 2 channels at 48000 Hz.
     *
     * This device node will have a buffer (e.g. the number of samples that
     * the node plays at a time) of {@link AudioManager#getReadSize()} samples.
     * By default, this is 512 samples. At 48000 Hz, this means that the node
     * has a potential lag of 11 ms, which is a single animation frame at 60
     * fps. Since audio is double buffered, this means a play request may be
     * delayed by two frames.
     *
     * An output device is initialized with both active and paused as false.
     * That means it will begin playback as soon as {@link AudioManager} sets
     * this device to active.
     *
     * This method may fail if the given device is in use.
     *
     * @param device    The name of the output device
     *
     * @return true if initialization was successful
     */
    bool init(const std::string& device);
    
    /**
     * Initializes the output device with the given channels and sample rate.
     *
     * The buffer value is the number of samples collected at each poll. Smaller
     * buffers clearly tax the CPU, as the node is collecting data at a higher
     * rate. Furthermore, if the value is too small, the time to collect the
     * data may be larger than the time to play it. This will result in pops
     * and crackles in the audio.
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
     * An output device is initialized with both active and paused as false.
     * That means it will begin playback as soon as {@link AudioManager} sets
     * this device to active.
     *
     * This method may fail if the given device is in use.
     *
     * @param device    The name of the output device
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param buffer    The size of the buffer to play audio
     *
     * @return true if initialization was successful
     */
    bool init(const std::string& device, Uint8 channels, Uint32 rate, Uint32 buffer);
    
    /**
     * Disposes any resources allocated for this output device node.
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
    /**
     * Sets the active status of this node.
     *
     * An active device will have its {@link read()} method called at regular
     * intervals.  This setting is to allow {@link AudioManager to pause and
     * resume an output device without override the user pause settings.
     *
     * @param active    Whether to set this node to active
     */
    void setActive(bool active);

    /** Allow AudioManager to access intializers */
    friend class cugl::AudioManager;
    
public:
    /**
     * Creates a degenerate audio output node.
     *
     * The node has not been initialized, so it is not active.  The node
     * must be initialized to be used.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on
     * the heap, use the factory in {@link AudioManager}.
     */
    AudioOutput();
    
    /**
     * Deletes the audio output node, disposing of all resources
     */
    ~AudioOutput();

#pragma mark -
#pragma mark Data Access
    /**
     * Returns the audio device identifier associated with this audio output.
     *
     * @return the audio device identifier associated with this audio output.
     */
    const SDL_AudioDeviceID getAUID() const  { return _device; }
    
    /**
     * Returns the device associated with this output node.
     *
     * @return the device associated with this output node.
     */
    const std::string& getDevice() const;

    /**
     * Returns true if this output node is associated with the default device
     *
     * A graph on the default device will switch devices whenever the default
     * device changes.
     *
     * @return true if this audio graph is associated with the default device
     */
    bool isDefault() const { return _dvname == ""; }
    
    /**
     * Returns the buffer size of this output node.
     *
     * The buffer value is the number of samples collected at each poll. Smaller
     * buffers clearly tax the CPU, as the node is collecting data at a higher
     * rate. Furthermore, if the value is too small, the time to collect the
     * data may be larger than the time to play it. This will result in pops
     * and crackles in the audio.
     *
     * However, larger values increase the audio lag.  For example, a buffer
     * of 1024 for a sample rate of 48000 Hz corresponds to 21 milliseconds.
     * This is the delay between when sound is gathered and it is played.
     * But this gathering process is also buffered, so this means that any
     * sound effect generated at the same time that the output node executes
     * must wait 42 milliseconds before it can play.  A value of 512 is the
     * perferred value for 60 fps framerate. With that said, many devices
     * cannot handle this rate and need a buffer size of 1024 instead.
     *
     * @return the buffer size of this output node.
     */
    Uint16 getCapacity()  const { return _audiospec.samples;  }

    /**
     * Returns the native bit rate of this device.
     *
     * The bit rate is the number of bits per sample.  By default, the audio
     * graph assumes 32 bits (for float-sized samples).  However, some devices
     * (paricularly Android devices) have a smaller bit rate.  This value is used
     * by the internal resampler to convert to the proper rate on output.
     *
     * @return the native bit rate of this device.
     */
     size_t getBitRate()  const { return _bitrate;  }
    
#pragma mark -
#pragma mark Audio Graph
    /**
     * Attaches an audio graph to this output node.
     *
     * This method will fail if the channels of the audio graph do not agree
     * with the number of the channels of this node.
     *
     * @param node  The terminal node of the audio graph
     *
     * @return true if the attachment was successful
     */
    bool attach(const std::shared_ptr<AudioNode>& node);
    
    /**
     * Detaches an audio graph from this output node.
     *
     * If the method succeeds, it returns the terminal node of the audio graph.
     *
     * @return  the terminal node of the audio graph (or null if failed)
     */
    std::shared_ptr<AudioNode> detach();
    
    /**
     * Returns the terminal node of the audio graph
     *
     * @return the terminal node of the audio graph
     */
    std::shared_ptr<AudioNode> getInput() { return _input; }
    
#pragma mark -
#pragma mark Playback Control
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
     * If the node is not paused, this method has no effect.  It is possible to
     * resume an node that is not yet activated by {@link AudioManager}.  When
     * that happens, data will be read as soon as the node becomes active.
     *
     * @return true if the node was successfully resumed
     */
    virtual bool resume() override;
    
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
   
    /**
     * Reboots the audio output node without interrupting any active polling.
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * This method will close and reopen the associated audio device.  It
     * is primarily used when an node on the default device needs to migrate
     * between devices.
     */
    void reboot();
    
    /**
     * Returns the number of microseconds needed to render the last audio frame.
     *
     * This method is primarily for debugging.
     *
     * @return the number of microseconds needed to render the last audio frame.
     */
    Uint64 getOverhead() const;
    
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
  
    // TODO: May need to add a push option.
};
    }
}

#endif /* __CU_AUDIO_OUTPUT_H__ */
