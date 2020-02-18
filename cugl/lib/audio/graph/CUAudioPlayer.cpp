//
//  CUAudioPlayer.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides the interface for representing an playback instance
//  of an audio sample.  A player is attached to a single sound asset, though
//  it may be disposed and reinitialized to contain another asset (in order to
//  to limit object creation).  To rapidly swap between sounds, or to play them
//  in order, this node should be combined with the AudioScheduler node.
//
//  This class is necessary bcause samples may have multiple instances,
//  particularly if they are playing simultaneously.  The complexity of stream
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
#include <cugl/audio/CUAudioManager.h>
#include <cugl/audio/CUAudioSample.h>
#include <cugl/audio/graph/CUAudioPlayer.h>
#include <cugl/base/CUApplication.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUTimestamp.h>
#include <cugl/math/dsp/CUDSPMath.h>
#include <cugl/audio/codecs/cu_codecs.h>

using namespace cugl::audio;
using namespace cugl;

#pragma mark Constructors
/**
 * Creates a degenerate audio player with no associated source.
 *
 * The player has no channels or source file, so read options will do nothing.
 * The player must be initialized to be used.
 */
AudioPlayer::AudioPlayer() : AudioNode(),
_offset(0),
_marked(0),
_buffer(nullptr),
_decoder(nullptr),
_source(nullptr),
_chunker(nullptr),
_chklimt(0),
_chklast(0),
_chksize(0),
_dirty(false) {
    _classname = "AudioPlayer";
}

/**
 * Initializes a player for the given audio sample.
 *
 * The player will be set for a single playthrough of this given sample.
 * However the player may be reset or reinitialized.
 *
 * @param sample    the audio sample to be played.
 *
 * @return true if initialization was successful
 */
bool AudioPlayer::init(const std::shared_ptr<AudioSample>& source) {
    if (AudioNode::init(source->getChannels(),source->getRate())) {
        _source = source;
        _buffer = source->getBuffer();
        _dirty  = false;
        
        // TODO: Require manager active and access buffer from it.
        _decoder = source->getDecoder();
        if (source->isStreamed() && _decoder != nullptr) {
            Uint32 channels = _decoder->getChannels();
            _chksize  = _decoder->getPageSize();
            _chklimt  = _chksize;
            _chklast  = _chksize;
            _chunker  = (float*)malloc(_chksize*channels*sizeof(float));
            std::memset(_chunker,0,_chksize*channels*sizeof(float));
        }
        return true;
    }
    return false;
}

/**
 * Disposes any resources allocated for this player
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioPlayer::dispose() {
    if (_booted) {
        AudioNode::dispose();
        _source = nullptr;
        _decoder = nullptr;
        _offset.store(0);
        _marked.store(0);
        _buffer  = nullptr;
        _calling.store(false);
        _callback = nullptr;
        _chksize = 0;
        _chklimt = 0;
        _chklast = 0;
        if (_chunker) {
            free(_chunker);
            _chunker = nullptr;
        }
    }
}

#pragma mark -
#pragma mark Read Forward Access
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
Uint32 AudioPlayer::read(float* buffer, Uint32 frames) {
    if (_paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,frames*sizeof(float)*_channels);
        return frames;
    }
    
    _polling.store(true);
    Uint64 off = _offset.load(std::memory_order_acquire);
    if (!_source || off >= _source->getLength()) {
        return 0;
    }
    
    Uint32 amt = frames;
    if (_buffer) {
        float* input  = _buffer;
        input += off*_source->getChannels();
    
        amt = (Uint32)(off+amt > _source->getLength() ? _source->getLength()-off : amt);
        std::memcpy(buffer,input,sizeof(float)*amt*_source->getChannels());
    } else {
        if (_dirty.load(std::memory_order_acquire)) {
            scan(off);
            _dirty.store(false,std::memory_order_relaxed);
        }
        
        Uint32 remnant  = frames;
        Uint32 channels = _decoder->getChannels();
        bool okay = true;
        while (okay && remnant) {
            if (_chklast >= _chklimt) {
                _chklimt = _decoder->pagein(_chunker);
                _chklast = 0;
            }
            Uint32 avail = std::min((_chklimt-_chklast),remnant);
            if (avail == 0) {
                okay = false;
            } else {
                std::memcpy(buffer+(frames-remnant)*channels, _chunker+_chklast*channels, avail*channels*sizeof(float));
                remnant  -= avail;
                _chklast += avail;
            }
        }
        amt -= remnant;
    }

    dsp::DSPMath::scale(buffer,_ndgain.load(std::memory_order_relaxed),buffer,amt*_channels);
    _offset.store(off+amt,std::memory_order_release);
    _polling.store(false);
    Timestamp end;
    return amt;
}

/**
 * Returns true if this audio node has no more data.
 *
 * A completed audio node is one that will return 0 (no frames read) on
 * subsequent threads read.
 *
 * @return true if this audio node has no more data.
 */
