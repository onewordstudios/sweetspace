#ifndef AUDIO_CONTROLLER_H
#define AUDIO_CONTROLLER_H

#include <cugl/cugl.h>

/** Simple singleton class to mute and unmute music and sound effects */
class AudioController {
   private:
	/** Whether music is currently active */
	bool musicActive;
	/** Whether sound effects are currentlya ctive */
	bool sfxActive;

	AudioController() : musicActive(true), sfxActive(true){};
	~AudioController() = default;

   public:
	AudioController(AudioController const&) = delete;
	void operator=(AudioController const&) = delete;

	/** Get the singleton instance of this class */
	static AudioController& getInstance() {
		static AudioController a;
		return a;
	}

	/** Returns true iff music is unmuted */
	bool isMusicActive() const { return musicActive; }
	/** Returns true iff sound effects are unmuted */
	bool isSfxActive() const { return sfxActive; }

	/** Toggle whether music is muted */
	void toggleMusic() {
		if (musicActive) {
			cugl::AudioChannels::get()->pauseMusic();
		} else {
			cugl::AudioChannels::get()->resumeMusic();
		}
		musicActive = !musicActive;
	}

	/** Toggle whether sound effects are muted */
	void toggleSfx() {
		if (sfxActive) {
			cugl::AudioChannels::get()->pauseAllEffects();
		} else {
			cugl::AudioChannels::get()->resumeAllEffects();
		}
		sfxActive = !sfxActive;
	}
};

#endif