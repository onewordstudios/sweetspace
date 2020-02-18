//
//  CUAudioSpinnner.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a spatial audio audio panner.  It is used to rotate or
//  or "spin" a sound input about a sound field.  Doing this requires
//  specification of the audio channels angles about a circle.  There are
//  several default sound set-ups, but the user can specify any configuration
//  that they want.  This module is also useful for directing sound to a
//  subwoofer.
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
//  Version: 12/5/18
//
#include <cugl/audio/graph/CUAudioSpinner.h>
#include <cugl/audio/CUAudioManager.h>
#include <cugl/util/CUDebug.h>
#include <cmath>

using namespace cugl::audio;

/** The default crossover for the subwoofer */
#define DEFAULT_CROSSOVER 100.0

/**
 * Returns the normal form [0,2*M_PI] of an angle.
 *
 * This method works even if the original angle is negative.
 *
 * @param angle The angle to normalize.
 *
 * @return the normal form [0,2*M_PI] of an angle.
 */
float inline mod_angle(float angle) {
    return angle-2*M_PI*std::floor(angle/(2*M_PI));
}

#pragma mark -
#pragma mark Constructors
/**
 * Creates a degenerate audio spinner.
 *
 * The node has no channels, so read options will do nothing. The node must
 * be initialized to be used.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
 * the heap, use one of the static constructors instead.
 */
AudioSpinner::AudioSpinner() : AudioNode(),
_field(0),
_angle(0),
_crossover(0),
_dirtycross(false),
_input(nullptr),
_inlines(nullptr),
_outlines(nullptr),
_buffer(nullptr) {
    _inplan  = Plan::CUSTOM;
    _outplan = Plan::CUSTOM;
    _input = nullptr;
    _classname = "AudioSpinner";

}

/**
 * Initializes the node with default stereo settings
 *
 * The number of channels is two, for stereo output.  The sample rate is
 * the modern standard of 48000 HZ.  The spinner will start with left and right
 * mapped to the appropriate locations.
 *
 * @return true if initialization was successful
 */
bool AudioSpinner::init() {
    return init(DEFAULT_CHANNELS,DEFAULT_CHANNELS,DEFAULT_SAMPLING);
}

/**
 * Initializes the node with the given number of channels and sample rate
 *
 * The field will be the same as the number of channels.  By default, each
 * input channel will map to itself as an output channel (until the angle
 * changes).  Both the input and output will share the same (default)
 * layout plan.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in HZ
 *
 * @return true if initialization was successful
 */
bool AudioSpinner::init(Uint8 channels, Uint32 rate) {
    return init(channels,channels,rate);
}

/**
 * Initializes the node with the given number of input/output channels.
 *
 * The number of input channels is given by `field`, while `channels` is
 * the number of output channels. The input and output will each have the
 * default layout plan for the given size.
 *
 * @param channels  The number of output channels
 * @param field     The number of input channels
 * @param rate      The sample rate (frequency) in HZ
 *
 * @return true if initialization was successful
 */
bool AudioSpinner::init(Uint8 channels, Uint8 field, Uint32 rate) {
    if (AudioNode::init(channels,rate)) {
        _field = field;
        _inplan   = getDefaultPlan(field);
        _outplan  = getDefaultPlan(channels);
        _inlines  = new std::atomic<float>[field];
        _outlines = new std::atomic<float>[channels];
        initPlan(_inplan,_inlines);
        initPlan(_outplan,_outlines);
        _capacity = AudioManager::get()->getReadSize();
        _buffer = (float*)malloc(_capacity*_field*sizeof(float));
        
        _crossover = DEFAULT_CROSSOVER;
        _dirtycross = false;
        _filter = new dsp::BiquadIIR(field);
        _filter->setType(dsp::BiquadIIR::Type::LOWPASS,_crossover/rate,1.0f);
        return true;
    }
    return false;
}

/**
 * Disposes any resources allocated for this spinner.
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioSpinner::dispose() {
    if (_booted) {
        AudioNode::dispose();
        delete[] _inlines;
        delete[] _outlines;
        _inlines = nullptr;
        _outlines = nullptr;
        _input = nullptr;
        
        free(_buffer);
        _buffer   = nullptr;
        _capacity = 0;
        _inplan  = Plan::CUSTOM;
        _outplan = Plan::CUSTOM;
        _field = 0;
        _angle = 0;
        _crossover = 0;
        _dirtycross = false;
        delete _filter;
        _filter = nullptr;
    }
}


#pragma mark -
#pragma mark Plan Initializers
/**
 * Returns the default plan for the given number of channels.
 *
 * This is used for initializing this node.
 *
 * @param channels  The number of channels in the sound field
 *
 * @return the default plan for the given number of channels.
 */
