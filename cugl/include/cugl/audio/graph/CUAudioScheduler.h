//
//  CUAudioScheduler.h
//  Cornell University Game Library (CUGL)
//
//  Thie module provides an audio graph node for scheduling different audio
//  nodes.  When combined with AudioPlayer, this provides a classic player
//  node such as you might find in AVFoundation.  However, by generalizing
//  this concept, we are able to schedule arbitrary audio patches as well.
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
#ifndef __CU_AUDIO_SEQUENCER_H__
#define __CU_AUDIO_SEQUENCER_H__
#include <SDL/SDL.h>
#include "CUAudioNode.h"
#include "CUAudioPlayer.h"
#include <functional>
#include <deque>

namespace cugl {
    namespace audio {

#pragma mark -
#pragma mark Audio Node Queue
/**
 * This class provides is a lock free producer-consumer queue,
 *
 * This queue allows us to add buffers to the source node without interrupting
 * playback.  Its implementation is taken from
 *
 * http://www.drdobbs.com/parallel/writing-lock-free-code-a-corrected-queue/210604448
 *
 * This queue is only designed to support two threads. The producer is the main
 * thread, while the consumer is the audio thread.
 *
 * This queue does not have a lot of bells and whistles because it is only
 * intended for thread synchronization.  We expect the user to maintain what
 * has and has not been appended to the queue.
 */
class AudioNodeQueue {
private:
    /**
     * An entry in the queue for this player.
     *
     * Queued entries remember their gain and loop status
     */
    struct Entry {
        /** The audio source for this entry */
        std::shared_ptr<AudioNode> value;
        /** Whether to loop this audio node */
        Sint32 loops;
        /** THe next entry in the queue (or null if at end) */
        Entry* next;
        
        /**
         * Appends an entry for the given audio node
         *
         * The loop value is an integer.  If it is 0, the audio node will not
         * be looped.  If it is positive, it will loop the audio that many
         * (additional) times.  If it is negative, the audio node will be
         * looped indefinitely until it is stopped.
         *
         * @param node  The audio node
         * @param loop  The number of times to loop the audio
         */
        Entry(const std::shared_ptr<AudioNode>& node, Sint32 loop) :
            value(node), loops(loop), next(nullptr) { }
    };
    
    /** THe first element int the queue */
    Entry* _first;
    /** Pointer to the front of the queue (to remove elements) */
    std::atomic<Entry*> _divide;
     /** Pointer to the end of the queue (to add elements) */
    std::atomic<Entry*> _last;
    
    
public:
#pragma mark Constructors
    /**
     * Creates an empty player queue
     */
    AudioNodeQueue();
    
    /**
     * Disposes of the player queue, releasing all resources
     */
    ~AudioNodeQueue();
    
    
#pragma mark Queue Methods
    /**
     * Returns true if the queue is empty.
     *
     * This method is atomic and thread-safe
     *
     * @return true if the queue is empty.
     */
    bool empty() const { return _divide == _last; }
    
    /**
     * Adds an entry to the end of this queue.
     *
     * The loop value is an integer.  If it is 0, the audio node will not
     * be looped.  If it is positive, it will loop the audio that many
     * (additional) times.  If it is negative, the audio node will be
     * looped indefinitely until it is stopped.
     *
     * This method is thread-safe
     *
     * @param node  The node to be scheduled
     * @param loops	The number of times to loop the audio
     */
    void push(const std::shared_ptr<AudioNode>& node, Sint32 loops=0);

    /**
     * Looks at the front element this queue.
     *
     * The element will be stored in the (shared) pointer result.  If there
     * is nothing to see, the pointer will store null and the method will
     * return false.
     *
     * This method is thread-safe
     *
     * @param node  the pointer to store the audio node
     * @param loop  the pointer to store the number of loops
     *
     * @return true if the operation was successful
     */
    bool peek(std::shared_ptr<AudioNode>& node, Sint32& loop) const;
    
    /**
     * Removes an entry from the front of this queue.
     *
     * The element will be stored in the (shared) pointer result.  If there
     * is nothing to remove, the pointer will store null and the method will
     * return false.
     *
     * This method is thread-safe
     *
     * @param node  the pointer to store the audio node
     * @param loop  the pointer to store the number of loops
     *
     * @return true if the operation was successful
     */
    bool pop(std::shared_ptr<AudioNode>& node, Sint32& loop);
    
