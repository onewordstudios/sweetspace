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
#include <cugl/audio/CUAudioManager.h>
#include <cugl/audio/CUAudioSample.h>
#include <cugl/audio/graph/CUAudioScheduler.h>
#include <cugl/math/dsp/CUDSPMath.h>
#include <cugl/base/CUApplication.h>
#include <cugl/util/CUDebug.h>
#include <cmath>

using namespace cugl::audio;

#pragma mark Player Queue
/**
 * Creates an empty player queue
 */
AudioNodeQueue::AudioNodeQueue() {
    // Add dummy separator
    _first = new Entry(std::shared_ptr<AudioNode>(),0);
    _divide.store(_first, std::memory_order_relaxed);
    _last.store(_first, std::memory_order_relaxed);
}


/**
 * Disposes of the player queue, releasing all resources
 */
AudioNodeQueue::~AudioNodeQueue() {
    while( _first != nullptr ) {
        Entry* tmp = _first;
        _first = tmp->next;
        delete tmp;
    }
}

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
 * @param loop  The number of times to loop the audio
 */
void AudioNodeQueue::push(const std::shared_ptr<AudioNode>& node, Sint32 loops) {
    Entry* last = _last.load(std::memory_order_relaxed);
    
    // Add the new item
    last->next = new Entry(node,loops);
    _last.store(last->next, std::memory_order_relaxed);
    
    // Trim unused nodes
    while( _first != _divide) {
        Entry* tmp = _first;
        _first = _first->next;
        delete tmp;
    }
}

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
bool AudioNodeQueue::pop(std::shared_ptr<AudioNode>& node, Sint32& loop) {
    if ( _divide != _last ) {
        Entry* div = _divide.load(std::memory_order_relaxed);
        node = div->next->value;
        loop = div->next->loops;
        _divide.store(div->next, std::memory_order_relaxed);
        return true;
    }
    return false;
}

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
bool AudioNodeQueue::peek(std::shared_ptr<AudioNode>& node, Sint32& loop) const {
    if ( _divide != _last ) {
        Entry* div = _divide.load(std::memory_order_relaxed);
        node = div->next->value;
        loop = div->next->loops;
        return true;
    }
    return false;    
}

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
bool AudioNodeQueue::fill(std::deque<std::shared_ptr<AudioNode>>& container) const {
    if ( _divide != _last ) {
        Entry* div = _divide.load(std::memory_order_relaxed);
        while (div->next) {
            div = div->next;
            container.push_back(div->value);
        }
        return true;
    }
    return false;
}

/**
 * Clears all elements in this queue.
 *
 * This method is thread-safe
 */
void AudioNodeQueue::clear() {
    // Defer clean up to push
    while ( _divide != _last ) {
        Entry* div = _divide.load(std::memory_order_relaxed);
        _divide.store(div->next, std::memory_order_relaxed);
    }
}


#pragma mark -
#pragma mark Player Node
/**
 * Creates an inactive player node.
 *
 * The node will become active when a source is added to the queue.
 */
AudioScheduler::AudioScheduler() : AudioNode(),
_previous(nullptr),
_buffer(nullptr),
_loops(0),
_qsize(0),
_qskip(0),
_overlap(0),
_mempos(-1) {
    _classname = "AudioScheduler";
}

/**
 * Deletes the playernode, disposing of all resources
 */
AudioScheduler::~AudioScheduler() {
    dispose();
}

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
bool AudioScheduler::init() {
    if (AudioNode::init()) {
        Uint32 size   = AudioManager::get()->getReadSize();
        _buffer  = (float*)malloc(size*_channels*sizeof(float));
        return true;
    }
    return false;
}

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
bool AudioScheduler::init(Uint8 channels, Uint32 rate) {
    if (AudioNode::init(channels,rate)) {
        Uint32 size   = AudioManager::get()->getReadSize();
        _buffer  = (float*)malloc(size*channels*sizeof(float));
        return true;
    }
    return false;
}


/**
 * Disposes any resources allocated for this node
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioScheduler::dispose() {
    if (_booted) {
        clear(true);
        if (_buffer) {
            free(_buffer);
            _buffer = nullptr;
        }
        AudioNode::dispose();
        _memory.clear();
        _loops = 0;
        _qsize = 0;
        _qskip = 0;
        _overlap = 0;
        _mempos = 0;
        _current  = nullptr;
        _previous = nullptr;
    }
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
void AudioScheduler::play(const std::shared_ptr<AudioNode>& node, Sint32 loop) {
    if (node->getChannels() != _channels) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "AudioNode has the wrong number of channels: %d",
                     node->getChannels());
        return;
    } else if (node->getRate() != _sampling) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "AudioNode has the wrong frequency: %d",
                     node->getRate());
        return;
    }
    _queue.push(node,loop);
    _qsize.exchange(_qsize.load(std::memory_order_relaxed)+1,std::memory_order_release);
    _qskip.store(_qsize.load(std::memory_order_relaxed)-1,std::memory_order_release);

}

/**
 * Appends a new audio node for playback.
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
void AudioScheduler::append(const std::shared_ptr<AudioNode>& node, Sint32 loop) {
    if (node->getChannels() != _channels) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "AudioNode has the wrong number of channels: %d",
                     node->getChannels());
        return;
    } else if (node->getRate() != _sampling) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "AudioNode has the wrong frequency: %d",
                     node->getRate());
        return;
    }
    
    _queue.push(node,loop);
    _qsize.store(_qsize.load(std::memory_order_relaxed)+1,std::memory_order_release);
}

/**
 * Returns the audio node currently being played.
 *
 * If the user has provided an optional callback function, this will be
 * called when this node is no longer active
 *
 * @return the audio node currently being played.
 */
