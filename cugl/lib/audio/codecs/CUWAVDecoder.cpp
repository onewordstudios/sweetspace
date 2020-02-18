//
//  CUWAVDecoder.cpp
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
#include <cugl/audio/codecs/CUWAVDecoder.h>
#include <cugl/util/CUDebug.h>
#include <cassert>
#include <climits>

using namespace cugl::audio;

#pragma mark CONSTANTS

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF            0x46464952      /* "RIFF" */
#define WAVE            0x45564157      /* "WAVE" */
#define FACT            0x74636166      /* "fact" */
#define LIST            0x5453494c      /* "LIST" */
#define BEXT            0x74786562      /* "bext" */
#define JUNK            0x4B4E554A      /* "JUNK" */
#define FMT             0x20746D66      /* "fmt " */
#define DATA            0x61746164      /* "data" */
#define PCM_CODE        0x0001
#define MS_ADPCM_CODE   0x0002
#define IEEE_FLOAT_CODE 0x0003
#define IMA_ADPCM_CODE  0x0011
#define MP3_CODE        0x0055
#define WAVE_MONO       1
#define WAVE_STEREO     2
#define PAGE_SIZE       4096


#pragma mark -
#pragma mark Chunk Management

/**
 * This struct represents a general data chunk
 */
typedef struct Chunk {
    Uint32 magic;
    Uint32 length;
    Uint8 *data;
} Chunk;

/**
 * Reads a single chunk of data from the given file.
 *
 * If the read is successful, this method returns the number of bytes read.
 * If it is not successful, it will return a negative number with the 
 * appropriate error code.
 *
 * @param src   The source file
 * @param chunk The struct to store the data read
 *
 * @return the number of bytes read (or an error code on faulure)
 */
static int readChunk(SDL_RWops * src, Chunk * chunk) {
    chunk->magic = SDL_ReadLE32(src);
    chunk->length = SDL_ReadLE32(src);
    chunk->data = (Uint8 *) SDL_malloc(chunk->length);
    if (chunk->data == NULL) {
        return SDL_OutOfMemory();
    }
    if (SDL_RWread(src, chunk->data, chunk->length, 1) != 1) {
        SDL_free(chunk->data);
        chunk->data = NULL;
        return SDL_Error(SDL_EFREAD);
    }
    return (chunk->length);
}


#pragma mark -
#pragma mark ADPCM Decoder
/**
 * Creates an initialized decoder proxy
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
ADPCMDecoder::~ADPCMDecoder() {
    if (_blkbuffer != nullptr) {
        SDL_free(_blkbuffer);
    }
}

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
bool ADPCMDecoder::init(WaveFMT * format) {
    _wavefmt.encoding = SDL_SwapLE16(format->encoding);
    _wavefmt.channels = SDL_SwapLE16(format->channels);
    _wavefmt.frequency = SDL_SwapLE32(format->frequency);
    _wavefmt.byterate = SDL_SwapLE32(format->byterate);
    _wavefmt.blockalign = SDL_SwapLE16(format->blockalign);
    _wavefmt.bitspersample = SDL_SwapLE16(format->bitspersample);
    _blkbuffer = (Uint8*)SDL_malloc(_wavefmt.blockalign);
    return true;
}


#pragma mark -
#pragma mark MS ADPCM Decoder
/**
 * This struct represents the decoding state for MS ADPCM decoding
 *
 * This data is stored as a struct to simplify stereo decoding.
 */
typedef struct MS_state {
    Uint8 hPredictor;
    Uint16 iDelta;
    Sint16 iSamp1;
    Sint16 iSamp2;
} MS_state;

/**
 * This class represents an internal decoder for MS ADPCM decoding.
 *
 * This decoder is a proxy decoder for ADPCM files, which are compressed.
 * This class is a object oriented rewrite of the code in SDL_wave.c.
 */
class MSDecoder : public ADPCMDecoder {
protected:
    /** The number of decoding coefficients */
    Uint16   _numCoef;
    /** The decoding coefficients */
    Sint16   _coeff[7][2];
    /** The decoding state (for mono or stereo decoding) */
    MS_state _state[2];
    
public:
    /**
     * Creates an initialized decoder proxy
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    MSDecoder() : ADPCMDecoder() {}
    
    /**
     * Initializes a new decoder proxy from the given WAV header.
     *
     * This method will fail if the header (and file) is not compatible with 
     * MS ADPCM encoding.
     *
     * @param format    The WAV header
     *
     * @return true if the decoder proxy was initialized successfully
     */
    bool init(WaveFMT * format) override;
    