    /**
     * Stores all values in the provided dequeue.
     *
     * This method only stores the values, not the loop settings. If the queue is
     * empty, the deque is not altered and this method returns false.
     *
     * This method is thread-safe ASSUMING that {@link push} is only ever called in
     * the same thread (e.g. this is a producer method).
     *
     * @param container the container to store the values
     *
     * @return true if the operation was successful
     */
    bool fill(std::deque<std::shared_ptr<AudioNode>>& container) const;
    
    /**
     * Clears all elements in this queue.
     *
     * This method is thread-safe
     */
    void clear();
};
    
    
#pragma mark -
#pragma mark Sequencer Node
/**
 * This class is capable of scheduling audio nodes in sequence.
 *
 * This node is important for supporting dynamic playback.  While we can
 * safely rearrange nodes in the audio graph when it is not active, this
 * allows us to schedule nodes while playback is ongoing.  When combined with
 * {@link AudioPlayer}, this provides a classic player node such as you might
 * find in AVFoundation.  However, by generalizing this concept, we are able to
 * schedule arbitrary audio patches as well.
 *
 * To support seamless audio, a scheduler is fed by a queue.  That way the user
 * can queue up a new source while the current one is playing.  However, to
 * simplify the data structures and ensure thread safety, we do not allow the
 * user to look at the contents of the queue.  The user can only look at
 * the currently playing node.
 *
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the user.
 *
 * This audio node supports the callback functions in {@link AudioNode#setCallback}.
 * This function function is called whenever a node is removed from the scheduler.
 * This may be because the node played to completion (defined as a
 * {@link AudioScheduler#read()} result that returns 0) or it was interrupted.
 */
class AudioScheduler : public AudioNode {
private:
    /** The currently active audio node */
    std::shared_ptr<AudioNode> _current;
    /** The previously active audio node  (for overlaps) */
    std::shared_ptr<AudioNode> _previous;
    /** The remaining number of loops for the current audio */
    std::atomic<Sint32> _loops;
    /** The desired overlap amount */
    std::atomic<Uint32> _overlap;
    /** A buffer to handle the overlap (as necessary) */
    float* _buffer;

    /** The queue of all sources waiting to be played next */
    AudioNodeQueue _queue;
    
    /** Counter to track queue size */
    std::atomic<Uint32> _qsize;
    /** Counter to track queue skips (for clearing or advancement) */
    std::atomic<Uint32> _qskip;

    /** Stored results after a mark is set */
    std::deque<std::shared_ptr<AudioNode>> _memory;
    /** The current position in the mark memory; -1 if inactive */
    Sint64 _mempos;
    
public:
#pragma mark Constructors
    /**
     * Creates an inactive scheduler node.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
     * the heap, use one of the static constructors instead.
     */
    AudioScheduler();
    
    /**
     * Deletes the scheduler node, disposing of all resources
     */
    ~AudioScheduler();

    /**
     * Initializes the scheduler with default stereo settings
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
     * Initializes the scheduler with the given number of channels and sample rate
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
     * Disposes any resources allocated for this node
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
    /**
     * Returns an allocated player with the given number of channels and sample rate
     *
     * These values determine the buffer the structure for all {@link read}
     * operations.  In addition, they also detemine what types of sources that
     * the player can support.  A player can only play assets with the right
     * sampling rate and number of channels.
     *
     * The node starts off inactive. It will become active when a source is 
     * added to the queue.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in HZ
     *
     * @return an allocated player with the given number of channels and sample rate
     */
    static std::shared_ptr<AudioScheduler> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioScheduler> result = std::make_shared<AudioScheduler>();
        return (result->init(channels, rate) ? result : nullptr);
    }
    
#pragma mark Queue Management
    /**
     * Immediately schedules a new audio node for playback.
     *
     * This method clears the queue and immediately schedules the node for
     * the next audio render frame.
     *
     * This audio node may be any satisfying class, though it is typically
     * an instance of {@link AudioPlayer}.  Gain control is handled in the
     * node itself (though the scheduler can add extra gain). The only
     * new feature added is looping.
     *
     * The loop value is an integer.  If it is 0, the audio node will not
     * be looped.  If it is positive, it will loop the audio that many
     * (additional) times.  If it is negative, the audio node will be
     * looped indefinitely until it is stopped.
     *
     * If the user has provided an optional callback function, this will be
     * called when the node is removed, either because it completed (defined
     * by {@link AudioNode#completed()}) or is interrupted.
     *
     * @param node  The audio node for playback
     * @param loop  The number of times to loop the audio
     */
    void play(const std::shared_ptr<AudioNode>& node, Sint32 loop = 0);
    
