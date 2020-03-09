//
//  CUAudioOutput.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides interface to an audio output device. As such, it is
//  often the final node in an audio stream DAG. It is analogous to AVAudioEngine
//  in Apple's AVFoundation API. The main differences is that it does not have
//  a dedicated mixer node.  Instead, you attach the single terminal node of the
//  audio graph.  In addition, it is possible to have a distinct audio graph for
//  each output device.
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
//  Version: 8/20/18
//
#include <cugl/audio/graph/CUAudioOutput.h>
#include <cugl/audio/CUAudioManager.h>
#include <cugl/util/CUDebug.h>
#include <cugl/util/CUTimestamp.h>
#include <atomic>
#include <cstring>

using namespace cugl::audio;

#pragma mark Static Attributes
/** The default (display) name; This is brittle.  It hopefully does not conflict. */
const static std::string DEFAULT_NAME("(DEFAULT DEVICE)");

#pragma mark -
#pragma mark SDL Audio Loop
/**
 * The SDL callback function
 *
 * This is the function that SDL uses to populate the audio buffer
 */
static void audioCallback(void* userdata, Uint8* stream, int len) {
	AudioOutput* device = (AudioOutput*)userdata;
	Uint32 count = (Uint32)(len / (device->getChannels() * device->getBitRate()));
	float* output = (float*)stream;
	device->read(output, count);
}

/**
 * Returns the bit size of the given audio format.
 *
 * This function converts SDL format types to their appropriate bit rate for size measurement.
 *
 * @param format    The SDL audio format
 *
 * @return the bit size of the given audio format.
 */
static size_t sizeof_format(SDL_AudioFormat format) {
	switch (format) {
	case AUDIO_S8:
	case AUDIO_U8:
		return sizeof(Uint8);
	case AUDIO_S16LSB:
	case AUDIO_S16MSB:
	case AUDIO_U16LSB:
	case AUDIO_U16MSB:
		return sizeof(Uint16);
	case AUDIO_S32LSB:
	case AUDIO_S32MSB:
		return sizeof(Uint32);
	default:
		return sizeof(float);
	}

}

#pragma mark -
#pragma mark AudioManager Methods
/**
 * Creates a degenerate audio output node.
 *
 * The node has not been initialized, so it is not active.  The node
 * must be initialized to be used.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on
 * the heap, use one of the static constructors instead.
 */
AudioOutput::AudioOutput() : AudioNode(),
_dvname(""),
_overhd(0),
_cvtratio(1.0f),
_cvtbuffer(nullptr),
_input(nullptr) {
	_classname = "AudioOutput";
	_resampler = NULL;
	_bitrate = sizeof(float);
}

/**
 * Deletes the audio output node, disposing of all resources
 */
AudioOutput::~AudioOutput() {
	dispose();
}

/**
 * Initializes the default output device with 2 channels at 48000 Hz.
 *
 * This device node will have a buffer (e.g. the number of samples that
 * the node plays at a time) of {@link AudioManager#getReadSize()} samples.
 * By default, this is 512 samples. At 48000 Hz, this means that the node
 * has a potential lag of 11 ms, which is a single animation frame at 60
 * fps. Since audio is double buffered, this means a play request may be
 * delayed by two frames.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as {@link AudioManager} sets
 * this device to active.
 *
 * This node is always logically attached to the default output device.
 * That means it will switch devices whenever the default output changes.
 * This method may fail if the default device is in use.
 *
 * @return true if initialization was successful
 */
bool AudioOutput::init() {
	CUAssertLog(AudioManager::get(), "Attempt to allocate a node without an active AudioManager");
	return init("", DEFAULT_CHANNELS, DEFAULT_SAMPLING, AudioManager::get()->getReadSize());
}

/**
 * Initializes the default output device with the given channels and sample rate.
 *
 * This device node will have a buffer (e.g. the number of samples that
 * the node plays at a time) of {@link AudioManager#getReadSize()} samples.
 * By default, this is 512 samples. At 48000 Hz, this means that the node
 * has a potential lag of 11 ms, which is a single animation frame at 60
 * fps. Since audio is double buffered, this means a play request may be
 * delayed by two frames.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as {@link AudioManager} sets
 * this device to active.
 *
 * This node is always logically attached to the default output device.
 * That means it will switch devices whenever the default output changes.
 * This method may fail if the default output device is in use.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 *
 * @return true if initialization was successful
 */
