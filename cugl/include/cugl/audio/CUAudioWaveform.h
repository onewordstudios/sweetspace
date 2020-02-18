//
//  CUAudioWaveform.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the interface for creating a single-frequency waveform.
//  Examples include sine waves, square waves, sawtooth waves, and triangle
//  waves. This waveform may serve as the source node in an audio graph.
//
//  This module is not intended to be "music quality".  The audio waveforms
//  are good enough for procedural sound generation in most games.  In
//  particular, we use the PolyBLEP technique for bandwidth-limiting our
//  various waveforms. This technique is known to have audible aliasing near
//  the Nyquist frequency and overly attenuate higher frequencies.  However,
//  it is compact and ideal for real-time sound generation.  Because the
//  problems are less of an issue for sound-effect generation, this the
//  approach that we have chosen.
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
//  Version: 12/20/18
//
#ifndef __CU_AUDIO_WAVE_FORM_H__
#define __CU_AUDIO_WAVE_FORM_H__
#include <SDL/SDL.h>
#include <cugl/assets/CUJsonValue.h>
#include "CUSound.h"
#include <random>

namespace  cugl {
/**
 * This class represents a single-frequency waveform.
 *
 * Intuitively, this class is used to represent a pure sine wave, which can
 * be read and included in an audio graph.  However, this class also supports
 * more traditional computer-music waveforms, like square waves and sawtooth
 * waves.  The type of waveform is specified by the {@link getType()} attribute.
 *
 * We support both naive waveforms and bandwidth-limited forms. Bandwidth-limited
 * forms are design to reduce the aliasing that can occur at the discontinuites:
 *
 *     https://ccrma.stanford.edu/~stilti/papers/blit.pdf
 *
 * For simplicity, we do not use the BLIT integration techniques of Stilson
 * and Smith.  Those techniques are subject to error creep over time unless a
 * a backing table is used.  Instead, we use the PolyBLEP technique:
 *
 *     https://ieeexplore.ieee.org/document/4117934
 *
 * This technique is not "music quality." It is known to have audible aliasing
 * near the Nyquist frequency and overly attenuate higher frequencies. However,
 * it is compact and ideal for real-time sound generation. It is also good
 * enough for procedural sound generation in most games.
 *
 * The audio graph should only be accessed in the main thread.  In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the user.
 *
 * This class does not support any actions for the {@link audio::AudioNode#setCallback}.
 */
class AudioWaveform : public Sound {
public:
    /** The default fundamental frequency */
    const static float DEFAULT_FREQUENCY;
    
    /**
     * The wave generator type.
     *
     * These types are inspired by STK, the Synthesis Toolkit (even though we
     * chose not to adopt BLIT integration). They are not necessarily complete
     * and additional wave form types may be added at any time. For that reason,
     * you should never refer to a type by its raw number.
     */
    enum class Type : int {
        /**
         * Creates random noise using the C rand() function.  The quality of
         * the rand() function varies from one OS to another.
         */
        NOISE           = 0,
        /**
         * Creates a sine wave with the given frequency.
         */
        SINE            = 1,
        /**
         * Creates a naive square triangular with the given frequency.
         *
         * The waveform will have first-order discontinuities at PI and 2PI.
         * This will create a more smoother sound than a square or sawtooth
         * wave of the same frequency.
         */
        NAIVE_TRIANG    = 2,
        /**
         * Creates a naive square wave with the given frequency.
         *
         * The waveform will have discontinuities at PI and 2PI. This will
         * create a harsh sound reminiscent of old-school games.
         */
        NAIVE_SQUARE    = 3,
        /**
         * Creates a naive sawtooth wave with the given frequency.
         *
         * The waveform will have a discontinuity at 2PI. This will create a
         * harsh sound reminiscent of old-school games.
         */
        NAIVE_TOOTH     = 4,
        /**
         * Creates an alternating sign impulse train with the given frequency.
         *
         * The frequence given is the twice the period of the impulse because
         * the signs will alternate when {@link isUpper()} is false.
         */
        NAIVE_TRAIN     = 5,
        /**
         * Creates a bandwidth-limited triangle wave with the given frequency.
         *
         * The algorithm uses a PolyBLEP curve to create a bandwidth-limited
         * square wave, as reported in "Antialiasing Oscillators in Subtractive
         * Synthesis" by Valimaki and Huovilainen (2007).  This wave is then
         * integrated to produce a triangle wave, using the leaky integration
         * in "Alias-Free Digital Synthesis of Classic Analog Waveforms" by
         * Stilson and Smith (1996). This particular version is adapted from
         *
         *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
         */
        POLY_TRIANG     = 6,
        /**
         * Creates a bandwidth-limited square wave with the given frequency.
         *
         * The algorithm uses a PolyBLEP curve as reported in "Antialiasing
         * Oscillators in Subtractive Synthesis" by Valimaki and Huovilainen
         * (2007). This particular version is adapted from
         *
         *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
         */
        POLY_SQUARE     = 7,
        /**
         * Creates a bandwidth-limited sawtooth wave with the given frequency.
         *
         * The algorithm uses a PolyBLEP curve as reported in "Antialiasing
         * Oscillators in Subtractive Synthesis" by Valimaki and Huovilainen
         * (2007). This particular version is adapted from
         *
         *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
         */
        POLY_TOOTH      = 8,
        /**
         * Creates a band-limited impulse train.
         *
         * This algorithm uses the closed-form algorithm  "Alias-Free Digital
         * Synthesis of Classic Analog Waveforms" by Stilson and Smith (1996).
         * This implementation assumes the maximum number of harmonics.
         *
         * Based on code by Robin Davies, Gary Scavone, 2005 - 2006.
         */
        BLIT_TRAIN     = 9,
        /**
         * An unknown type
         */
        UNKNOWN        = 10
    };
    
protected:
    /** Atomic proxy for the signal type */
    std::atomic<int>    _type;
    /** Whether to limit the waveform to the positive y-axis. */
    std::atomic<bool>   _upper;
    /** The (normalized) fundamental frequency */
    std::atomic<float>  _frequency;
    /** Whether the frequency has changed recently */
    std::atomic<bool>   _newfreq;
    /** The duration in seconds; -1 if infinite */
    std::atomic<double> _duration;