    /**
     * Appends a new audio node for playback.
     *
     * This method appends to the node to the playback queue.  It will be
     * played as soon as the nodes that are earlier in the queue have completed
     * playing.
     *
     * This audio node may be any satisfying class, though it is typically
     * an instance of {@link AudioPlayer}.  Gain control is handled in the
     * node itself (though the scheduler can add extra gain). The only
     * new feature added is looping.
     *
     * The loop value is an integer.  If it is 0, the audio node will not
     * be looped.  If it is positive, it will loop the audio that many
     * (additional) times.  If it is negative, the audio node will be
     * looped indefinitely until it is stopped.
     *
     * If the user has provided an optional callback function, this will be
     * called when the node is removed, either because it completed (defined
     * by {@link AudioNode#completed()}) or is interrupted.
     *
     * @param node  The audio node for playback
     * @param loop  The number of times to loop the audio
     */
    void append(const std::shared_ptr<AudioNode>& node, Sint32 loop = 0);
    
    /**
     * Returns the audio node currently being played.
     *
     * If the user has provided an optional callback function, this will be
     * called when this node is no longer active
     *
     * @return the audio node currently being played.
     */
    std::shared_ptr<AudioNode> getCurrent() const;
    
    /**
     * Returns all audio nodes waiting to be played.
     *
     * This method only returns the nodes.  It does not return any loop
     * information.
     *
     * @return all audio nodes waiting to be played.
     */
    std::deque<std::shared_ptr<AudioNode>> getTail() const;
    
    /**
     * Returns the number audio nodes waiting to be played.
     *
     * The currently playing audio is not included.
     *
     * @return the number audio nodes waiting to be played.
     */
    Uint32 getTailSize() const;
    
    /**
     * Stops the current playback and empties the queue.
     *
     * To ensure consistency, this method only flags the nodes for deletion.
     * Clean-up will occur in the audio thread.  This ensures that the callback
     * function (if provided) is called from the audio thread for all of the
     * nodes removed from the queue (as well as the current node). The complete
     * flag will be false, indicating that they were interrupted.
     *
     * The optional force argument allows for sounds to be purged immediately
     * (such as during clean-up).  However, doing so will not invoke the callback
     * function, even if it is provided.
     *
     * @param force whether to delete the queue immediately, in the current thread
     */
    void clear(bool force=false);
    
    /**
     * Empties the queue without stopping the current playback.
     *
     * This method is useful when we want to clear the queue, but to smoothly
     * fade-out the current playback.
     */
    void trim(Sint32 size = -1);
    
    /**
     * Skips forward to a future nodes in the queue.
     *
     * The optional parameter n specifies the number of additional nodes to skip.
     * If n is 0, it will just go the front element of the queue.  Otherwise, 
     * it will skip to the n element after the head of the queue.  If n is larger
     * than the size of the queue, this is the same as clear().
     *
     * If the user has provided an optional callback function, this will be
     * called for all of the nodes removed from the queue (as well as the
     * current sound).  The complete flag will be false, indicating that they
     * were interrupted.
     */
    void skip(Uint32 n=0);
    
    /**
     * Returns true if the scheduler has an active audio node
     *
     * This method only checks if there is a current active node.  This method
     * may return true even if the node is paused.
     *
     * return true if the scheduler has an active audio node
     */
    bool isPlaying();
    
    /**
     * Returns the overlap time in seconds.
     *
     * The overlap time is the amount of time to cross-fade between a node
     * on the queue and the next.  It does not apply to looped nodes; nodes
     * can never cross-fade with themselves.
     *
     * The cross-fade is triggered when a node implements the method
     * {@link AudioNode#getRemaining()}, and this value is less than or equal
     * to the overlap.  It does not trigger if that method is not supported.
     * In addition, if a node is forced to complete before the normal time
     * remaining, the overlap will not apply.
     *
     * The overlap should be chosen with care.  If the play length of an
     * audio node is less than the overlap, the results are undefined.
     *
     * @return the overlap time in seconds.
     */
    double getOverlap() const;
    
