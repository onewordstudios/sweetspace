//
//  CUSound.cpp
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
#include <cugl/audio/CUSound.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

/**
 * Creates a degenerate audio sample with no resources.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
Sound::Sound() :
_rate(0),
_channels(0) {
    _file = "";
}

/**
 * Deletes the sound resources and resets all attributes.
 *
 * This will delete the file reference and any allocated resources. You must
 * reinitialize the sound data to use the object.
 */
void Sound::dispose() {
    _rate = 0;
    _file = "";
    _channels = 0;
}

/**
 * Returns the file suffix for this sound asset.
 *
 * Until we expose more functionality about the encoding, this is a poor
 * man's way of determining the file format.
 *
 * @return the file suffix for this sound asset.
 */
std::string Sound::getSuffix() const {
    size_t pos = _file.rfind(".");
    return (pos == std::string::npos ? "" : _file.substr(pos));
}

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
void Sound::setVolume(float volume) {
    CUAssertLog(0 <= volume && volume <= 1, "The volume %.3f is out of range",volume);
    _volume = volume;
}
