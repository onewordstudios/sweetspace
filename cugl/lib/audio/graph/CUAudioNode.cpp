//
//  CUAudioNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the base class for a node in the audio graph.  While
//  you will never use this class directly, it provides the base features that
//  allow for a polymorphic audio graph.
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
//  coordinateion algorithms only assume coordination between two threads.
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
#include <cugl/audio/graph/CUAudioNode.h>
#include <cugl/audio/CUAudioManager.h>
#include <cugl/base/CUApplication.h>
#include <cugl/util/CUDebug.h>
#include <sstream>

using namespace cugl::audio;

#pragma mark Static Attributes
/** The default number of channels for an audio graph node */
const Uint32 AudioNode::DEFAULT_CHANNELS = 2;

// We had to make this 48000 for the iPhone (for now)
/** The default sampling frequency for an audio graph node */
const Uint32 AudioNode::DEFAULT_SAMPLING = 48000;

#pragma mark -
#pragma mark Constructors

/**
 * Creates a degenerate audio graph node
 *
 * The node has no channels, so read options will do nothing. The node must
 * be initialized to be used.
 */
AudioNode::AudioNode() : _channels(0), _sampling(0) {
    _localname  = "";
    _classname  = "AudioNode";
    _callback = nullptr;
    _calling  = false;
    _hashOfName = 0;
    _ndgain = 1.0f;
    _paused = false;
    _polling = false;
    _booted = false;
    _tag = -1;
}

/**
 * Deletes the audio graph node, disposing of all resources
 */
AudioNode::~AudioNode() { }

/**
 * Initializes the node with default stereo settings
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
bool AudioNode::init() {
    return init(DEFAULT_CHANNELS,DEFAULT_SAMPLING);
}

/**
 * Initializes the node with the given number of channels and sample rate
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
bool AudioNode::init(Uint8 channels, Uint32 rate) {
    if (_booted) {
        CUAssertLog(false,"This node has already been initialized");
        return false;
    } else if (!AudioManager::get()) {
        CUAssertLog(false,"Attempt to allocate a node without an active AudioManager");
        return false;
    }
    _channels = channels;
    _sampling = rate;
    _booted = true;
    return true;
}

/**
 * Disposes any resources allocated for this node
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioNode::dispose() {
    _booted = false;
    _paused.store(true);
    _channels  = 0;
    _sampling  = 0;
    _callback = nullptr;
    _calling.store(false);
    _ndgain.store(1.0f);
    _polling.store(false);
    _paused.store(false);
    _hashOfName = 0;
    _localname = "";
    _tag = -1;
}

#pragma mark -
#pragma mark Node Attributes
/**
 * Returns the current (volume) gain of this node.
 *
 * During processing, the sample data is multiplied by the gain.  This value
 * is generally between 0 and 1, though it may be any float.  The result for
 * values outside the range [0,1] are undefined.
 *
 * @return the current (volume) gain of this node.
 */
float AudioNode::getGain() {
    return _ndgain.load(std::memory_order_relaxed);
}

/**
 * Sets the current (volume) gain of this node.
 *
 * During processing, the sample data is multiplied by the gain.  This value
 * is generally between 0 and 1, though it may be any float.  The result for
 * values outside the range [0,1] are undefined.
 *
 * @param gain  the (volume) gain of this node.
 */
void AudioNode::setGain(float gain) {
    _ndgain.store(gain, std::memory_order_relaxed);
}

/**
 * Sets a string that is used to identify the node.
 *
 * This name is primarily used in debugging. For best results, a name should be
 * unique within an audio graph. It is empty if undefined.
 *
 * @param name  A string that is used to identify the node.
 */
void AudioNode::setName(const std::string& name) {
    _localname = name;
    _hashOfName = std::hash<std::string>()(_localname);
}

/**
 * Returns a string representation of this audio node for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this node for debugging purposes.
 */
std::string AudioNode::toString(bool verbose) const {
    std::stringstream ss;
    if (verbose) {
        ss << "cugl::audio::" << _classname;
    }
    ss << "(name:" << _localname;
    if (_tag >= 0) {
        ss << "[" << _tag << "]";
    }
    ss << ")";
    return ss.str();
}

#pragma mark -
#pragma mark Playback Control
/**
 * Returns the callback function for this node
 *
 * The callback function is called whenever an action takes places. Actions
 * are subclass dependent.  See the class documentation for what callbacks
 * are supported.
 *
 * @return the callback function for this node
 */
AudioNode::Callback AudioNode::getCallback() {
    return _callback;
}

/**
 * Sets the callback function for this node
 *
 * The callback function is called whenever an action takes places. Actions
 * are subclass dependent.  See the class documentation for what callbacks
 * are supported.
 *
 * @param callback  the callback function for this node
 */
void AudioNode::setCallback(Callback callback) {
    _callback = callback;
    _calling.store(callback != nullptr, std::memory_order_release);
}

/**
 * Invokes the callback functions for the given action.
 *
 * The callback function can be changed at any given time while the
 * audio is running.  While the callback gets information from the audio
 * thread, we want to execute it in the main thread, where we do not have
 * to worry about performance issues (as much).
 *
 * This means that callback execution is delayed and the callback function
 * might change during that delay.  This is a wrapper to ensure that this
 * potential race condition happens gracefully and does not have any
 * unexpected side effects.
 */
void AudioNode::notify(const std::shared_ptr<AudioNode>& node, AudioNode::Action action) {
    Application::get()->schedule([=] {
        if (_callback) {
            _callback(node,action);
        }
        return false;
    });
}

/**
 * Returns true if this node is currently paused
 *
 * @return true if this node is currently paused
 */
bool AudioNode::isPaused() {
    return _paused.load(std::memory_order_relaxed);
}

/**
 * Pauses this node, preventing any data from being read.
 *
 * If the node is already paused, this method has no effect.
 *
 * @return true if the node was successfully paused
 */
bool AudioNode::pause() {
    return !_paused.exchange(true);
}

/**
 * Resumes this previously paused node, allowing data to be read.
 *
 * If the node is not paused, this method has no effect.
 *
 * @return true if the node was successfully resumed
 */
bool AudioNode::resume() {
    return _paused.exchange(false);
}

#pragma mark -
#pragma mark Graph Methods
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
Uint32 AudioNode::read(float* buffer, Uint32 frames) {
    std::memset(buffer, 0, sizeof(float)*frames*_channels);
    return frames;
}
