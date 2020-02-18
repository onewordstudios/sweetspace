//
//  CUFLACDecoder.cpp
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
#include <cugl/audio/codecs/CUFLACDecoder.h>
#include <cassert>
#include <climits>

using namespace cugl::audio;

#pragma mark FLAC Utilities
/** The default page size (in bytes) */
#define PAGE_SIZE   4096

/**
 * Performs a read of the underlying file stream for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).  If method reads less than the
 * requested number of bytes, the actual value is stored in the provided
 * parameter pointer.
 *
 * @param decoder   The FLAC decoder struct
 * @param buffer    The buffer to start the data read
 * @param bytes     The number of bytes to read
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderReadStatus flac_read(const FLAC__StreamDecoder *decoder,
                                        FLAC__byte buffer[], size_t *bytes,
                                        void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    return handler->doRead(buffer,bytes);
}

/**
 * Performs a seek of the underlying file stream for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).  The offset provided is from
 * the file beginning (e.g. SEEK_SET).
 *
 * @param decoder   The FLAC decoder struct
 * @param offset    The number of bytes from the beginning of the file
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderSeekStatus flac_seek(const FLAC__StreamDecoder *decoder,
                                        FLAC__uint64 offset,
                                        void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    return handler->doSeek(offset);
}

/**
 * Performs a tell of the underlying file stream for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).  The value computed is the
 * file offset relative to the beginning of the file.  The value read is
 * stored in the provided parameter pointer.
 *
 * @param decoder   The FLAC decoder struct
 * @param offset    The pointer to store the offset from the beginning
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderTellStatus flac_tell(const FLAC__StreamDecoder *decoder,
                                        FLAC__uint64 *offset,
                                        void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    return handler->doTell(offset);
}

/**
 * Performs a length computation of the underlying file for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).  The value computed is the
 * length in bytes.  The value read is stored in the provided parameter
 * pointer.
 *
 * @param decoder   The FLAC decoder struct
 * @param length    The pointer to store the file length
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderLengthStatus flac_size(const FLAC__StreamDecoder *decoder,
                                          FLAC__uint64 *length,
                                          void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    return handler->doSize(length);
}

/**
 * Performs an eof computation of the underlying file for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).
 *
 * @param decoder   The FLAC decoder struct
 * @param cdata     The AUFLACDecoder instance
 *
 * @return true if the stream is at the end of the file
 */
FLAC__bool flac_eof(const FLAC__StreamDecoder *decoder, void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    return handler->isEOF();
}

/**
 * Performs a write of decoded sample data
 *
 * This method is the primary write method for decoded sample data.  The
 * data is converted to a float format and stored in the backing buffer
 * for later access.
 *
 * @param decoder   The FLAC decoder struct
 * @param frame     The frame header for the current data block
 * @param buffer    The decoded samples for this block
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderWriteStatus flac_write(const FLAC__StreamDecoder *decoder,
                                          const FLAC__Frame *frame,
                                          const FLAC__int32 * const buffer[],
                                          void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    return handler->doWrite(frame,buffer);
}

/**
 * Performs a write of the file metadata
 *
 * This method is called when the decoder is initialized to query the
 * stream info data. This is how the decoder gathers the important
 * decoding information like sample rate and channel layout.
 *
 * @param decoder   The FLAC decoder struct
 * @param metadata  The file metadata
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
void flac_metadata(const FLAC__StreamDecoder *decoder,
                   const FLAC__StreamMetadata *metadata,
                   void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    handler->doMeta(metadata);
}

/**
 * Records an error in the underlying decoder
 *
 * This method does not abort decoding. Instead, it records the error
 * with SDL_SetError for later retrieval.
 *
 * @param decoder   The FLAC decoder struct
 * @param state     The error status
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
void flac_error(const FLAC__StreamDecoder *decoder,
                FLAC__StreamDecoderErrorStatus status,
                void *cdata) {
    FLACDecoder* handler = (FLACDecoder*)cdata;
    handler->doError(status);
}


#pragma mark -
#pragma mark FLAC Decoder
/**
 * Creates an initialized audio decoder
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
FLACDecoder::FLACDecoder() : AudioDecoder(),
_decoder(nullptr),
_buffer(nullptr),
_sampsize(0),
_buffsize(0),
_bufflast(0)
{}

/**
 * Deletes this decoder, disposing of all resources.
 */
FLACDecoder::~FLACDecoder() {
    if (_buffer != nullptr) {
        free(_buffer);
        FLAC__stream_decoder_delete(_decoder);
        SDL_RWclose(_source);
        _buffer = nullptr;
    }
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
bool FLACDecoder::init(const std::string& file) {
    _file = file;
    
    _source = SDL_RWFromFile(file.c_str(), "r");
    if (_source == nullptr) {
        SDL_SetError("Could not open '%s'",file.c_str());
        return false;
    }

    if (!(_decoder = FLAC__stream_decoder_new())) {
        SDL_SetError("Could not allocate FLAC decoder");
        return false;
    }
    
    (void)FLAC__stream_decoder_set_md5_checking(_decoder, true);
    FLAC__StreamDecoderInitStatus status;
    
    status = FLAC__stream_decoder_init_stream(_decoder,
                                              flac_read, flac_seek, flac_tell,
                                              flac_size, flac_eof,
                                              flac_write, flac_metadata, flac_error,
                                              this);
    if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        SDL_SetError("FLAC initialization error: %s",FLAC__StreamDecoderInitStatusString[status]);
        FLAC__stream_decoder_delete(_decoder);
        return false;
    }

    bool ok = FLAC__stream_decoder_process_until_end_of_metadata(_decoder);
    if (!ok || _pagesize == 0) {
        SDL_SetError("FLAC '%s' does not have a stream_info header",file.c_str());
        FLAC__stream_decoder_delete(_decoder);
        return false;
    }
    
    _buffer   = (Sint32*)malloc(_pagesize*_channels*sizeof(Sint32));
    return true;
}

