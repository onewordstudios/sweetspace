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
#include <cugl/audio/CUAudioWaveform.h>
#include <cugl/audio/graph/CUAudioNode.h>
#include <cugl/math/dsp/CUDSPMath.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUStrings.h>
#include <chrono>
#include <algorithm>

using namespace cugl;

/**
 * Returns the value of a PolyBLEP curve at time t.
 *
 * This code is adapted from "Antialiasing Oscillators in Subtractive Synthesis"
 * by Valimaki and Huovilainen (2007), more specifically the version at
 *
 *     http://www.kvraudio.com/forum/viewtopic.php?t=375517
 *
 * @param t     the time parameter
 * @param dt    the time resolution (frequency/rate)
 *
 * @return the value of a PolyBLEP curve at time t.
 */
double poly_blep(double t, double dt) {
    t = std::fmod(t,1);
    
    if (t < dt) {               // 0 <= t < 1
        t /= dt;
        return t+t - t*t - 1.0;
    } else if (t > 1.0 - dt) {  // -1 < t < 0
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }
    
    // 0 otherwise
    return 0.0;
}

#pragma mark -
#pragma mark AudioWaveNode Interface

/**
 * This class is an audio node instance of a waveform.
 *
 * Instances of this class should never be allocated directly.  They are created
 * by the {@link AudioWaveform#createNode()} method.
 *
 * A wavenode is always associated with a node in the audio graph. As such, it
 * should only be accessed in the main thread.  In addition, no methods marked
 * as AUDIO THREAD ONLY should ever be accessed by the user. The only exception
 * to this rule is by another (custom) audio graph node in its audio thread
 * methods.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioWaveNode : public audio::AudioNode {
protected:
    /** The generating waveform */
    std::shared_ptr<AudioWaveform> _waveform;

    /** To allow manual stopping of this node */
    std::atomic<Sint64> _timeout;
    /** The current read position */
    std::atomic<Uint64> _offset;
    /** The last marked position (starts at 0) */
    std::atomic<Uint64> _marked;

    /** A reference to the last sample created (for integration purposes). */
    float _last;

