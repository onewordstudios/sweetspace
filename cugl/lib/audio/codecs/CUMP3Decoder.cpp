//
//  CUMP3Decoder.cpp
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
#include <cugl/audio/codecs/CUMP3Decoder.h>
#include <cassert>
#include <climits>

using namespace cugl::audio;


#pragma mark Constructors
/**
 * Creates an initialized audio decoder
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
MP3Decoder::MP3Decoder() : AudioDecoder(),
_booted(false),
_decoder(nullptr),
_chunker(nullptr) {
}


/**
 * Deletes this decoder, disposing of all resources.
 */
MP3Decoder::~MP3Decoder() {
    dispose();
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
bool MP3Decoder::init(const std::string& file) {
    _file = file;
    if (!_loader.open(file.c_str())) {
        SDL_SetError("Could not open '%s': Code %d\n",
                     _file.c_str(), _loader.geterrorcode());
        return false;
    }
    
    _decoder = new Mpegtoraw(&_loader);
    _decoder->initialize(nullptr);
    
    if (_decoder->geterrorcode() != SOUND_ERROR_OK) {
        SDL_SetError("Could decode '%s' as an MP3 file: Code %d\n",
                     _file.c_str(), _decoder->geterrorcode());
        delete _decoder;
        return false;
    }
    
    _channels = (_decoder->isstereo() ? 2 : 1);
    _rate     = _decoder->getfrequency();
    _pagesize = _decoder->getpcmperframe();
    _chunker  = (Sint16 *)malloc(_pagesize*_channels*sizeof(Sint16));

    // Now find the length
    Uint32 amount = 0;
    do {
        amount = (Uint32)_decoder->run(_chunker,1);
        _frames += amount;
    } while (amount > 0);
    _frames /= _channels;
    _lastpage = (Uint32)(_frames/_pagesize);
    
    _decoder->setframe(0);
    _booted = true;
    return true;
}

/**
 * Deletes the decoder resources and resets all attributes.
 *
 * This will close the associated file. You must reinitialize the decoder
 * to use it.
 */
void MP3Decoder::dispose() {
    if (_decoder != nullptr) {
        delete _decoder;
        _decoder = nullptr;
    }
    
    if (_chunker != nullptr) {
        free(_chunker);
        _chunker = nullptr;
    }
    
    if (_booted) {
        _loader.setposition(0);
        _booted = false;
    }
}


#pragma mark -
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
std::shared_ptr<AudioDecoder> MP3Decoder::alloc(const std::string& file) {
    std::shared_ptr<MP3Decoder> result = std::make_shared<MP3Decoder>();
    if (result->init(file)) {
        return std::dynamic_pointer_cast<AudioDecoder>(result);
    }
    return nullptr;
}


#pragma mark -
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
Sint32 MP3Decoder::pagein(float* buffer) {
    if (_currpage > _lastpage) {
        return 0;
    }
    
    Uint32 amount = (Uint32)_decoder->run(_chunker,1);
    double factor = 1/(SHRT_MAX+1);

    Uint32 temp = amount;
    float* output = buffer;
    short* input  = _chunker;

    while (temp--) {
        *output = (float)((*input)*factor);
        output++;
        input++;
    }
    
    _currpage++;
    return amount/_channels;
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
void MP3Decoder::setPage(Uint64 page) {
    if (page*_pagesize > _frames) {
        page = _decoder->gettotalframe();
    }
    
    _currpage = page;
    _decoder->setframe((int)page);
}
