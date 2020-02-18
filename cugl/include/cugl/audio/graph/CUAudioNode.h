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
//  Version: 11/20/18
//
#ifndef __CU_AUDIO_NODE_H__
#define __CU_AUDIO_NODE_H__
#include <SDL/SDL.h>
#include <atomic>
#include <memory>
#include <functional>
#include <string>

namespace cugl {
    
    namespace audio {
    
/**
 * This class is a node in the audio graph
 *
 * This class provides the base of any nodes in the audio graph.  All nodes
 * work on a pull model, where reading from a node reads from all of its input
 * nodes (if any exist).
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
 * user. The only exception to this rule is by another (custom) audio graph
 * node in its audio thread methods.
 *
 * For polymorphic reasons, this class has several optional methods.  These
 * methods are not guaranteed to be supported in all classes.  However, if a
 * method is not supported, it returns a simple error value and will not
 * crash the program.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioNode : public std::enable_shared_from_this<AudioNode> {
public:
    /**
     * An enumeration of possible node actions.
     *
     * These are possible things that that can happen to audio, and which we
     * might want to be notified about.  Not all actions are supported by
     * all nodes.  Indeed, this enumeration provides a collection of all
     * possible actions supported by subclasses of AudioNode.
     *
     * This list is not comprehensive and can change at any time.  Never use
     * the numeric values directly.
     */
    enum Action : int {
        /** This audio node has completed normally */
        COMPLETE  = 0,
        /** This audio node completed via an abnormal interruption */
        INTERRUPT = 1,
        /** An audio node completed as the result of a fade-out. */
        FADE_OUT  = 2,
        /** The audio node has completed an initial fade-in. */
        FADE_IN   = 3,
        /** The audio node has paused after a temporary fade-out. */
        FADE_DIP  = 4,
        /** This audio node has reset and looped back to the beginning */
        LOOPBACK  = 5
    };
    
    /**
     * @typedef Callback
     *
     * This type represents a callback function for {@link AudioNode}
     *
     * The callback is executed when an audio action occurs. The second
     * parameter in the callback is the type of action that took place.  See
     * the description of each audio node 
     *
     * @param node  The audio player for this callback.
     * @param type  The type of action completed.
     */
    typedef std::function<void (const std::shared_ptr<AudioNode>& node, Action type)> Callback;

    /** The class name for the specific subclass */
    std::string _classname;

#pragma mark Values
protected:
    /** The number of channels output by this node */
    Uint8  _channels;
    
    /** The sampling rate (frequency) of this node */
    Uint32 _sampling;

    /** Whether or not the output node has been initialized */
    bool _booted;
    
    /** The (volume) gain of this node */
    std::atomic<float> _ndgain;
    
    /** Whether or not this node is currently paused */
    std::atomic<bool> _paused;

    /** Whether or not this node is in an active poll */
    std::atomic<bool> _polling;
    
    /** The callback function for when a node finishes */
    Callback _callback;
    /** An atomic to mark that the callback is active (to give lock-free safety) */
    std::atomic<bool> _calling;

    /** An identifying integer */
    Sint32 _tag;
    
    /**
     * A descriptive, identifying tag.
     *
     * This is used to identify a node for debugging purposes.
     */
    std::string _localname;
    
    /** The class name for the specific subclass */
    //std::string _classname;
    
    /**
     * A cached has value of _name.
     *
     * This value is used to speed up look-ups by string.
     */
    size_t _hashOfName;
    
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
    void notify(const std::shared_ptr<AudioNode>& node, Action action);
    
#pragma mark -
#pragma mark Static Attributes
public:
    /** The default number of channels for an audio node */
    const static Uint32 DEFAULT_CHANNELS;
    
    /** The default sampling frequency for an audio node */
    const static Uint32 DEFAULT_SAMPLING;
    
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a degenerate audio graph node
     *
     * The node has no channels, so read options will do nothing. The node must
     * be initialized to be used.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
     * the heap, use one of the static constructors instead.
     */
    AudioNode();
    
