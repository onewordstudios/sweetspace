//
//  CUAudioInput.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides interface to an audio input device. This makes it
//  typically a leaf node in the audio stream DAG.  This can be used to
//  record audio, analyze the audio, or even play it back in real time.
//  However, all real-time processing should be cognizant of the (necessary)
//  latency in playing back the input.  Also, this node does not handle
//  audio feedback well at all.
//
//  The audio graph and its nodes will always be accessed by two threads: the
//  main thread and the audio thread.  The audio graph is designed to safely
//  coordinate between these two threads.  However, it is minimizes locking
//  and instead relies on a fail-fast model.  If part of the audio graph is
//  not in a state to be used by the audio thread, it will skip over that part
//  of the graph until the next render frame.  Hence some changes should only
//  be made if the graph is paused.  When there is some question about the
//  thread safety, the methods are clearly marked.
//
//  It is NEVER safe to access the audio graph outside of the main thread. The
//  coordination algorithms only assume coordination between two threads.
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
#include <cugl/audio/graph/CUAudioInput.h>
#include <cugl/audio/CUAudioManager.h>
#include <cugl/audio/CUAudioSample.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUTimestamp.h>
#include <algorithm>

using namespace cugl::audio;

#pragma mark Static Attributes
/** The default (display) name; This is brittle.  It hopefully does not conflict. */
const static std::string DEFAULT_NAME("(DEFAULT DEVICE)");

#pragma mark -
#pragma mark SDL Audio Loop
/**
 * The SDL callback function
 *
 * This is the function that SDL uses to pass the recorded audio
 */
static void audioCallback(void*  userdata, Uint8* stream, int len) {
    AudioInput* device = (AudioInput*)userdata;
    Uint32 count = (Uint32)(len/(device->getChannels()*sizeof(float)));
    float* output = (float*)stream;
    device->record(output,count);
}

/** The default delay (in frames) for an input device */
Uint32 const AudioInput::DEFAULT_DELAY = 1024;

#pragma mark -
#pragma mark AudioManager Methods
/**
 * Creates a degenerate audio input node.
 *
 * The node has not been initialized, so it is not active.  The node
 * must be initialized to be used.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on
 * the heap, use the factory in {@link AudioManager}.
 */
AudioInput::AudioInput() : AudioNode(),
_dvname(""),
_record(true),
_active(true),
_buffer(nullptr),
_capacity(0),
_buffsize(0),
_buffhead(0),
_bufftail(0),
_timeout(-1),
_playpost(-1),
_playmark(-1) {
    _classname  = "AudioInput";
}

/**
 * Deletes the audio input node, disposing of all resources
 */
AudioInput::~AudioInput() {
    dispose();
}

/**
 * Initializes the default input device with 2 channels at 48000 Hz.
 *
 * This device node will have a buffer capacity of
 * {@link AudioManager#getWriteSize()} samples, and an equal delay.  By
 * default, this value is 1024 samples. This means that, at 48000 Hz, the
 * recording delay is 21ms and the playback delay is an (additional)
 * 21 ms.  So 42 ms passes between data is captured at the hardware device
 * and when it can be processed by the audio graph. While this may seem
 * like a lot of overhead, our experience (particularly on MacOS, iOS)
 * has shown this is necessary for smooth real-time processing.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as {@link AudioManager}
 * sets this device to active. In addition, it is also unpaused, meaning
 * that playback will start as soon as it is attached to an audio graph.
 *
 * This node is always logically attached to the default input device.
 * That means it will switch devices whenever the default input changes.
 * This method may fail if the default device is in use.
 *
 * @return true if initialization was successful
 */
bool AudioInput::init() {
    CUAssertLog(AudioManager::get(),"Attempt to allocate a node without an active AudioManager");
    Uint32 delay = AudioManager::get()->getWriteSize();
    return init("",DEFAULT_CHANNELS,DEFAULT_SAMPLING,delay,delay);
}

/**
 * Initializes the default input device with the given channels and sample rate.
 *
 * This device node will have a buffer capacity of
 * {@link AudioManager#getWriteSize()} samples, and an equal delay.  By
 * default, this value is 1024 samples. This means that, at 44800 Hz, the
 * recording delay is 21ms and the playback delay is an (additional)
 * 21 ms.  So 42 ms passes between data is captured at the hardware device
 * and when it can be processed by the audio graph. While this may seem
 * like a lot of overhead, our experience (particularly on MacOS, iOS)
 * has shown this is necessary for smooth real-time processing.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as {@link AudioManager}
 * sets this device to active. In addition, it is also unpaused, meaning
 * that playback will start as soon as it is attached to an audio graph.
 *
 * This node is always logically attached to the default input device.
 * That means it will switch devices whenever the default input changes.
 * This method may fail if the default input device is in use.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 *
 * @return true if initialization was successful
 */