    /**
     * Creates a newly allocated decoder proxy from the given WAV header.
     *
     * This method will fail and return nullptr if the header (and file) is not 
     * compatible with MS ADPCM encoding.
     *
     * @param format    The WAV header
     *
     * @return a newly allocated decoder proxy from the given WAV header.
     */
    static std::shared_ptr<ADPCMDecoder> alloc(WaveFMT * format) {
        std::shared_ptr<MSDecoder> result = std::make_shared<MSDecoder>();
        if (result->init(format)) {
            return std::dynamic_pointer_cast<ADPCMDecoder>(result);
        }
        return nullptr;
    }
    
    /**
     * Returns a single sample extracted from the encoded data
     *
     * @param state     The decoder state
     * @param nybble    The byte to convert
     * @param coeff     The decoder coefficients
     *
     * @return a single sample extracted from the encoded data
     */
    Sint32 nibble(MS_state* state, Uint8 nybble, Sint16 * coeff);
    
    /**
     * Reads a single page from the given file.
     *
     * The buffer should be able to store block size * channels * 2 bytes of
     * data (the latter 2 representing sizeof(Sint16) ).
     *
     * @param source    The source file
     * @param buffer    The buffer to store the decoded data
     *
     * @return the number of bytes read
     */
    Sint32 read(SDL_RWops* _source, Uint8* buffer) final;
};


#pragma mark Method Implementations
/**
 * Initializes a new decoder proxy from the given WAV header.
 *
 * This method will fail if the header (and file) is not compatible with
 * MS ADPCM encoding.
 *
 * @param format    The WAV header
 *
 * @return true if the decoder proxy was initialized successfully
 */
bool MSDecoder::init(WaveFMT * format) {
    Uint8 *rogue_feel = (Uint8 *) format + sizeof(*format);
    if (sizeof(*format) == 16) {
        rogue_feel += sizeof(Uint16);
    }
    _blocksize = ((rogue_feel[1] << 8) | rogue_feel[0]);
    rogue_feel += sizeof(Uint16);
    
    _numCoef = ((rogue_feel[1] << 8) | rogue_feel[0]);
    rogue_feel += sizeof(Uint16);
    if (_numCoef != 7) {
        SDL_SetError("Unknown set of MS_ADPCM coefficients");
        return false;
    }
    
    for (int i = 0; i < _numCoef; ++i) {
        _coeff[i][0] = ((rogue_feel[1] << 8) | rogue_feel[0]);
        rogue_feel += sizeof(Uint16);
        _coeff[i][1] = ((rogue_feel[1] << 8) | rogue_feel[0]);
        rogue_feel += sizeof(Uint16);
    }
    
    return ADPCMDecoder::init(format);
}

/**
 * Returns a single sample extracted from the encoded data
 *
 * @param state     The decoder state
 * @param nybble    The byte to convert
 * @param coeff     The decoder coefficients
 *
 * @return a single sample extracted from the encoded data
 */
Sint32 MSDecoder::nibble(MS_state *state, Uint8 nybble, Sint16 * coeff) {
    const Sint32 max_audioval = ((1 << (16 - 1)) - 1);
    const Sint32 min_audioval = -(1 << (16 - 1));
    const Sint32 adaptive[] = {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
    };
    Sint32 new_sample, delta;
    
    new_sample = ((state->iSamp1 * coeff[0]) +
                  (state->iSamp2 * coeff[1])) / 256;
    if (nybble & 0x08) {
        new_sample += state->iDelta * (nybble - 0x10);
    } else {
        new_sample += state->iDelta * nybble;
    }
    if (new_sample < min_audioval) {
        new_sample = min_audioval;
    } else if (new_sample > max_audioval) {
        new_sample = max_audioval;
    }
    delta = ((Sint32) state->iDelta * adaptive[nybble]) / 256;
    if (delta < 16) {
        delta = 16;
    }
    state->iDelta = (Uint16) delta;
    state->iSamp2 = state->iSamp1;
    state->iSamp1 = (Sint16) new_sample;
    return (new_sample);
}

