//
//  CUAudioSample.h
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
#ifndef __CU_AUDIO_SAMPLE_H__
#define __CU_AUDIO_SAMPLE_H__
#include <SDL/SDL.h>
#include <cugl/assets/CUJsonValue.h>
#include "CUSound.h"
#include <string>
#include <atomic>

namespace  cugl {
    /** Forward reference to a decoder type */
    namespace audio {
        class AudioDecoder;
    }
    
/**
 * This class represents a sample that can be played by a {@link audio::AudioPlayer}.
 *
 * This class provides support for both in-memory audio samples and streaming
 * audio. The former is ideal for sound effects, but not long-playing music.
 * The latter introduces some latency and is only ideal for long-playing music.
 *
 * The choice of buffered or streaming is independent of the file type.  
 * Currently, we support four file types: WAV (including ADPCM encodings), 
 * MP3, Ogg (Vorbis), and Flac.  As a general rule, we prefer WAV for sound 
 * effects and Ogg for music.
 *
 * All audio samples consist of float-formated PCM data. We assume channels are
 * interleaved.  We support up to 32 channels, though it is unlikely for that
 * many channels to be encoded in a sound file.  SDL itself only supports 8
 * channels for (7.1 surround) playback.
 */
class AudioSample : public Sound {
public:
#pragma mark File Types
    /**
     * This enum represents the possible audio sample sources.
     *
     * Right, we only support file types that are easy to stream into a
     * Linear PCM format.  We recommend that you use OGG for music and
     * WAV for sound effects.
     */
    enum class Type : int {
        /** An unknown audio file source */
        UNKNOWN   = -1,
        /** A (Windows-style) WAV file */
        WAV_FILE  = 0,
        /** A simple MP3 without VBR encoding */
        MP3_FILE  = 1,
        /** An ogg vorbis file */
        OGG_FILE  = 2,
        /** A FLAC file */
        FLAC_FILE = 3,
        /** An in-memory sound source (generator) */
        IN_MEMORY = 4
    };

protected:
    /** The number of frames in this audio sample */
    Uint64 _frames;
    
    /** The encoding type (WAV, MP3, OGG, FLAC) of this source */
    Type _type;
        
    /** Whether or not this sample is streamed or in-memory */
    bool _stream;

    /** The in-memory sound buffer for this sound source (OPTIONAL) */
    float* _buffer;
    
public:
#pragma mark Constructors
    /**
     * Creates a degenerate audio sample with no buffer.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    AudioSample();
    
    /**
     * Deletes this audio sample, disposing of all resources.
     */
    ~AudioSample() { dispose(); }
    
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
    bool init(const char* file, bool stream=false);
    
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
    bool init(const std::string& file, bool stream=false) {
        return init(file.c_str(),stream);
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
    bool init(Uint8 channels, Uint32 rate, Uint32 frames);
    
    /**
     * Deletes the sample resources and resets all attributes.
     *
     * This will delete the file reference and any allocated buffers. You must
     * reinitialize the sound data to use the object.
     */
    virtual void dispose() override;

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
    static Type guessType(const char* file);

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
    static Type guessType(const std::string& file) {
        return guessType(file.c_str());
    }
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated audio sample for the given file.
     *
     * The choice of buffered or streaming is independent of the file type.
     * If the file is streamed, it will not be loaded into memory.  Otherwise,
     * this initializer will allocate memory to read the asset into memory.
     *
     * @param file      The source file for the audio sample
     * @param stream    Wether to stream the audio from the file.
     *
     * @return a newly allocated audio sample for the given file.
     */
    static std::shared_ptr<AudioSample> alloc(const char* file, bool stream=false) {
        std::shared_ptr<AudioSample> result = std::make_shared<AudioSample>();
        return (result->init(file,stream) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated audio sample for the given file.
     *
     * The choice of buffered or streaming is independent of the file type.
     * If the file is streamed, it will not be loaded into memory.  Otherwise,
     * this initializer will allocate memory to read the asset into memory.
     *
     * @param file      The source file for the audio sample
     * @param stream    Wether to stream the audio from the file.
     *
     * @return a newly allocated audio sample for the given file.
     */
    static std::shared_ptr<AudioSample> alloc(const std::string& file, bool stream=false) {
        return alloc(file.c_str(), stream);
    }
    
    /**
     * Returns an empty audio sample of the given size.
     *
     * The audio sample will be in-memory (not streamed).  The contents of the
     * buffer will all be 0s.  Use the {@link getBuffer()} method to write data
     * to this buffer.
     *
     * @param channels  the number of audio channels
     * @param rate      the sampling rate of this source
     * @param frames    the number of frames in this source
     *
     * @return an empty audio sample of the given size.
     */
    static std::shared_ptr<AudioSample> alloc(Uint8 channels, Uint32 rate, Uint32 frames) {
        std::shared_ptr<AudioSample> result = std::make_shared<AudioSample>();
        return (result->init(channels,rate,frames) ? result : nullptr);
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
    static std::shared_ptr<AudioSample> allocWithData(const std::shared_ptr<JsonValue>& data);

        
#pragma mark Attributes
    /**
     * Returns true if this is an streaming audio asset.
     *
     * This method is to prevent the overhead of run-time typing.
     *
     * @return true if this is an streaming audio asset.
     */
    bool isStreamed() const { return _stream; }

    /**
     * Returns the encoding type for this audio sample
     *
     * The type should be one of WAV, MP3, OGG, or FLAC.
     *
     * @return the encoding type for this audio sample
     */
    Type getType() const { return _type; }
    
    /**
     * Returns the frame length of this audio sample.
     *
     * The frame length is the duration times the sample rate.
     *
     * @return the frame length of this audio sample.
     */
    virtual Sint64 getLength() const override { return _frames; }
    
    /**
     * Returns the length of this audio sample in seconds.
     *
     * The accuracy of this method depends on the specific implementation.
     *
     * @return the length of this audio sample in seconds.
     */
    virtual double getDuration() const override { return (double)_frames/(double)_rate; }
    
#pragma mark Playback Support
    /**
     * Returns the underlying PCM data buffer.
     *
     * This pointer will be null if the sample is streamed.  Otherwise, the
     * the buffer will contain channels * frames many elements. It is okay to
     * write data to the buffer, but it cannot be resized or reassigned.
     *
     * @return the underlying PCM data buffer.
     */
    float* getBuffer() { return _buffer; }
        
    /**
     * Returns a new decoder for this audio sample
     *
     * A decoder is used to extract the sound data into a PCM buffer.  It should
     * not be accessed directly. Instead it is used by the audio graph to acquire
     * playback data.
     *
     * @return a new decoder for this audio sample
     */
    std::shared_ptr<audio::AudioDecoder> getDecoder();
    
    /**
     * Returns a playble audio node for this asset.
     *
     * This audio node may be attached to an {@link audio::AudioOutput} for immediate
     * playback.  Nodes are distinct.  Each call to this method allocates
     * a new audio node.
     *
     * @return a playble audio node for this asset.
     */
    virtual std::shared_ptr<audio::AudioNode> createNode() override;
};
    
}


#endif /* __CU_AUDIO_SAMPLE_H__ */
