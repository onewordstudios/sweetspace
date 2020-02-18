//
//  CUAudiomanager.h
//  Cornell University Game Library (CUGL)
//
//  This module is a singleton for managing audio in the game engine. This
//  singleton can support multiple input and output devices for complex
//  filter graphs. This class is for when developers needs direct access
//  to the audio graph(s).  In most cases, developers can use AudioChannels
//  instead, which is built on top of this manager.
//
//  Because this is a singleton, there are no publicly accessible constructors
//  or intializers.  Use the static methods instead.
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
#include <cugl/audio/CUAudioManager.h>
#include <cugl/audio/graph/CUAudioOutput.h>
#include <cugl/audio/graph/CUAudioInput.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

#if CU_PLATFORM == CU_PLATFORM_MACOS
#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>

/** Address to write results of the callback function */
static const AudioObjectPropertyAddress test_address =
{
    kAudioHardwarePropertyDefaultOutputDevice,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster,
};

/**
 * Callback function to detect audio device resets in MacOS
 *
 * While SDL supports automatic roll-over for default devices on iOS (because
 * it uses AVFoundation), it does not support this on MacOS where it uses
 * the lower-level CoreAudio instead.
 */
static OSStatus
device_unplugged(AudioObjectID devid, UInt32 num_addr, const AudioObjectPropertyAddress *addrs, void *data)
{
    ((AudioManager*)data)->reset();
    return 0;
}
#endif

#pragma mark Constructors

/** Reference to the sound manager singleton */
AudioManager* AudioManager::_gManager = nullptr;

#if CU_PLATFORM == CU_PLATFORM_ANDROID
/** The default input buffer size for each audio node */
const Uint32 AudioManager::DEFAULT_OUTPUT_BUFFER = 2048;
#else
/** The default input buffer size for each audio node */
const Uint32 AudioManager::DEFAULT_OUTPUT_BUFFER = 1024;
#endif

/** The default input buffer size for each audio node */
const Uint32 AudioManager::DEFAULT_INPUT_BUFFER = 1024;

/**
 * Creates, but does not initialize the singleton audio manager
 *
 * The manager must be initialized before is can be used.
 */
AudioManager::AudioManager() :
_active(false),
_output(0),
_input(0) {
}

/**
 * Initializes the audio manager.
 *
 * This method initializes the platform-specific audio manager, acquiring
 * any necessary resources.
 *
 * While input and output devices do not need to have uniform buffer sizes,
 * we require this to ensure that audio graph nodes are all interchangeable.
 *
 * @param output    The size of the read buffer for output devices
 * @param intput    The size of the write buffer for input devices
 *
 * @return true if the audio manager was successfully initialized.
 */
bool AudioManager::init(Uint32 output, Uint32 input) {
    CUAssertLog(output,"Read buffer size is 0");
    CUAssertLog(input,"Write buffer size is 0");
    if (!_output) {
#if CU_PLATFORM == CU_PLATFORM_MACOS
        AudioObjectAddPropertyListener(kAudioObjectSystemObject, &test_address,  device_unplugged, this);
#endif
        _output = output;
        _input  = input;
        return true;
    }
    return false;
}

/**
 * Releases all resources for this singleton audio manager.
 *
 * Output and input devices can no longer be used, and no instances of
 * {@link AudioNode} may be created. If you need to use the manager again,
 * you must call init().
 */
void AudioManager::dispose() {
    if (_output) {
        deactivate();
        _outputs.clear();
        _inputs.clear();

#if CU_PLATFORM == CU_PLATFORM_MACOS
        AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &test_address, device_unplugged, this);
#endif
        _output = 0;
        _input  = 0;
        _active = false;
    }
}