/**
 * Reads a single page from the given file.
 *
 * The buffer should be able to store block size * channels * 2 bytes of
 * data (the latter 2 representing sizeof(Sint16) ).  If the read fails,
 * this method returns -1.
 *
 * @param source    The source file
 * @param buffer    The buffer to store the decoded data
 *
 * @return the number of bytes read (or -1 on error)
 */
Sint32 MSDecoder::read(SDL_RWops* _source, Uint8* buffer) {
    // Read in a single block align
    if (!SDL_RWread(_source, _blkbuffer, _wavefmt.blockalign, 1)) {
        return -1;
    }
    
    MS_state *state[2];
    Uint8 *encoded, *decoded;
    Sint32 samplesleft;
    Sint32 output_len;
    Sint32 new_sample;
    Sint16 *coeff[2];
    Sint8 nybble;
    Uint8 stereo;
    
    // Adjustable pointers to input and output
    output_len = _blocksize * _wavefmt.channels * sizeof(Sint16);
    encoded = _blkbuffer;
    decoded = buffer;
    
    // Handle mono or stereo
    stereo = (_wavefmt.channels == 2);
    state[0] = &(_state[0]);
    state[1] = &(_state[stereo]);
    
    // Grab the initial information for this block
    state[0]->hPredictor = *encoded++;
    if (stereo) {
        state[1]->hPredictor = *encoded++;
    }
    state[0]->iDelta = ((encoded[1] << 8) | encoded[0]);
    encoded += sizeof(Sint16);
    if (stereo) {
        state[1]->iDelta = ((encoded[1] << 8) | encoded[0]);
        encoded += sizeof(Sint16);
    }
    state[0]->iSamp1 = ((encoded[1] << 8) | encoded[0]);
    encoded += sizeof(Sint16);
    if (stereo) {
        state[1]->iSamp1 = ((encoded[1] << 8) | encoded[0]);
        encoded += sizeof(Sint16);
    }
    state[0]->iSamp2 = ((encoded[1] << 8) | encoded[0]);
    encoded += sizeof(Sint16);
    if (stereo) {
        state[1]->iSamp2 = ((encoded[1] << 8) | encoded[0]);
        encoded += sizeof(Sint16);
    }
    coeff[0] = _coeff[state[0]->hPredictor];
    coeff[1] = _coeff[state[1]->hPredictor];
        
    // Store the two initial samples we start with
    decoded[0] = state[0]->iSamp2 & 0xFF;
    decoded[1] = state[0]->iSamp2 >> 8;
    decoded += 2;
    if (stereo) {
        decoded[0] = state[1]->iSamp2 & 0xFF;
        decoded[1] = state[1]->iSamp2 >> 8;
        decoded += 2;
    }
    decoded[0] = state[0]->iSamp1 & 0xFF;
    decoded[1] = state[0]->iSamp1 >> 8;
    decoded += 2;
    if (stereo) {
        decoded[0] = state[1]->iSamp1 & 0xFF;
        decoded[1] = state[1]->iSamp1 >> 8;
        decoded += 2;
    }
        
    // Decode and store the other samples in this block
    samplesleft = (_blocksize - 2) * _wavefmt.channels;
    while (samplesleft > 0) {
        nybble = (*encoded) >> 4;
        new_sample = nibble(state[0], nybble, coeff[0]);
        decoded[0] = new_sample & 0xFF;
        new_sample >>= 8;
        decoded[1] = new_sample & 0xFF;
        decoded += 2;
            
        nybble = (*encoded) & 0x0F;
        new_sample = nibble(state[1], nybble, coeff[1]);
        decoded[0] = new_sample & 0xFF;
        new_sample >>= 8;
        decoded[1] = new_sample & 0xFF;
        decoded += 2;
            
        ++encoded;
        samplesleft -= 2;
    }
    
    return output_len;
}


#pragma mark -
#pragma mark IMA ADPCM Decoder
/**
 * This struct represents the decoding state for IMA ADPCM decoding
 *
 * This data is stored as a struct to simplify stereo decoding.
 */
typedef struct IMA_state
{
    Sint32 sample;
    Sint8 index;
} IMA_state;

