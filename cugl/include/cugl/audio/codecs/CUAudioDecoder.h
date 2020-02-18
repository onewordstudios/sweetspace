//
//  CUAudioDecoder.h
//  Cornell University Game Library (CUGL)
//
//  This is the base class for an audio decoder.  An audio decoder converts a
//  a binary file into a pageable PCM data stream.  This unifies the API for
//  all of the supported audio codes (WAV, MP3, OGG, FLAC)
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
#ifndef __CU_AUDIO_DECODER_H__
#define __CU_AUDIO_DECODER_H__
#include <SDL/SDL.h>
#include <string>

namespace cugl {
    namespace audio {

/**
 * This class abstracts an audio codec for decoding.
 *
 * By providing a single interface for all audio codecs, we make it easy to
 * to support multiple file types.  Currently we support four file types:
 * WAV (including ADPCM encodings), MP3, Ogg (Vorbis), and Flac.  As a general
 * rule, we prefer WAV for sound effects and Ogg for music.
 *
 * A decoder breaks up the sound into pages for streaming access.  While some
 * codecs refer to pages as "frames', we reserve that term for groups of 
 * samples at a single moment in time, as is the case for the rest of the API.
 *
 * A decoder is NOT thread safe.  If a decoder is used by an audio thread, then
 * it should not be accessed directly in the main thread, and vice versa.
 */
class AudioDecoder {
protected:
    /** The number of channels in this sound source (max 32) */
    Uint8  _channels;
    
    /** The sampling rate (frequency) of this sound source */
    Uint32 _rate;
    
    /** The number of frames in this sounds source */
    Uint64 _frames;
    
    /** The source for this buffer (may be empty) */
    std::string _file;

    /** The size of a decoder chunk */
    Uint32 _pagesize;
    
    /** The current page in the stream */
    Uint64 _currpage;
    /** The previous page in the stream */
    Uint64 _lastpage;
    
public:
    /**
     * Creates an initialized audio decoder
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    AudioDecoder();
    
    /**
     * Deletes this decoder, disposing of all resources.
     */
    ~AudioDecoder() {}
    
    /**
     * Initializes a new decoder for the given file.
     *
     * This initializer is an abstract method in the base class. The file is
     * either streamed or completely read into memory according to the specific
     * implementation.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    virtual bool init(const char* file) {
        return init(std::string(file));
    }

    /**
     * Initializes a new decoder for the given file.
     *
     * This initializer is an abstract method in the base class. The file is
     * either streamed or completely read into memory according to the specific
     * implementation.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    virtual bool init(const std::string& file) = 0;

    /**
     * Deletes the decoder resources and resets all attributes.
     *
     * This will close the associated file. You must reinitialize the decoder
     * to use it.
     */
    virtual void dispose() = 0;

    
#pragma mark Attributes
    /**
     * Returns the length of this sound source in seconds.
     *
     * The accuracy of this method depends on the specific implementation.
     *
     * @return the length of this sound source in seconds.
     */
    double getDuration() const { return (double)_frames/(double)_rate; }
    
    /**
     * Returns the sample rate of this sound source.
     *
     * @return the sample rate of this sound source.
     */
    Uint32 getSampleRate() const { return _rate; }
    
    /**
     * Returns the frame length of this sound source.
     *
     * The frame length is the duration times the sample rate.
     *
     * @return the frame length of this sound source.
     */
    Uint64 getLength() const { return _frames; }
    
    /**
     * Returns the number of channels used by this sound source
     *
     * A value of 1 means mono, while 2 means stereo. Depending on the file
     * format, other channels are possible. For example, 6 channels means
     * support for 5.1 surround sound.
     *
     * We support up to 32 possible channels.
     *
     * @return the number of channels used by this sound asset
     */
    Uint32 getChannels() const { return _channels; }
    
    /**
     * Returns the file for this audio source
     *
     * This value is the empty string if there was no source file.
     *
     * @return the file for this audio source
     */
    std::string getFile() const { return _file; }

    /**
     * Returns the number of frames in a single page of data
     *
     * When multiplied by the number of channels, this gives the number of
     * samples read per page
     *
     * @return the number of frames in a single page of data
     */
    Uint32 getPageSize() const { return _pagesize; }

    
#pragma mark Decoding
    /**
     * Returns true if there are still data to be read by the decoder
     *
     * This value will return false if the decoder is at the end of the file
     *
     * @return true if there are still data to be read by the decoder
     */
    bool ready() {
        return (_currpage < getPageCount());
    }
    
    /**
     * Reads a page of data into the provided buffer.
     *
     * The buffer should be able to hold channels * page size many elements.
     * The data is interpretted as floats and channels are all interleaved.
     * If a full page is read, this method should return the page size.  If
     * it reads less, it will return the number of frames read.  It will 
     * return -1 on a processing error.
     *
     * @param buffer    The buffer to store the audio data
     *
     * @return the number of frames actually read (-1 on error).
     */
    virtual Sint32 pagein(float* buffer) = 0;

    /**
     * Returns the current page of this decoder 
     *
     * This value is the next page to be read in with the {@link pagein()} command.
     *
     * @return the current page of this decoder
     */
    Uint64 getPage() const { return _currpage; }

    /**
     * Sets the current page of this decoder
     *
     * This value is the next page to be read in with the {@link pagein()} command.
     * If the page is greater than the total number of pages, it will be set
     * just beyond the last page.
     *
     * @param page  The new page of this decoder
     */
    virtual void setPage(Uint64 page) = 0;
    
    /**
     * Returns the total number of pages in this decoder
     *
     * This value is the maximum value for the {@link setPage} command.
     *
     * @return total number of pages in this decoder
     */
    Uint64 getPageCount() const {
        return _frames % _pagesize == 0 ? _lastpage : _lastpage+1;
    }
    
    /**
     * Rewinds this decoder back the beginning of the stream
     */
    void rewind() { setPage(0); }
    
    /**
     * Decodes the entire audio file, storing its value in buffer.
     *
     * The buffer should be able to hold channels * frames many elements.
     * The data is interpretted as floats and channels are all interleaved.
     * If the method returns -1, then an error occurred during reading.
     *
     * @param buffer    The buffer to store the audio data
     *
     * @return the number of frames actually read (-1 on error).
     */
    Sint32 decode(float* buffer);

};
    }
}


#endif /* __CU_AUDIO_DECODER_H__ */