    /**
     * Sets the overlap time in seconds.
     *
     * The overlap time is the amount of time to cross-fade between a node
     * on the queue and the next.  It does not apply to looped nodes; nodes
     * can never cross-fade with themselves.
     *
     * The cross-fade is triggered when a node implements the method
     * {@link AudioNode#getRemaining()}, and this value is less than or equal
     * to the overlap.  It does not trigger if that method is not supported.
     * In addition, if a node is forced to complete before the normal time
     * remaining, the overlap will not apply.
     *
     * The overlap should be chosen with care.  If the play length of an
     * audio node is less than the overlap, the results are undefined.
     *
     * @param time  The overlap time in seconds.
     */
    void setOverlap(double time);
    
#pragma mark Playback Sequencing
    /**
     * Returns number of loops remaining for the active audio node.
     *
     * If the value is 0, then the audio node will be removed from the queue
     * when it completes (as defined by {@link AudioNode#completed()}.  A value
     * greater than 0 will repeat that many times, assuming that the method
     * {@link AudioNode#reset()} is implemented (a node that cannot be reset
     * cannot be looped).  Finally, a negative value will be played indefinitely,
     * unless it is stopped or or the loop count is changed.
     *
     * This method returns 0 if there is no active audio node
     *
     * return true if the active audio node is looped.
     */
    Sint32 getLoops() const;

    /**
     * Sets number of loops remaining for the active audio node.
     *
     * If the value is 0, then the audio node will be removed from the queue
     * when it completes (as defined by {@link AudioNode#completed()}.  A value
     * greater than 0 will repeat that many times, assuming that the method
     * {@link AudioNode#reset()} is implemented (a node that cannot be reset
     * cannot be looped).  Finally, a negative value will be played indefinitely,
     * unless it is stopped or or the loop count is changed.
     *
     * This method does nothing if there is no active audio node.
     *
     * @param loop  The number of times to loop the audio
     */
    void setLoops(Sint32 loop);
    