    /** The random generator for noise */
    std::minstd_rand0   _random;
    
public:
#pragma mark Constructors
    /**
     * Creates a degenerate waveform with no frequency.
     *
     * The waveform has no channels or frequency, so read options will do nothing.
     * The waveform must be initialized to be used.
     */
    AudioWaveform();
    
    /**
     * Deletes this waveform, disposing of all resources.
     */
    ~AudioWaveform() { dispose(); }
    
    /**
     * Initializes a stereo sine wave of 480 Hz.
     *
     * When included in an audio graph, the node will support 2 channels at a
     * sampling rate of 48000 Hz.
     *
     * @return true if initialization was successful
     */
    bool init();
    
    /**
     * Initializes a sine wave of 480 Hz.
     *
     * When included in an audio graph, the node will support the given number
     * of channels at the given sampling rate.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     *
     * @return true if initialization was successful
     */
    bool init(Uint8 channels, Uint32 rate);
    
    /**
     * Initializes a wave form of the given type and frequency.
     *
     * The frequency is specified is the fundamental frequency of the
     * wave form. However, for the {@link Type#NOISE} type, it is the seed
     * of the random number generator.
     *
     * The frequency specified is independent of the sampling rate. The
     * wave form algorithms will create the correct date for both the sampling
     * rate and frequency.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param type      The waveform type
     * @param frequency The waveform fundamental frequency
     *
     * @return true if initialization was successful
     */
    bool init(Uint8 channels, Uint32 rate, Type type, float frequency);
    