bool AudioPlayer::completed() {
    return _offset.load(std::memory_order_relaxed) >= _source->getLength();
}

/**
 * Marks the current read position in the audio steam.
 *
 * This method is used by {@link reset()} to determine where to restore
 * the read position.
 *
 * @return true if the read position was marked.
 */
bool AudioPlayer::mark() {
    _marked.store(_offset.load(std::memory_order_relaxed),std::memory_order_relaxed);
    return true;
}

/**
 * Clears the current marked position.
 *
 * Clearing the mark in a player is equivelent to setting the mark at
 * the beginning of the audio asset.  Future calls to {@link reset()}
 * will return to the start of the audio stream.
 *
 * @return true if the read position was cleared.
 */
bool AudioPlayer::unmark() {
    _marked.store(0,std::memory_order_relaxed);
    return true;
}

/**
 * Resets the read position to the marked position of the audio stream.
 *
 * If no mark is set, this will reset to the player to the beginning of
 * the audio sample.
 *
 * @return true if the read position was moved.
 */
bool AudioPlayer::reset() {
    _offset.store(_marked.load(std::memory_order_relaxed),std::memory_order_relaxed);
    _dirty.store(true);
    return true;
}

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
Sint64 AudioPlayer::advance(Uint32 frames) {
    return setPosition((Uint32)_offset.load(std::memory_order_relaxed)+frames);
}


#pragma mark -
#pragma mark Random Access
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
Sint64 AudioPlayer::setPosition(Uint32 position) {
    Uint64 off  = position > _source->getLength() ? _source->getLength() : position;
    _offset.store(off, std::memory_order_release);
    _dirty.store(true);
    return off;
}

/**
 * Returns the current frame position of this audio node
 *
 * The value returned will always be the absolute frame position regardless
 * of the presence of any marks.
 *
 * @return the current frame position of this audio node.
 */
Sint64 AudioPlayer::getPosition() const {
    return (Sint64)_offset.load(std::memory_order_relaxed);
}

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
double AudioPlayer::setElapsed(double time) {
    double result;
    Uint64 off;
    if (time <= 0) {
        result = 0.0;
        off = 0;
    } else {
        off = (Uint64)(time*_source->getRate());
        off = off > _source->getLength() ? _source->getLength() : off;
        result = off/_source->getRate();
    }
    _offset.store(off, std::memory_order_relaxed);
    _dirty.store(true);
    return result;
}

/**
 * Returns the elapsed time in seconds.
 *
 * The value returned is always measured from the start of the steam,
 * regardless of the presence of any marks.
 *
 * @return the elapsed time in seconds.
 */
double AudioPlayer::getElapsed() const {
    Uint64 offset = _offset.load(std::memory_order_relaxed);
    return (double)offset/(double)_source->getRate();
}

/**
 * Returns the remaining time in seconds.
 *
 * The remaining time is duration from the current read position to the
 * end of the sample. It is not effected by any fade-out.
 *
 * @return the remaining time in seconds.
 */
double AudioPlayer::getRemaining() const {
    Uint64 offset = _offset.load(std::memory_order_relaxed);
    offset = _source->getLength()-offset;
    return (double)offset/(double)_source->getRate();
}

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
double AudioPlayer::setRemaining(double time) {
    double result;
    Uint64 off;
    if (time >= _source->getDuration()) {
        result = _source->getDuration();
        off = 0;
    } else {
        off = (Uint64)(time*_source->getRate());
        off = off > _source->getLength() ? 0 : _source->getLength()-off;
        result = (_source->getLength()-off)/_source->getRate();
    }
    _offset.store(off, std::memory_order_relaxed);
    _dirty.store(true);
    return result;
}


#pragma mark -
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
void AudioPlayer::scan(Uint64 frame) {
    Uint32 page = (Uint32)(frame/_chksize);
    _decoder->setPage(page);
    _chklimt = (Uint32)_decoder->pagein(_chunker);
    _chklast = (Uint32)(_chklimt == 0 ? _chksize : frame % _chksize);
}