public:
    /**
     * Creates a degenerate waveform with no frequency.
     *
     * The waveform has no channels or frequency, so read options will do nothing.
     * The waveform must be initialized to be used.
     */
    AudioWaveNode();
    
    /**
     * Deletes this waveform, disposing of all resources.
     */
    ~AudioWaveNode() { dispose(); }
    
    /**
     * Initializes a stereo sine wave of 480 Hz.
     *
     * When included in an audio graph, the node will support 2 channels at a
     * sampling rate of 48000 Hz.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
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
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
    /**
     * Initializes a node for the given waveform.
     *
     * When included in an audio graph, the node will support the same number
     * of channels and sampling rate as the underlying waveform.
     *
     * @param waveform  The waveform to instantiate
     *
     * @return true if initialization was successful
     */
    bool init(const std::shared_ptr<AudioWaveform>& waveform);
    
    /**
     * Disposes any resources allocated for this waveform node.
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
    /**
     * Returns a newly allocated sine wave of 480 Hz.
     *
     * When included in an audio graph, the node will support 2 channels at a
     * sampling rate of 48000 Hz.
     *
     * @return a newly allocated sine wave of 480 Hz.
     */
    static std::shared_ptr<AudioWaveNode> alloc() {
        std::shared_ptr<AudioWaveNode> result = std::make_shared<AudioWaveNode>();
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
    static std::shared_ptr<AudioWaveNode> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AudioWaveNode> result = std::make_shared<AudioWaveNode>();
        return (result->init(channels,rate) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated a node for the given waveform.
     *
     * When included in an audio graph, the node will support the same number
     * of channels and sampling rate as the underlying waveform.
     *
     * @param waveform  The waveform to instantiate
     *
     * @return a newly allocated a node for the given waveform.
     */
    static std::shared_ptr<AudioWaveNode> alloc(const std::shared_ptr<AudioWaveform>& waveform) {
        std::shared_ptr<AudioWaveNode> result = std::make_shared<AudioWaveNode>();
        return (result->init(waveform) ? result : nullptr);
    }
    
    /**
     * Reads up to the specified number of frames into the given buffer
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * The buffer should have enough room to store frames * channels elements.
     * The channels are interleaved into the output buffer.
     *
     * This method will always forward the read position after reading. Reading
     * again may return different data.
     *
     * @param buffer    The read buffer to store the results
     * @param frames    The maximum number of frames to read
     *
     * @return the actual number of frames read
     */
    virtual Uint32 read(float* buffer, Uint32 frames) override;
    
    /**
     * Returns true if this audio node has no more data.
     *
     * A completed audio node is one that will return 0 (no frames read) on
     * subsequent threads read.
     *
     * @return true if this audio node has no more data.
     */
    virtual bool completed() override;
    
    /**
     * Marks the current read position in the audio steam.
     *
     * This method is used by {@link reset()} to determine where to restore
     * the read position.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * Clearing the mark in a player is equivelent to setting the mark at
     * the beginning of the audio asset.  Future calls to {@link reset()}
     * will return to the start of the audio stream.
     *
     * @return true if the read position was cleared.
     */
    virtual bool unmark() override;
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * If no mark is set, this will reset to the player to the beginning of
     * the audio sample.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;
    
    /**
     * Advances the stream by the given number of frames.
     *
     * This method only advances the read position, it does not actually
     * read data into a buffer.
     *
     * @param frames    The number of frames to advace
     *
     * @return the actual number of frames advanced; -1 if not supported
     */
    virtual Sint64 advance(Uint32 frames) override;
    
    /**
     * Returns the current frame position of this audio node
     *
     * The value returned will always be the absolute frame position regardless
     * of the presence of any marks.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * The value set will always be the absolute frame position regardless
     * of the presence of any marks.
     *
     * @param position  the current frame position of this audio node.
     *
     * @return the new frame position of this audio node.
     */
    virtual Sint64 setPosition(Uint32 position) override;
    
    /**
     * Returns the elapsed time in seconds.
     *
     * The value returned is always measured from the start of the steam,
     * regardless of the presence of any marks.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * The value returned is always measured from the start of the steam,
     * regardless of the presence of any marks.
     *
     * @param time  The elapsed time in seconds.
     *
     * @return the new elapsed time in seconds.
     */
    virtual double setElapsed(double time) override;
    
    /**
     * Returns the remaining time in seconds.
     *
     * The remaining time is duration from the current read position to the
     * end of the sample.  It is not effected by any fade-out.
     *
     * @return the remaining time in seconds.
     */
    virtual double getRemaining() const override;
    
    /**
     * Sets the remaining time in seconds.
     *
     * This method will move the read position so that the distance between
     * it and the end of the same is the given number of seconds.
     *
     * @param time  The remaining time in seconds.
     *
     * @return the new remaining time in seconds.
     */
    virtual double setRemaining(double time) override;
};

#pragma mark -
#pragma mark AudioWaveNode Implementation

/**
 * Creates a degenerate waveform with no frequency.
 *
 * The waveform has no channels or frequency, so read options will do nothing.
 * The waveform must be initialized to be used.
 */
AudioWaveNode::AudioWaveNode() : audio::AudioNode(),
_timeout(-1),
_offset(0),
_marked(0),
_last(0.0f) {
    _waveform = nullptr;
    _classname = "AudioWaveNode";
}

/**
 * Initializes a stereo sine wave of 480 Hz.
 *
 * When included in an audio graph, the node will support 2 channels at a
 * sampling rate of 48000 Hz.
 *
 * @return true if initialization was successful
 */
bool AudioWaveNode::init() {
    if (audio::AudioNode::init()) {
        _waveform = AudioWaveform::alloc();
        return true;
    }
    return false;
}

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
bool AudioWaveNode::init(Uint8 channels, Uint32 rate) {
    if (audio::AudioNode::init(channels,rate)) {
        _waveform = AudioWaveform::alloc(channels,rate);
        return true;
    }
    return false;
}

bool AudioWaveNode::init(const std::shared_ptr<AudioWaveform>& waveform) {
    if (audio::AudioNode::init(waveform->getChannels(),waveform->getRate())) {
        _waveform = waveform;
        return true;
    }
    return false;
}

/**
 * Disposes any resources allocated for this waveform node.
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioWaveNode::dispose() {
    if (_booted) {
        AudioNode::dispose();
        _waveform = nullptr;
        _offset.store(0);
        _marked.store(0);
        _timeout.store(-1);
        _last = 0.0f;
    }

}

#pragma mark Read Forward Access
/**
 * Reads up to the specified number of frames into the given buffer
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioNode.
 *
 * The buffer should have enough room to store frames * channels elements.
 * The channels are interleaved into the output buffer.
 *
 * This method will always forward the read position after reading. Reading
 * again may return different data.
 *
 * @param buffer    The read buffer to store the results
 * @param frames    The maximum number of frames to read
 *
 * @return the actual number of frames read
 */
Uint32 AudioWaveNode::read(float* buffer, Uint32 frames) {
    if (_paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,frames*sizeof(float)*_channels);
        return frames;
    }
    Sint64 timeout = _timeout.load(std::memory_order_relaxed);
    Uint32 amt = frames;
    if (timeout > 0) {
        amt = std::min((Uint32)timeout,frames);
    }
    
    _polling.store(true);
    Uint64 offset = _offset.load(std::memory_order_relaxed);
    Uint32 read = _waveform->generate(buffer,amt,offset,_last);
    dsp::DSPMath::scale(buffer,_ndgain.load(std::memory_order_relaxed),buffer,read*_channels);
    if (read > 0) {
        _last = buffer[(read-1)*_channels];
    }
    _offset.store(offset+read,std::memory_order_relaxed);
    if (timeout > 0) {
        _timeout.store(timeout-amt,std::memory_order_relaxed);
    } else if (read < amt) {
        _timeout.store(0,std::memory_order_relaxed);
    }
    _polling.store(false);
    return read;
}

