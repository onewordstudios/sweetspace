//
//  CUSound.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a sound asset. Historically, these would just be
//  prerecorded sound files encoded a WAV, MP3, or OGG files.  However, our
//  long term roadmap is to support arbitrary audio graphs created by programs
//  like PureData, Max DSP or Abelton. For that reason, the sound asset is
//  an abstract class that is the base for several asset types.
//
//  To get a specific sound asset type, either use a specific class (like
//  AudioSample or AudioWaveform) or use the factory allocator in this class.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 11/20/18
//
#ifndef __CU_SOUND_H__
#define __CU_SOUND_H__
#include <cugl/audio/graph/CUAudioNode.h>
#include <cugl/audio/graph/CUAudioNode.h>
#include <cugl/math/CUMathBase.h>

namespace  cugl {
/**
 * This class is the base type for a sound asset.
 *
 * Eventually, our goal is to support arbitrary audio graphs as assets
 * (e.g. PureData patch files).  But we need a way of distinguishing a graph
 * as an asset and a graph as an active audio instance.  That is the purpose
 * of this class.  While an {@link audio::AudioNode} is an active sound instance,
 * this is the class of an asset file.
 *
 * This is an abstract class.  You should never make a sound object directly.
 * The factory allocator will create the proper subclass. Currently there are
 * only two completed subclass: {@link AudioSample} and {@link AudioWaveform}.
 * Will expand on this in future releases.
 */
class Sound : public std::enable_shared_from_this<Sound> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(Sound);
    
protected:
    /** The number of channels in this sound (max 32) */
    Uint8  _channels;
    
    /** The sampling rate (frequency) of this sound */
    Uint32 _rate;
    
    /** The source for this buffer (may be empty) */
    std::string _file;

    /** The default volume for this sound */
    float _volume;
    
public:
#pragma mark Constructors
    /**
     * Creates a degenerate sound with no resources.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    Sound();
    
    /**
     * Deletes this sound, disposing of all resources.
     */
    ~Sound() { dispose(); }

    /**
     * Deletes the sound resources and resets all attributes.
     *
     * This will delete the file reference and any allocated resources. You must
     * reinitialize the sound data to use the object.
     */
    virtual void dispose();
    
#pragma mark Attributes
    /**
     * Returns the sample rate of this sound.
     *
     * @return the sample rate of this sound.
     */
    Uint32 getRate() const { return _rate; }
    
    /**
     * Returns the number of channels used by this sound.
     *
     * A value of 1 means mono, while 2 means stereo. Depending on the file
     * format, other channels are possible. For example, 6 channels means
     * support for 5.1 surround sound. 8 channels (7.1 surround) is the current
     * maximum.
     *
     * We support up to 32 possible channels.
     *
     * @return the number of channels used by this sound.
     */
    Uint32 getChannels() const { return _channels; }
    
    /**
     * Returns the frame length of this sound.
     *
     * The frame length is the number of audio samples in the asset.  If the
     * asset is infinite (e.g. {@link AudioWaveform}), then this method returns
     * a negative value.
     *
     * @return the frame length of this sound.
     */
    virtual Sint64 getLength() const { return -1; }
    
    /**
     * Returns the length of this sound in seconds.
     *
     * The accuracy of this method depends on the specific implementation. If
     * the asset is infinite (e.g. {@link AudioWaveform}), then this method
     * returns a negative value.
     *
     * @return the length of this sound in seconds.
     */
    virtual double getDuration() const { return -1; }

    /**
     * Returns the file for this sound
     *
     * This value is the empty string if there was no source file.
     *
     * @return the file for this sound
     */
    std::string getFile() const { return _file; }
    
    /**
     * Returns the file suffix for this sound asset.
     *
     * Until we expose more functionality about the encoding, this is a poor
     * man's way of determining the file format.
     *
     * @return the file suffix for this sound asset.
     */
    std::string getSuffix() const;
    
    /**
     * Returns the default volume of this sound asset.
     *
     * This default value will be used when the sound is played without a
     * specified volume. The value is between 0 and 1, where 0 means muted
     * and 1 is maximum volume.
     *
     * Changing this value will only affect future calls to {@link createNode()}.
     *
     * @return the default volume of this sound asset.
     */
    float getVolume() const { return _volume; }
    
    /**
     * Sets the default volume of this sound asset.
     *
     * This default value will be used when the sound is played without a
     * specified volume. The value is between 0 and 1, where 0 means muted
     * and 1 is maximum volume.
     *
     * Changing this value will only affect future calls to {@link createNode()}.
     *
     * @param volume    The default volume of this sound asset.
     */
    void setVolume(float volume);
    
    /**
     * Returns a playble audio node for this asset.
     *
     * This audio node may be attached to an {@link audio::AudioOutput} for immediate
     * playback.  Nodes are distinct.  Each call to this method allocates
     * a new audio node.
     *
     * @return a playble audio node for this asset.
     */
    virtual std::shared_ptr<audio::AudioNode> createNode() { return nullptr; };
};
    
}

#endif /* __CU_SOUND_H__ */