    /**
     * Disposes any resources allocated for this waveform
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated sine wave of 480 Hz.
     *
     * When included in an audio graph, the node will support 2 channels at a
     * sampling rate of 48000 Hz.
     *
     * @return a newly allocated sine wave of 480 Hz.
     */
    static std::shared_ptr<AudioWaveform> alloc() {
        std::shared_ptr<AudioWaveform> result = std::make_shared<AudioWaveform>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated sine wave of 480 Hz.
     *
     * When included in an audio graph, the node will support the given number
     * of channels at the given sampling rate.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     *
     * @return a newly allocated sine wave of 480 Hz.
     */
    static std::shared_ptr<AudioWaveform> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioWaveform> result = std::make_shared<AudioWaveform>();
        return (result->init(channels,rate) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated wave form of the given type and frequency.
     *
     * The frequency is specified is the fundamental frequency of the
     * wave form. However, for the {@link Type#NOISE} type, it is the seed
     * of the random number generator.
     *
     * The frequency specified is independent of the sampling rate. The
     * wave form algorithms will create the correct date for both the sampling
     * rate and frequency.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param type      The waveform type
     * @param frequency The waveform fundamental frequency
     *
     * @return a newly allocated wave form of the given type and frequency.
     */
    static std::shared_ptr<AudioWaveform> alloc(Uint8 channels, Uint32 rate, Type type, float frequency) {
        std::shared_ptr<AudioWaveform> result = std::make_shared<AudioWaveform>();
        return (result->init(channels,rate,type,frequency) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated waveform with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports the
     * following attribute values:
     *
     *      "shape":    The wave shape as a string (e.g. "sine", "triangle")
     *      "tone":     A float, representing the frequency
     *      "channels": An int, representing the number of channels
     *      "rate":     An int, representing the sample rate
     *      "volume":   A float, representing the volume
     *      "duration"  A float, representing the duration in seconds
     *
     * All attributes are optional.  There are no required attributes. The
     * recognized shapes are as follows: noise, sine, native triangle, naive
     * square, naive sawtooth, naive impulse, triangle, square, sawtooth,
     * and impulse.  The non-naive names are all bandwidth limited.
     *
     * @param data      The JSON object specifying the waveform
     *
     * @return a newly allocated waveform with the given JSON specification.
     */
    static std::shared_ptr<AudioWaveform> allocWithData(const std::shared_ptr<JsonValue>& data);

#pragma mark Generator Attributes
    /**
     * Returns the waveform type.
     *
     * @return the waveform type.
     */
    Type getType() const;
    
    /**
     * Sets the waveform type.
     *
     * @param type  the waveform type.
     */
    void setType(Type type);

    /**
     * Returns true if the waveform has only nonnegative samples.
     *
     * Mathematically, we sometimes want a waveform to have only non-negative
     * values.  For an impulse train, this means a train with only positive
     * poles (as opposed to a bipolar train).  For triangle, square, and
     * sawtooth waves, the result is a waveform of the same shape but from
     * 0 to 1 instead of -1 to 1.  For a sine wave, the result is the absolute
     * value (or a rectified sine wave).  For noise, this has no effect.
     *
     * @return true if the waveform has only nonnegative samples.
     */
    bool isUpper() const;
    
    /**
     * Sets whether the waveform has only nonnegative samples.
     *
     * Mathematically, we sometimes want a waveform to have only non-negative
     * values.  For an impulse train, this means a train with only positive
     * poles (as opposed to a bipolar train).  For triangle, square, and
     * sawtooth waves, the result is a waveform of the same shape but from
     * 0 to 1 instead of -1 to 1.  For a sine wave, the result is the absolute
     * value (or a rectified sine wave).  For noise, this has no effect.
     *
     * @param upper Whether the waveform has only nonnegative samples.
     */
    void setUpper(bool upper);

    /**
     * Returns the fundamental frequency of this waveform.
     *
     * @return the fundamental frequency of this waveform.
     */
    float getFrequency() const;
    
    /**
     * Sets the fundamental frequency of this waveform.
     *
     * @param frequency the fundamental frequency of this waveform.
     */
    void setFrequency(float frequency);
    
    /**
     * Returns the frame length of this waveform.
     *
     * The frame length is the number of audio samples in the asset.  If the
     * asset is infinite (e.g. {@link AudioWaveform}), then this method returns
     * a negative value.
     *
     * @return the frame length of this waveform.
     */
    virtual Sint64 getLength() const override;
    
    /**
     * Returns the length of this waveform in seconds.
     *
     * The accuracy of this method depends on the specific implementation. If
     * the asset is infinite (e.g. {@link AudioWaveform}), then this method
     * returns a negative value.
     *
     * @return the length of this waveform in seconds.
     */
    virtual double getDuration() const override;
    
    /**
     * Sets the length of this waveform in seconds.
     *
     * The accuracy of this method depends on the specific implementation. If
     * the asset is infinite (e.g. {@link AudioWaveform}), then this value is
     * negative.
     *
     * @param time the length of this waveform in seconds.
     */
    void setDuration(double time);
    
    /**
     * Generates the given number of frames for the waveform fom the given offset.
     *
     * This function is used by {@link audio::AudioNode} to generate the correct data
     * for each type. For reasons of precision, the offset is given in frames
     * and not the phase (which is real-valued).
     *
     * Some waveforms require discrete integration.  This is the purpose of
     * last, which was the last sample generated.  It is up to the user to
     * remember this value.  The method returns the frame position of the
     * last sample generated.
     *
     * @param buffer    The read buffer to store the results
     * @param frames    The maximum number of frames to read
     * @param offset    The phase offset measured in frames
     * @param last      The last value generated (for integration purposes)
     *
     * @return the number of frames generated
     */
    Uint32 generate(float* buffer, Uint32 frames, Uint64 offset, float last);
    
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

#endif /* __CU_AUDIO_WAVE_FORM_H__ */