/**
 * Returns true if this audio node has no more data.
 *
 * A completed audio node is one that will return 0 (no frames read) on
 * subsequent threads read.
 *
 * @return true if this audio node has no more data.
 */
bool AudioWaveNode::completed() {
    return _timeout.load(std::memory_order_relaxed) == 0;
}

/**
 * Marks the current read position in the audio steam.
 *
 * This method is used by {@link reset()} to determine where to restore
 * the read position.
 *
 * @return true if the read position was marked.
 */
bool AudioWaveNode::mark() {
    _marked.store(_offset.load(std::memory_order_relaxed),std::memory_order_relaxed);
    return true;
}

/**
 * Clears the current marked position.
 *
 * Clearing the mark in a player is equivelent to setting the mark at
 * the beginning of the audio asset.  Future calls to {@link reset()}
 * will return to the start of the audio stream.
 *
 * @return true if the read position was cleared.
 */
bool AudioWaveNode::unmark() {
    _marked.store(0,std::memory_order_relaxed);
    return true;
}

/**
 * Resets the read position to the marked position of the audio stream.
 *
 * If no mark is set, this will reset to the player to the beginning of
 * the audio sample.
 *
 * @return true if the read position was moved.
 */
bool AudioWaveNode::reset() {
    _offset.store(_marked.load(std::memory_order_relaxed),std::memory_order_relaxed);
    _timeout.store(-1,std::memory_order_relaxed);
    return true;
}

/**
 * Advances the stream by the given number of frames.
 *
 * This method only advances the read position, it does not actually
 * read data into a buffer.
 *
 * @param frames    The number of frames to advace
 *
 * @return the actual number of frames advanced; -1 if not supported
 */
Sint64 AudioWaveNode::advance(Uint32 frames) {
    return setPosition((Uint32)_offset.load(std::memory_order_relaxed)+frames);
}


#pragma mark Random Access
/**
 * Sets the current frame position of this audio node.
 *
 * The value set will always be the absolute frame position regardless
 * of the presence of any marks.
 *
 * @param position  the current frame position of this audio node.
 *
 * @return the new frame position of this audio node.
 */
Sint64 AudioWaveNode::setPosition(Uint32 position) {
    _offset.store(position,std::memory_order_relaxed);
    return (Sint64)position;
}

/**
 * Returns the current frame position of this audio node
 *
 * The value returned will always be the absolute frame position regardless
 * of the presence of any marks.
 *
 * @return the current frame position of this audio node.
 */
Sint64 AudioWaveNode::getPosition() const {
    return (Sint64)_offset.load(std::memory_order_relaxed);
}

/**
 * Sets the read position to the elapsed time in seconds.
 *
 * The value returned is always measured from the start of the steam,
 * regardless of the presence of any marks.
 *
 * @param time  The elapsed time in seconds.
 *
 * @return the new elapsed time in seconds.
 */
double AudioWaveNode::setElapsed(double time) {
    double result = 0.0;
    Uint64 off = 0;
    if (time <= 0) {
        result = 0.0;
        off = 0;
    } else {
        off = (Uint64)(time*_sampling);
        result = time;
    }
    _offset.store(off, std::memory_order_relaxed);
    return result;
}

