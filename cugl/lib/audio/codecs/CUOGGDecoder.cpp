//
//  CUOGGDecoder.cpp
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
#include <cugl/audio/codecs/CUOGGDecoder.h>
#include <cassert>
#include <climits>

using namespace cugl::audio;

#pragma mark OGG Utilities
/** The default page size (in bytes) */
#define PAGE_SIZE   4096

/**
 * Returns the SDL channel for the given OGG channel
 *
 * The channel layout for Ogg data is nonstandard (e.g. channels > 3 are not 
 * stereo compatible), so this function standardizes the channel layout to 
 * agree with FLAC and other data encodings.
 *
 * @param ch        The OGG channel position
 * @param channels  The total number of channels
 *
 * @return the SDL channel for the given OGG channel
 */
inline Uint32 ogg2sdl(Uint32 ch, Uint32 channels) {
    switch (channels) {
        case 3:
        case 5:
        {
            switch (ch) {
                case 1:
                    return 2;
                case 2:
                    return 1;
            }
        }
            break;
        case 6:
        {
            switch (ch) {
                case 1:
                    return 2;
                case 2:
                    return 1;
                case 3:
                    return 4;
                case 4:
                    return 5;
                case 5:
                    return 3;
            }
        }
    }
    return ch;
}

/**
 * Performs a read of the underlying file stream for the OGG decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).  
 *
 * @param ptr           The buffer to start the data read
 * @param size          The number of bytes per element
 * @param nmemb         The number of elements to read
 * @param datasource    The file to read from
 * 
 * @return the number of elements actually read.
 */
static size_t ogg_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
    return SDL_RWread((SDL_RWops*)datasource, ptr, size, nmemb);
}

/**
 * Performs a seek of the underlying file stream for the OGG decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).  The value whence is one the 
 * classic values SEEK_CUR, SEEK_SET, or SEEK_END.
 *
 * @param datasource    The file to seek
 * @param offset        The offset to seek to
 * @param whence        The position to offset from
 *
 * @return the new file position
 */
static int    ogg_seek(void *datasource, ogg_int64_t offset, int whence) {
    if(datasource==NULL)return(-1);
    return (int)SDL_RWseek((SDL_RWops*)datasource,offset,whence);
}

/**
 * Performs a tell of the underlying file stream for the OGG decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android). This function returns the current
 * offset (from the beginning) of the file position.
 *
 * @param datasource    The file to query
 *
 * @return the current file position
 */
static long   ogg_tell(void *datasource) {
    return (long)SDL_RWtell((SDL_RWops*)datasource);
}


#pragma mark -
#pragma mark OGG Decoder
/**
 * Creates an initialized audio decoder
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
OGGDecoder::OGGDecoder() : AudioDecoder(),
_source(nullptr),
_bitstream(0)
{}

/**
 * Deletes this decoder, disposing of all resources.
 */
OGGDecoder::~OGGDecoder() {
    if (_source != nullptr) {
        ov_clear(&_oggfile);
        SDL_RWclose(_source);
        _source = nullptr;
    }
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
bool OGGDecoder::init(const std::string& file) {
    _file = file;
    
    _source = SDL_RWFromFile(file.c_str(), "rb");
    if (_source == nullptr) {
        SDL_SetError("Could not open '%s'",file.c_str());
        return false;
    }
    
    _bitstream = -1;
    
    ov_callbacks calls;
    calls.read_func = ogg_read;
    calls.seek_func = ogg_seek;
    calls.tell_func = ogg_tell;
    calls.close_func = NULL;

    //int error = ov_fopen(file.c_str(),&_oggfile);
    int error = ov_open_callbacks(_source, &_oggfile, nullptr, 0, calls);
    if (error) {
        SDL_SetError("File '%s' is not an OFF file", file.c_str());
        SDL_RWclose(_source);
        _source = nullptr;
        return false;
    }
    
    vorbis_info* info = ov_info(&_oggfile, _bitstream);
    _channels = info->channels;
    _rate     = (Uint32)info->rate;
    _frames   = ov_pcm_total(&_oggfile, _bitstream);
    _pagesize = PAGE_SIZE/(sizeof(float)*_channels);
    return true;
}

/**
 * Deletes the decoder resources and resets all attributes.
 *
 * This will close the associated file. You must reinitialize the decoder
 * to use it.
 */
void OGGDecoder::dispose() {
    if (_source != nullptr) {
        ov_clear(&_oggfile);
        SDL_RWclose(_source);
        _source = nullptr;
    }
}


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
std::shared_ptr<AudioDecoder> OGGDecoder::alloc(const std::string& file) {
    std::shared_ptr<OGGDecoder> result = std::make_shared<OGGDecoder>();
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
Sint32 OGGDecoder::pagein(float* buffer) {
    Sint32 read = 0;
    
    // Now read from the stream
    float** pcmb;
    while (read < _pagesize) {
        Sint32 avail = (_pagesize - read >= _pagesize ? _pagesize : _pagesize - read);
        avail = (int)ov_read_float(&_oggfile, &pcmb, avail, &_bitstream);
        if (avail < 0) {
            if (avail == OV_HOLE) {
                SDL_SetError("OGG file experienced an interruption in data");
            } else if (avail == OV_EBADLINK) {
                SDL_SetError("OGG file has an invalid stream section");
            } else if (avail == OV_EINVAL) {
                SDL_SetError("The OGG file headers cannot be read");
            }
            return avail;
        } else if (avail == 0) {
            break;
        }
        
        // Copy everything into its place
        for (Uint32 ch = 0; ch < _channels; ++ch) {
            // OGG representation differs from SDL representation
            Uint32 outch = ogg2sdl(ch,_channels);
            
            float* output = buffer+(read*_channels)+outch;
            float* input  = pcmb[ch];
            Uint32 temp = avail;
            while (temp--) {
                *output = *input;
                output += _channels;
                input++;
            }
        }
        
        read += avail;
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
void OGGDecoder::setPage(Uint64 page) {
    Uint32 frame = (Uint32)(page*_pagesize);
    if (frame > _frames) {
        frame = (Uint32)_frames;
    }
    ov_pcm_seek(&_oggfile,frame);
    _currpage = frame/_pagesize;
}