#pragma mark -
#pragma mark Static Accessors
/**
 * Starts the singleton audio manager.
 *
 * Once this method is called, the method get() will no longer return
 * nullptr.  Calling the method multiple times (without calling stop) will
 * have no effect. In addition, an audio manager will start off as inactive,
 * and must be activated.
 *
 * Instances of {@link AudioNode} (and its subclasses) cannot be initialized
 * until this manager is activated.  That is because audio nodes need a
 * uniform buffer size (set by this method) in order to coordinate with one
 * another.
 *
 * This method will create a manager where the input and output buffer
 * sizes are the default values.
 */
void AudioManager::start() {
    start(DEFAULT_OUTPUT_BUFFER,DEFAULT_INPUT_BUFFER);
}

/**
 * Starts the singleton audio manager.
 *
 * Once this method is called, the method get() will no longer return
 * nullptr.  Calling the method multiple times (without calling stop) will
 * have no effect. In addition, an audio manager will start off as inactive,
 * and must be activated.
 *
 * Instances of {@link AudioNode} (and its subclasses) cannot be initialized
 * until this manager is activated.  That is because audio nodes need a
 * uniform buffer size (set by this method) in order to coordinate with one
 * another.
 *
 * This method will create a manager where the output and input buffer
 * share the same size.
 *
 * @param frames    The output and input buffer size in frames.
 */
void AudioManager::start(Uint32 frames) {
    start(frames,frames);
}

/**
 * Starts the singleton audio manager.
 *
 * Once this method is called, the method get() will no longer return
 * nullptr.  Calling the method multiple times (without calling stop) will
 * have no effect. In addition, an audio manager will start off as inactive,
 * and must be activated.
 *
 * Instances of {@link AudioNode} (and its subclasses) cannot be initialized
 * until this manager is activated.  That is because audio nodes need a
 * uniform buffer size (set by this method) in order to coordinate with one
 * another.
 *
 * This method will create a manager where the output and input buffer
 * have the specified sizes. It is not necessary for the buffer value of an
 * input device match the buffer value of an output device.  Indeed, on many
 * systems, an input buffer size of less than 1024 samples is not supported,
 * while output devices can process much faster than that. What is important
 * is ensuring enough delay so that the audio graph does not outrun the input
 * device. Therefore, an input delay of less than the input buffer size is
 * not recommended for real-time audio processing.
 *
 * @param output    The size of the output buffer in frames.
 * @param input     The size of the input buffer in frames.
 */
void AudioManager::start(Uint32 output, Uint32 input) {
    if (_gManager) {
        CUAssertLog(!_gManager, "Audio Manager is already in use");
        return;
    }
    _gManager = new AudioManager();
    _gManager->init(output,input);
}

/**
 * Stops the singleton audio manager, releasing all resources.
 *
 * Once this method is called, the method get() will return nullptr.
 * Calling the method multiple times (without calling stop) will have
 * no effect.  In addition, the audio manager will no longer be active.
 *
 * Once this method is called, all instances of {@link AudioNode} become
 * invalid.  In addition, no future instances of {@link AudioNode} may
 * be created.  This method should only be called at application shutdown.
 */
void AudioManager::stop() {
    if (!_gManager) {
        CUAssertAlwaysLog(_gManager, "Audio Manager is not currently active");
        return;
    }
    _gManager->dispose();
    delete _gManager;
}

/**
 * Returns the list of all the audio devices
 *
 * This value may change and should be polled regularly to provide an
 * up-to-date list.  The provided argument determines whether this is
 * for output or input devices.
 *
 * @param output    Whether to list output (instead of input) devices
 *
 * @return the list of all the audio devices
 */
std::vector<std::string> AudioManager::devices(bool output) {
    std::vector<std::string> result;
    for(int ii = 0; ii < SDL_GetNumAudioDevices(!output); ii++) {
        result.push_back(std::string(SDL_GetAudioDeviceName(ii, !output)));
    }
    return result;
}

/**
 * Returns the list of devices with attached audio nodes.
 *
 * If there is an audio node on the default device, this will include
 * the current default. The provided argument determines whether this is
 * for output or input devices.
 *
 * @param output    Whether to list output (instead of input) devices
 *
 * @return the list of devices with attached audio nodes.
 */
