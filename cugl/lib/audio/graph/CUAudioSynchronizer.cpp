//
//  CUAudioSynchronizer.cpp
//  CUGL
//
//  Created by Walker White on 1/27/19.
//  Copyright © 2019 Game Design Initiative at Cornell. All rights reserved.
//
#include <cugl/audio/graph/CUAudioSynchronizer.h>
#include <cugl/base/CUApplication.h>
#include <cugl/audio/CUAudioManager.h>
#include <cugl/util/CUDebug.h>
#include <cmath>

using namespace cugl::audio;

/**
 * Creates a degenerate audio synchronizer
 *
 * The node has no channels, so read options will do nothing. The node must
 * be initialized to be used.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
 * the heap, use one of the static constructors instead.
 */
AudioSynchronizer::AudioSynchronizer() : AudioNode(),
_jitter(-1),
_overhead(0.0),
_inputBPM(0.0),
_prevbeat(-1),
_liveStart(-1),
_waitStart(-1),
_liveDone(-1),
_waitDone(-1),
_capacity(0),
_buffer(nullptr) {
    _input = nullptr;
    _classname = "AudioSynchronizer";
}

/**
 * Initializes the synchronizr with default stereo settings
 *
 * The number of output channels is two, for stereo output. Input nodes
 * must either match this (for no carrier signal) or have one additional
 * channel. The sample rate is the modern standard of 48000 HZ.
 *
 * @return true if initialization was successful
 */
bool AudioSynchronizer::init() {
    return init(DEFAULT_CHANNELS,DEFAULT_SAMPLING);
}

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
bool AudioSynchronizer::init(Uint8 channels, Uint32 rate) {
    if (AudioNode::init(channels,rate)) {
        _capacity = AudioManager::get()->getReadSize();
        _buffer = (float*)malloc(_capacity*(_channels+1)*sizeof(float));
        Timestamp start;
        _timestamp = start.getTime();
        _overhead  = 1.0/Application::get()->getFPS();
        return true;
    }
    return false;
}

/**
 * Disposes any resources allocated for this panner
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioSynchronizer::dispose() {
    if (_booted) {
        AudioNode::dispose();
        free(_buffer);
        _buffer = nullptr;
        _capacity = 0;
        _overhead = 0;
        _jitter = -1;
        _inputBPM = 0.0;
        _prevbeat = -1;
        _liveStart = -1;
        _waitStart = -1;
        _liveDone = -1;
        _waitDone = -1;
         _input = nullptr;
    }
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
bool AudioSynchronizer::attach(const std::shared_ptr<AudioNode>& node, double bpm) {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot attach to an uninitialized audio node");
        return false;
    } else if (node == nullptr) {
        detach();
        return true;
    } else if (node->getChannels() != _channels && node->getChannels() != _channels+1) {
        CUAssertLog(false,"Input node has wrong number of channels: %d", node->getChannels());
        return false;
    } else if (node->getRate() != _sampling) {
        CUAssertLog(false,"Input node has wrong sample rate: %d", node->getRate());
        return false;
    }
    
    
    {
        std::unique_lock<std::mutex> lk(_mutex);
        _inputBPM.store(bpm);
        _prevbeat.store(-1);
        std::atomic_exchange_explicit(&_input,node,std::memory_order_relaxed);
    }
    return true;
}

/**
 * Detaches an audio graph from this output node.
 *
 * This method will fail if the output node is playing.  You must stop the
 * output node to reconfigure the audio graph.  If the method succeeds, it
 * returns the terminal node of the audio graph.
 *
 * @param node  The terminal node of the audio graph
 *
 * @return  the terminal node of the audio graph (or null if failed)
 */
std::shared_ptr<AudioNode> AudioSynchronizer::detach() {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot detach from an uninitialized output device");
        return nullptr;
    }
    
    std::shared_ptr<AudioNode> result;
    {
        std::unique_lock<std::mutex> lk(_mutex);
        result = std::atomic_exchange_explicit(&_input,{},std::memory_order_relaxed);
        _inputBPM.store(0,std::memory_order_relaxed);
        _prevbeat.store(-1,std::memory_order_relaxed);
    }
    return result;
}

#pragma mark -
#pragma mark Synchronization Methods
void AudioSynchronizer::setOverhead(double overhead) {
    CUAssertLog(overhead >= 0, "Overhead cannot be negative.");
   _overhead.store(overhead,std::memory_order_relaxed);
}


void AudioSynchronizer::clearJitter() {
    _jitter.store(-1,std::memory_order_relaxed);
}

