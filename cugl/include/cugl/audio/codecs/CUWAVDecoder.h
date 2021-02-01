//
//  CUWAVDecoder.h
//  Cornell University Game Library (CUGL)
//
//  This is class for decoding WAV files. It supports PCM, IEEE Float, and
//  ADPCM encoding (both MS and IMA).  However, it does not support MP3 data
//  stored in a WAV file.  MP3 data should be stored in an MP3 file.
//
//  This code is heavily adapted from SDL_wave.c in SDL 2.0.5 by Sam Lantinga.
//  That implementation does not allow WAV files to be streamed.  In addition,
//  it required the audio thread to be initialized before loading assets.  This
//  implementation decouples the decoding process, removing these concerns.
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
#ifndef __CU_WAV_DECODER_H__
#define __CU_WAV_DECODER_H__
#include "CUAudioDecoder.h"
#include <SDL/SDL.h>
#include <memory>

namespace cugl {
    namespace audio {
        
#pragma mark WAV Header
/**
 * This struct contains the RIFF information at the start of a WAVE file
 *
 * Normally, this is stored in the first three chunks at the start of a
 * WAVE file.
 */
typedef struct WaveFMT {
    /** The encoding type (PCM, IEEE, MS ADPCM, IMA ADPCM) */
    Uint16 encoding;
    /** The number of channels  (1 = mono, 2 = stereo) */
    Uint16 channels;
    /** The sample rate (11025, 22050, or 44100 Hz) */
    Uint32 frequency;
    /** The average bytes per second */
    Uint32 byterate;
    /** The number of bytes per sample block */
    Uint16 blockalign;
    /** One of 8, 12, 16, or 4 for ADPCM */
    Uint16 bitspersample;
} WaveFMT;


#pragma mark -
#pragma mark ADPCM Decoder
/**
 * This class represents an internal decoder for ADPCM encoded WAV files.
 *
 * This decoder is a proxy decoder for ADPCM files, which are compressed.
 * This is the abstract base class for either MS or IMA decoding.  This 
 * class is for internal use and should never be instantiated by the user.
 */
class ADPCMDecoder {
protected:
    /** The RIFF header */
    WaveFMT _wavefmt;
    /** The buffer size for AD PCM decoding */
    Uint8*  _blkbuffer;
    /** The internal buffer for AD PCM decoding */
    Uint16  _blocksize;
    
public:
#pragma mark Constructors
    /**
     * Creates an initialized decoder proxy
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    ADPCMDecoder() : _blocksize(0), _blkbuffer(nullptr) {}
    
    /**
     * Deletes this decoder proxy, disposing of all resources.
     */
    virtual ~ADPCMDecoder();
    
    /**
     * Initializes a new decoder proxy from the given WAV header.
     *
     * This is an abstract method that depends on the specific ADPCM encoding.
     * This method will fail if the header is not compatible with the 
     * appropriate encoding.
     *
     * @param format    The WAV header
     *
     * @return true if the decoder proxy was initialized successfully
     */
    virtual bool init(WaveFMT* format);

#pragma mark Decoding

    /**
     * Returns the number of frames to decompress from the given number of bytes
     *
     * ADPCM decoding expands the number of frames that a group of bytes can
     * produce.  This method allows the main decoder to align with the proxy.
     *
     * @param bytes The number of bytes to convert
     *
     * @return the number of frames to decompress from the given number of bytes
     */
    Uint64 getFrames(Uint64 bytes) const {
        return (_blocksize*bytes)/_wavefmt.blockalign;
    }

    /** 
     * Returns the block size of a single page
     *
     * This is the size of buffer for the {@link read} methd.
     *
     * @return the block size of a single page
     */
    Uint32 getBlockSize() const {
        return _blocksize;
    }
    