AudioSpinner::Plan AudioSpinner::getDefaultPlan(Uint8 channels) {
    switch (channels) {
        case 1:
            return Plan::MONAURAL;
        case 2:
            return Plan::SIDE_STEREO;
        case 3:
            return Plan::SIDE_CENTER;
        case 4:
            return Plan::CORNER_QUADS;
        case 5:
        case 6:
            return Plan::SIDE_5_1;
        case 7:
        case 8:
            return Plan::CORNER_7_1;
    }
    return Plan::CUSTOM;
}

/**
 * Returns true if the plan is valid for the given number of channels.
 *
 * This is used to check user settings.
 *
 * @param plan      The layout plan
 * @param channels  The number of channels in the sound field
 *
 * @return true if the plan is valid for the given number of channels.
 */
bool AudioSpinner::isValidPlan(Plan plan, Uint8 channels) {
    switch (plan) {
        case Plan::MONAURAL:
            return channels == 1;
            break;
        case Plan::FRONT_STEREO:
        case Plan::SIDE_STEREO:
            return channels == 2;
            break;
        case Plan::FRONT_CENTER:
        case Plan::SIDE_CENTER:
            return channels == 3;
            break;
        case Plan::FRONT_QUADS:
        case Plan::CORNER_QUADS:
            return channels == 4;
            break;
        case Plan::BACK_5_1:
        case Plan::SIDE_5_1:
        case Plan::CORNER_5_1:
            return channels == 6;
            break;
        case Plan::BACK_7_1:
        case Plan::CORNER_7_1:
            return channels == 8;
            break;
        case Plan::CUSTOM:
            ; // pass
    }
    return true;
}

/**
 * Initializes the given array with the specified plan.
 *
 * This method assumes lines is an arra of the right length.  It initializes
 * lines with the right angles for the given plan.
 *
 * @param plan  The layout plan
 * @param lines The array with the channel orientations
 */