std::vector<std::string> AudioManager::occupied(bool output) {
    if (!_gManager) {
        return std::vector<std::string>();
    }
    std::vector<std::string> result;
    if (output) {
        for(auto it = _gManager->_outputs.begin(); it != _gManager->_outputs.end(); ++it) {
            result.push_back(it->second->getDevice());
        }
    }
    return result;
}

#pragma mark -
#pragma mark Manager Properties
/**
 * Returns true if the audio manager is active.
 *
 * An active audio manager will regularly poll data from any unpaused
 * output node, and regular write data to any unreleased input node.
 *
 * @return true if the audio manager is active.
 */
bool AudioManager::isActive() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _active;
}

/**
 * Activates the audio manager.
 *
 * This method is used to resume audio behavior after a call to the method
 * {@link deactivate()}.  This provides a uniform way of renabling audio
 * devices (such as after an application switch).
 *
 * This method is not the same as {@link start()}.  It does not allocate
 * any new resources.
 */
void AudioManager::activate() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (!_active) {
        _active = true;
        for(auto it = _outputs.begin(); it != _outputs.end(); ++it) {
            it->second->setActive(true);
        }
        for(auto it = _inputs.begin(); it != _inputs.end(); ++it) {
            it->second->setActive(true);
        }
    }
}

/**
 * Deactivates the audio manager.
 *
 * This method is used to pause all output nodes and release all input
 * nodes from recording.  This is important during an application switch,
 * such as when the game goes into the background.  All of the devices
 * may be resumed with a call to {@link deactivate()}.
 *
 * This method is not the same as {@link stop()}.  It does not release
 * any resources and no audio graphs are invalidated.
 */
void AudioManager::deactivate() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_active) {
        _active = false;
        for(auto it = _outputs.begin(); it != _outputs.end(); ++it) {
            it->second->setActive(false);
        }
        for(auto it = _inputs.begin(); it != _inputs.end(); ++it) {
            it->second->setActive(false);
        }
    }
}

/**
 * Resets any stopped or failed audio devices.
 *
 * This method will also roll over the default output (not input) device
 * if it changes.
 *
 * This method is necessary for when an audio device is unplugged.  While
 * SDL often does this automatically, this method is provided for platforms
 * (e.g. CoreAudio on MacOS) where this must be done explicitly.
 */

void AudioManager::reset() {
    std::unique_lock<std::mutex> lock(_mutex);
    for(auto it = _outputs.begin(); it != _outputs.end(); ++it) {
        if (SDL_GetAudioDeviceStatus(it->second->getAUID()) == SDL_AUDIO_STOPPED) {
            it->second->reboot();
        } else if (it->first == "") {
            it->second->reboot();
        }
    }
    for(auto it = _inputs.begin(); it != _inputs.end(); ++it) {
        if (SDL_GetAudioDeviceStatus(it->second->getAUID()) == SDL_AUDIO_STOPPED) {
            it->second->reboot();
        }
    }
}

#pragma mark -
#pragma mark Output Devices
/**
 * Returns the default output device with 2 channels at 48000 Hz.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as the audio manager is
 * activated.
 *
 * This node is always logically attached to the default output device.
 * That means it will switch devices whenever the default output changes.
 * This method may fail if the default device is in use.
 *
 * @return the default output device with 2 channels at 48000 Hz.
 */
std::shared_ptr<cugl::audio::AudioOutput> AudioManager::openOutput() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_outputs.find("") != _outputs.end()) {
        CULogError("Default output device is in use.");
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioOutput> result = std::make_shared<cugl::audio::AudioOutput>();
    if (result && result->init()) {
        _outputs[std::string("")] = result;
        if (_active) {
            result->setActive(true);
        }
        return result;
    }
    return nullptr;
}

