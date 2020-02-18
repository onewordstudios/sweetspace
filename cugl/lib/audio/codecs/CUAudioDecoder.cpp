//
//  CUAudioDecoder.cpp
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
//  Version: 6/29/17
//
#include <cugl/audio/codecs/CUAudioDecoder.h>
#include <cugl/util/CUDebug.h>

using namespace cugl::audio;

/**
 * Creates an initialized audio decoder
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
AudioDecoder::AudioDecoder() :
_rate(0),
_file(""),
_frames(0),
_channels(0),
_pagesize(0),
_lastpage(0),
_currpage(0)
{
}

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
Sint32 AudioDecoder::decode(float* buffer) {
    assert (getPage() == 0);
    Uint32 total = 0;
    Sint32 amt = 0;
    do {
        amt = pagein(buffer+total*_channels);
        total += amt;
    } while (amt > 0);
    
    return (amt >= 0 ? total : amt);
}