void AudioSpinner::initPlan(Plan plan, std::atomic<float>* lines) {
    switch (plan) {
        case Plan::MONAURAL:
            lines[0].store(        0, std::memory_order_relaxed);
            break;
        case Plan::FRONT_STEREO:
            lines[0].store(   M_PI/6, std::memory_order_relaxed);
            lines[1].store(11*M_PI/6, std::memory_order_relaxed);
            break;
        case Plan::SIDE_STEREO:
            lines[0].store(   M_PI_2, std::memory_order_relaxed);
            lines[1].store( 3*M_PI_2, std::memory_order_relaxed);
            break;
        case Plan::FRONT_CENTER:
            lines[0].store(   M_PI_4, std::memory_order_relaxed);
            lines[1].store( 7*M_PI_4, std::memory_order_relaxed);
            lines[2].store(        0, std::memory_order_relaxed);
            break;
        case Plan::SIDE_CENTER:
            lines[0].store(   M_PI_2, std::memory_order_relaxed);
            lines[1].store( 3*M_PI_2, std::memory_order_relaxed);
            lines[2].store(        0, std::memory_order_relaxed);
            break;
        case Plan::FRONT_QUADS:
            lines[0].store(   M_PI/6, std::memory_order_relaxed);
            lines[1].store(11*M_PI/6, std::memory_order_relaxed);
            lines[2].store( 5*M_PI/6, std::memory_order_relaxed);
            lines[3].store( 7*M_PI/6, std::memory_order_relaxed);
            break;
        case Plan::CORNER_QUADS:
            lines[0].store(   M_PI_4, std::memory_order_relaxed);
            lines[1].store( 7*M_PI_4, std::memory_order_relaxed);
            lines[2].store( 3*M_PI_4, std::memory_order_relaxed);
            lines[3].store( 5*M_PI_4, std::memory_order_relaxed);
            break;
        case Plan::BACK_5_1:
            lines[0].store(   M_PI_4, std::memory_order_relaxed);
            lines[1].store( 7*M_PI_4, std::memory_order_relaxed);
            lines[2].store(        0, std::memory_order_relaxed);
            lines[3].store(       -1, std::memory_order_relaxed); // woof
            lines[4].store( 5*M_PI/6, std::memory_order_relaxed);
            lines[5].store( 7*M_PI/6, std::memory_order_relaxed);
            break;
        case Plan::SIDE_5_1:
            lines[0].store(   M_PI_4, std::memory_order_relaxed);
            lines[1].store( 7*M_PI_4, std::memory_order_relaxed);
            lines[2].store(        0, std::memory_order_relaxed);
            lines[3].store(       -1, std::memory_order_relaxed); // woof
            lines[4].store(   M_PI_2, std::memory_order_relaxed);
            lines[5].store( 3*M_PI_2, std::memory_order_relaxed);
            break;
        case Plan::CORNER_5_1:
            lines[0].store(   M_PI_4, std::memory_order_relaxed);
            lines[1].store( 7*M_PI_4, std::memory_order_relaxed);
            lines[2].store(        0, std::memory_order_relaxed);
            lines[3].store(       -1, std::memory_order_relaxed); // woof
            lines[4].store( 3*M_PI_4, std::memory_order_relaxed);
            lines[5].store( 5*M_PI_4, std::memory_order_relaxed);
        case Plan::BACK_7_1:
            lines[0].store(   M_PI_4, std::memory_order_relaxed);
            lines[1].store( 7*M_PI_4, std::memory_order_relaxed);
            lines[2].store(        0, std::memory_order_relaxed);
            lines[3].store(       -1, std::memory_order_relaxed); // woof
            lines[4].store( 5*M_PI/6, std::memory_order_relaxed);
            lines[5].store( 7*M_PI/6, std::memory_order_relaxed);
            lines[6].store(   M_PI_2, std::memory_order_relaxed);
            lines[7].store( 3*M_PI_2, std::memory_order_relaxed);
            break;
        case Plan::CORNER_7_1:
            lines[0].store(   M_PI_4, std::memory_order_relaxed);
            lines[1].store( 7*M_PI_4, std::memory_order_relaxed);
            lines[2].store(        0, std::memory_order_relaxed);
            lines[3].store(       -1, std::memory_order_relaxed); // woof
            lines[4].store( 3*M_PI_4, std::memory_order_relaxed);
            lines[5].store( 5*M_PI_4, std::memory_order_relaxed);
            lines[6].store(   M_PI_2, std::memory_order_relaxed);
            lines[7].store( 3*M_PI_2, std::memory_order_relaxed);
            break;
        case Plan::CUSTOM:
            ; // pass
    }

}

#pragma mark -
#pragma mark Audio Graph
/**
 * Attaches an audio node to this spinner.
 *
 * This method will fail if the channels of the audio node do not agree
 * with the field size of this panner
 *
 * @param node  The audio node to pan
 *
 * @return true if the attachment was successful
 */
bool AudioSpinner::attach(const std::shared_ptr<AudioNode>& node) {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot attach to an uninitialized audio node");
        return nullptr;
    } else if (node == nullptr) {
        detach();
        return true;
    } else if (node->getChannels() != _field) {
        CUAssertLog(false,"Input node has wrong number of channels: %d", node->getChannels());
        return false;
    } else if (node->getRate() != _sampling) {
        CUAssertLog(false,"Input node has wrong sample rate: %d", node->getRate());
        return false;
    }
    
    std::atomic_exchange_explicit(&_input,node,std::memory_order_relaxed);
    return true;
}

/**
 * Detaches an audio node from this spinner.
 *
 * If the method succeeds, it returns the audio node that was removed.
 *
 * @return  The audio node to detach (or null if failed)
 */
std::shared_ptr<AudioNode> AudioSpinner::detach() {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot detach from an uninitialized audio node");
        return nullptr;
    }
    
    std::shared_ptr<AudioNode> result = std::atomic_exchange_explicit(&_input,{},std::memory_order_relaxed);
    return result;
}

#pragma mark -
#pragma mark Sound Field
/**
 * Returns the layout plan for the audio input.
 *
 * This is the intended layout of the sound source, unrotated.
 *
 * @return the layout plan for the audio input.
 */
AudioSpinner::Plan  AudioSpinner::getFieldPlan() const {
    return _inplan;
}

