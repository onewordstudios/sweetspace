//
//  CUAudioInput.h
//  CUGL
//
//  Created by Walker White on 11/24/18.
//  Copyright © 2018 Game Design Initiative at Cornell. All rights reserved.
//
// TODO: Make record vs playback clearer (active only for recording)
#ifndef __CU_AUDIO_INPUT_H__
#define __CU_AUDIO_INPUT_H__
#include <SDL/SDL.h>
#include "CUAudioNode.h"
#include <unordered_set>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <mutex>

namespace cugl {
    
    /** Forward reference to the audio manager */
    class AudioManager;
    /** Forward reference to an audio sample */
    class AudioSample;
    
    namespace audio {
/**
 * This class provides a graph node interface for an audio recording device.
 *
 * This audio node provides a modern, graph-based approach to sound design.
 * This input can be plugged into the audio graph and piped to the output
 * device with a small (but significant) amount of latency.  This node is
 * intended to be the root of an audio DAG, much like {@link AudioPlayer}.
 *
 * One of the important issues to understand about this class is that there
 * are actually two different latency values: the recording latency and the
 * playback latency.  The recording latency is time from when the data is
 * captured at the hardware device and is recorded at this node.  It is
 * entirely determined by the buffer size of this device, much like the
 * buffer determines the playback latency for {@link AudioOutput}.  However,
 * unlike AudioOutput, there is no guarantee that recording happens at uniform
 * time intervals. For example, on MacOS at 48000 Hz, a buffer size of 512 will
 * record twice (in immediate sucession) every 21 ms instead of once every
 * 11 ms (a buffer size of 1024 appers to record once every 21 ms as expected).
 *
 * The playback latency is the time from when data is passed to {@link record()}
 * and made available to {@link read()}; it does not include the latency of
 * any output device.  It is determined by the additional delay value, which is
 * the number of frames that must be recorded before any can be read.  So a
 * delay of 0 means that data is instantly available, while a delay of the
 * buffer size means that no data can be read until more than a single buffer
 * has been recorded (which means that at least two buffers worth of data must
 * have been recorded).  Because output and input devices run in different
 * threads and have no coordination at all, a delay of at least one buffer is
 * recommended for real-time playback.
 *
 * This method has no public initializers or non-degenerate constructors. That
 * is because all input nodes should be created by the factory methods in
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
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the
 * user.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioInput : public AudioNode {
private:
    /** The device name for this output node.  Empty string for default */
    std::string _dvname;
    
    /** The audio device in use */
    SDL_AudioDeviceID _device;
    /** The audio specification */
    SDL_AudioSpec  _audiospec;

    /** Whether the device is actively recording */
    std::atomic<bool> _record;
    
    /** Whether or not the device is currently active */
    std::atomic<bool> _active;

    /** To allow manual stopping of this node */
    std::atomic<Sint64> _timeout;

    /** Mutex to protect more sophisticated synchronization */
    mutable std::mutex _buffmtex;

    // We were forced to use mutexes for this class, so there are no atomics
    /** The recording buffer */
    float* _buffer;
    /** The absolute buffer capacity */
    Uint32 _capacity;
    /** The number of elements currently in the buffer */
    Uint32 _buffsize;
    /** The head of the circular buffer queue */
    Uint32 _buffhead;
    /** The tail of the circular buffer queue */
    Uint32 _bufftail;
    