    // TODO: Add cross-fade support


#pragma mark Overriden Methods
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
     * Marks the current read position in the audio steam.
     *
     * DELEGATED METHOD: This method delegates its call to the current audio
     * node.  It returns false if there is no active node or if this method is
     * unsupported.
     *
     * Once this method is called, the scheduler will mark the current audio
     * node and buffer all subsequent audio nodes. It will create a secondary
     * queue to prevent these nodes from being released. A call to the method
     * {@link reset()} will return to the marked position of the current audio
     * node and replay all subsequent audio nodes before returning to the audio
     * queue.
     *
     * This secondary queue will continue accumulating audio nodes until
     * {@link unmark()} is called.  It is not recommended for a marks to
     * remain indefinitely.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * The method {@link mark()} creates a second queue for buffering audio.
     * This queue will continue accumulating audio nodes until this method
     * is called.
     *
     * This method has no effect if there is no current mark.
     *
     * @return true if the read position was cleared.
     */
    virtual bool unmark() override;
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * DELEGATED METHOD: This method delegates its call to the current audio
     * node.  It returns false if there is no active node or if this method is
     * unsupported.
     *
     * This method returns the playback to the audio node and position set
     * by a call to {@link mark()}.  If mark has not been called, this method
     * has no effect.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * DELEGATED METHOD: This method delegates its call to the current audio
     * node.  It returns -1 if there is no active node or if this method is
     * unsupported.
     *
     * If the number of frames is set beyond the bounds of the current node,
     * the outcome will depend on the state of the audio queue.  A looped node
     * will simply loop the given number of frames.  Otherwise, if this position
     * causes the audio node to complete, it will continue to advance through
     * the queue so long as this method (and {@link completed()}) is supported.
     *
     * This method is thread safe, and may be called outside the audio thread.
     * However, the accuracy of the result on a non-paused audio source is
     * subject to minor race conditions (e.g. you may set the time, but have
     * it forwarded by the next window size without reading). In addition,
     * some streaming formats are less accurate than others.
     *
     * This method will work even if {@link AudioNode#mark()} is not set.
     *
     * @param frames    The number of frames to advance
     *
     * @return the actual number of frames advanced; -1 if not supported
     */
    virtual Sint64 advance(Uint32 frames) override;
    
    /**
     * Returns the current frame position of this audio node.
     *
     * DELEGATED METHOD: This method delegates its call to the current audio
     * node.  It returns -1 if there is no active node or if this method is
     * unsupported.
     *
     * This method is thread safe, and may be called outside the audio thread.
     * However, the accuracy of the result on a non-paused audio source is
     * subject to minor race conditions (e.g. you may set the time, but have
     * it forwarded by the next window size without reading). In addition,
     * some streaming formats are less accurate than others.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * DELEGATED METHOD: This method delegates its call to the current audio
     * node.  It returns -1 if there is no active node or if this method is
     * unsupported.
     *
     * This method has no effect unless {@link mark()} is called.  All frame
     * positions are relative from the marked position.
     *
     * If the number of frames is set beyond the bounds of the current node,
     * the outcome will depend on the state of the audio queue.  A looped node
     * will simply loop the given number of frames.  Otherwise, if this position
     * causes the audio node to complete, it will continue to advance through
     * the queue so long as this method (and {@link completed()}) is supported.
     *
     * This method is thread safe, and may be called outside the audio thread.
     * However, the accuracy of the result on a non-paused audio source is
     * subject to minor race conditions (e.g. you may set the time, but have
     * it forwarded by the next window size without reading). In addition,
     * some streaming formats are less accurate than others.
     *
     * @param position  the current frame position of this audio node.
     *
     * @return the new frame position of this audio node.
     */
    virtual Sint64 setPosition(Uint32 position) override;

    /**
     * Returns the elapsed time in seconds.
     *
     * DELEGATED METHOD: This method delegates its call to the current audio
     * node.  It returns -1 if there is no active node or if this method is
     * unsupported.
     *
     * This method has no effect unless {@link mark()} is called.  All time
     * is relative from the marked position.
     *
     * This method is thread safe, and may be called outside the audio thread.
     * However, the accuracy of the result on a non-paused audio source is
     * subject to minor race conditions (e.g. you may set the time, but have
     * it forwarded by the next window size without reading). In addition,
     * some streaming formats are less accurate than others.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * DELEGATED METHOD: This method delegates its call to the current audio
     * node.  It returns -1 if there is no active node or if this method is
     * unsupported.
     *
     * This method has no effect unless {@link mark()} is called.  All time
     * is relative from the marked position.
     *
     * If the time is set beyond the bounds of the current node, the outcome
     * will depend on the state of the audio queue.  A looped node will simply
     * loop the elapsed time.  Otherwise, if this position causes the audio
     * node to complete, it will continue to advance through the queue so
     * long as this method (and {@link completed()})  is supported.
     *
     * This method is thread safe, and may be called outside the audio thread.
     * However, the accuracy of the result on a non-paused audio source is
     * subject to minor race conditions (e.g. you may set the time, but have
     * it forwarded by the next window size without reading). In addition,
     * some streaming formats are less accurate than others.
     *
     * @param time  The elapsed time in seconds.
     *
     * @return the new elapsed time in seconds.
     */
    virtual double setElapsed(double time) override;
    
#pragma mark Scheduling Helpers
private:
    /**
     * Returns an audio node for playback
     *
     * If skip is not present, this method either returns the current audio
     * node or pulls the first one from the queue if there is no current node.
     * Otherwise, it skips ahead the given number of elements.
     *
     * AUDIO THREAD ONLY: This is an internal method for queue management.
     * Indeed, only the audio thread is allowed to delete from the playback
     * queue.  All main thread methods do is place requests that are managed
     * at the next poll from the audio thread.
     *
     * @param loop      Reference variable to store remaining number of loops
     * @param skip      The number of elements to skip forward
     * @param action    The callback result on a skip
     *
     * @return the next audio instance for playback
     */
    std::shared_ptr<AudioNode> acquire(Sint32& loop, Uint32 skip=0, Action action=Action::COMPLETE);
};
    }
}

#endif /* __CU_AUDIO_SEQUENCER_H__ */