bool AudioInput::init(Uint8 channels, Uint32 rate) {
    CUAssertLog(AudioManager::get(),"Attempt to allocate a node without an active AudioManager");
    Uint32 delay = AudioManager::get()->getWriteSize();
    return init("",channels,rate,delay,delay);
}

/**
 * Initializes the default input device with the given channels and sample rate.
 *
 * The buffer value is the number of samples recorded at each poll, while
 * the delay is the number of frames that must be recorded before a
 * single frame can be read.  These determine the recording latency and
 * playback latency, respectively.
 *
 * It is not necessary for the buffer value of an input device match the
 * buffer value of an output device.  Indeed, some many systems, an input
 * buffer size of less than 1024 samples is not supported, while output
 * devices can process much faster than that. What is important is ensuring
 * enough delay so that the audio graph does not outrun the input device.
 * Therefore, a delay of less than the buffer size is not recommended for
 * real-time audio processing.
 *
 * We have found that minimum buffer size of 1024 frames and an equal delay
 * of 1024 is the minimum value for most systems. That is because there is
 * no thread coordination at all between the {@link record()} method
 * (called by the input device) and the {@link read()} method (called by
 * the audio graph).
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as {@link AudioManager}
 * sets this device to active. In addition, it is also unpaused, meaning
 * that playback will start as soon as it is attached to an audio graph.
 *
 * This node is always logically attached to the default input device.
 * That means it will switch devices whenever the default input changes.
 * This method may fail if the default input device is in use.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 * @param buffer    The size of the buffer to record audio
 * @param delay     The frame delay between recording and reading
 *
 * @return true if initialization was successful
 */
bool AudioInput::init(Uint8 channels, Uint32 rate, Uint32 buffer, Uint32 delay) {
    return init("",channels,rate,buffer,delay);
}

/**
 * Initializes the given input device with 2 channels at 48000 Hz.
 *
 * This device node will have a buffer capacity of
 * {@link AudioManager#getWriteSize()} samples, and an equal delay.  By
 * default, this value is 1024 samples. This means that, at 48000 Hz, the
 * recording delay is 21ms and the playback delay is an (additional)
 * 21 ms.  So 42 ms passes between data is captured at the hardware device
 * and when it can be processed by the audio graph. While this may seem
 * like a lot of overhead, our experience (particularly on MacOS, iOS)
 * has shown this is necessary for smooth real-time processing.
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as {@link AudioManager}
 * sets this device to active. In addition, it is also unpaused, meaning
 * that playback will start as soon as it is attached to an audio graph.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the input device
 *
 * @return true if initialization was successful
 */
bool AudioInput::init(const std::string& device) {
    CUAssertLog(AudioManager::get(),"Attempt to allocate a node without an active AudioManager");
    Uint32 delay = AudioManager::get()->getWriteSize();
    return init(device,DEFAULT_CHANNELS,DEFAULT_SAMPLING,delay,delay);
}

/**
 * Initializes the input device with the given channels and sample rate.
 *
 * The buffer value is the number of samples recorded at each poll, while
 * the delay is the number of frames that must be recorded before a
 * single frame can be read.  These determine the recording latency and
 * playback latency, respectively.
 *
 * It is not necessary for the buffer value of an input device match the
 * buffer value of an output device.  Indeed, some many systems, an input
 * buffer size of less than 1024 samples is not supported, while output
 * devices can process much faster than that. What is important is ensuring
 * enough delay so that the audio graph does not outrun the input device.
 * Therefore, a delay of less than the buffer size is not recommended for
 * real-time audio processing.
 *
 * We have found that minimum buffer size of 1024 frames and an equal delay
 * of 1024 is the minimum value for most systems. That is because there is
 * no thread coordination at all between the {@link record()} method
 * (called by the input device) and the {@link read()} method (called by
 * the audio graph).
 *
 * An input device is initialized with both active as false and record as
 * true. That means it will start recording as soon as {@link AudioManager}
 * sets this device to active. In addition, it is also unpaused, meaning
 * that playback will start as soon as it is attached to an audio graph.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the input device
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 * @param buffer    The size of the buffer to play audio
 * @param delay     The frame delay between recording and reading
 *
 * @return true if initialization was successful
 */