    /** The location of a data mark in playback */
    Sint64 _playmark;
    /** The current read head position of playback */
    Sint64 _playpost;
    /** The data buffer for recording when mark is set */
    std::deque<float> _playback;

#pragma mark -
#pragma mark AudioManager Methods
    /**
     * Initializes the default input device with 2 channels at 48000 Hz.
     *
     * This device node will have a buffer capacity of
     * {@link AudioManager#getWriteSize()} samples, and an equal delay.  By
     * default, this value is 1024 samples. This means that, at 48000 Hz, the
     * recording delay is 21ms and the playback delay is an (additional)
     * 21 ms.  So 42 ms passes between data is captured at the hardware device
     * and when it can be processed by the audio graph. While this may seem
     * like a lot of overhead, our experience (particularly on MacOS, iOS)
     * has shown this is necessary for smooth real-time processing.
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioManager}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This node is always logically attached to the default input device.
     * That means it will switch devices whenever the default input changes.
     * This method may fail if the default device is in use.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
    /**
     * Initializes the default input device with the given channels and sample rate.
     *
     * This device node will have a buffer capacity of
     * {@link AudioManager#getWriteSize()} samples, and an equal delay.  By
     * default, this value is 1024 samples. This means that, at 48000 Hz, the
     * recording delay is 21ms and the playback delay is an (additional)
     * 21 ms.  So 42 ms passes between data is captured at the hardware device
     * and when it can be processed by the audio graph. While this may seem
     * like a lot of overhead, our experience (particularly on MacOS, iOS)
     * has shown this is necessary for smooth real-time processing.
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioManager}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This node is always logically attached to the default input device.
     * That means it will switch devices whenever the default input changes.
     * This method may fail if the default input device is in use.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
    /**
     * Initializes the default input device with the given channels and sample rate.
     *
     * The buffer value is the number of samples recorded at each poll, while
     * the delay is the number of frames that must be recorded before a
     * single frame can be read.  These determine the recording latency and
     * playback latency, respectively.
     *
     * It is not necessary for the buffer value of an input device match the
     * buffer value of an output device.  Indeed, some many systems, an input
     * buffer size of less than 1024 samples is not supported, while output
     * devices can process much faster than that. What is important is ensuring
     * enough delay so that the audio graph does not outrun the input device.
     * Therefore, a delay of less than the buffer size is not recommended for
     * real-time audio processing.
     *
     * We have found that minimum buffer size of 1024 frames and an equal delay
     * of 1024 is the minimum value for most systems. That is because there is
     * no thread coordination at all between the {@link record()} method
     * (called by the input device) and the {@link read()} method (called by
     * the audio graph).
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioManager}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This node is always logically attached to the default input device.
     * That means it will switch devices whenever the default input changes.
     * This method may fail if the default input device is in use.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param buffer    The size of the buffer to record audio
     * @param delay     The frame delay between recording and reading
     *
     * @return true if initialization was successful
     */
    bool init(Uint8 channels, Uint32 rate, Uint32 buffer, Uint32 delay);
    
    /**
     * Initializes the given input device with 2 channels at 48000 Hz.
     *
     * This device node will have a buffer capacity of
     * {@link AudioManager#getWriteSize()} samples, and an equal delay.  By
     * default, this value is 1024 samples. This means that, at 48000 Hz, the
     * recording delay is 21ms and the playback delay is an (additional)
     * 21 ms.  So 42 ms passes between data is captured at the hardware device
     * and when it can be processed by the audio graph. While this may seem
     * like a lot of overhead, our experience (particularly on MacOS, iOS)
     * has shown this is necessary for smooth real-time processing.
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioManager}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This method may fail if the given device is in use.
     *
     * @param device    The name of the input device
     *
     * @return true if initialization was successful
     */
    bool init(const std::string& device);
    
    /**
     * Initializes the input device with the given channels and sample rate.
     *
     * The buffer value is the number of samples recorded at each poll, while
     * the delay is the number of frames that must be recorded before a
     * single frame can be read.  These determine the recording latency and
     * playback latency, respectively.
     *
     * It is not necessary for the buffer value of an input device match the
     * buffer value of an output device.  Indeed, some many systems, an input
     * buffer size of less than 1024 samples is not supported, while output
     * devices can process much faster than that. What is important is ensuring
     * enough delay so that the audio graph does not outrun the input device.
     * Therefore, a delay of less than the buffer size is not recommended for
     * real-time audio processing.
     *
     * We have found that minimum buffer size of 1024 frames and an equal delay
     * of 1024 is the minimum value for most systems. That is because there is
     * no thread coordination at all between the {@link record()} method
     * (called by the input device) and the {@link read()} method (called by
     * the audio graph).
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioManager}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This method may fail if the given device is in use.
     *
     * @param device    The name of the input device
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param buffer    The size of the buffer to play audio
     * @param delay     The frame delay between recording and reading
     *
     * @return true if initialization was successful
     */
    bool init(const std::string& device, Uint8 channels, Uint32 rate, Uint32 buffer, Uint32 delay);
    
    /**
     * Disposes any resources allocated for this input device node.
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
    /**
     * Sets the active status of this node.
     *
     * An active device will have its {@link record()} method called at regular
     * intervals.  This setting is to allow {@link AudioManager to release and
     * acquire an input device without override the user settings.
     *
     * @param active    Whether to set this node to active
     */
    void setActive(bool active);
    