bool AudioSynchronizer::onBeat() {
    timestamp_t previous;
    double overhead, jitter;
    Sint32 liveStart, liveDone, waitStart, waitDone;
    {
        std::unique_lock<std::mutex> lk(_mutex);
        previous = _timestamp.load(std::memory_order_relaxed);
        overhead = _overhead.load(std::memory_order_relaxed);
        jitter = _jitter.load(std::memory_order_relaxed);
        liveStart = _liveStart.load(std::memory_order_relaxed);
        liveDone  = _liveDone.load(std::memory_order_relaxed);
        waitStart = _waitStart.load(std::memory_order_relaxed);
        waitDone  = _waitDone.load(std::memory_order_relaxed);
    }

    // Unreliable.  Factor out to read specific values.
    Uint32 size = AudioManager::get()->getReadSize();

    // Compute the time bounds on the audio rendering
    timestamp_t current = cuclock_t::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(current-previous);
    //Sint64 lower = ((elapsed.count()/1000000.0)-overhead-jitter)*getRate();
    //Sint64 upper = ((elapsed.count()/1000000.0)+overhead+jitter)*getRate();
    Sint64 straight = ((elapsed.count()/1000000.0))*getRate();

    
    bool beat = false;
    if (liveStart >= 0 && waitStart >= 0) {
        if (waitDone == -1) {
            // Beat continued
            beat = true;
        } else {
            // Beat end
            //beat = lower < waitDone+size;
            beat = straight < waitDone+size;
        }
    } else if (liveStart == -1 && waitStart >= 0) {
        // Beat start
        //beat = upper > waitStart+size;
        beat = straight > waitStart+size;
    } else if (liveStart >= 0 && waitStart == -1) {
        // Beat end
        //beat = (liveDone == -1 && lower < size) || (lower < liveDone);
        beat = (liveDone == -1 && straight < size) || (straight < liveDone);
    }
    return beat;
}

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
bool AudioSynchronizer::completed() {
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
Uint32 AudioSynchronizer::read(float* buffer, Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    _liveStart.store(_waitStart.load(std::memory_order_relaxed),std::memory_order_relaxed);
    _liveDone.store(_waitDone.load(std::memory_order_relaxed),std::memory_order_relaxed);
    
    // Compute jitter
    Timestamp current;
    double jitter = _jitter.load(std::memory_order_relaxed);
    if (jitter < -0.5f) {
        _jitter.store(-0.5f,std::memory_order_relaxed);
    } else if (jitter >= 0) {
        timestamp_t previous = _timestamp.load(std::memory_order_relaxed);
        auto elapsed  = std::chrono::duration_cast<std::chrono::microseconds>(current.getTime()-previous);
        Sint64 micros = (Sint64)elapsed.count();
        Sint64 expect = (1000000*frames)/getRate();
        // CULog("Expect %d vs got %d",expect,micros);
        micros = (micros > expect ? micros-expect : expect-micros);
        if (micros/1000000.0 > jitter) {
            _jitter.store(micros/1000000.0,std::memory_order_relaxed);
        }
    } else {
        _jitter.store(0,std::memory_order_relaxed);
    }
    
    Uint32 amt = frames;
    if (input == nullptr || _paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,amt*_channels*sizeof(float));
    } else if (input->getChannels() != _channels) {
        std::unique_lock<std::mutex> lk(_mutex);
        amt = std::min(frames,_capacity);
        amt = input->read(_buffer, amt);
        float* output = buffer;
        float* input  = _buffer;
        
        // Extract the sound data
        float gain = _ndgain.load(std::memory_order_relaxed);
        for(int ii = 0; ii < amt; ii++) {
            for(int jj = 0; jj < _channels; jj++) {
                *output = *input*gain;
                output++;
                input++;
            }
            input++;
        }
        
        // Search the carrier signal
        input  = _buffer+_channels;
        Uint32 factor = _channels+1;
        Sint32 waitStart = -1;
        Sint32 waitDone  = -1;
        float thresh = 0.001;
        for(int ii = 0; ii < amt && waitStart == -1; ii++) {
            if (input[ii*factor] < -thresh || input[ii*factor] > thresh) {
                waitStart = (ii == 0 ? ii : ii-1);
            }
        }
        for(int jj = amt-1; jj >= 0 && waitDone == -1; jj--) {
            if (input[jj*factor] < -thresh || input[jj*factor] > thresh) {
                waitDone = jj+1;
            } else if (waitStart != -1) {
                //CULog("%d is %f",jj,input[jj*factor]);
            }
        }
        if (waitDone >= amt-1) { waitDone = -1; }
        //CULog("Data is %d to %d for %d",waitStart,waitDone,amt);
        _waitStart.store(waitStart,std::memory_order_relaxed);
        _waitDone.store(waitDone,std::memory_order_relaxed);
    } else {
        CULog("Channel match");
        std::unique_lock<std::mutex> lk(_mutex);
        amt = input->read(buffer, frames);
        double inputBPM = _inputBPM.load(std::memory_order_relaxed);
        if (inputBPM > 0) {
            Sint32 duration = (60.0/(2*inputBPM))*getRate();
            Sint32 prevbeat = _prevbeat.load(std::memory_order_relaxed);
            if (prevbeat < 0) {
                _waitStart.store(0,std::memory_order_relaxed);
                _waitDone.store(duration < amt ? duration : -1,std::memory_order_relaxed);
                prevbeat = amt;
            } else if (prevbeat < duration) {
                _waitStart.store(0,std::memory_order_relaxed);
                _waitDone.store(duration-prevbeat < amt ? duration-prevbeat : -1,std::memory_order_relaxed);
                prevbeat += amt;
            } else if (prevbeat+amt >= 2*duration) {
                Sint32 pos = 2*duration-prevbeat;
                pos = (pos < 0 ? 0 : pos);
                _waitStart.store(pos,std::memory_order_relaxed);
                _waitDone.store(duration+pos < amt ? duration+pos : -1,std::memory_order_relaxed);
                _prevbeat = pos+amt;
            } else {
                _waitStart.store(-1, std::memory_order_relaxed);
                _waitDone.store( -1, std::memory_order_relaxed);
                prevbeat += amt;
            }
            _prevbeat.store(prevbeat,std::memory_order_relaxed);
        }
    }
    _timestamp.store(current.getTime(),std::memory_order_relaxed);
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
bool AudioSynchronizer::mark() {
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
bool AudioSynchronizer::unmark() {
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
bool AudioSynchronizer::reset() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        bool result = input->reset();
        if (result) {
            _prevbeat.store(-1,std::memory_order_relaxed);
            _jitter.store(-1,std::memory_order_relaxed);
        }
        return result;
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
Sint64 AudioSynchronizer::advance(Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        Sint64 result = input->advance(frames);
        if (result >= 0) {
            _jitter.store(-1,std::memory_order_relaxed);
            
            double inputBPM = _inputBPM.load(std::memory_order_relaxed);
            Sint32 duration = (60.0/inputBPM)*getRate();
            Sint32 prevbeat = _prevbeat.load(std::memory_order_relaxed);
            prevbeat = (prevbeat + result) % duration;
            _prevbeat.store(prevbeat,std::memory_order_relaxed);
        }
        return result;
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
Sint64 AudioSynchronizer::getPosition() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getPosition();
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
Sint64 AudioSynchronizer::setPosition(Uint32 position) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    _waitStart.store(-1,std::memory_order_relaxed);
    _waitDone.store(-1,std::memory_order_relaxed);
    if (input) {
        Sint64 result = input->setPosition(position);
        if (result >= 0) {
            _jitter.store(-1,std::memory_order_relaxed);
            
            double inputBPM = _inputBPM.load(std::memory_order_relaxed);
            Sint32 duration = (60.0/inputBPM)*getRate();
            _prevbeat.store(result % duration,std::memory_order_relaxed);
        }
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
double AudioSynchronizer::getElapsed() const {
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
double AudioSynchronizer::setElapsed(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    _waitStart.store(-1,std::memory_order_relaxed);
    _waitDone.store(-1,std::memory_order_relaxed);
    if (input) {
        double result = input->setElapsed(time);
        if (result >= 0) {
            _jitter.store(-1,std::memory_order_relaxed);
            
            double inputBPM = _inputBPM.load(std::memory_order_relaxed);
            Sint32 duration = (60.0/inputBPM)*getRate();
            Sint32 frame = result*getRate();
            _prevbeat.store(frame % duration,std::memory_order_relaxed);
        }
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
double AudioSynchronizer::getRemaining() const {
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
double AudioSynchronizer::setRemaining(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    _waitStart.store(-1,std::memory_order_relaxed);
    _waitDone.store(-1,std::memory_order_relaxed);
    if (input) {
        double result = input->setRemaining(time);
        if (result >= 0) {
            _jitter.store(-1,std::memory_order_relaxed);
            
            double current = input->getElapsed();
            double inputBPM = _inputBPM.load(std::memory_order_relaxed);
            Sint32 duration = (60.0/inputBPM)*getRate();
            Sint32 frame = current*getRate();
            _prevbeat.store(frame >= 0 ? frame % duration : -1,std::memory_order_relaxed);
        }
    }
    return -1;
}