/**
 * Sets the layout plan for the audio input.
 *
 * This is the intended layout of the sound source, unrotated.  If the
 * value is CUSTOM, the current orientations will not be affected. Instead,
 * the user should set the orientations manually.
 *
 * @param plan  The layout plan for the audio input.
 */
void  AudioSpinner::setFieldPlan(AudioSpinner::Plan plan) {
    _inplan = plan;
    initPlan(plan,_inlines);
}

/**
 * Returns the layout plan for the audio output.
 *
 * This is the layout of the output channels.
 *
 * @return the layout plan for the audio output.
 */
AudioSpinner::Plan  AudioSpinner::getChannelPlan() const {
    return _outplan;
}

/**
 * Sets the layout plan for the audio output.
 *
 * This is the layout of the output channels.  If the value is CUSTOM, the
 * current orientations will not be affected. Instead, the user should set
 * the orientations manually.
 *
 * @param plan  The layout plan for the audio output.
 */
void  AudioSpinner::setChannelPlan(AudioSpinner::Plan plan) {
    _outplan = plan;
    initPlan(plan,_outlines);
}

/**
 * Returns the orientation of an input channel.
 *
 * This is the intended layout of the sound source, unrotated.
 *
 * @param channel   The input channel
 *
 * @return the orientation of an input channel.
 */
float AudioSpinner::getFieldOrientation(Uint32 channel) const {
    CUAssertLog(channel < _field, "Field %d is out of range",channel);
    return _inlines[channel].load(std::memory_order_relaxed);
}

/**
 * Sets the orientation of an input channel.
 *
 * This is the intended layout of the sound source, unrotated.
 *
 * @param channel   The input channel
 * @param angle     The angle from the listener's forward position.
 *
 * @return the orientation of an input channel.
 */
void  AudioSpinner::setFieldOrientation(Uint32 channel,float angle) {
    CUAssertLog(channel < _field, "Field %d is out of range",channel);
    return _inlines[channel].store(mod_angle(angle),std::memory_order_relaxed);
}

/**
 * Returns the orientation of an output channel.
 *
 * @param channel   The output channel
 *
 * @return the orientation of an output channel.
 */
float AudioSpinner::getChannelOrientation(Uint32 channel) const {
    CUAssertLog(channel < _channels, "Channel %d is out of range",channel);
    return _outlines[channel].load(std::memory_order_relaxed);
}

/**
 * Sets the orientation of an output channel.
 *
 * @param channel   The output channel
 * @param angle     The angle from the listener's forward position.
 *
 * @return the orientation of an output channel.
 */
void  AudioSpinner::setChannelOrientation(Uint32 channel,float angle) {
    CUAssertLog(channel < _channels, "Channel %d is out of range",channel);
    return _outlines[channel].store(mod_angle(angle),std::memory_order_relaxed);
}

/**
 * Returns the crossover frequency (in Hz) for the subwoofer.
 *
 * Sounds below this frequency will be sent to the subwoofer, regardless
 * of the input channel.
 *
 * @return the crossover frequency (in Hz) for the subwoofer.
 */
float AudioSpinner::getSubwoofer() const {
    return _crossover.load(std::memory_order_relaxed);
}

/**
 * Sets the crossover frequency (in Hz) for the subwoofer.
 *
 * Sounds below this frequency will be sent to the subwoofer, regardless
 * of the input channel.
 *
 * @param frequency The crossover frequency (in Hz) for the subwoofer.
 */
void  AudioSpinner::setSubwoofer(float frequency) {
    _dirtycross.store(true,std::memory_order_relaxed);
    _crossover.store(frequency,std::memory_order_relaxed);
}

#pragma mark -
#pragma mark Playback Control
/**
 * Returns the angle of the sound source.
 *
 * If this angle is not 0, the input orientation will be rotated by
 * the given angle to align it with the output orientation.  Input
 * channels that are between two output channels will be interpolated.
 *
 * @return the angle of the sound source.
 */
float AudioSpinner::getAngle() const {
    return _angle.load(std::memory_order_relaxed);
}

/**
 * Sets the angle of the sound source.
 *
 * If this angle is not 0, the input orientation will be rotated by
 * the given angle to align it with the output orientation.  Input
 * channels that are between two output channels will be interpolated.
 *
 * @param angle The angle of the sound source.
 */