/**
 * Returns the default output device with the given channels and sample rate.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as the audio manager is
 * activated.
 *
 * This node is always logically attached to the default output device.
 * That means it will switch devices whenever the default output changes.
 * This method may fail if the default output device is in use.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 *
 * @return the default output device with the given channels and sample rate.
 */
std::shared_ptr<cugl::audio::AudioOutput> AudioManager::openOutput(Uint8 channels, Uint32 rate) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_outputs.find("") != _outputs.end()) {
        CULogError("Default output device is in use.");
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioOutput> result = std::make_shared<cugl::audio::AudioOutput>();
    if (result && result->init(channels,rate,_output)) {
        _outputs[std::string("")] = result;
        if (_active) {
            result->setActive(true);
        }
        return result;
    }
    return nullptr;
}

/**
 * Returns the given output device with 2 channels at 48000 Hz.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as the audio manager is
 * activated.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 *
 * @return the given output device with 2 channels at 48000 Hz.
 */
std::shared_ptr<audio::AudioOutput> AudioManager::openOutput(const char* device) {
    std::unique_lock<std::mutex> lock(_mutex);
    std::string name(device);
    if (_outputs.find(name) != _outputs.end()) {
        CULogError("Device '%s' is in use.",device);
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioOutput> result = std::make_shared<cugl::audio::AudioOutput>();
    if (result && result->init(name)) {
        _outputs[name] = result;
        if (_active) {
            result->setActive(true);
        }
        return result;
    }
    return nullptr;
}

/**
 * Returns the given output device with 2 channels at 48000 Hz.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as the audio manager is
 * activated.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 *
 * @return the given output device with 2 channels at 48000 Hz.
 */
std::shared_ptr<audio::AudioOutput> AudioManager::openOutput(const std::string& device) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_outputs.find(device) != _outputs.end()) {
        CULogError("Device '%s' is in use.",device.c_str());
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioOutput> result = std::make_shared<cugl::audio::AudioOutput>();
    if (result && result->init(device)) {
        _outputs[device] = result;
        if (_active) {
            result->setActive(true);
        }
        return result;
    }
    return nullptr;

}

/**
 * Returns the output device with the given channels and sample rate.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as the audio manager is
 * activated.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 *
 * @return the output device with the given channels and sample rate.
 */
std::shared_ptr<audio::AudioOutput> AudioManager::openOutput(const char* device, Uint8 channels, Uint32 rate) {
    std::unique_lock<std::mutex> lock(_mutex);
    std::string name(device);
    if (_outputs.find(name) != _outputs.end()) {
        CULogError("Device '%s' is in use.",device);
        return nullptr;
    }


    std::shared_ptr<cugl::audio::AudioOutput> result = std::make_shared<cugl::audio::AudioOutput>();
    if (result && result->init(name, channels, rate, _output)) {
        _outputs[name] = result;
        if (_active) {
            result->setActive(true);
        }
        return result;
    }
    return nullptr;
}

/**
 * Returns the output device with the given channels and sample rate.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as the audio manager is
 * activated.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 *
 * @return the output device with the given channels and sample rate.
 */
std::shared_ptr<audio::AudioOutput> AudioManager::openOutput(const std::string& device, Uint8 channels, Uint32 rate) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_outputs.find(device) != _outputs.end()) {
        CULogError("Device '%s' is in use.",device.c_str());
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioOutput> result = std::make_shared<cugl::audio::AudioOutput>();
    if (result && result->init(device, channels, rate, _output)) {
        _outputs[device] = result;
        if (_active) {
            result->setActive(true);
        }
        return result;
    }
    return nullptr;
}

/**
 * Closes the output device and disposes all resources.
 *
 * Once this method is called, the {@link AudioOutput} is invalidated and
 * is no longer safe to use.
 *
 * @param device    The output device to close
 *
 * @return whether the device was successfully closed.
 */
