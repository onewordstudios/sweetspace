//
//  CUFLACDecoder.h
//  Cornell University Game Library (CUGL)
//
//  This is class for decoding FLAC files with native encoding. It does not
//  support Ogg files with FLAC data.
//
//  FLAC supports up to 8 channels (7.1 stereo), though SDL is limited to
//  6 channels (5.1 stereo). Flac channel interleavings are compatible with
//  SDL, so they are preserved.
//
//  This code is built atop the 1.3.2 release of the Flac reference library
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
//  Version: 8/20/18
//
#ifndef __CU_FLAC_DECODER_H__
#define __CU_FLAC_DECODER_H__

#include "CUAudioDecoder.h"
#include <SDL/SDL.h>
#include <codecs/FLAC/stream_decoder.h>
#include <memory>

namespace cugl {
    namespace audio {
        
/**
 * This class represents an FLAC decoder.
 *
 * This class only supports native file encodings.  It does not support FLAC
 * data encoded in an Ogg file container.  In addition, the FLAC data must
 * have a complete stream info header containing the size and channel data.
 *
 * FLAC supports up to 8 channels (7.1 stereo), though SDL is limited to
 * 6 channels (5.1 stereo). Flac channel interleavings are compatible with
 * SDL, so they are preserved.
 *
 * FLAC files are not guaranteed to have uniform page sizes. This decoder tries
 * to balance memory requirements with efficiency in paging frame data.
 *
 * A decoder is NOT thread safe.  If a decoder is used by an audio thread, then
 * it should not be accessed directly in the main thread, and vice versa.
 */
class FLACDecoder : public AudioDecoder {
protected:
    /** The file for loading in information */
    SDL_RWops* _source;
    /** The FLAC decoder struct */
    FLAC__StreamDecoder* _decoder;
    /** The intermediate buffer for uniformizing FLAC data */
    Sint32* _buffer;
    /** The size of the intermediate buffer */
    Uint64  _buffsize;
    /** The last element read from the intermediate buffer */
    Uint64  _bufflast;
    /** The number of bits used to encode the sample data */
    Uint32  _sampsize;
    
    
public:
#pragma mark Constructors
    /**
     * Creates an initialized audio decoder
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    FLACDecoder();
    
    /**
     * Deletes this decoder, disposing of all resources.
     */
    ~FLACDecoder();
    
    /**
     * Initializes a new decoder for the given FLAC file.
     *
     * This method will fail if the file does not have a properly formed
     * stream info header.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    bool init(const char* file) override {
        return init(std::string(file));
    }
    
    /**
     * Initializes a new decoder for the given FLAC file.
     *
     * This method will fail if the file does not have a properly formed
     * stream info header.
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
     * Creates a newly allocated decoder for the given FLAC file.
     *
     * This method will fail and return nullptr if the file does not have a
     * properly formed stream info header.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given FLAC file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const char* file) {
        return alloc(std::string(file));
    }
    
    /**
     * Creates a newly allocated decoder for the given FLAC file.
     *
     * This method will fail and return nullptr if the file does not have a
     * properly formed stream info header.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given FLAC file.
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
    
    void setPage(Uint64 page) override;
    
#pragma mark FLAC Methods
    /**
     * Performs a read of the underlying file stream for the FLAC decoder
     *
     * This method abstracts the file access to allow us to read the asset on
     * non-standard platforms (e.g. Android).  If method reads less than the
     * requested number of bytes, the actual value is stored in the provided
     * parameter pointer.
     *
     * @param buffer    The buffer to start the data read
     * @param bytes     The number of bytes to read
     *
     * @return the callback status (error or continue)
     */
    FLAC__StreamDecoderReadStatus   doRead(FLAC__byte buffer[], size_t *bytes);
    
    /**
     * Performs a seek of the underlying file stream for the FLAC decoder
     *
     * This method abstracts the file access to allow us to read the asset on
     * non-standard platforms (e.g. Android).  The offset provided is from
     * the file beginning (e.g. SEEK_SET).
     *
     * @param offset    The number of bytes from the beginning of the file
     *
     * @return the callback status (error or continue)
     */
    FLAC__StreamDecoderSeekStatus   doSeek(FLAC__uint64 offset);
    
    /**
     * Performs a tell of the underlying file stream for the FLAC decoder
     *
     * This method abstracts the file access to allow us to read the asset on
     * non-standard platforms (e.g. Android).  The value computed is the 
     * file offset relative to the beginning of the file.  The value read is
     * stored in the provided parameter pointer.
     *
     * @param offset    The pointer to store the offset from the beginning
     *
     * @return the callback status (error or continue)
     */
    FLAC__StreamDecoderTellStatus   doTell(FLAC__uint64 *offset);
    
    /**
     * Performs a length computation of the underlying file for the FLAC decoder
     *
     * This method abstracts the file access to allow us to read the asset on
     * non-standard platforms (e.g. Android).  The value computed is the
     * length in bytes.  The value read is stored in the provided parameter 
     * pointer.
     *
     * @param length    The pointer to store the file length
     *
     * @return the callback status (error or continue)
     */
    FLAC__StreamDecoderLengthStatus doSize(FLAC__uint64 *length);
    
    /**
     * Performs a write of decoded sample data
     *
     * This method is the primary write method for decoded sample data.  The
     * data is converted to a float format and stored in the backing buffer
     * for later access.
     *
     * @param frame     The frame header for the current data block
     * @param buffer    The decoded samples for this block
     *
     * @return the callback status (error or continue)
     */
    FLAC__StreamDecoderWriteStatus  doWrite(const FLAC__Frame *frame,
                                            const FLAC__int32 * const buffer[]);

    /**
     * Performs an eof computation of the underlying file for the FLAC decoder
     *
     * This method abstracts the file access to allow us to read the asset on
     * non-standard platforms (e.g. Android). 
     *
     * @return true if the stream is at the end of the file
     */
    FLAC__bool isEOF();

    /**
     * Performs a write of the file metadata
     *
     * This method is called when the decoder is initialized to query the
     * stream info data. This is how the decoder gathers the important 
     * decoding information like sample rate and channel layout.
     *
     * @param metadata  The file metadata.
     *
     * @return the callback status (error or continue)
     */
    void doMeta(const FLAC__StreamMetadata *metadata);

    /**
     * Records an error in the underlying decoder
     *
     * This method does not abort decoding. Instead, it records the error 
     * with SDL_SetError for later retrieval.
     *
     * @param status     The error status.
     *
     * @return the callback status (error or continue)
     */
    void doError(FLAC__StreamDecoderErrorStatus status);

};
    }
}

#endif /* __CU_FLAC_DECODER_H__ */