    /** Allow AudioManager to access intializers */
    friend class cugl::AudioManager;
    
public:
    /** The default delay (in frames) for an input device */
    const static Uint32 DEFAULT_DELAY;

    /**
     * Creates a degenerate audio input node.
     *
     * The node has not been initialized, so it is not active.  The node
     * must be initialized to be used.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on
     * the heap, use the factory in {@link AudioManager}.
     */
    AudioInput();
    
    /**
     * Deletes the audio input node, disposing of all resources
     */
    ~AudioInput();
    
#pragma mark -
#pragma mark Data Access
    /**
     * Returns the audio device identifier associated with this audio input.
     *
     * @return the audio device identifier associated with this audio input.
     */
    const SDL_AudioDeviceID getAUID() const  { return _device; }
    
    /**
     * Returns the device associated with this input node.
     *
     * @return the device associated with this input node.
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
     * Returns the buffer size of this input node.
     *
     * The buffer value is the number of samples recorded at each poll. Smaller
     * buffers clearly tax the CPU, as the device is collecting data at a higher
     * rate. Furthermore, if the value is too small, the recording device may
     * not be able to keep up with the output device.  For example, we find
     * on MacOS that the input device at a sample rate of 48000 Hz can only
     * record data every 21 milliseconds.  Hence so it can only (at best) keep
     * up with a 1024 playback buffer.
     *
     * @return the buffer size of this output node.
     */
    Uint16 getCapacity()  const { return _audiospec.samples;  }
    
    /**
     * Returns the current playback delay (in frames) of this audio node
     *
     * The playback delay is the number of frames between when an audio
     * frame is recorded by {@link record()} to when it can be accessed by
     * {@link read()}.  Typically, this is the delay value set by the
     * initialzer (commonly 1024 frames).  However, if the read position
     * has been changed by {@link setPosition()} or {@link setElapsed()},
     * then this method will reflect the new delay.
     *
     * Because of the real-time nature of this node, this value is only
     * an approximation.
     *
     * @return the current playback delay (in frames) of this audio node
     */
    Uint32 getDelay() const;
    
#pragma mark -
#pragma mark Playback Control
    /**
     * Pauses this node, preventing any data from being played back.
     *
     * As with all other audio nodes, pausing effects the playback.  However, it
     * does not affect recording.  Recording will still happen in the background
     * and may be recovered if {@link mark()} is set. To stop recording (but
     * not playback) call {@link release()} instead.
     *
     * If the node is already paused, this method has no effect. Pausing will
     * not go into effect until the next render call in the audio thread.
     *
     * @return true if the node was successfully paused
     */
    virtual bool pause() override;
    
    /**
     * Resumes this previously paused node, allowing data to be played back.
     *
     * As with all other audio nodes, pausing effects the playback.  However,
     * does not affect recording.  When play is resumed, the playback will
     * either return with immediate playback or the recording buffer,
     * depending on whether {@link mark()} is set.
     *
     * If the node is not paused, this method has no effect.
     *
     * @return true if the node was successfully resumed
     */
    virtual bool resume() override;
    
    /**
     * Returns true if this node is currently recording audio.
     *
     * Recording is completely independent of playback.  An input node can
     * be recording, but have its played paused, and vice versa.
     *
     * @return true if this node is currently recording audio.
     */
    bool isRecording() const;
    
    /**
     * Stops this input node from recording.
     *
     * This method does not effect playback.  Unpaused playback will continue
     * until the delay has caught up.  After that point, it will only play
     * silence.
     *
     * If the node is not recording, this method has no effect.
     *
     * @return true if the node was successfully released
     */
    bool release();

    /**
     * Resumes recording for a previously released node.
     *
     * This method does not effect playback.  If playback is paused, then
     * recording will be buffered if {@link mark()} is set, or else it will
     * overwrite itself in the circular buffer.
     *
     * If the node is already recording, this method has no effect.
     *
     * @return true if the node was successfully acquired
     */
    bool acquire();
    
    /**
     * Instantly stops this node from both recording and playback.
     *
     * This method is the same as calling both the methods {@link pause()} and
     * {@link release()}.  In addition, the input node will be marked as
     * {@link completed()} for the purpose of the audio graph.
     */
    void stop();