bool AudioManager::closeOutput(const std::shared_ptr<audio::AudioOutput>& output) {
    std::unique_lock<std::mutex> lock(_mutex);
    output->setActive(false);
    bool success = false;
    for(auto it = _outputs.begin(); it != _outputs.end(); ) {
        if (it->second.get() == output.get()) {
            it = _outputs.erase(it);
            success = true;
        } else {
            ++it;
        }
    }
    return success;
}

#pragma mark -
#pragma mark Input Devices
/**
 * Returns the default input device with 2 channels at 48000 Hz.
 *
 * The input delay will be equal to the value {@link getWriteSize()}.
 * This means that playback is only available after two calls to
 * {@link AudioInput#record()}.  This is the minimal value for smooth
 * real-time playback of recorded audio.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as the AudioManager
 * is activated.  In addition, it is also unpaused, meaning that playback
 * will start as soon as it is attached to an audio graph.
 *
 * This node is always logically attached to the default input device.
 * That means it will switch devices whenever the default input changes.
 * This method may fail if the default device is in use.
 *
 * @return the default input device with 2 channels at 48000 Hz.
 */
std::shared_ptr<cugl::audio::AudioInput> AudioManager::openInput() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_inputs.find("") != _inputs.end()) {
        CULogError("Default output device is in use.");
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioInput> result = std::make_shared<cugl::audio::AudioInput>();
    if (result && result->init()) {
        _inputs[std::string("")] = result;
        return result;
    }
    return nullptr;
}

/**
 * Returns the default input device with the given channels and sample rate.
 *
 * The delay value is the number of frames that must be recorded before a
 * single frame.  This determines the playback latency.  While it is
 * possible to have a delay of 0, this is unlikely to provide smooth
 * real-time playback of recorded audio.  That is because there are no
 * guarantees about the thread interleaving of input and output devices.
 * A delay of at least {@link getWriteSize()}, and maybe even more, is
 * recommended.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as the AudioManager
 * is activated.  In addition, it is also unpaused, meaning that playback
 * will start as soon as it is attached to an audio graph.
 *
 * This node is always logically attached to the default input device.
 * That means it will switch devices whenever the default input changes.
 * This method may fail if the default input device is in use.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 * @param delay     The playback delay between recording and reading
 *
 * @return the default input device with the given channels and sample rate.
 */
std::shared_ptr<cugl::audio::AudioInput> AudioManager::openInput(Uint8 channels, Uint32 rate, Uint32 delay) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_inputs.find("") != _inputs.end()) {
        CULogError("Default output device is in use.");
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioInput> result = std::make_shared<cugl::audio::AudioInput>();
    if (result && result->init(channels,rate,_input,delay)) {
        _inputs[std::string("")] = result;
        return result;
    }
    return nullptr;
}

/**
 * Returns the given input device with 2 channels at 48000 Hz.
 *
 * The input delay will be equal to the value {@link getWriteSize()}.
 * This means that playback is only available after two calls to
 * {@link AudioInput#record()}.  This is the minimal value for smooth
 * real-time playback of recorded audio.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as the AudioManager
 * is activated.  In addition, it is also unpaused, meaning that playback
 * will start as soon as it is attached to an audio graph.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 *
 * @return the given input device with 2 channels at 48000 Hz.
 */
std::shared_ptr<audio::AudioInput> AudioManager::openInput(const char* device) {
    std::unique_lock<std::mutex> lock(_mutex);
    std::string name(device);
    if (_inputs.find(name) != _inputs.end()) {
        CULogError("Device '%s' is in use.",device);
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioInput> result = std::make_shared<cugl::audio::AudioInput>();
    if (result && result->init(name)) {
        _inputs[name] = result;
        return result;
    }
    return nullptr;
}

/**
 * Returns the given input device with 2 channels at 48000 Hz.
 *
 * The input delay will be equal to the value {@link getWriteSize()}.
 * This means that playback is only available after two calls to
 * {@link AudioInput#record()}.  This is the minimal value for smooth
 * real-time playback of recorded audio.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as the AudioManager
 * is activated.  In addition, it is also unpaused, meaning that playback
 * will start as soon as it is attached to an audio graph.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 *
 * @return the given input device with 2 channels at 48000 Hz.
 */
std::shared_ptr<audio::AudioInput> AudioManager::openInput(const std::string& device) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_inputs.find(device) != _inputs.end()) {
        CULogError("Device '%s' is in use.",device.c_str());
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioInput> result = std::make_shared<cugl::audio::AudioInput>();
    if (result && result->init(device)) {
        _inputs[device] = result;
        return result;
    }
    return nullptr;

}

