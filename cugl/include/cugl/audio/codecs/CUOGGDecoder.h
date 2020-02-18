//
//  CUOGGDecoder.h
//  Cornell University Game Library (CUGL)
//
//  This is class for decoding OGG Vorbis files. It only supports Vorbis
//  encodings.  It does not support Flac data encoded in an Ogg file container.
//
//  Ogg Vorbis supports up to 8 channels (7.1 stereo), though SDL is limited to
//  6 channels (5.1 stereo).  The channel layout for Ogg data is nonstandard
//  (e.g. channels > 3 are not stereo compatible), so this decoder standardizes
//  the channel layout to agree with FLAC and other data encodings.
//
//  This code is built atop the 1.3.2 release of the Ogg reference library
//  making it compatible with our license.
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
//  Authors: Sam Lantinga, Walker White
//  Version: 6/29/17
//
#ifndef __CU_OGG_DECODER_H__
#define __CU_OGG_DECODER_H__
#include "CUAudioDecoder.h"
#include <SDL/SDL.h>
#include <codecs/vorbis/vorbisfile.h>

namespace cugl {
    namespace audio {
/**
 * This class represents an OGG decoder.
 *
 * This class only supports Vorbis encodings.  It does not support Flac data 
 * encoded in an Ogg file container.  It also does not support the newer 
 * Opus codec.
 *
 * Ogg Vorbis supports up to 8 channels (7.1 stereo), though SDL is limited to
 * 6 channels (5.1 stereo).  The channel layout for Ogg data is nonstandard
 * (e.g. channels > 3 are not stereo compatible), so this decoder standardizes
 * the channel layout to agree with FLAC and other data encodings.  The
 * channels are interleaved.
 *
 * OGG files are not guaranteed to have uniform page sizes. This decoder tries
 * to balance memory requirements with efficiency in paging frame data.
 *
 * A decoder is NOT thread safe.  If a decoder is used by an audio thread, then
 * it should not be accessed directly in the main thread, and vice versa.
 */
class OGGDecoder : public AudioDecoder {
protected:
    /** The file for loading in information */
    SDL_RWops* _source;
    /** The OGG decoder struct */
    OggVorbis_File _oggfile;
    /** Reference to the logical bitstream for decoding */
    int _bitstream;

public:
#pragma mark Constructors
    /**
     * Creates an initialized audio decoder
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    OGGDecoder();
    
    /**
     * Deletes this decoder, disposing of all resources.
     */
    ~OGGDecoder();
    
    /**
     * Initializes a new decoder for the given OGG file.
     *
     * This method will fail if the file does not contain Vorbis data.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    bool init(const char* file) override {
        return init(std::string(file));
    }
    
    /**
     * Initializes a new decoder for the given OGG file.
     *
     * This method will fail if the file does not contain Vorbis data.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    bool init(const std::string& file) override;
    
    /**
     * Deletes the decoder resources and resets all attributes.
     *
     * This will close the associated file. You must reinitialize the decoder
     * to use it.
     */
    void dispose() override;
    
    
#pragma mark Static Constructors
    /**
     * Creates a newly allocated decoder for the given OGG file.
     *
     * This method will fail and return nullptr if the file does not contain 
     * Vorbis data.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given OGG file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const char* file) {
        return alloc(std::string(file));
    }
    
    /**
     * Creates a newly allocated decoder for the given OGG file.
     *
     * This method will fail and return nullptr if the file does not contain
     * Vorbis data.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given OGG file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const std::string& file);
    
    
#pragma mark Decoding
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
    Sint32 pagein(float* buffer) override;
    
    /**
     * Sets the current page of this decoder
     *
     * This value is the next page to be read in with the {@link pagein()} command.
     * If the page is greater than the total number of pages, it will be set
     * just beyond the last page.
     *
     * @param page  The new page of this decoder
     */
    void setPage(Uint64 page) override;
    
};
    }
}

#endif /* __CU_OGG_DECODER_H__ */