    /**
     * Returns any cached data as an in-memory audio sample.
     *
     * This method is potentially expensive and should only be called when
     * the audio node has stopped recording (via the {@link release()} method,
     * and when the node is not part of an audio graph giving real-time
     * playback.
     *
     * If {@link mark()} is not set, this will return null rather than return
     * an empty audio sample.
     *
     * @return any cached data as an in-memory audio sample.
     */
    std::shared_ptr<AudioSample> save();

#pragma mark -
#pragma mark Audio Graph
    /**
     * Returns true if this audio node has no more data.
     *
     * An audio node is typically completed if it return 0 (no frames read) on
     * subsequent calls to {@link read()}. However, input nodes may run
     * infinitely.  Therefore this method only returns true when either of the
     * methods {@link stop()} or {@link setRemaining()} are called.
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
     * The channels are interleaved into the output buffer. The number of
     * frames read is determined by the audio graph, not the buffer of this
     * device.
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
     * Records the specified number of frames to this audio node
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioOutput.
     *
     * If {@link mark()} is not set, this method records to a circular buffer
     * that has the given {@link getDelay()}.  Data that is not read in a
     * timely manner is lost from the buffer.
     *
     * However, if mark is set, then this method writes to an ever-increasing
     * queue.  This queue can be accessed at any time with {@link reset()}
     * or {@link setPosition()}.  This can potentially take a lot of memory
     * and so it should be used carefully.  Use {@link release()} to stop
     * recording to the buffer while still having access to it.
     */
    Uint32 record(float* buffer, Uint32 frames);
    
    /**
     * Reboots the audio input node without interrupting any active polling.
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

#pragma mark -
#pragma mark Optional Methods
    /**
     * Marks the current read position in the audio steam.
     *
     * This methods creates an internal buffer for recording audio data.
     * This buffer may be returned to at any time with {@link reset()} command.
     * Doing so introduces an inherent delay going forward, as the playback
     * comes from the recorded buffer.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * If the method {@link mark()} has started recording to a buffer, then
     * this method will stop recording and release the buffer.  When the mark
     * is cleared the method {@link reset()} will no longer work.
     *
     * @return true if the read position was marked.
     */
    virtual bool unmark() override;
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * This method does nothing (and returns false) if no {@link mark()} is set.
     * Otherwise, it resets to the start of the buffer created by the call to
     * mark. This introduces an inherent delay going forward, as the playback
     * comes from the recorded buffer.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;
    
    /**
     * Returns the current frame position of this audio node
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, the position will be the number of frames since
     * the mark.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, it will set the position to the number of frames
     * since the mark.  If the position is in the future (a frame not already
     * buffered) then this method will fail and return -1.
     *
     * @param position  the current frame position of this audio node.
     *
     * @return the new frame position of this audio node.
     */
    virtual Sint64 setPosition(Uint32 position) override;
    
    /**
     * Returns the elapsed time in seconds.
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, the position will be the number of seconds since
     * the mark.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, it will set the position to the number of seconds
     * since the mark.  If the position is in the future (a time not already
     * buffered) then this method will fail and return -1.
     *
     * @param time  The elapsed time in seconds.
     *
     * @return the new elapsed time in seconds.
     */
    virtual double setElapsed(double time) override;
    
    /**
     * Returns the remaining time in seconds.
     *
     * This method returns -1 (indicating it is not supported) if the method
     * {@link setRemaining()} has not been called or has been interrupted.
     * Otherwise, it returns the amount of time left in the countdown timer
     * until this node completes.
     *
     * @return the remaining time in seconds.
     */
    virtual double getRemaining() const override;
    
    /**
     * Sets the remaining time in seconds.
     *
     * This method sets a countdown timer on the input node, forcing it to
     * complete in the given number of seconds.  If the audio has been reading
     * from the buffer (because of a call to {@link setPosition()}, this method
     * immediately skips ahead to real-time recording.  Any call to
     * {@link setPosition()} or {@link setElapsed()} before this time is up
     * will cancel the countdown.
     *
     * @param time  The remaining time in seconds.
     *
     * @return the new remaining time in seconds.
     */
    virtual double setRemaining(double time) override;
    
};
    }
}
#endif /* __CU_AUDIO_INPUT_H__ */