void AudioSpinner::setAngle(float angle) {
    _angle.store(angle,std::memory_order_relaxed);
}

/**
 * Returns the input node of this spinner.
 *
 * @return the input node of this spinner.
 */
bool AudioSpinner::completed() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    return (input == nullptr || input->completed());
}

/**
 * Reads up to the specified number of frames into the given buffer
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioOutput.
 *
 * The buffer should have enough room to store frames * channels elements.
 * The channels are interleaved into the output buffer.
 *
 * This method will always forward the read position.
 *
 * @param buffer    The read buffer to store the results
 * @param frames    The maximum number of frames to read
 *
 * @return the actual number of frames read
 */
Uint32 AudioSpinner::read(float* buffer, Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input == nullptr || _paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,frames*_channels*sizeof(float));
    } else if (_angle == 0.0f && _field == _channels) {
        Uint32 amt = input->read(buffer, frames);
        if (amt < frames) {
            std::memset(buffer+amt*_channels,0,(frames-amt)*_channels*sizeof(float));
        }
    } else if (_channels == 1) {
        frames = std::min(frames,_capacity);
        Uint32 amt = input->read(_buffer, frames);
        if (amt < frames) {
            std::memset(_buffer+amt*_field,0,(frames-amt)*_field*sizeof(float));
        }
        std::memset(buffer,0,frames*_channels*sizeof(float));
        for(int ii = 0; ii < _field; ii++) {
            float* output = buffer;
            float* input  = _buffer+ii;
            Uint32 amt = frames;
            while (amt--) {
                *output = *input;
                output++;
                input += _field;
            }
        }
    } else {
        // Read into local buffer
        frames = std::min(frames,_capacity);
        Uint32 amt = input->read(_buffer, frames);
        if (amt < frames) {
            std::memset(_buffer+amt*_field,0,(frames-amt)*_field*sizeof(float));
        }
        std::memset(buffer,0,frames*_channels*sizeof(float));
        
        // Now spin
        float angle = _angle.load(std::memory_order_relaxed);
        for (int ii = 0; ii < _field; ii++) {
            float iangle = mod_angle(_inlines[ii].load(std::memory_order_relaxed)+angle);
            float langle = 3*M_PI;
            float rangle = -1;
            Sint32 left = -1;
            Sint32 rght = -1;
            float  minang = 3*M_PI;
            float  maxang = -1;
            Sint32 minpos = -1;
            Sint32 maxpos = -1;

            // Find the two sandwiching angles
            for (int jj = 0; jj < _channels; jj++) {
                float oangle = _outlines[jj].load(std::memory_order_relaxed);
                if (oangle >= iangle) {
                    if (oangle < langle) {
                        langle = oangle;
                        left = jj;
                    }
                } else {
                    if (oangle > rangle) {
                        rangle = oangle;
                        rght = jj;
                    }
                }
                if (minang > 0 && minang > oangle) {
                    minang = oangle;
                    minpos = jj;
                }
                if (maxang < oangle) {
                    maxang = oangle;
                    maxpos = jj;
                }
             }
            if (left < 0) {
                langle = minang;
                left = minpos;
            }
            if (rght < 0) {
                rangle = maxang;
                rght = maxpos;
            }

            // Use these two angles to interpolate
            float factor = mod_angle(langle-iangle);
            float spannr = mod_angle(langle-rangle);
            factor = factor/spannr;

            float* output = buffer;
            float* input  = _buffer+ii;
            Uint32 tmp = frames;
            while (tmp--) {
                *(output+left) += *input*(1-factor);
                *(output+rght) += *input*factor;
                output += _channels;
                input += _field;
            }
        }
        
        // Compute the subwoofer component.
        bool  dirty = _dirtycross.load(std::memory_order_relaxed);
        float cross = _crossover.load(std::memory_order_relaxed);
        if (_channels > 4 && cross) {
            if (dirty) {
                _dirtycross.store(false,std::memory_order_relaxed);
                cross = _crossover.load(std::memory_order_relaxed);
                _filter->setType(dsp::BiquadIIR::Type::LOWPASS,cross/_sampling,1.0f);
            }
            if (_field > 1) {
                for(int ii = 1; ii < _field; ii++) {
                    float* output = _buffer;
                    float* input  = _buffer+ii;
                    Uint32 tmp = frames;
                    while (tmp--) {
                        *output += *input;
                        output += _field;
                        input  += _field;
                    }
                }
            }
            _filter->calculate(1.0, _buffer, _buffer, frames);
            float* output = buffer+3;
            float* input  = _buffer;
            Uint32 tmp = frames;
            while (tmp--) {
                *output += *input;
                output += _channels;
                input++;
            }
        }
    }
    return frames;
}