/**
 * This class represents an internal decoder for IMA ADPCM decoding.
 *
 * This decoder is a proxy decoder for ADPCM files, which are compressed.
 * This class is a object oriented rewrite of the code in SDL_wave.c.
 */
class IMADecoder : public ADPCMDecoder {
protected:
    /** The decoding state (for mono or stereo decoding) */
    IMA_state _state[2];
    
public:
    /**
     * Creates an initialized decoder proxy
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    IMADecoder() : ADPCMDecoder() {}
    
    /**
     * Initializes a new decoder proxy from the given WAV header.
     *
     * This method will fail if the header (and file) is not compatible with
     * IMA ADPCM encoding.
     *
     * @param format    The WAV header
     *
     * @return true if the decoder proxy was initialized successfully
     */
    bool init(WaveFMT * format) override;

    /**
     * Creates a newly allocated decoder proxy from the given WAV header.
     *
     * This method will fail and return nullptr if the header (and file) is not
     * compatible with IMA ADPCM encoding.
     *
     * @param format    The WAV header
     *
     * @return a newly allocated decoder proxy from the given WAV header.
     */
    static std::shared_ptr<ADPCMDecoder> alloc(WaveFMT * format) {
        std::shared_ptr<IMADecoder> result = std::make_shared<IMADecoder>();
        if (result->init(format)) {
            return std::dynamic_pointer_cast<ADPCMDecoder>(result);
        }
        return nullptr;
    }

    /**
     * Returns a single sample extracted from the encoded data
     *
     * @param state     The decoder state
     * @param nybble    The byte to convert
     *
     * @return a single sample extracted from the encoded data
     */
    Sint32 nibble(IMA_state* state, Uint8 nybble);
    
    /**
     * Fills the array with a single data block for the given channel
     *
     * This method reads 8 samples at a time into the decoded array.
     *
     * @param decoded       The place to store the decoded data
     * @param encoded       The encoded data to read
     * @param channel       The channel to decode
     * @param numchannels   The total number of channels
     * @param state         The decoder state
     */
    void fill(Uint8 * decoded, Uint8 * encoded, int channel, int numchannels, IMA_state *state);

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
    Sint32 read(SDL_RWops* _source, Uint8* buffer) final;
};


#pragma mark Method Implementations
/**
 * Initializes a new decoder proxy from the given WAV header.
 *
 * This method will fail if the header (and file) is not compatible with
 * IMA ADPCM encoding.
 *
 * @param format    The WAV header
 *
 * @return true if the decoder proxy was initialized successfully
 */
bool IMADecoder::init(WaveFMT * format) {
    Uint8 *rogue_feel = (Uint8 *) format + sizeof(*format);
    if (sizeof(*format) == 16) {
        rogue_feel += sizeof(Uint16);
    }
    _blocksize = ((rogue_feel[1] << 8) | rogue_feel[0]);
    return ADPCMDecoder::init(format);
}

/**
 * Returns a single sample extracted from the encoded data
 *
 * @param state     The decoder state
 * @param nybble    The byte to convert
 *
 * @return a single sample extracted from the encoded data
 */
Sint32 IMADecoder::nibble(IMA_state* state, Uint8 nybble) {
    const Sint32 max_audioval = ((1 << (16 - 1)) - 1);
    const Sint32 min_audioval = -(1 << (16 - 1));
    const int index_table[16] = {
        -1, -1, -1, -1,
        2, 4, 6, 8,
        -1, -1, -1, -1,
        2, 4, 6, 8
    };
    const Sint32 step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
        34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
        143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
        449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
        1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
        3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
        9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
        22385, 24623, 27086, 29794, 32767
    };
    Sint32 delta, step;
    
    // Compute difference and new sample value
    if (state->index > 88) {
        state->index = 88;
    } else if (state->index < 0) {
        state->index = 0;
    }
    
    // Explicit cast to avoid gcc warning about using 'char' as array index
    step = step_table[(int)state->index];
    delta = step >> 3;
    if (nybble & 0x04)
        delta += step;
    if (nybble & 0x02)
        delta += (step >> 1);
    if (nybble & 0x01)
        delta += (step >> 2);
    if (nybble & 0x08)
        delta = -delta;
    state->sample += delta;
    
    // Update index value
    state->index += index_table[nybble];
    
    // Clamp output sample
    if (state->sample > max_audioval) {
        state->sample = max_audioval;
    } else if (state->sample < min_audioval) {
        state->sample = min_audioval;
    }
    return (state->sample);
}