    /**
     * Deletes the audio graph node, disposing of all resources
     */
    virtual ~AudioNode();
    
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
    virtual bool init();
    
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
    virtual bool init(Uint8 channels, Uint32 rate);
    
    /**
     * Disposes any resources allocated for this node
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose();
    
#pragma mark -
#pragma mark Node Attributes
    /**
     * Returns the number of output channels of this node
     *
     * The standard values are 1 for mono or 2 for stereo.  However, we allow
     * greater values for surround sound. The semantics of each channel are
     * system dependent.
     *
     * @return the number of output channels of this node
     */
    Uint8 getChannels() const { return _channels; }
    
    /**
     * Returns the sample rate of this node
     *
     * The sample rate is that of the output produced by the {@link read} methods.
     * If this node reads from other nodes, it may or may not agree with their
     * frequency, particularly if the effect is a resampler.
     *
     * @return the sample rate of this node
     */
    Uint32 getRate() const { return _sampling; }
    
    /**
     * Returns the current (volume) gain of this node.
     *
     * During processing, the sample data is multiplied by the gain.  This value
     * is generally between 0 and 1, though it may be any float.  The result for
     * values outside the range [0,1] are undefined.
     *
     * @return the current (volume) gain of this node.
     */
    float getGain();
    
    /**
     * Sets the current (volume) gain of this node.
     *
     * During processing, the sample data is multiplied by the gain.  This value
     * is generally between 0 and 1, though it may be any float.  The result for
     * values outside the range [0,1] are undefined.
     *
     * @param gain  the (volume) gain of this node.
     */
    virtual void setGain(float gain);
    
    /**
     * Returns a string that is used to identify the node.
     *
     * This name is primarily used in debugging. For best results, a name should be
     * unique within an audio graph. It is empty if undefined.
     *
     * @return a string that is used to identify the node.
     */
    const std::string& getName() const { return _localname; }
    
    /**
     * Sets a string that is used to identify the node.
     *
     * This name is primarily used in debugging. For best results, a name should be
     * unique within an audio graph. It is empty if undefined.
     *
     * @param name  A string that is used to identify the node.
     */
    void setName(const std::string& name);

    /**
     * Sets a string that is used to identify the node.
     *
     * This name is primarily used in debugging. For best results, a name should be
     * unique within an audio graph. It is empty if undefined.
     *
     * @param name  A string that is used to identify the node.
     */
    void setName(const char* name) {
        setName(std::string(name));
    }
    
    /**
     * Returns an integer that is used to identify the node.
     *
     * This tage is primarily used for debugging and/or hashing.  For best
     * results, a name should be unique within an audio graph. It is -1 if
     * undefined.
     *
     * @return an integer that is used to identify the node.
     */
    Sint32 getTag() const { return _tag; }
    
    /**
     * Sets an integer that is used to identify the node.
     *
     * This tage is primarily used for debugging and/or hashing.  For best
     * results, a name should be unique within an audio graph. It is -1 if
     * undefined.
     *
     * @param tag   An integer that is used to identify the node.
     */
    void setTag(Sint32 tag) { _tag = tag; }

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
    virtual std::string toString(bool verbose = false) const;
    
    /** Cast from a Node to a string. */
    operator std::string() const { return toString(); }
    
#pragma mark -
#pragma mark Playback Controls
    /**
     * Returns the callback function for this node
     *
     * The callback function is called whenever an action takes places. Actions
     * are subclass dependent.  See the class documentation for what callbacks
     * are supported.
     *
     * @return the callback function for this node
     */
    Callback getCallback();
    
    /**
     * Sets the callback function for this node
     *
     * The callback function is called whenever an action takes places. Actions
     * are subclass dependent.  See the class documentation for what callbacks
     * are supported.
     *
     * @param callback  the callback function for this node
     */
    void setCallback(Callback callback);
    
