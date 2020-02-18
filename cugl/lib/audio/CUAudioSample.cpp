//
//  CUAudioSample.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides the base class for representing an audio sample, or
//  a file with prerecorded audio. An audio sample is not a node in the audio
//  graph.  Instead, a sample is provided to an AudioPlayer for playback.
//  Multiple AudioPlayers can share the same sample, allowing copies of the
//  sound to be played simultaneously
//
//  This module provides support for both in-memory audio samples and streaming
//  audio. The former is ideal for sound effects, but not long-playing music.
//  The latter introduces some latency and is only ideal for long-playing music.
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
//  Version: 11/20/18
//
#include <cugl/audio/CUAudioSample.h>
#include <cugl/audio/graph/CUAudioPlayer.h>
#include <cugl/util/CUDebug.h>
#include <cugl/audio/codecs/cu_codecs.h>

using namespace cugl;

#pragma mark Constructors

/**
 * Creates a degenerate audio sample with no buffer.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
AudioSample::AudioSample() : Sound(),
_frames(0),
_stream(false),
_buffer(nullptr) {
    _type = Type::UNKNOWN;
}

/**
 * Initializes a new audio sample for the given file.
 *
 * The choice of buffered or streaming is independent of the file type.
 * If the file is streamed, it will not be loaded into memory.  Otherwise,
 * this initializer will allocate memory to read the asset into memory.
 *
 * @param file      The source file for the audio sample
 * @param stream    Wether to stream the audio from the file.
 *
 * @return true if the sound source was initialized successfully
 */
bool AudioSample::init(const char* file, bool stream) {
    _file = file;
    _type = guessType(file);
    _stream = stream;
    
    std::shared_ptr<audio::AudioDecoder> decoder = getDecoder();
    if (decoder == nullptr) {
        CULogError("Could not open '%s': %s\n", file, SDL_GetError());
        return false;
    }
    
    _channels = decoder->getChannels();
    _frames = decoder->getLength();
    _rate   = decoder->getSampleRate();
    
    if (!_stream) {
        _buffer = (float*)SDL_malloc((size_t)(_frames*_channels*sizeof(float)));
        Sint64 size = decoder->decode(_buffer);
        return size >= 0;
    }
    return true;
}

/**
 * Initializes an empty audio sample of the given size.
 *
 * The audio sample will be in-memory (not streamed).  The contents of the
 * buffer will all be 0s.  Use the {@link getBuffer()} method to write data
 * to this buffer.
 *
 * @param channels  the number of audio channels
 * @param rate      the sampling rate of this source
 * @param frames    the number of frames in this source
 *
 * @return true if the audio sample was initialized successfully
 */
bool AudioSample::init(Uint8 channels, Uint32 rate, Uint32 frames) {
    _channels = channels;
    _frames = frames;
    _rate = rate;
    _buffer = (float*)SDL_malloc((size_t)(_channels*_frames*sizeof(float)));
    _stream = false;
    _type  = Type::IN_MEMORY;
    return true;
}

/**
 * Returns a newly allocated audio sample with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link SceneLoader}.  This JSON format supports the
 * following attribute values:
 *
 *      "file":     The path to the source, relative to the asset directory
 *      "stream":   A boolean, indicating whether to stream the sample
 *      "volume":   A float, representing the volume
 *
 * All attributes are optional.  There are no required attributes. By default,
 * audio samples are not streamed, meaning they are fully loaded into memory.
 * This is recommended for sound effects, but not for music.
 *
 * @param data      The JSON object specifying the audio sample
 *
 * @return a newly allocated audio sample with the given JSON specification.
 */
std::shared_ptr<AudioSample> AudioSample::allocWithData(const std::shared_ptr<JsonValue>& data) {
    std::string source = data->getString("file","");

    // Make sure we reference the asset directory
#if defined (__WINDOWS__)
    bool absolute = (bool)strstr(source.c_str(),":") || source[0] == '\\';
#else
    bool absolute = source[0] == '/';
#endif
    CUAssertLog(!absolute, "The asset directory should not referece absolute paths.");
    
    bool stream = data->getBool("stream",false);
    return AudioSample::alloc(source,stream);
}

/**
 * Deletes the sample resources and resets all attributes.
 *
 * This will delete the file reference and any allocated buffers. You must
 * reinitialize the sound data to use the object.
 */
void AudioSample::dispose() {
    _rate = 0;
    _frames = 0;
    _channels = 0;
    _stream = false;
    if (_buffer != nullptr) {
        SDL_free(_buffer);
        _buffer = nullptr;
    }
    _type = Type::UNKNOWN;
}

#pragma mark -
#pragma mark Decoder Supports
/**
 * Returns the type suggested by the given file name
 *
 * The type will be determined from the file extension (e.g. .wav, .mp3,
 * .ogg, etc).
 *
 * @param file  The file name
 *
 * @return the type suggested by the given file name
 */
AudioSample::Type AudioSample::guessType(const char* file) {
    Type type = Type::UNKNOWN;
    const char *ret = std::strrchr(file, '.');
    if (ret == nullptr) {
        return Type::UNKNOWN;
    }
    
    ret++;
    switch(*ret) {
        case 'w':
        case 'W':
            if ((strcasecmp(ret, "wav") == 0 || strcasecmp(ret, "wave") == 0)) {
                type = Type::WAV_FILE;
            }
            break;
        case 'm':
        case 'M':
            if ((strcasecmp(ret, "mp3") == 0 || strcasecmp(ret, "mpg") == 0)) {
                type = Type::MP3_FILE;
            }
            break;
        case 'o':
        case 'O':
            if ((strcasecmp(ret, "ogg") == 0 || strcasecmp(ret, "oga") == 0)) {
                type = Type::OGG_FILE;
            }
            break;
        case 'f':
        case 'F':
            if ((strcasecmp(ret, "flac") == 0 || strcasecmp(ret, "flc") == 0)) {
                type = Type::FLAC_FILE;
            }
            break;
        default:
            type = Type::UNKNOWN;
    }
    
    return type;
}

/**
 * Returns a new decoder for this audio sample
 *
 * A decoder is used to extract the sound data into a PCM buffer.  It should
 * not be accessed directly. Instead it is used by the audio graph to acquire
 * playback data.
 *
 * @return a new decoder for this audio sample
 */
std::shared_ptr<audio::AudioDecoder> AudioSample::getDecoder() {
    switch(_type) {
        case Type::WAV_FILE:
            return audio::WAVDecoder::alloc(_file);
            break;
        case Type::MP3_FILE:
            return audio::MP3Decoder::alloc(_file);
            break;
        case Type::OGG_FILE:
            return audio::OGGDecoder::alloc(_file);
            break;
        case Type::FLAC_FILE:
            return audio::FLACDecoder::alloc(_file);
        default:
            return nullptr;
    }
    
    return nullptr;
}

/**
 * Returns a playble audio node for this asset.
 *
 * This audio node may be attached to an {@link AudioOutput} for immediate
 * playback.  Nodes are distinct.  Each call to this method allocates
 * a new audio node.
 *
 * @return a playble audio node for this asset.
 */
std::shared_ptr<audio::AudioNode> AudioSample::createNode() {
    std::shared_ptr<Sound> source = shared_from_this();
    std::shared_ptr<audio::AudioPlayer> player = audio::AudioPlayer::alloc(std::dynamic_pointer_cast<AudioSample>(source));
    player->setGain(_volume);
    return std::dynamic_pointer_cast<audio::AudioNode>(player);
}