/**
 * Deletes the decoder resources and resets all attributes.
 *
 * This will close the associated file. You must reinitialize the decoder
 * to use it.
 */
void FLACDecoder::dispose() {
    if (_buffer != nullptr) {
        free(_buffer);
        FLAC__stream_decoder_delete(_decoder);
        SDL_RWclose(_source);
        _buffer = nullptr;
    }
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
std::shared_ptr<AudioDecoder> FLACDecoder::alloc(const std::string& file) {
    std::shared_ptr<FLACDecoder> result = std::make_shared<FLACDecoder>();
    if (result->init(file)) {
        return std::dynamic_pointer_cast<AudioDecoder>(result);
    }
    return nullptr;
}


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
Sint32 FLACDecoder::pagein(float* buffer) {
    Sint32 read = 0;
    
    while (read < _pagesize) {
        // First see how much is available
        Sint32 avail = (Sint32)(_buffsize - _bufflast);
        avail = (_pagesize - read < avail ? _pagesize - read : avail);
        
        Uint32 temp = avail*_channels;
        float* output = buffer+(_bufflast*_channels);
        Sint32* input = _buffer;
        double factor = 1/(1L << _sampsize);
        while (temp--) {
            *output = (float)(*input*factor);
            output++;
            input++;
        }
        read += avail;
        _bufflast += avail;
        
        // Page in more if we need it
        if (read < _pagesize) {
            if (!FLAC__stream_decoder_process_single(_decoder) || _bufflast == _buffsize) {
                return read;
            }
        }
    }
    
    _currpage++;
    return read;
}

/**
 * Sets the current page of this decoder
 *
 * This value is the next page to be read in with the {@pagein()} command.
 * If the page is greater than the total number of pages, it will be set
 * just beyond the last page.
 *
 * @param page  The new page of this decoder
 */
void FLACDecoder::setPage(Uint64 page) {
    if (!FLAC__stream_decoder_seek_absolute(_decoder,page*_pagesize)) {
        SDL_SetError("Seek is not supported");
    }
    _currpage = page;
}


#pragma mark FLAC Method
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
FLAC__StreamDecoderReadStatus FLACDecoder::doRead(FLAC__byte buffer[], size_t *bytes) {
    if (SDL_RWtell(_source) == SDL_RWsize(_source)) {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    *bytes = SDL_RWread(_source, buffer, 1, *bytes);
    if (*bytes == -1) {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

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
FLAC__StreamDecoderSeekStatus FLACDecoder::doSeek(FLAC__uint64 offset) {
    if (SDL_RWseek(_source,offset, RW_SEEK_SET) == -1) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

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
FLAC__StreamDecoderTellStatus FLACDecoder::doTell(FLAC__uint64 *offset) {
    *offset = SDL_RWtell(_source);
    return (*offset == -1 ? FLAC__STREAM_DECODER_TELL_STATUS_ERROR : FLAC__STREAM_DECODER_TELL_STATUS_OK);
}

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
FLAC__StreamDecoderLengthStatus FLACDecoder::doSize(FLAC__uint64 *length) {
    *length = SDL_RWsize(_source);
    return (*length == -1 ? FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR : FLAC__STREAM_DECODER_LENGTH_STATUS_OK);
}

/**
 * Performs an eof computation of the underlying file for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).
 *
 * @return true if the stream is at the end of the file
 */
FLAC__bool FLACDecoder::isEOF() {
    return SDL_RWtell(_source) == SDL_RWsize(_source);
}

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
FLAC__StreamDecoderWriteStatus FLACDecoder::doWrite(const FLAC__Frame *frame,
                                                    const FLAC__int32 * const buffer[]) {
    if(frame->header.channels != _channels) {
        SDL_SetError("FLAC has changed number of channels from %d to %d", _channels, frame->header.channels);
        _buffsize = 0;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    
    _buffsize = frame->header.blocksize;
    for(Uint32 ch = 0; ch < _channels; ch++) {
        if (buffer[ch] == NULL) {
            SDL_SetError("FLAC channel %d is NULL", ch);
            _buffsize = 0;
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        
        Sint32* output = _buffer+ch;
        const FLAC__int32* input = buffer[ch];
        for (Uint32 ii = 0; ii < _buffsize; ii++) {
            *output = *input;
            output += _channels;
            input++;
        }
    }
    
    _bufflast = 0;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

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
void FLACDecoder::doMeta(const FLAC__StreamMetadata *metadata) {
    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        _pagesize = metadata->data.stream_info.max_blocksize;
        _channels = metadata->data.stream_info.channels;
        _sampsize = metadata->data.stream_info.bits_per_sample;
        _frames = metadata->data.stream_info.total_samples;
        _rate   = metadata->data.stream_info.sample_rate;
    }
}

/**
 * Records an error in the underlying decoder
 *
 * This method does not abort decoding. Instead, it records the error
 * with SDL_SetError for later retrieval.
 *
 * @param state     The error status.
 *
 * @return the callback status (error or continue)
 */
void FLACDecoder::doError(FLAC__StreamDecoderErrorStatus status) {
    SDL_SetError("FLAC error: %s",FLAC__StreamDecoderErrorStatusString[status]);
}
