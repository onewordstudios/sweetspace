//
//  CUMP3Decoder.h
//  Cornell University Game Library (CUGL)
//
//  This is class for decoding MP3 files.  It is based on Jung woo-jae's
//  MPEG/WAVE Sound library.  This 1997 library has no known license (though
//  several people have extended it with LGPL additions), but is passed around
//  on the internet as if it were attribution-only. We are using only Jung
//  woo-jae's classes (updated for C++11 compatibility), so we assume that this
//  exempts us from a LGPL license.  In addition, at the time of this writing,
//  all MP3 patents have recently expired, allowing us to use this license
//  library free.
//
//  CUGL MIT License:
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
#ifndef __CU_MP3_DECODER_H__
#define __CU_MP3_DECODER_H__
#include "CUAudioDecoder.h"
#include <codecs/mpg/mpegsound.h>
#include <memory>

namespace cugl {
    namespace audio {
/**
 * This class represents an MP3 decoder.
 *
 * This decoder accesses the MP3 in the traditionally grouping of 1152 frames
 * (we use frames to refer to samples, consistent with modern usage) per page.
 * This class uses Jung woo-jae's free MPEG/WAVE Sound library to convert
 * the individual pages into raw PCM data.
 *
 * All channels are interleaved.  Because of the age of the MPEG/WAVE Sound
 * library, this decoder only supports stereo and mono MP3 files.  In particular,
 * it does not support MP3 surround.
 *
 * A decoder is NOT thread safe.  If a decoder is used by an audio thread, then
 * it should not be accessed directly in the main thread, and vice versa.
 */
class MP3Decoder : public AudioDecoder {
protected:
    /** The loader for reading from a file */
    Soundinputstream _loader;
    
    /** The MP3 decoder */
    Mpegtoraw* _decoder;
    
    /** The buffer for reading pages */
    Sint16* _chunker;
    
    /** Whether this decoder was successfully booted */
    bool _booted;

public:
#pragma mark Constructors
    /**
     * Creates an initialized audio decoder
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    MP3Decoder();
    
    /**
     * Deletes this decoder, disposing of all resources.
     */
    ~MP3Decoder();
    
    /**
     * Initializes a new decoder for the given MP3 file.
     *
     * This method will fail if the file is not a (stereo or mono) MP3 file.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    bool init(const char* file) override {
        return init(std::string(file));
    }
    
    /**
     * Initializes a new decoder for the given MP3 file.
     *
     * This method will fail if the file is not a (stereo or mono) MP3 file.
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
     * Creates a newly allocated decoder for the given MP3 file.
     *
     * This method will fail and return nullptr if the file is not a (stereo 
     * or mono) MP3 file.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given MP3 file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const char* file) {
        return alloc(std::string(file));
    }
    
    /**
     * Creates a newly allocated decoder for the given MP3 file.
     *
     * This method will fail and return nullptr if the file is not a (stereo
     * or mono) MP3 file.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given MP3 file.
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

#endif /* __CU_MP3_DECODER_H__ */