    /**
     * Returns true if this node is currently paused
     *
     * @return true if this node is currently paused
     */
    virtual bool isPaused();
    
    /**
     * Pauses this node, preventing any data from being read.
     *
     * If the node is already paused, this method has no effect. Pausing will
     * not go into effect until the next render call in the audio thread.
     *
     * @return true if the node was successfully paused
     */
    virtual bool pause();
    
    /**
     * Resumes this previously paused node, allowing data to be read.
     *
     * If the node is not paused, this method has no effect.
     *
     * @return true if the node was successfully resumed
     */
    virtual bool resume();
    
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
    virtual bool completed() { return false; }
    
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
    virtual Uint32 read(float* buffer, Uint32 frames);

#pragma mark -
#pragma mark Optional Methods
    /**
     * Marks the current read position in the audio steam.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return true if the mark is successful/supported and false
     * otherwise.
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
    virtual bool mark() { return false; }

    /**
     * Clears the current marked position.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return true if the clear is successful/supported and false
     * otherwise.
     *
     * If the method {@link mark()} started recording to a buffer (such as
     * with {@link AudioInput}), this method will stop recording and release
     * the buffer.  When the mark is cleared, {@link reset()} may or may not
     * work depending upon the specific node.
     *
     * @return true if the read position was cleared.
     */
    virtual bool unmark() { return false; }
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return true if the reset is successful/supported and false
     * otherwise.
     *
     * When no {@link mark()} is set, the result of this method is node
     * dependent.  Some nodes (such as {@link AudioPlayer}) will reset to the
     * beginning of the stream, while others (like {@link AudioInput}) only
     * support a rest when a mark is set. Pay attention to the return value of
     * this method to see if the call is successful.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() { return false; }
    
    /**
     * Advances the stream by the given number of frames.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return the number of frames advanced if it is successful/supported
     * and -1 otherwise.
     *
     * This method only advances the read position, it does not actually
     * read data into a buffer. This method is generally not supported
     * for nodes with real-time input like {@link AudioInput}.
     *
     * @param frames    The number of frames to advance
     *
     * @return the actual number of frames advanced; -1 if not supported
     */
    virtual Sint64 advance(Uint32 frames) { return -1; }
    
    /**
     * Returns the current frame position of this audio node
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return the current position if it is successful/supported and
     * -1 otherwise.
     *
     * In some nodes like {@link AudioInput}, this method is only supported
     * if {@link mark()} is set.  In that case, the position will be the
     * number of frames since the mark. Other nodes like {@link AudioPlayer}
     * measure from the start of the stream.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const { return -1; }
    
    /**
     * Sets the current frame position of this audio node.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return the new position if it is successful/supported and
     * -1 otherwise.
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
    virtual Sint64 setPosition(Uint32 position) { return -1; }
    
    /**
     * Returns the elapsed time in seconds.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return the elapsed time in seconds if it is successful/supported
     * and -1 otherwise.
     *
     * In some nodes like {@link AudioInput}, this method is only supported
     * if {@link mark()} is set.  In that case, the times will be the
     * number of seconds since the mark. Other nodes like {@link AudioPlayer}
     * measure from the start of the stream.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const { return -1; }
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return the new elapsed time if it is successful/supported and
     * -1 otherwise.
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
    virtual double setElapsed(double time) { return -1; }

    /**
     * Returns the remaining time in seconds.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return the elapsed time in seconds if it is successful/supported
     * and -1 otherwise.
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
    virtual double getRemaining() const { return -1; }
    
    /**
     * Sets the remaining time in seconds.
     *
     * OPTIONAL METHOD: This method is not supported by all node subclasses.
     * It will return the remaining time if it is successful/supported and
     * -1 otherwise.
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
    virtual double setRemaining(double time) { return -1; }
    
};
    }
}

#endif /* __CU_AUDIO_NODE_H__ */