std::shared_ptr<AudioNode> AudioScheduler::getCurrent() const {
    // Most compilers do not appear to support an atomic operation here
    // But the failure mode is acceptable.
    return _current;
}

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
void AudioScheduler::clear(bool force) {
    if (!force) {
        _qskip.store(_qsize.load(std::memory_order_relaxed)+1,std::memory_order_relaxed);
    } else {
        bool orig = _paused.exchange(true,std::memory_order_relaxed);
        _queue.clear();
        _current = nullptr;
        _paused.store(orig, std::memory_order_relaxed);
    }
}

/**
 * Returns all audio nodes waiting to be played.
 *
 * This method only returns the nodes.  It does not return any loop
 * information.  The currently playing audio is not included.
 *
 * @return all audio nodes waiting to be played.
 */
std::deque<std::shared_ptr<AudioNode>> AudioScheduler::getTail() const {
    std::deque<std::shared_ptr<AudioNode>> results;
    _queue.fill(results);
    return results;
}

/**
 * Returns the number audio nodes waiting to be played.
 *
 * The currently playing audio is not included.
 *
 * @return the number audio nodes waiting to be played.
 */
Uint32 AudioScheduler::getTailSize() const {
    return _qsize.load(std::memory_order_relaxed);
}


/**
 * Skips forward to a future song in the queue.
 *
 * The optional parameter n specifies the number of additional sounds to skip.
 * If n is 0, it will just go the front element of the queue.  Otherwise,
 * it will skip to the n element after the head of the queue.  If n is larger
 * than the size of the queue, this is the same as clear().
 *
 * If the user has provided an optional callback function, this will be
 * called for all of the sounds removed from the queue (as well as the
 * current sound).  The complete flag will be false, indicating that they
 * were interrupted.
 */
void AudioScheduler::skip(Uint32 n) {
    _qskip.fetch_add(n,std::memory_order_relaxed);
}

/**
 * Empties the queue without stopping the current playback.
 *
 * This method is useful when we want to clear the queue, but to smoothly
 * fade-out the current playback.
 */
void AudioScheduler::trim(Sint32 size) {
    if (size < 0) {
        _queue.clear();
    } else {
        Uint32 qsize = _qsize.load(std::memory_order_relaxed);
        Sint32 actly = (Sint32)qsize-size;
        actly = std::max(actly,0);
        actly = _qsize.exchange(actly,std::memory_order_relaxed);
        size -= (qsize-actly);
        std::shared_ptr<AudioNode> result;
        while(size--) {
            _queue.pop(result,actly);
        }
    }
}

/**
 * Returns true if the scheduler has an active audio node
 *
 * This method only checks if there is a current active node.  This method
 * may return true even if the node is paused.
 *
 * return true if the scheduler has an active audio node
 */
bool AudioScheduler::isPlaying() {
    // Most compilers do not appear to support an atomic operation here
    // But the failure mode is acceptable.
    return _current != nullptr;
}

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
 *
 * The overlap should be chosen with care.  If the play length of an
 * audio node is less than the overlap, the results are undefined.
 *
 * @param time  The overlap time in seconds.
 */
void AudioScheduler::setOverlap(double time) {
    _previous = nullptr;
    _overlap.store((Uint32)(time*_sampling),std::memory_order_release);
}

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
 *
 * The overlap should be chosen with care.  If the play length of an
 * audio node is less than the overlap, the results are undefined.
 *
 * @return the overlap time in seconds.
 */
