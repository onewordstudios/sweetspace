//
//  CUAudioSynchronizer.h
//  CUGL
//
//  Created by Walker White on 1/27/19.
//  Copyright © 2019 Game Design Initiative at Cornell. All rights reserved.
//

#ifndef __CU_AUDIO_SYNCHRONIZER_H__
#define __CU_AUDIO_SYNCHRONIZER_H__
#include <cugl/audio/graph/CUAudioNode.h>
#include <cugl/util/CUTimestamp.h>
#include <atomic>
#include <mutex>

namespace cugl {
    namespace audio {
/**
 * A class providing visual synchronization for an audio node.
 *
 * Because of variable latency issues on mobile platforms, synchronization for
 * rhythm games is always difficult.  This EXPERIMENTAL class is attempt to
 * resolve this issue.
 *
 * WARNING: This class is largely untested.  Use at your own risk.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioSynchronizer : public AudioNode {
private:
    /** The audio input node */
    std::shared_ptr<AudioNode> _input;
    
    /** This class needs a proper lock guard; too many race conditions */
    std::mutex _mutex;
    
    /** The (projected) overhead of reading the audio graph */
    std::atomic<double> _overhead;

    /** The current audio graph jitter (-1 if not set yet) */
    std::atomic<double> _jitter;

    /** The bpm settings (for non-carrier signal inputs) */
    std::atomic<double> _inputBPM;
    std::atomic<Sint32> _prevbeat;


    /** The timestamp of the most recent call to this node */
    std::atomic<timestamp_t> _timestamp;
    
    /** The start frame of a signal in the audio buffer (-1 if not present) */
    std::atomic<Sint32> _liveStart;
    /** The final frame of a signal in the audio buffer (-1 if to the end)  */
    std::atomic<Sint32> _liveDone;
    /** The start frame of a signal in the audio queue (-1 if not present) */
    std::atomic<Sint32> _waitStart;
    /** The final frame of a signal in the audio queue (-1 if to the end)  */
    std::atomic<Sint32> _waitDone;

    /** The intermediate read buffer */
    float* _buffer;
    /** The capacity of the intermediate buffer */
    Uint32 _capacity;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degenerate audio synchronizer
     *
     * The node has no channels, so read options will do nothing. The node must
     * be initialized to be used.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
     * the heap, use one of the static constructors instead.
     */
    AudioSynchronizer();
    
    /**
     * Deletes the audio synchronizer, disposing of all resources
     */
    ~AudioSynchronizer() { dispose(); }
    
    /**
     * Initializes the synchronizr with default stereo settings
     *
     * The number of output channels is two, for stereo output. Input nodes
     * must either match this (for no carrier signal) or have one additional
     * channel. The sample rate is the modern standard of 48000 HZ.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
    /**
     * Initializes the synchronizer with the given number of channels and sample rate
     *
     * The channels specifies the number of output channels. Input nodes
     * must either match this (for no carrier signal) or have one additional
     * channel.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
    /**
     * Disposes any resources allocated for this synchronizer
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated synchronizer with default stereo settings
     *
     * The number of output channels is two, for stereo output. Input nodes
     * must either match this (for no carrier signal) or have one additional
     * channel. The sample rate is the modern standard of 48000 HZ.
     *
     * @return a newly allocated synchronizer with default stereo settings
     */
    static std::shared_ptr<AudioSynchronizer> alloc() {
        std::shared_ptr<AudioSynchronizer> result = std::make_shared<AudioSynchronizer>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated synchronizer with the given number of channels and sample rate
     *
     * The channels specifies the number of output channels. Input nodes
     * must either match this (for no carrier signal) or have one additional
     * channel.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return a newly allocated synchronizer with the given number of channels and sample rate
     */
    static std::shared_ptr<AudioSynchronizer> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioSynchronizer> result = std::make_shared<AudioSynchronizer>();
        return (result->init(channels,rate) ? result : nullptr);
    }

#pragma mark -
#pragma mark Synchronization Methods
	/**
	 * Returns the (projected) overhead of reading the audio graph 
	 *
	 * @return the (projected) overhead of reading the audio graph 
	 */
    double getOverhead() const {
        return _overhead.load(std::memory_order_relaxed);
    }

	/**
	 * Sets the projected overhead of reading the audio graph 
	 *
	 * @param overhead	The projected overhead of reading the audio graph 
	 */
    void setOverhead(double overhead);

	/**
	 * Returns the currently observed jitter
	 *
	 * Returns -1 if there is no jitter so far
	 */
    double getJitter() const {
        return _jitter.load(std::memory_order_relaxed);
    }

	/**
	 * Clears the jitter readings to reset the calculations
	 */
    void clearJitter();
    
    /**
     * Returns true if the music is on the beat
     *
     * @return true if the music is on the beat
     */
    bool onBeat();
    
#pragma mark -
#pragma mark Audio Graph
    /**
     * Attaches an audio node to this synchronizer.
     *
     * The audio node must agree with the sample rate of this synchronizer.
     * If it has a carrier signal, then it should have one more channel than
     * this node, with the extra channel delivering the signal.  If it does
     * not have a carrier signal, it must agree with the number of channels
     * of this node.
     *
     * The optional bpm (beats-per-minute) argument is only applicable if the
     * node does not have a carrier signal.  In that case, it will use the
     * timestamps to guess at the synchronization information.  If the bpm
     * value is not positive, it will not attempt to synchronize.
     *
     * @param node  The audio node to synchronize
     * @param bpm   The beats-per-minute if a carrier signal is missing
     *
     * @return true if the attachment was successful
     */
    bool attach(const std::shared_ptr<AudioNode>& node, double bpm=0.0);
    
    /**
     * Detaches an audio node from this synchronizer.
     *
     * If the method succeeds, it returns the audio node that was removed.
     *
     * @return  The audio node to detach (or null if failed)
     */
    std::shared_ptr<AudioNode> detach();
    
    /**
     * Returns the input node of this synchronizer.
     *
     * @return the input node of this synchronizer.
     */
    std::shared_ptr<AudioNode> getInput() const { return _input; }
    
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
#endif /* __CU_AUDIO_SYNCHRONIZER_H__ */