bool AudioOutput::init(Uint8 channels, Uint32 rate) {
	CUAssertLog(AudioManager::get(), "Attempt to allocate a node without an active AudioManager");
	return init("", channels, rate, AudioManager::get()->getReadSize());
}

/**
 * Initializes the default output device with the given channels and sample rate.
 *
 * The buffer value is the number of samples collected at each poll. Smaller
 * buffers clearly tax the CPU, as the device is collecting data at a higher
 * rate. Furthermore, if the value is too small, the time to collect the
 * data may be larger than the time to play it. This will result in pops
 * and crackles in the audio.
 *
 * However, larger values increase the audio lag.  For example, a buffer
 * of 1024 for a sample rate of 48000 Hz corresponds to 21 milliseconds.
 * This is the delay between when sound is gathered and it is played.
 * But this gathering process is also buffered, so this means that any
 * sound effect generated at the same time that the audio device executes
 * must wait 46 milliseconds before it can play.  A value of 512 is the
 * perferred value for 60 fps framerate, and lower values may be desired
 * for music-based apps.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as {@link AudioManager} sets
 * this device to active.
 *
 * This node is always logically attached to the default output device.
 * That means it will switch devices whenever the default output changes.
 * This method may fail if the default output device is in use.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 * @param buffer    The size of the buffer to play audio
 *
 * @return true if initialization was successful
 */
bool AudioOutput::init(Uint8 channels, Uint32 rate, Uint32 buffer) {
	return init("", channels, rate, buffer);
}

/**
 * Initializes the given output device with 2 channels at 48000 Hz.
 *
 * This device node will have a buffer (e.g. the number of samples that
 * the node plays at a time) of {@link AudioManager#getReadSize()} samples.
 * By default, this is 512 samples. At 48000 Hz, this means that the node
 * has a potential lag of 11 ms, which is a single animation frame at 60
 * fps. Since audio is double buffered, this means a play request may be
 * delayed by two frames.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as {@link AudioManager} sets
 * this device to active.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 *
 * @return true if initialization was successful
 */
bool AudioOutput::init(const std::string& device) {
	CUAssertLog(AudioManager::get(), "Attempt to allocate a node without an active AudioManager");
	return init(device, DEFAULT_CHANNELS, DEFAULT_SAMPLING, AudioManager::get()->getReadSize());
}


/**
 * Initializes the output device with the given channels and sample rate.
 *
 * The buffer value is the number of samples collected at each poll. Smaller
 * buffers clearly tax the CPU, as the node is collecting data at a higher
 * rate. Furthermore, if the value is too small, the time to collect the
 * data may be larger than the time to play it. This will result in pops
 * and crackles in the audio.
 *
 * However, larger values increase the audio lag.  For example, a buffer
 * of 1024 for a sample rate of 48000 Hz corresponds to 21 milliseconds.
 * This is the delay between when sound is gathered and it is played.
 * But this gathering process is also buffered, so this means that any
 * sound effect generated at the same time that the audio device executes
 * must wait 46 milliseconds before it can play.  A value of 512 is the
 * perferred value for 60 fps framerate, and lower values may be desired
 * for music-based apps.
 *
 * An output device is initialized with both active and paused as false.
 * That means it will begin playback as soon as {@link AudioManager} sets
 * this device to active.
 *
 * This method may fail if the given device is in use.
 *
 * @param device    The name of the output device
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in Hz
 * @param buffer    The size of the buffer to play audio
 *
 * @return true if initialization was successful
 */