bool AudioInput::init(const std::string& device, Uint8 channels, Uint32 rate, Uint32 buffer, Uint32 delay) {
    _dvname = device;
    
    SDL_AudioSpec want;
    want.freq = rate;
    want.channels = channels;
    want.samples = buffer;
    want.format = AUDIO_F32SYS;
    
    want.callback = audioCallback;
    want.userdata = this;
    
    _device = SDL_OpenAudioDevice((_dvname == "" ? NULL : _dvname.c_str()),
                                  1, &want, &_audiospec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (_device == 0 || _audiospec.format != AUDIO_F32SYS) {
        CULogError("Audio error: %s", SDL_GetError());
        return false;
    } else if (!AudioNode::init(_audiospec.channels,_audiospec.freq)) {
        return false;
    }
    _capacity = _audiospec.samples+delay;
    _buffsize = delay;
    _buffhead = delay;
    _bufftail = 0;
    _buffer = (float*)malloc(_capacity*_channels*sizeof(float));
    std::memset(_buffer,0,_capacity*_channels*sizeof(float));

    _booted = true;
    _active = false;
    _paused = false;
    _record = true;
    return true;
}

/**
 * Disposes any resources allocated for this input device node.
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioInput::dispose() {
    if (_booted) {
        SDL_free(_buffer);
        _capacity = 0;
        _buffhead = 0;
        _bufftail = 0;
        _buffsize = 0;
        _timeout  = -1;
        _playpost = -1;
        _playmark = -1;
        _booted = false;
        _active = false;
        _paused = false;
        _record = true;
        _playback.clear();
        SDL_PauseAudioDevice(_device, 1);
        SDL_CloseAudioDevice(_device);
    }
}

/**
 * Sets the active status of this node.
 *
 * An active device will have its {@link record()} method called at regular
 * intervals.  This setting is to allow {@link AudioManager to release and
 * acquire an input device without override the user settings.
 *
 * @param active    Whether to set this node to active
 */
void AudioInput::setActive(bool active) {
    _active.store(active,std::memory_order_relaxed);
    if (_record.load(std::memory_order_relaxed)) {
        SDL_PauseAudioDevice(_device, !active);
    }
}

#pragma mark -
#pragma mark Playback Control
/**
 * Pauses this node, preventing any data from being played back.
 *
 * As with all other audio nodes, pausing effects the playback.  However, it
 * does not affect recording.  Recording will still happen in the background
 * and may be recovered if {@link mark()} is set. To stop recording (but
 * not playback) call {@link release()} instead.
 *
 * If the node is already paused, this method has no effect. Pausing will
 * not go into effect until the next render call in the audio thread.
 *
 * @return true if the node was successfully paused
 */
bool AudioInput::pause() {
    bool success = !_paused.exchange(true);
    return success;
}

/**
 * Resumes this previously paused node, allowing data to be played back.
 *
 * As with all other audio nodes, pausing effects the playback.  However,
 * does not affect recording.  When play is resumed, the playback will
 * either return with immediate playback or the recording buffer,
 * depending on whether {@link mark()} is set.
 *
 * If the node is not paused, this method has no effect.
 *
 * @return true if the node was successfully resumed
 */
bool AudioInput::resume() {
    bool success = _paused.exchange(false);
    return success;
}

#pragma mark -
#pragma mark Recording Control
/**
 * Returns true if this node is currently recording audio.
 *
 * Recording is completely independent of playback.  An input node can
 * be recording, but have its played paused, and vice versa.
 *
 * @return true if this node is currently recording audio.
 */
bool AudioInput::isRecording() const {
    return _record.load(std::memory_order_relaxed);
}

/**
 * Stops this input node from recording.
 *
 * This method does not effect playback.  Unpaused playback will continue
 * until the delay has caught up.  After that point, it will only play
 * silence.
 *
 * If the node is not recording, this method has no effect.
 *
 * @return true if the node was successfully released
 */
bool AudioInput::release() {
    bool success = _record.exchange(false);
    if (success && _active.load(std::memory_order_relaxed)) {
        SDL_PauseAudioDevice(_device, 1);
    }
    return success;
}

/**
 * Resumes recording for a previously released node.
 *
 * This method does not effect playback.  If playback is paused, then
 * recording will be buffered if {@link mark()} is set, or else it will
 * overwrite itself in the circular buffer.
 *
 * If the node is already recording, this method has no effect.
 *
 * @return true if the node was successfully acquired
 */
bool AudioInput::acquire() {
    bool success = !_record.exchange(true);
    if (success && _active.load(std::memory_order_relaxed)) {
        SDL_PauseAudioDevice(_device, 0);
    }
    return success;
}

/**
 * Instantly stops this node from both recording and playback.
 *
 * This method is the same as calling both the methods {@link pause()} and
 * {@link release()}.  In addition, the input node will be marked as
 * {@link completed()} for the purpose of the audio graph.
 */
void AudioInput::stop() {
    release();
    pause();
    _timeout.store(0,std::memory_order_relaxed);
}

/**
 * Returns any cached data as an in-memory audio sample.
 *
 * This method is potentially expensive and should only be called when
 * the audio node has stopped recording (via the {@link release()} method,
 * and when the node is not part of an audio graph giving real-time
 * playback.
 *
 * If {@link mark()} is not set, this will return null rather than return
 * an empty audio sample.
 *
 * @return any cached data as an in-memory audio sample.
 */
std::shared_ptr<cugl::AudioSample> AudioInput::save() {
    std::unique_lock<std::mutex> lock(_buffmtex);
    std::shared_ptr<AudioSample> sample = nullptr;
    if (_playmark >= 0) {
        Uint32 _frames = (Uint32)(_playback.size()/_channels);
        sample = AudioSample::alloc(_channels,_sampling,_frames);
        float* output = sample->getBuffer();
        for(auto it=_playback.begin(); it != _playback.end(); ++it) {
            *output = *it;
            output++;
        }
    }
    return sample;
}

#pragma mark -
#pragma mark Audio Graph
/**
 * Returns the device associated with this input node.
 *
 * @return the device associated with this input node.
 */
const std::string& AudioInput::getDevice() const {
    if (_dvname == "") {
        return DEFAULT_NAME;
    }
    return _dvname;
}

/**
 * Returns true if this audio node has no more data.
 *
 * An audio node is typically completed if it return 0 (no frames read) on
 * subsequent calls to {@link read()}.  However, for infinite-running
 * audio threads, it is possible for this method to return true even when
 * data can still be read; in that case the node is notifying that it
 * should be shut down.
 *
 * @return true if this audio node has no more data.
 */
bool AudioInput::completed() {
    if (_timeout.load(std::memory_order_relaxed) == 0) {
        release();
        return true;
    }
    return false;
}

/**
 * Reads up to the specified number of frames into the given buffer
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioOutput.
 *
 * The buffer should have enough room to store frames * channels elements.
 * The channels are interleaved into the output buffer. The number of
 * frames read is determined by the audio graph, not the buffer of this
 * device.
 *
 * This method will always forward the read position.
 *
 * @param buffer    The read buffer to store the results
 * @param frames    The maximum number of frames to read
 *
 * @return the actual number of frames read
 */
Uint32 AudioInput::read(float* buffer, Uint32 frames) {
    Sint64 timeout = _timeout.load(std::memory_order_relaxed);
    if (_paused.load(std::memory_order_relaxed) || timeout == 0) {
        std::memset(buffer,0,frames*_channels*sizeof(float));
    } else {
        std::unique_lock<std::mutex> lock(_buffmtex);
        float* output = buffer;
        Uint32 amount = frames;
        if (_playpost >= 0) {
            Uint32 read = std::min(frames*_channels,(Uint32)(_playback.size()-_playpost*_channels));
            auto it = _playback.begin()+(size_t)(_playpost*_channels);
            Uint32 temp = read;
            while(temp) {
                *output = *it;
                output++;
                ++it;
                temp--;
            }
            _playpost += read/_channels;
            amount -= read/_channels;
        }
        
        Uint32 actual = std::min(amount,_buffsize);
        if (timeout >= 0) {
            actual = std::min((Uint32)timeout,actual);
        }
        Uint32 temp = actual;
        while (temp) {
            if (_buffhead+temp > _capacity) {
                std::memcpy(output,_buffer+_buffhead*_channels,(_capacity-_buffhead)*_channels*sizeof(float));
                output += (_capacity-_buffhead)*_channels;
                _buffsize -= temp;
                temp -= _capacity-_buffhead;
                _buffhead = 0;
            } else {
                std::memcpy(output,_buffer+_buffhead*_channels,temp*_channels*sizeof(float));
                output += temp*_channels;
                _buffsize -= temp;
                _buffhead += temp;
                temp = 0;
            }
        }
        amount -= actual;
        if (amount > 0) {
            std::memset(output,0,amount*_channels*sizeof(float));
        }
        if (timeout >= 0) {
            timeout -= std::min(frames,(Uint32)_timeout);
        }
        _timeout.store(timeout,std::memory_order_relaxed);
    }
    return frames;
}

/**
 * Records the specified number of frames to this audio node
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioOutput.
 *
 * If {@link mark()} is not set, this method records to a circular buffer
 * that has the given {@link getDelay()}.  Data that is not read in a
 * timely manner is lost from the buffer.
 *
 * However, if mark is set, then this method writes to an ever-increasing
 * queue.  This queue can be accessed at any time with {@link reset()}
 * or {@link setPosition()}.  This can potentially take a lot of memory
 * and so it should be used carefully.  Use {@link release()} to stop
 * recording to the buffer while still having access to it.
 */
Uint32 AudioInput::record(float* buffer, Uint32 frames) {
    if (_timeout.load(std::memory_order_relaxed) && _record.load(std::memory_order_relaxed)) {
        std::unique_lock<std::mutex> lock(_buffmtex);
        float* input = buffer;
        Uint32 temp = frames;
        if (_playmark >= 0) {
            Uint32 temp = frames*_channels;
            while (temp--) {
                _playback.push_back(*buffer);
                buffer++;
            }
        }
        while (temp) {
            if (_bufftail+temp > _capacity) {
                std::memcpy(_buffer+_bufftail*_channels,input,(_capacity-_bufftail)*_channels*sizeof(float));
                input += (_capacity-_bufftail)*_channels;
                _buffsize += temp;
                temp -= _capacity-_bufftail;
                _bufftail = 0;
            } else {
                std::memcpy(_buffer+_bufftail*_channels,input,temp*_channels*sizeof(float));
                input += temp*_channels;
                _buffsize += temp;
                _bufftail += temp;
                temp = 0;
            }
        }
        if (_buffsize > _capacity) {
            _buffsize = _capacity;
            _buffhead = (_bufftail+1) % _buffsize;
        }
    }
    return frames;
}

/**
 * Reboots the audio input node without interrupting any active polling.
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioNode.
 *
 * This method will close and reopen the associated audio device.  It
 * is primarily used when an node on the default device needs to migrate
 * between devices.
 */
void AudioInput::reboot() {
    bool active = _active.exchange(false);
    if (active && _record.load(std::memory_order_relaxed)) {
        SDL_PauseAudioDevice(_device, 1);
    }
    SDL_CloseAudioDevice(_device);
    SDL_AudioSpec want = _audiospec;
    _device = SDL_OpenAudioDevice((_dvname == "" ? NULL : _dvname.c_str()),
                                  1, &want, &_audiospec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (_device == 0 || _audiospec.format != AUDIO_F32SYS) {
        CULogError("Reboot of audio device '%s' failed.",_dvname.c_str());
        _booted = false;
        return;
    }
    if (active && _record.load(std::memory_order_relaxed)) {
        SDL_PauseAudioDevice(_device, 0);
    }
    _active.store(active,std::memory_order_relaxed);
}


#pragma mark -
#pragma mark Optional Methods
/**
 * Marks the current read position in the audio steam.
 *
 * This methods creates an internal buffer for recording audio data.
 * This buffer may be returned to at any time with {@link reset()} command.
 * Doing so introduces an inherent delay going forward, as the playback
 * comes from the recorded buffer.
 *
 * @return true if the read position was marked.
 */
bool AudioInput::mark() {
    std::unique_lock<std::mutex> lock(_buffmtex);
    if (_playpost < _playback.size()) {
        _playmark = _playpost;
    } else {
        _playmark = 0;
        _playback.clear();
    }
    return true;
}

/**
 * Clears the current marked position.
 *
 * If the method {@link mark()} has started recording to a buffer, then
 * this method will stop recording and release the buffer.  When the mark
 * is cleared the method {@link reset()} will no longer work.
 *
 * @return true if the read position was marked.
 */
bool AudioInput::unmark() {
    std::unique_lock<std::mutex> lock(_buffmtex);
    _playmark = -1;
    _playpost = -1;
    _playback.clear();
    return true;
}

/**
 * Resets the read position to the marked position of the audio stream.
 *
 * This method does nothing (and returns false) if no {@link mark()} is set.
 * Otherwise, it resets to the start of the buffer created by the call to
 * mark. This introduces an inherent delay going forward, as the playback
 * comes from the recorded buffer.
 *
 * @return true if the read position was moved.
 */
bool AudioInput::reset() {
    std::unique_lock<std::mutex> lock(_buffmtex);
    _playpost = _playmark;
    _timeout.store(-1,std::memory_order_relaxed);
    return _playmark >= 0;
}

/**
 * Returns the current frame position of this audio node
 *
 * This method returns -1 (indicating it is not supported) if {@link mark()}
 * is not set.  Otherwise, the position will be the number of frames since
 * the mark.
 *
 * @return the current frame position of this audio node.
 */
Sint64 AudioInput::getPosition() const {
    std::unique_lock<std::mutex> lock(_buffmtex);
    if (_playmark >= 0) {
        return _playpost >= 0 ? _playpost-_playmark : _playback.size();
    }
    return -1;
}

/**
 * Sets the current frame position of this audio node.
 *
 * This method returns -1 (indicating it is not supported) if {@link mark()}
 * is not set.  Otherwise, it will set the position to the number of frames
 * since the mark.  If the position is in the future (a frame not already
 * buffered) then this method will fail and return -1.
 *
 * @param position  the current frame position of this audio node.
 *
 * @return the new frame position of this audio node.
 */
Sint64 AudioInput::setPosition(Uint32 position) {
    std::unique_lock<std::mutex> lock(_buffmtex);
    if (_playmark < 0) {
        return -1;
    }else if (position > _playback.size()-_playmark) {
        _playpost = _playback.size();
    } else {
        _playpost = position+_playmark;
    }
    _timeout.store(-1,std::memory_order_relaxed);
    return _playpost;
}

/**
 * Returns the elapsed time in seconds.
 *
 * This method returns -1 (indicating it is not supported) if {@link mark()}
 * is not set.  Otherwise, the position will be the number of seconds since
 * the mark.
 *
 * @return the elapsed time in seconds.
 */
double AudioInput::getElapsed() const {
    double result = getPosition();
    if (result >= 0) {
        result /= getRate();
    }
    return result;
}

/**
 * Sets the read position to the elapsed time in seconds.
 *
 * This method returns -1 (indicating it is not supported) if {@link mark()}
 * is not set.  Otherwise, it will set the position to the number of seconds
 * since the mark.  If the position is in the future (a time not already
 * buffered) then this method will fail and return -1.
 *
 * @param time  The elapsed time in seconds.
 *
 * @return the new elapsed time in seconds.
 */
double AudioInput::setElapsed(double time) {
    if (time < 0) {
        return -1;
    }
    double result = setPosition((Uint32)(time*getRate()));
    if (result >= 0) {
        result /= getRate();
    }
    return result;
}

/**
 * Returns the remaining time in seconds.
 *
 * This method returns -1 (indicating it is not supported) if the method
 * {@link setRemaining()} has not been called or has been interrupted.
 * Otherwise, it returns the amount of time left in the countdown timer
 * until this node completes.
 *
 * @return the remaining time in seconds.
 */
double AudioInput::getRemaining() const {
    Sint64 timeout = _timeout.load(std::memory_order_relaxed);
    if (timeout >= 0) {
        return ((double)timeout)/getRate();
    } else if (_playpost >= 0) {
        return ((double)(_playback.size()-_playpost))/getRate();
    }
    return 0;
}

/**
 * Sets the remaining time in seconds.
 *
 * This method sets a countdown timer on the input node, forcing it to
 * complete in the given number of seconds.  If the audio has been reading
 * from the buffer (because of a call to {@link setPosition()}, this method
 * immediately skips ahead to real-time recording.  Any call to
 * {@link setPosition()} or {@link setElapsed()} before this time is up
 * will cancel the countdown.
 *
 * @param time  The remaining time in seconds.
 *
 * @return the new remaining time in seconds.
 */
double AudioInput::setRemaining(double time) {
    std::unique_lock<std::mutex> lock(_buffmtex);
    if (time < 0) {
        _timeout.store(-1,std::memory_order_relaxed);
    } else {
        _timeout.store((Sint32)(time*getRate()),std::memory_order_relaxed);
    }
    if (_playmark > 0) {
        _playpost = _playback.size();
    }
    return _timeout;
}