    /**
     * Reads a single page from the given file.
     *
     * The buffer should be able to store block size * channels * 2 bytes of
     * data (the latter 2 representing sizeof(Sint16) ). If the read fails,
     * this method returns -1.
     *
     * @param source    The source file
     * @param buffer    The buffer to store the decoded data
     *
     * @return the number of bytes read (or -1 on error)
     */
    virtual Sint32 read(SDL_RWops* source, Uint8* buffer) = 0;
};
   

#pragma mark -
#pragma mark WAV Decoder
/**
 * This class represents an WAV decoder.
 *
 * This decoder supports  supports PCM, IEEE Float, and ADPCM encoding (both
 * MS and IMA).  However, it does not support MP3 data stored in a WAV file.  
 * MP3 data should be stored in an MP3 file.
 *
 * WAV files are not normally paged (except in the case of AD PCM). This
 * decoder tries to balance memory requirements with efficiency in paging
 * the WAV data.  For ADPCM files, the page size is the same as the block 
 * alignment size.
 *
 * All channels are interleaved.  ADPCM only supports stereo, but more 
 * channels may be supported in general WAV files.  SDL supports up to 6 
 * channels in general.
 *
 * A decoder is NOT thread safe.  If a decoder is used by an audio thread, then
 * it should not be accessed directly in the main thread, and vice versa.
 */
class WAVDecoder : public AudioDecoder {
public:
    /**
     * This represents the supported WAV encoding types
     *
     * More obscure coding types (e.g. DTS WAV) may or may not be supported.
     */
    enum class Type : int {
        /** Raw PCM data in 16bit samples (the most common format) */
        PCM_DATA   = 0,
        /** Raw PCM data with 32bit float samples */
        IEEE_FLOAT = 1,
        /** MS encoded ADPCM data */
        MS_ADPCM   = 2,
        /** IMA encoded ADPCM data */
        IMA_ADPCM  = 3,
        /** MP3 data encoded in a WAV file */
        MP3_DATA   = 4,
        /** Unsupported WAV encoding */
        UNKNOWN    = 5
    };
    
protected:
    /** The file for loading in information */
    SDL_RWops* _source;
    /** The buffer for reading pages */
    Uint8* _chunker;
    /** The encoding type */
    Type   _datatype;

    /** The number of bits per sample represented as a flag mask */
    Uint32 _sampbits;
    /** The number of bytes per sample */
    Uint32 _sampsize;
    /** The start of the audio stream, just after the header */
    Sint64 _datamark;
    
    /** An optional proxy for decoding ACPCM data */
    std::shared_ptr<ADPCMDecoder> _adpcm;
    
public:
#pragma mark Constructors
    /**
     * Creates an initialized audio decoder
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    WAVDecoder();
    
    /**
     * Deletes this decoder, disposing of all resources.
     */
    ~WAVDecoder();
    
    /**
     * Initializes a new decoder for the given WAV file.
     *
     * This method will fail if the file is not a supported WAV file.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    bool init(const char* file) override {
        return init(std::string(file));
    }
    
    /**
     * Initializes a new decoder for the given WAV file.
     *
     * This method will fail if the file is not a supported WAV file.
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
     * Creates a newly allocated decoder for the given WAV file.
     *
     * This method will fail and return nullptr if the file is not a supported
     * WAV file.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given WAV file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const char* file) {
        return alloc(std::string(file));
    }
    
    /**
     * Creates a newly allocated decoder for the given WAV file.
     *
     * This method will fail and return nullptr if the file is not a supported
     * WAV file.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given WAV file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const std::string& file);
    
    
#pragma mark Decoding
    /**
     * Returns the WAV encoding type
     *
     * @return the WAV encoding type
     */
    Type getType() const { return _datatype; }
    
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
    virtual Sint32 pagein(float* buffer) override;
    
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

private:
    /**
     * Returns true if this is an ADPCM encoded WAV fault
     *
     * @return true if this is an ADPCM encoded WAV fault
     */
    bool isADPCM() const { return _datatype == Type::MS_ADPCM || _datatype == Type::IMA_ADPCM; }

    /**
     * Bootstraps the given file and readies it for decoding.
     *
     * This method reads in the initial header and forwards the file pointer
     * to the start of the audio data.  This method is a reworking of 
     * SDL_LoadWAV_RW to allow data streaming.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was boot strapped successfully
     */
    bool bootstrap(const std::string& file);
    
};
    }
}

#endif /* __CU_WAV_DECODER_H__ */