double AudioScheduler::getOverlap() const {
    Uint32 frames = _overlap.load(std::memory_order_relaxed);
    return ((double)frames)/_sampling;
}

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
Sint32 AudioScheduler::getLoops() const {
    return _loops.load(std::memory_order_relaxed);
}

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
void AudioScheduler::setLoops(Sint32 loop) {
    _loops.store(loop,std::memory_order_relaxed);
}

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
Uint32 AudioScheduler::read(float* buffer, Uint32 frames) {
    if (_paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,frames*sizeof(float)*_channels);
        return frames;
    }
    
    _polling.store(true);
    Uint32 skip = _qskip.exchange(0);
    
    Sint32 loop;
    std::shared_ptr<AudioNode> previous = _previous;
    std::shared_ptr<AudioNode> current  = acquire(loop,skip,Action::INTERRUPT);
    Uint32 overlap = _overlap.load(std::memory_order_acquire);
    
    Uint32 amt = 0;
    while (amt < frames && current != nullptr) {
        Uint32 need = frames-amt;
        if (previous && current && overlap > 0) {
            // Continue an existing overlap
            float* output = buffer+amt*_channels;
            float* input  = _buffer;
            
            Sint64 remain = previous->getRemaining()*_sampling;
            Uint32 goal = std::min((Uint32)std::max(remain,(Sint64)0),need);
            Uint32 real = current->read(output,goal);
            goal = previous->read(input,real);
            if (goal < real) {
                // Possible in rare cases with a fade-out in place
                std::memset(input+goal*_channels,0,(real-goal)*_channels*sizeof(float));
                goal = real;
            }
            amt += goal;
            
            // Now mix
            real = goal*_channels;
            Uint32 step = std::min((Uint32)remain,overlap);
            while (real--) {
                float factor = (float)(step)/overlap;
                *output = *input*factor+*output*(1-factor);
                output++;
                input++;
                if (real % _channels == 0 && step > 0) {
                    step--;
                }
            }
\
            // And shift if we are done.
            if (goal >= remain) {
                if (_calling.load(std::memory_order_relaxed)) {
                    notify(previous,Action::COMPLETE);
                }
                previous  = nullptr;
                _previous = nullptr;
            }
            
            // Handle very short current
            if (current->completed()) {
                current = acquire(loop,1,Action::COMPLETE);
            }
        } else if (overlap > 0 && loop == 0 && _qsize.load(std::memory_order_acquire)) {
            // Check whether we need to overlap
            Sint64 remain = current->getRemaining()*_sampling;
            if (remain >= 0 && remain-overlap <= need) {
                if (remain > overlap) {
                    amt += current->read(&(buffer[amt*_channels]),(Uint32)(remain-overlap));
                }
                _previous = current;
                previous = _previous;
                _queue.pop(_current,loop);
                current = _current;
            } else {
                amt += current->read(&(buffer[amt*_channels]),need);
                if (amt < frames || current->completed()) {
                    current = acquire(loop,1,Action::COMPLETE);
                }
            }
        } else {
            // Perform a normal read
            amt += current->read(&(buffer[amt*_channels]),need);
            if (loop && amt < frames) {
                if (!current->reset()) {
                    current = nullptr;
                    _current = nullptr;
                } else if (_calling.load(std::memory_order_acquire)) {
                    notify(current,Action::LOOPBACK);
                }
                if (loop > 0) { loop--;}
            } else if (amt < frames || (!loop && current->completed())) {
                current = acquire(loop,1,Action::COMPLETE);
            }
        }
    }
    
    dsp::DSPMath::scale(buffer,_ndgain.load(std::memory_order_relaxed),buffer,amt*_channels);
    if (amt < frames) {
        std::memset(buffer+amt*_channels,0,(frames-amt)*sizeof(float)*_channels);
    }
    
    _loops.store(loop,std::memory_order_relaxed);
    _polling.store(false);
    return frames;
}

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
bool AudioScheduler::mark() {
    // TODO: FINISH ME
    return false;
}

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
bool AudioScheduler::unmark() {
    // TODO: FINISH ME
    return false;
}

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
bool AudioScheduler::reset() {
    // TODO: FINISH ME
    return false;
}

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
Sint64 AudioScheduler::advance(Uint32 frames) {
    // TODO: FINISH ME
    return -1;
}

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
Sint64 AudioScheduler::getPosition() const  {
    // TODO: FINISH ME
    return -1;
}

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
Sint64 AudioScheduler::setPosition(Uint32 position)  {
    // TODO: FINISH ME
    return -1;
}


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
double AudioScheduler::getElapsed() const  {
    // TODO: FINISH ME
    return -1.0;
}


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
double AudioScheduler::setElapsed(double time)  {
    // TODO: FINISH ME
    return -1.0;
}

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
std::shared_ptr<AudioNode> AudioScheduler::acquire(Sint32& loop, Uint32 skip, AudioNode::Action action) {
    // Most compilers do not appear to support an atomic operation here
    // But getting a local variable is good enough.
    std::shared_ptr<AudioNode> result = _current;
    Uint32 size = _qsize.load(std::memory_order_acquire);
    bool callback = _calling.load(std::memory_order_relaxed);
    bool change = false;
    
    loop = _loops.load(std::memory_order_relaxed);
    while (skip && size) {
        if (result != nullptr && callback) {
            notify(result,action);
        }
        _queue.pop(result,loop);
        size--;
        skip--;
        change = true;
    }
    if (skip) {
        if (result != nullptr && callback) {
            notify(result,action);
        }
        result = nullptr;
        loop = 0;
        change = true;
    } else if (result == nullptr && size) {
        _queue.pop(result,loop);
        size--;
        change = true;
    }

    if (change) {
        _qsize.store(size,std::memory_order_release);
        _loops.store(loop,std::memory_order_relaxed);
        _current = result;
    }
    return result;
}