/**
 * Returns the elapsed time in seconds.
 *
 * The value returned is always measured from the start of the steam,
 * regardless of the presence of any marks.
 *
 * @return the elapsed time in seconds.
 */
double AudioWaveNode::getElapsed() const {
    Uint64 offset = _offset.load(std::memory_order_relaxed);
    return (double)offset/(double)_sampling;
}

/**
 * Returns the remaining time in seconds.
 *
 * The remaining time is duration from the current read position to the
 * end of the sample. It is not effected by any fade-out.
 *
 * @return the remaining time in seconds.
 */
double AudioWaveNode::getRemaining() const {
    Sint64 timeout = _timeout.load(std::memory_order_relaxed);
    if (timeout < 0) {
        return -1;
    }
    return (double)timeout/(double)_sampling;
}

/**
 * Sets the remaining time in seconds.
 *
 * This method will move the read position so that the distance between
 * it and the end of the same is the given number of seconds.
 *
 * @param time  The remaining time in seconds.
 *
 * @return the new remaining time in seconds.
 */
double AudioWaveNode::setRemaining(double time) {
    double result = 0.0;
    Uint64 timeout = time*_sampling;
    _timeout.store(timeout,std::memory_order_relaxed);
    return result;
}

#pragma mark -
#pragma mark AudioWaveform

/** The default fundamental frequency */
const float AudioWaveform::DEFAULT_FREQUENCY = 480.0f;


#pragma mark Constructors
/**
 * Creates a degenerate waveform with no frequency.
 *
 * The waveform has no channels or frequency, so read options will do nothing.
 * The waveform must be initialized to be used.
 */
AudioWaveform::AudioWaveform() : Sound(),
_type(0),
_upper(false),
_newfreq(false),
_duration(-1),
_frequency(-1) {
}

/**
 * Initializes a stereo sine wave of 480 Hz.
 *
 * When included in an audio graph, the node will support 2 channels at a
 * sampling rate of 48000 Hz.
 *
 * @return true if initialization was successful
 */
