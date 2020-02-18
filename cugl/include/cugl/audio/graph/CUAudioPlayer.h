//
//  CUAudioPlayer.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the interface for representing an playback instance
//  of an audio sample.  A player is attached to a single sound asset, though
//  it may be disposed and reinitialized to contain another asset (in order to
//  to limit object creation). To rapidly swap between sounds, or to play them
//  in order, this node should be combined with the AudioScheduler node.
//
//  This class is necessary bcause samples may have multiple instances,
//  particularly if they are playing simultaneously. The complexity of stream
//  decoding forces us to put decoding state in these classes and not in the
//  asset file (particularly when there are multiple streams).
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
#ifndef __CU_AUDIO_PLAYER_H__
#define __CU_AUDIO_PLAYER_H__
#include <SDL/SDL.h>
#include <cugl/audio/CUAudioSample.h>
#include "CUAudioNode.h"
#include <functional>
#include <string>
#include <atomic>

// TODO: Move fade-in/fade-out support to new class
namespace  cugl {
    namespace audio {
 
#pragma mark -
#pragma mark Base Player
/**
 * This class represents a playback instance for AudioPlayback.
 *
 * A single sound asset may have multiple instances playing simultaneously,
 * particularly in the case of sound effects. This node allows us to keep
 * the playback distinct for each instance.
 *
 * A player can be reset and can jump to anywhere in the sounds.  However, it
 * cannot be set to loop or sequence two sound assets together.  To do that
 * you should combine this node with {@link AudioScheduler}.
 *
 * This class is medium-weight, and has a lot of buffers to support stream
 * decoding (when appropriate).  In practice, it may be best to create a
 * memory pool of preallocated players (which are reinitialized) than to
 * construct them on the fly.
 *
 * A player is always associated with a node in the audio graph. As such, it
 * should only be accessed in the main thread.  In addition, no methods marked
 * as AUDIO THREAD ONLY should ever be accessed by the user. The only exception
 * to this rule is by another (custom) audio graph node in its audio thread
 * methods.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 * Fade in/out and scheduling have been refactored into other nodes to provide
 * proper audio patch support.
 */
class AudioPlayer : public AudioNode {
protected:
    /** The original source for this instance */
    std::shared_ptr<AudioSample> _source;
    /** The decoder for the current asset */
    std::shared_ptr<AudioDecoder> _decoder;

    /** The current read position */
    std::atomic<Uint64> _offset;
    /** The last marked position (starts at 0) */
    std::atomic<Uint64> _marked;
    
    /** A reference to the underlying data buffer (IN-MEMORY ACCESS) */
    float* _buffer;
    
    // Streaming support
    /** A buffer for storing each chunk as we need it */
    float* _chunker;
    /** The size of a single chunk in frames */
    Uint32 _chksize;
    /** The pointer to the last available frame in the chunk */
    Uint32 _chklimt;
    /** The number of the last read frame in the chunk */
    Uint32 _chklast;
        
    /** Whether or not we need to reposition (STREAMING ACCESS) */
    std::atomic<bool> _dirty;

public:
#pragma mark Constructors
    /**
     * Creates a degenerate audio player with no associated source.
     *
     * The player has no channels or source file, so read options will do nothing.
     * The player must be initialized to be used.
     */
    AudioPlayer();
    
    /**
     * Deletes this audio player, disposing of all resources.
     */
    ~AudioPlayer() { dispose(); }
 
    /**
     * Initializes a player for the given audio sample.
     *
     * The player will be set for a single playthrough of this given sample.
     * However the player may be reset or reinitialized.
     *
     * @param source	The audio sample to be played.
     *
     * @return true if initialization was successful
     */
    bool init(const std::shared_ptr<AudioSample>& source);
    
    /**
     * Disposes any resources allocated for this player
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
    /**
     * Returns a newly allocated player for the given audio sample.
     *
     * The player will either be streamed or buffered, depending on the type
     * of audio sample.  We do not require separate players for each type.
     *
     * @param sample    the audio sample to be played.
     *
     * @return a newly allocated player for the given audio sample.
     */
    static std::shared_ptr<AudioPlayer> alloc(const std::shared_ptr<AudioSample>& sample) {
        std::shared_ptr<AudioPlayer> result = std::make_shared<AudioPlayer>();
        return (result->init(sample) ? result : nullptr);
    }

    /**
     * Returns the source for this instance
     *
     * @return the source for this instance
     */
    std::shared_ptr<AudioSample> getSource() { return _source; }

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
     * Returns true if this audio node has no more data.
     *
     * A completed audio node is one that will return 0 (no frames read) on
     * subsequent threads read.
     *
     * @return true if this audio node has no more data.
     */
    virtual bool completed() override;

    /**
     * Marks the current read position in the audio steam.
     *
     * This method is used by {@link reset()} to determine where to restore
     * the read position.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * Clearing the mark in a player is equivelent to setting the mark at
     * the beginning of the audio asset.  Future calls to {@link reset()}
     * will return to the start of the audio stream.
     *
     * @return true if the read position was cleared.
     */
    virtual bool unmark() override;

    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * If no mark is set, this will reset to the player to the beginning of
     * the audio sample.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;

    /**
     * Advances the stream by the given number of frames.
     *
     * This method only advances the read position, it does not actually
     * read data into a buffer.
     *
     * @param frames    The number of frames to advace
     *
     * @return the actual number of frames advanced; -1 if not supported
     */
    virtual Sint64 advance(Uint32 frames) override;
    
    /**
     * Returns the current frame position of this audio node
     *
     * The value returned will always be the absolute frame position regardless
     * of the presence of any marks.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * The value set will always be the absolute frame position regardless
     * of the presence of any marks.
     *
     * @param position  the current frame position of this audio node.
     *
     * @return the new frame position of this audio node.
     */
    virtual Sint64 setPosition(Uint32 position) override;
    
    /**
     * Returns the elapsed time in seconds.
     *
     * The value returned is always measured from the start of the steam,
     * regardless of the presence of any marks.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;

    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * The value returned is always measured from the start of the steam,
     * regardless of the presence of any marks.
     *
     * @param time  The elapsed time in seconds.
     *
     * @return the new elapsed time in seconds.
     */
    virtual double setElapsed(double time) override;

    /**
     * Returns the remaining time in seconds.
     *
     * The remaining time is duration from the current read position to the
     * end of the sample.  It is not effected by any fade-out.
     *
     * @return the remaining time in seconds.
     */
    virtual double getRemaining() const override;
    
    /**
     * Sets the remaining time in seconds.
     *
     * This method will move the read position so that the distance between
     * it and the end of the same is the given number of seconds.
     *
     * @param time  The remaining time in seconds.
     *
     * @return the new remaining time in seconds.
     */
    virtual double setRemaining(double time) override;
    
private:
#pragma mark Stream Decoding
    /**
     * Decodes the audio stream up to the given position.
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * If the frame is longer than the stream length, it goes to the end of
     * the stream.
     *
     * @param frame    The absolute frame to skip to
     */
    void scan(Uint64 frame);
};

    }
}

#endif /* __CU_AUDIO_PLAYER_H__ */