/**
 * Returns the given output device with the given channels and sample rate.
 *
 * The delay value is the number of frames that must be recorded before a
 * single frame.  This determines the playback latency.  While it is
 * possible to have a delay of 0, this is unlikely to provide smooth
 * real-time playback of recorded audio.  That is because there are no
 * guarantees about the thread interleaving of input and output devices.
 * A delay of at least {@link getWriteSize()}, and maybe even more, is
 * recommended.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as the AudioManager
 * is activated. In addition, it is also unpaused, meaning that playback
 * will start as soon as it is attached to an audio graph.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 * @param delay     The playback delay between recording and reading
 *
 * @return the given output device with the given channels and sample rate.
 */
std::shared_ptr<audio::AudioInput> AudioManager::openInput(const char* device, Uint8 channels, Uint32 rate, Uint32 delay) {
    std::unique_lock<std::mutex> lock(_mutex);
    std::string name(device);
    if (_inputs.find(name) != _inputs.end()) {
        CULogError("Device '%s' is in use.",device);
        return nullptr;
    }


    std::shared_ptr<cugl::audio::AudioInput> result = std::make_shared<cugl::audio::AudioInput>();
    if (result && result->init(name, channels, rate, _input, delay)) {
        _inputs[name] = result;
        return result;
    }
    return nullptr;
}

/**
 * Returns the given output device with the given channels and sample rate.
 *
 * The delay value is the number of frames that must be recorded before a
 * single frame.  This determines the playback latency.  While it is
 * possible to have a delay of 0, this is unlikely to provide smooth
 * real-time playback of recorded audio.  That is because there are no
 * guarantees about the thread interleaving of input and output devices.
 * A delay of at least {@link getWriteSize()}, and maybe even more, is
 * recommended.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as the AudioManager
 * is activated. In addition, it is also unpaused, meaning that playback
 * will start as soon as it is attached to an audio graph.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 * @param delay     The playback delay between recording and reading
 *
 * @return the given output device with the given channels and sample rate.
 */
std::shared_ptr<audio::AudioInput> AudioManager::openInput(const std::string& device, Uint8 channels, Uint32 rate, Uint32 delay) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_inputs.find(device) != _inputs.end()) {
        CULogError("Device '%s' is in use.",device.c_str());
        return nullptr;
    }

    std::shared_ptr<cugl::audio::AudioInput> result = std::make_shared<cugl::audio::AudioInput>();
    if (result && result->init(device, channels, rate, _input, delay)) {
        _inputs[device] = result;
        return result;
    }
    return nullptr;
}

/**
 * Closes the output device and disposes all resources.
 *
 * Once this method is called, the {@link AudioOutput} is invalidated and
 * is no longer safe to use.
 *
 * @param device    The output device to close
 *
 * @return whether the device was successfully closed.
 */
bool AudioManager::closeInput(const std::shared_ptr<audio::AudioInput>& input) {
    std::unique_lock<std::mutex> lock(_mutex);
    input->setActive(false);
    bool success = false;
    for(auto it = _inputs.begin(); it != _inputs.end(); ) {
        if (it->second.get() == input.get()) {
            it = _inputs.erase(it);
            success = true;
        } else {
            ++it;
        }
    }
    return success;
}