#pragma mark -
#pragma mark Optional Methods
/**
 * Marks the current read position in the audio steam.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * This method is typically used by {@link reset()} to determine where to
 * restore the read position. For some nodes (like {@link AudioInput}),
 * this method may start recording data to a buffer, which will continue
 * until {@link clear()} is called.
 *
 * It is possible for {@link reset()} to be supported even if this method
 * is not.
 *
 * @return true if the read position was marked.
 */
bool AudioSpinner::mark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->mark();
    }
    return false;
}

/**
 * Clears the current marked position.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * If the method {@link mark()} started recording to a buffer (such as
 * with {@link AudioInput}), this method will stop recording and release
 * the buffer.  When the mark is cleared, {@link reset()} may or may not
 * work depending upon the specific node.
 *
 * @return true if the read position was marked.
 */
bool AudioSpinner::unmark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->unmark();
    }
    return false;
}

/**
 * Resets the read position to the marked position of the audio stream.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * When no {@link mark()} is set, the result of this method is node
 * dependent.  Some nodes (such as {@link AudioPlayer}) will reset to the
 * beginning of the stream, while others (like {@link AudioInput}) only
 * support a rest when a mark is set. Pay attention to the return value of
 * this method to see if the call is successful.
 *
 * @return true if the read position was moved.
 */
bool AudioSpinner::reset() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->reset();
    }
    return false;
}

/**
 * Advances the stream by the given number of frames.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * This method only advances the read position, it does not actually
 * read data into a buffer. This method is generally not supported
 * for nodes with real-time input like {@link AudioInput}.
 *
 * @param frames    The number of frames to advace
 *
 * @return the actual number of frames advanced; -1 if not supported
 */
Sint64 AudioSpinner::advance(Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->advance(frames);
    }
    return -1;
}

/**
 * Returns the current frame position of this audio node
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the position will be the
 * number of frames since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @return the current frame position of this audio node.
 */
Sint64 AudioSpinner::getPosition() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getPosition();
    }
    return -1;
}

/**
 * Sets the current frame position of this audio node.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the position will be the
 * number of frames since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @param position  the current frame position of this audio node.
 *
 * @return the new frame position of this audio node.
 */
Sint64 AudioSpinner::setPosition(Uint32 position) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setPosition(position);
    }
    return -1;
}

/**
 * Returns the elapsed time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the times will be the
 * number of seconds since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @return the elapsed time in seconds.
 */
double AudioSpinner::getElapsed() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getElapsed();
    }
    return -1;
}

/**
 * Sets the read position to the elapsed time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the new time will be meaured
 * from the mark. Other nodes like {@link AudioPlayer} measure from the
 * start of the stream.
 *
 * @param time  The elapsed time in seconds.
 *
 * @return the new elapsed time in seconds.
 */
double AudioSpinner::setElapsed(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setElapsed(time);
    }
    return -1;
}

/**
 * Returns the remaining time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node or if this method is unsupported
 * in that node
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link setRemaining()} has been called.  In that case, the node will
 * be marked as completed after the given number of seconds.  This may or may
 * not actually move the read head.  For example, in {@link AudioPlayer} it
 * will skip to the end of the sample.  However, in {@link AudioInput} it
 * will simply time out after the given time.
 *
 * @return the remaining time in seconds.
 */
double AudioSpinner::getRemaining() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getRemaining();
    }
    return -1;
}

/**
 * Sets the remaining time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node or if this method is unsupported
 * in that node
 *
 * If this method is supported, then the node will be marked as completed
 * after the given number of seconds.  This may or may not actually move
 * the read head.  For example, in {@link AudioPlayer} it will skip to the
 * end of the sample.  However, in {@link AudioInput} it will simply time
 * out after the given time.
 *
 * @param time  The remaining time in seconds.
 *
 * @return the new remaining time in seconds.
 */
double AudioSpinner::setRemaining(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setRemaining(time);
    }
    return -1;
}