/**
 * Fills the array with a single data block for the given channel
 *
 * This method reads 8 samples at a time into the decoded array.
 *
 * @param decoded       The place to store the decoded data
 * @param encoded       The encoded data to read
 * @param channel       The channel to decode
 * @param numchannels   The total number of channels
 * @param state         The decoder state
 */
void IMADecoder::fill(Uint8 * decoded, Uint8 * encoded, int channel,
                      int numchannels, IMA_state *state) {
    Sint8 nybble;
    Sint32 new_sample;
    
    decoded += (channel * 2);
    for (int i = 0; i < 4; ++i) {
        nybble = (*encoded) & 0x0F;
        new_sample = nibble(state, nybble);
        decoded[0] = new_sample & 0xFF;
        new_sample >>= 8;
        decoded[1] = new_sample & 0xFF;
        decoded += 2 * numchannels;
        
        nybble = (*encoded) >> 4;
        new_sample = nibble(state, nybble);
        decoded[0] = new_sample & 0xFF;
        new_sample >>= 8;
        decoded[1] = new_sample & 0xFF;
        decoded += 2 * numchannels;
        
        ++encoded;
    }
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
Sint32 IMADecoder::read(SDL_RWops* _source, Uint8* buffer) {
    IMA_state *state;
    Uint8 *encoded, *decoded;
    Sint32 output_len, samplesleft;
    
    // Check to make sure we have enough variables in the state array
    if (_wavefmt.channels > SDL_arraysize(_state)) {
        SDL_SetError("IMA ADPCM decoder can only handle %u channels",
                     (unsigned int)SDL_arraysize(_state));
        return false;
    }
    state = _state;
    
    // Read in a single block align
    if (!SDL_RWread(_source, _blkbuffer, _wavefmt.blockalign, 1)) {
        return -1;
    }
    
    // Allocate the proper sized output buffer
    output_len = _blocksize * _wavefmt.channels * sizeof(Sint16);
    encoded = _blkbuffer;
    decoded = buffer;
    
    // Grab the initial information for this block
    for (Uint32 c = 0; c < _wavefmt.channels; ++c) {
        // Fill the state information for this block
        state[c].sample = ((encoded[1] << 8) | encoded[0]);
        encoded += 2;
        if (state[c].sample & 0x8000) {
            state[c].sample -= 0x10000;
        }
        state[c].index = *encoded++;
        // Reserved byte in buffer header, should be 0
        if (*encoded++ != 0) {
            // Uh oh, corrupt data?  Buggy code?
            assert(false);
        }
            
        // Store the initial sample we start with
        decoded[0] = (Uint8) (state[c].sample & 0xFF);
        decoded[1] = (Uint8) (state[c].sample >> 8);
        decoded += 2;
    }
        
    // Decode and store the other samples in this block
    samplesleft = (_blocksize - 1) * _wavefmt.channels;
    while (samplesleft > 0) {
        for (Uint32 c = 0; c < _wavefmt.channels; ++c) {
            fill(decoded, encoded, c, _wavefmt.channels, &state[c]);
            encoded += 4;
            samplesleft -= 8;
        }
        decoded += (_wavefmt.channels * 8 * 2);
    }

    return output_len;
}


#pragma mark -
#pragma mark WAV Decoder
/**
 * Creates an initialized audio decoder
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
WAVDecoder::WAVDecoder() : AudioDecoder(),
_source(nullptr),
_chunker(nullptr),
_adpcm(nullptr)
{}

/**
 * Deletes this decoder, disposing of all resources.
 */
WAVDecoder::~WAVDecoder() {
    if (_source != nullptr) {
        SDL_RWclose(_source);
        _source = nullptr;
    }
    
    if (_chunker != nullptr) {
        SDL_free(_chunker);
        _chunker = nullptr;
    }

    _adpcm = nullptr;
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
bool WAVDecoder::init(const std::string& file) {
    _file = file;
    if (bootstrap(file)) {
        _chunker = (Uint8 *)SDL_malloc(_pagesize*_channels*_sampsize);
        std::memset(_chunker,0,_pagesize*_channels*_sampsize);
        return true;
    }
    return false;
}

/**
 * Deletes the decoder resources and resets all attributes.
 *
 * This will close the associated file. You must reinitialize the decoder
 * to use it.
 */
void WAVDecoder::dispose() {
    if (_source != nullptr) {
        SDL_RWclose(_source);
        _source = nullptr;
    }
    
    if (_chunker != nullptr) {
        SDL_free(_chunker);
        _chunker = nullptr;
    }
    
    _adpcm = nullptr;
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
std::shared_ptr<AudioDecoder> WAVDecoder::alloc(const std::string& file) {
    std::shared_ptr<WAVDecoder> result = std::make_shared<WAVDecoder>();
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
Sint32 WAVDecoder::pagein(float* buffer) {
    // Read into the local chunk
    Uint32 avail = _pagesize*_channels*_sampsize;
    if (_currpage == _lastpage) {
        avail = (_frames % _pagesize)*_channels*_sampsize;
    } else if (_currpage > _lastpage) {
        avail = 0;
    }
    
    if (avail == 0) {
        return 0;
    } else if (isADPCM()) {
        if (!_adpcm->read(_source, _chunker)) {
            return 0;
        }
    } else {
        if (!SDL_RWread(_source, _chunker, avail, 1)) {
            return 0;
        }
    }
    
    // Now convert
    Uint32 temp = avail/_sampsize;
    float* output = buffer;
    switch (_sampbits) {
        case AUDIO_S16:
        {
            Sint16* input = (Sint16*)_chunker;
            double factor = 1.0/(1 << 16);
            while(temp--) {
                *output = *input*factor;
                output++;
                input++;
            }
        }
            break;
        case AUDIO_S8:
        {
            Sint8* input = (Sint8*)_chunker;
            double factor = 1.0/(1 << 8);
            while(temp--) {
                *output = *input*(factor);
                output++;
                input++;
            }
        }
            break;
        case AUDIO_S32:
        {
            Sint32* input = (Sint32*)_chunker;
            double factor = 1.0/(((Uint64)1) << 32);
            while(temp--) {
                *output = *input*(factor);
                output++;
                input++;
            }
        }
            break;
        case AUDIO_F32:
        {
            float* input = (float*)_chunker;
            while(temp--) {
                *output = *input;
                output++;
                input++;
            }
        }
            break;
            
        default:
            break;
    }
    
    _currpage++;
    return avail/(_channels*_sampsize);
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
void WAVDecoder::setPage(Uint64 page) {
    Uint64 offset = page*_pagesize*_channels*_sampsize;
    if (page >= _lastpage) {
        offset = _frames*_channels*_sampsize;
    }
    
    SDL_RWseek(_source, _datamark+offset, RW_SEEK_SET);
    _currpage = offset/(_pagesize*_channels*_sampsize);
}

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
bool WAVDecoder::bootstrap(const std::string& file) {
    Chunk chunk;
    
    int was_error;
    int lenread;
    
    // WAV magic header
    Uint32 RIFFchunk;
    Uint32 WAVEmagic;
    Uint32 wavelen = 0;
    
    // FMT chunk
    WaveFMT *format = NULL;
    
    // For seeking through chunk blocks
    Uint8* temp = NULL;
    Uint32 mark = JUNK;
    
    SDL_zero(chunk);
    
    _source = SDL_RWFromFile(file.c_str(),"r");
     was_error = 0;
    if (_source == NULL) {
        SDL_SetError("'%s' not found",file.c_str());
        was_error = 1;
        goto done;
    }
    
    // Check the magic header
    RIFFchunk = SDL_ReadLE32(_source);
    wavelen   = SDL_ReadLE32(_source);
    if (wavelen == WAVE) {
        // The RIFFchunk has already been read
        WAVEmagic = wavelen;
        wavelen = RIFFchunk;
        RIFFchunk = RIFF;
    } else {
        WAVEmagic = SDL_ReadLE32(_source);
    }
    
    if ((RIFFchunk != RIFF) || (WAVEmagic != WAVE)) {
        SDL_SetError("'%s' has unrecognized file type (not WAVE)", file.c_str());
        was_error = 1;
        goto done;
    }
    
    // Read the audio data format chunk
    chunk.data = NULL;
    do {
        SDL_free(chunk.data);
        chunk.data = NULL;
        lenread = readChunk(_source, &chunk);
        if (lenread < 0) {
            was_error = 1;
            goto done;
        }
    } while ((chunk.magic == FACT) || (chunk.magic == LIST) || (chunk.magic == BEXT) || (chunk.magic == JUNK));
    
    // Decode the audio data format
    format = (WaveFMT *) chunk.data;
    if (chunk.magic != FMT) {
        SDL_SetError("Complex WAVE files not supported");
        was_error = 1;
        goto done;
    }
    
    switch (SDL_SwapLE16(format->encoding)) {
        case PCM_CODE:
            // We can understand this
            _datatype = Type::PCM_DATA;
            break;
        case IEEE_FLOAT_CODE:
            // We can understand this
            _datatype = Type::IEEE_FLOAT;
            break;
        case MS_ADPCM_CODE:
            // Try to understand this
            _datatype = Type::MS_ADPCM;
            _adpcm = MSDecoder::alloc(format);
            if (_adpcm == nullptr) {
                was_error = 1;
                goto done;
            }
            break;
        case IMA_ADPCM_CODE:
            // Try to understand this
            _datatype = Type::MS_ADPCM;
            _adpcm = IMADecoder::alloc(format);
            if (_adpcm == nullptr) {
                was_error = 1;
                goto done;
            }
            break;
        case MP3_CODE:
            _datatype = Type::MP3_DATA;
            SDL_SetError("MPEG Layer 3 data is not supported in WAVE files");
            was_error = 1;
            goto done;
        default:
            _datatype = Type::UNKNOWN;
            SDL_SetError("Unknown WAVE data format: 0x%.4x",
                         SDL_SwapLE16(format->encoding));
            was_error = 1;
            goto done;
    }
    
    _rate = SDL_SwapLE32(format->frequency);
    _sampbits = AUDIO_S16;
    _channels = (Uint8) SDL_SwapLE16(format->channels);
    
    _sampsize = sizeof(Sint16);
    if (_datatype == Type::IEEE_FLOAT) {
        if ((SDL_SwapLE16(format->bitspersample)) != 32) {
            was_error = 1;
        } else {
            _sampbits = AUDIO_F32;
            _sampsize = sizeof(float);
        }
    } else {
        switch (SDL_SwapLE16(format->bitspersample)) {
            case 4:
                if (isADPCM()) {
                    _sampbits = AUDIO_S16;
                } else {
                    was_error = 1;
                }
                break;
            case 8:
                _sampbits = AUDIO_U8;
                _sampsize = sizeof(Uint8);
                break;
            case 16:
                _sampbits = AUDIO_S16;
                break;
            case 32:
                _sampbits = AUDIO_S32;
                _sampsize = sizeof(Sint32);
                break;
            default:
                was_error = 1;
                break;
        }
    }
    
    if (was_error) {
        SDL_SetError("Unknown %d-bit PCM data format",
                     SDL_SwapLE16(format->bitspersample));
        goto done;
    }
    
    // Seek ahead to the beginning of data
    do {
        mark = SDL_ReadLE32(_source);
        _frames = SDL_ReadLE32(_source);
        if (mark != DATA) {
            temp = (Uint8 *) SDL_malloc((size_t)_frames);
            if (SDL_RWread(_source, temp, (size_t)_frames, 1) != 1) {
                SDL_SetError("Data chunk is corrupted");
                was_error = 1;
                mark = DATA;
            }
            SDL_free(temp);
        }
    } while (mark != DATA);
    
    if (isADPCM()) {
        _pagesize = _adpcm->getBlockSize();
        _lastpage = (Uint32)(_frames/_pagesize);
        _frames  = _adpcm->getFrames(_frames);
    } else {
        // Good default buffer size
        _frames  /= (_sampsize*_channels);
        _pagesize = PAGE_SIZE/(_sampsize*_channels);
        _lastpage = (Uint32)(_frames/_pagesize);
    }
    
    _currpage = 0;
    _datamark = SDL_RWtell(_source);

done:
    SDL_free(format);
    if (was_error && _source != nullptr) {
        SDL_RWclose(_source);
        _source = nullptr;
        return false;
    }
    return !was_error;
}