bool AudioWaveform::init() {
    return init(audio::AudioNode::DEFAULT_CHANNELS,audio::AudioNode::DEFAULT_SAMPLING,Type::SINE,DEFAULT_FREQUENCY);
}

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
bool AudioWaveform::init(Uint8 channels, Uint32 rate) {
    return init(channels,rate,Type::SINE,DEFAULT_FREQUENCY);
}

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
bool AudioWaveform::init(Uint8 channels, Uint32 rate, Type type, float frequency) {
    _channels  = channels;
    _frequency = frequency;
    _newfreq = true;
    _rate = rate;
    _type = (int)type;
    return type != AudioWaveform::Type::UNKNOWN;
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
std::shared_ptr<AudioWaveform> AudioWaveform::allocWithData(const std::shared_ptr<JsonValue>& data) {
    std::string shape = data->getString("shape","sine");
    shape = cugl::to_lower(shape);
    
    float frequency = data->getFloat("tone",DEFAULT_FREQUENCY);
    Uint32 channels = data->getInt("channels",audio::AudioNode::DEFAULT_CHANNELS);
    Uint32 sampling = data->getInt("rate",audio::AudioNode::DEFAULT_SAMPLING);
    
    AudioWaveform::Type type = AudioWaveform::Type::UNKNOWN;
    if (shape == "noise") {
        type = AudioWaveform::Type::NOISE;
    } else if (shape == "sine") {
        type = AudioWaveform::Type::SINE;
    } else if (shape == "naive triangle") {
        type = AudioWaveform::Type::NAIVE_TRIANG;
    } else if (shape == "naive square") {
        type = AudioWaveform::Type::NAIVE_SQUARE;
    } else if (shape == "naive sawtooth") {
        type = AudioWaveform::Type::NAIVE_TOOTH;
    } else if (shape == "naive impulse") {
        type = AudioWaveform::Type::NAIVE_TRAIN;
    } else if (shape == "triangle") {
        type = AudioWaveform::Type::POLY_TRIANG;
    } else if (shape == "square") {
        type = AudioWaveform::Type::POLY_SQUARE;
    } else if (shape == "sawtooth") {
        type = AudioWaveform::Type::POLY_TOOTH;
    } else if (shape == "impulse") {
        type = AudioWaveform::Type::BLIT_TRAIN;
    }

    std::shared_ptr<AudioWaveform> wave = AudioWaveform::alloc(channels,sampling,type,frequency);
    if (wave) {
        wave->setUpper(data->getBool("upper",false));
        wave->setDuration(data->getFloat("duration",-1));
    }
    return wave;
}

/**
 * Disposes any resources allocated for this waveform
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioWaveform::dispose() {
    Sound::dispose();
    _type.store(0);
    _upper.store(false);
    _newfreq.store(false);
    _frequency.store(-1);
    _duration.store(-1);
}

#pragma mark Generator Attributes
/**
 * Returns the waveform type.
 *
 * @return the waveform type.
 */
AudioWaveform::Type AudioWaveform::getType() const {
    return (Type)_type.load(std::memory_order_relaxed);
}

/**
 * Sets the waveform type.
 *
 * @param type  the waveform type.
 */
void AudioWaveform::setType(Type type) {
    int value = (int)type;
    _type.store(value,std::memory_order_relaxed);
}

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
bool AudioWaveform::isUpper() const {
    return _upper.load(std::memory_order_relaxed);
}

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
void AudioWaveform::setUpper(bool upper) {
    _upper.store(upper,std::memory_order_relaxed);
}

/**
 * Returns the fundamental frequency of this waveform.
 *
 * @return the fundamental frequency of this waveform.
 */
float AudioWaveform::getFrequency() const {
    return _frequency.load(std::memory_order_relaxed);
}

/**
 * Sets the fundamental frequency of this waveform.
 *
 * @param frequency the fundamental frequency of this waveform.
 */
void AudioWaveform::setFrequency(float frequency) {
    _frequency.store(frequency,std::memory_order_relaxed);
    _newfreq.store(true,std::memory_order_relaxed);
}

/**
 * Returns the frame length of this waveform.
 *
 * The frame length is the number of audio samples in the asset.  If the
 * asset is infinite (e.g. {@link AudioWaveform}), then this method returns
 * a negative value.
 *
 * @return the frame length of this waveform.
 */
Sint64 AudioWaveform::getLength() const {
    double timeout = _duration.load(std::memory_order_relaxed);
    if (timeout >= 0) {
        return (Sint64)(timeout*_rate);
    }
    return -1;
}

/**
 * Returns the length of this waveform in seconds.
 *
 * The accuracy of this method depends on the specific implementation. If
 * the asset is infinite (e.g. {@link AudioWaveform}), then this method
 * returns a negative value.
 *
 * @return the length of this waveform in seconds.
 */
double AudioWaveform::getDuration() const {
    return _duration.load(std::memory_order_relaxed);
}

/**
 * Sets the length of this waveform in seconds.
 *
 * The accuracy of this method depends on the specific implementation. If
 * the asset is infinite (e.g. {@link AudioWaveform}), then this value is
 * negative.
 *
 * @param time the length of this waveform in seconds.
 */
void AudioWaveform::setDuration(double time) {
    _duration.store(time,std::memory_order_relaxed);
}

/**
 * Generates the given number of frames for the waveform fom the given offset.
 *
 * This function is used by {@link AudioNode} to generate the correct data
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
Uint32 AudioWaveform::generate(float* buffer, Uint32 frames, Uint64 offset, float last) {
    double ratio  = _frequency.load(std::memory_order_relaxed)/_rate;
    Sint64 timeout = (Sint64)(_duration.load(std::memory_order_relaxed)*_rate);
    Type type  = (Type)_type.load(std::memory_order_relaxed);
    bool upper = _upper.load(std::memory_order_relaxed);
    
    const double TWO_PI = 2 * M_PI;
    const double STEPSZ = ratio  * TWO_PI;
    
    if (_newfreq.load(std::memory_order_relaxed)) {
        _newfreq.store(false,std::memory_order_relaxed);
        if (type == Type::NOISE) {
            float freq = _frequency.load(std::memory_order_relaxed);
            if (freq < 0) {
                Uint32 seed = (Uint32)std::chrono::system_clock::now().time_since_epoch().count();
                _random.seed(seed);
            } else {
                Uint32 seed = std::min(freq,1.0f)*std::numeric_limits<Uint32>::max();
                _random.seed(seed);
            }
        }
    }

    // Number of harmonics for BLIT impulse
    Uint32 mval = 2*(unsigned int)std::floor(0.5/ratio)+1;
    
    Uint32 amt = timeout >= 0 ? std::min(frames,(Uint32)(timeout-offset)) : frames;
    Uint32 tmp = amt;
    
    Uint32 pos = (Uint32)offset;
    float* output = buffer;
    switch (type) {
        case Type::NOISE:
            while (tmp--) {
                double value = ( 2.0 * _random() / (_random.max() + 1.0) - 1.0 );
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = (float)value;
                }
            }
            break;
        case Type::SINE:
            while (tmp--) {
                last = std::sin(STEPSZ*(pos++));
                if (upper) last = std::fabs(last);
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::NAIVE_TRIANG:
            while (tmp--) {
                double t = std::fmod(ratio*(pos++),1);
                double value = 1.0 - 2.0*t;
                last = (float)(upper ? std::fabs(value) : 2.0*std::fabs(value) - 1.0);
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::NAIVE_SQUARE:
            while (tmp--) {
                double t = std::fmod(ratio*(pos++),1);
                last = t <= 0.5 ? 1.0f : (upper ? 0.0f : -1.0f);
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::NAIVE_TOOTH:
            while (tmp--) {
                double value  = std::fmod(STEPSZ*(pos++),TWO_PI);
                value = 1.0 - (2.0 * value / TWO_PI);
                last = (float)(upper ? 0.5*(value+1) : value);
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::NAIVE_TRAIN:
            while (tmp--) {
                double t = std::fmod(ratio*(pos++),1);
                if (std::fabs(t-0.25) < ratio) {
                    last = 1.0f;
                } else if (std::fabs(t-0.75) < ratio) {
                    last = upper ? 1.0f : -1.0f;
                } else {
                    last  = 0;
                }
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::POLY_TRIANG:
            while (tmp--) {
                double t = std::fmod(ratio*(pos++),1);
                double value = 2.0 * t - 1.0;
                value = 2.0 * (std::fabs(value) - 0.5);
                value += poly_blep(t,ratio);
                value -= poly_blep(std::fmod(t + 0.5, 1.0),ratio);
                // Leaky integrator: y[n] = A * x[n] + (1 - A) * y[n-1]
                value = STEPSZ * value + (1 - STEPSZ) * (double)last;
                last = (float)(upper ? 0.5*(value+1) : value);
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::POLY_SQUARE:
            while (tmp--) {
                double t = std::fmod(ratio*(pos++),1);
                double value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,ratio);
                value -= poly_blep(std::fmod(t + 0.5, 1.0),ratio);
                last = (float)(upper ? 0.5*(value+1) : value);
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::POLY_TOOTH:
            while (tmp--) {
                double t = std::fmod(ratio*(pos++),1);
                double value = 2.0 * t -1.0;
                value -= poly_blep(t,ratio);
                last = (float)(upper ? 0.5*(value+1) : value);
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::BLIT_TRAIN:
            while (tmp--) {
                double time1 = upper ? std::fmod(STEPSZ*pos,M_PI) : std::fmod(STEPSZ*pos,TWO_PI)/2;
                double time2 = std::fmod(STEPSZ*pos+M_PI,TWO_PI)/2;
                pos++;
                
                double denom1 = std::sin(time1);
                double denom2 = std::sin(time2);
                double value1 = 0.0;
                double value2 = 0.0;
                if ( std::fabs(denom1) <= std::numeric_limits<double>::epsilon()) {
                    value1 = 1.0;
                } else {
                    value1  = std::sin(mval*time1);
                    value1 /= mval*denom1;
                }
                if ( std::fabs(denom2) <= std::numeric_limits<double>::epsilon()) {
                    value2 = 1.0;
                } else {
                    value2  = std::sin(mval*time2);
                    value2 /= mval*denom2;
                }
                
                last = upper ? value1 : value1-value2;
                for(int ii = 0; ii < _channels; ii++) {
                    *output++ = last;
                }
            }
            break;
        case Type::UNKNOWN:
            ; // pass
    }
    return amt;
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
std::shared_ptr<audio::AudioNode> AudioWaveform::createNode() {
    std::shared_ptr<Sound> source = shared_from_this();
    std::shared_ptr<AudioWaveNode> node = AudioWaveNode::alloc(std::dynamic_pointer_cast<AudioWaveform>(source));
    node->setGain(_volume);
    return std::dynamic_pointer_cast<audio::AudioNode>(node);
}