bool AudioOutput::init(const std::string& device, Uint8 channels, Uint32 rate, Uint32 buffer) {
	_dvname = device;

	SDL_AudioSpec want;
	want.freq = rate;
	want.channels = channels;
	want.samples = buffer;
	want.format = AUDIO_F32SYS;

	want.callback = audioCallback;
	want.userdata = this;

	_device = SDL_OpenAudioDevice((_dvname == "" ? NULL : _dvname.c_str()),
		0, &want, &_audiospec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (_device == 0) {
		CULogError("[AUDIO] %s", SDL_GetError());
		return false;
	}
	else if (!AudioNode::init(want.channels, want.freq)) {
		return false;
	}

	// Because mobile devices often have other ideas...
	_bitrate = sizeof(float);
	if (want.format != _audiospec.format || want.freq != _audiospec.freq || want.channels != _audiospec.channels) {
		_resampler = SDL_NewAudioStream(want.format, want.channels, want.freq,
			_audiospec.format, _audiospec.channels, _audiospec.freq);
		if (_resampler == NULL) {
			SDL_CloseAudioDevice(_device);
			CULogError("[AUDIO] Could not create a resampler.");
			return false;
		}

		_cvtratio = ((float)want.freq) / _audiospec.freq;
		_bitrate = sizeof_format(_audiospec.format);
		Uint32 maxchan = std::max(want.channels, _audiospec.channels);
		size_t bsize = sizeof(float) * maxchan * std::ceil(_cvtratio * _audiospec.samples);

		_cvtbuffer = (float*)malloc(bsize);
		std::memset(_cvtbuffer, 0, bsize);

		// Initial 0s (else it will pop)
		SDL_AudioStreamPut(_resampler, _cvtbuffer, sizeof(float) * maxchan);
	}

	_booted = true;
	_active = false;
	_paused = false;
	return true;
}

/**
 * Disposes any resources allocated for this output device node.
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioOutput::dispose() {
	if (_booted) {
		AudioNode::dispose();
		SDL_PauseAudioDevice(_device, 1);
		SDL_CloseAudioDevice(_device);
		_active.store(false);
		std::atomic_store_explicit(&_input, {}, std::memory_order_relaxed);
		if (_resampler != NULL) {
			SDL_AudioStreamClear(_resampler);
			SDL_FreeAudioStream(_resampler);
			free(_cvtbuffer);
			_resampler = NULL;
			_cvtbuffer = nullptr;
		}
	}
}

/**
 * Sets the active status of this node.
 *
 * An active device will have its {@link read()} method called at regular
 * intervals.  This setting is to allow {@link AudioManager to pause and
 * resume an output device without override the user pause settings.
 *
 * @param active    Whether to set this node to active
 */
void AudioOutput::setActive(bool active) {
	_active.store(active, std::memory_order_relaxed);
	if (!_paused.load(std::memory_order_relaxed)) {
		SDL_PauseAudioDevice(_device, !active);
	}
}

#pragma mark -
#pragma mark Audio Graph
/**
 * Returns the device associated with this output node.
 *
 * @return the device associated with this output node.
 */
const std::string& AudioOutput::getDevice() const {
	if (_dvname == "") {
		return DEFAULT_NAME;
	}
	return _dvname;
}

/**
 * Attaches an audio graph to this output node.
 *
 * This method will fail if the channels of the audio graph do not agree
 * with the number of the channels of this node.
 *
 * @param node  The terminal node of the audio graph
 *
 * @return true if the attachment was successful
 */
bool AudioOutput::attach(const std::shared_ptr<AudioNode>& node) {
	if (!_booted) {
		CUAssertLog(_booted, "Cannot attach to an uninitialized output device");
		return false;
	}
	else if (node == nullptr) {
		detach();
		return true;
	}
	else if (node->getChannels() != _channels) {
		CUAssertLog(false, "Terminal node of audio graph has wrong number of channels: %d",
			node->getChannels());
		return false;
	}
	else if (node->getRate() != _sampling) {
		CUAssertLog(false, "Terminal node of audio graph has wrong sample rate: %d",
			node->getRate());
		return false;
	}

	std::atomic_exchange_explicit(&_input, node, std::memory_order_relaxed);
	return true;
}

/**
 * Detaches an audio graph from this output node.
 *
 * If the method succeeds, it returns the terminal node of the audio graph.
 *
 * @param node  The terminal node of the audio graph
 *
 * @return  the terminal node of the audio graph (or null if failed)
 */
std::shared_ptr<AudioNode> AudioOutput::detach() {
	if (!_booted) {
		CUAssertLog(_booted, "Cannot detach from an uninitialized output device");
		return nullptr;
	}

	std::shared_ptr<AudioNode> result = std::atomic_exchange_explicit(&_input, {}, std::memory_order_relaxed);
	return result;
}


#pragma mark -
#pragma mark Playback Control
/**
 * Pauses this node, preventing any data from being read.
 *
 * If the node is already paused, this method has no effect. Pausing will
 * not go into effect until the next render call in the audio thread.
 *
 * @return true if the node was successfully paused
 */
bool AudioOutput::pause() {
	bool success = !_paused.exchange(true);
	if (success && _active.load(std::memory_order_relaxed)) {
		SDL_PauseAudioDevice(_device, 1);
	}
	return success;
}

/**
 * Resumes this previously paused node, allowing data to be read.
 *
 * If the node is not paused, this method has no effect.  It is possible to
 * resume an node that is not yet activated by {@link AudioManager}.  When
 * that happens, data will be read as soon as the node becomes active.
 *
 * @return true if the node was successfully resumed
 */
bool AudioOutput::resume() {
	bool success = _paused.exchange(false);
	if (success && _active.load(std::memory_order_relaxed)) {
		SDL_PauseAudioDevice(_device, 0);
	}
	return success;
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
bool AudioOutput::completed() {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
Uint32 AudioOutput::read(float* buffer, Uint32 frames) {
	Timestamp start;

	Uint32 realchan = _audiospec.channels;
	if (_channels != realchan) {
		frames *= _channels;
		frames /= realchan;
	}

	char* realbuf = (char*)buffer;

	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
	if (input == nullptr || _paused.load(std::memory_order_relaxed)) {
		std::memset(realbuf, 0, frames * realchan * _bitrate);
	}
	else {
		// SDL double buffers, so there is no need to put this in another thread
		// Latency on Apple devices is roughly equal to duration of sample buffer
		// At 512 sample frames, we can safely take 9ms to do this operation.
		Sint32 take = 0;
		if (_resampler != NULL) {
			bool search = true;
			while (take < frames && search) {
				Sint32 amt = std::ceil(frames * _cvtratio);
				amt = input->read(_cvtbuffer, amt);
				SDL_AudioStreamPut(_resampler, _cvtbuffer, amt * sizeof(float) * _channels);
				amt = SDL_AudioStreamGet(_resampler,
					realbuf + take * realchan * _bitrate,
					(int)((frames - take) * realchan * _bitrate));
				if (amt == -1) {
					CULogError("[AUDIO] Resampling error.");
					std::memset(realbuf + take * realchan * _bitrate, 0, (frames - take) * realchan * _bitrate);
					take = frames;
				}
				else if (amt == 0) {
					search = false;
				}
				else {
					take += amt / _bitrate * realchan;
				}
			}
		}
		else {
			take = input->read(buffer, frames);
		}
		if (take < frames) {
			std::memset(realbuf + take * realchan * _bitrate, 0, (frames - take) * realchan * _bitrate);
		}
	}
	Timestamp end;
	Uint64 micros = Timestamp::ellapsedMicros(start, end);
	_overhd.store(micros, std::memory_order_relaxed);
	return frames;
}

/**
 * Reboots the audio output node without interrupting any active polling.
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioNode.
 *
 * This method will close and reopen the associated audio device.  It
 * is primarily used when an node on the default device needs to migrate
 * between devices.
 */
void AudioOutput::reboot() {
	bool active = _active.exchange(false);
	if (active && !_paused.load(std::memory_order_relaxed)) {
		SDL_PauseAudioDevice(_device, 1);
	}
	SDL_CloseAudioDevice(_device);
	SDL_AudioSpec want = _audiospec;
	_device = SDL_OpenAudioDevice((_dvname == "" ? NULL : _dvname.c_str()),
		0, &want, &_audiospec, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (_device == 0 || _audiospec.format != AUDIO_F32SYS) {
		CULogError("Reboot of audio device '%s' failed.", _dvname.c_str());
		_booted = false;
		return;
	}
	if (active && !_paused.load(std::memory_order_relaxed)) {
		SDL_PauseAudioDevice(_device, 0);
	}
	_active.store(active, std::memory_order_relaxed);
}

/**
 * Returns the number of microseconds needed to render the last audio frame.
 *
 * This method is primarily for debugging.
 *
 * @return the number of microseconds needed to render the last audio frame.
 */
Uint64 AudioOutput::getOverhead() const {
	return _overhd.load(std::memory_order_relaxed);
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
bool AudioOutput::mark() {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
bool AudioOutput::unmark() {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
bool AudioOutput::reset() {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
Sint64 AudioOutput::advance(Uint32 frames) {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
Sint64 AudioOutput::getPosition() const {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
Sint64 AudioOutput::setPosition(Uint32 position) {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
double AudioOutput::getElapsed() const {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
double AudioOutput::setElapsed(double time) {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
double AudioOutput::getRemaining() const {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
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
double AudioOutput::setRemaining(double time) {
	std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input, std::memory_order_relaxed);
	if (input) {
		return input->setRemaining(time);
	}
	return -1;
}