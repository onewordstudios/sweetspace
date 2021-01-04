#ifndef SOUND_EFFECT_CONTROLLER_H
#define SOUND_EFFECT_CONTROLLER_H
#include <cugl/cugl.h>

#include <map>

/**
 * This class represents sound effects
 *
 * This class is a singleton. It is initialized the first time the instance is acquired.
 */
class SoundEffectController {
   private:
	/**
	 * The singleton instance of this class.
	 */
	static std::shared_ptr<SoundEffectController> instance; // NOLINT

	/** List of actively playing effects. */
	std::map<std::pair<int, int>, bool> activeEffects;

	std::shared_ptr<cugl::Sound> jump;
	std::shared_ptr<cugl::Sound> doorCollide;
	std::shared_ptr<cugl::Sound> fixBreach;
	std::shared_ptr<cugl::Sound> slowBreach;
	std::shared_ptr<cugl::Sound> click;
	std::shared_ptr<cugl::Sound> teleport;

	static constexpr auto JUMP_FILE = "jump";
	static constexpr auto DOOR_FILE = "doorCollide";
	static constexpr auto FIX_FILE = "fixBreach";
	static constexpr auto SLOW_FILE = "slowBreach";
	static constexpr auto CLICK_FILE = "click";
	static constexpr auto TELEPORT_FILE = "teleport";

	/**
	 * Creates a new sound effect controller.
	 *
	 * This constructor does NOT do any initialzation.  It simply allocates the
	 * object. This makes it safe to use this class without a pointer.
	 */
	SoundEffectController() = default;

   public:
	enum Effect { JUMP = 0, DOOR = 1, FIX = 2, SLOW = 3, CLICK = 4, TELEPORT = 5 };

#pragma region Constructors

	/**
	 * Grab a pointer to the singleton instance of this class.
	 *
	 */
	static std::shared_ptr<SoundEffectController> getInstance() {
		if (instance == nullptr) {
			instance = std::shared_ptr<SoundEffectController>(new SoundEffectController());
		}
		return instance;
	}

	/**
	 * Initialize the Sound Effect controller with the given assets.
	 *
	 */
	void init(const std::shared_ptr<cugl::AssetManager>& assets) {
		jump = assets->get<cugl::Sound>(JUMP_FILE);
		doorCollide = assets->get<cugl::Sound>(DOOR_FILE);
		fixBreach = assets->get<cugl::Sound>(FIX_FILE);
		slowBreach = assets->get<cugl::Sound>(SLOW_FILE);
		click = assets->get<cugl::Sound>(CLICK_FILE);
		teleport = assets->get<cugl::Sound>(TELEPORT_FILE);
	}

	/**
	 * Register an event occurring, and if a sound has not already been played for it, play a sound
	 * effect.
	 *
	 *
	 */
	void startEvent(Effect e, int id) {
		// Check if this event has already been registered
		if (!activeEffects[{e, id}]) {
			activeEffects[{e, id}] = true;
			std::shared_ptr<cugl::Sound> sound; // NOLINT switch statement sets these
			const char* key;					// NOLINT
			switch (e) {
				case JUMP:
					sound = jump;
					key = JUMP_FILE;
					break;
				case DOOR:
					sound = doorCollide;
					key = DOOR_FILE;
					break;
				case FIX:
					sound = fixBreach;
					key = FIX_FILE;
					break;
				case SLOW:
					sound = slowBreach;
					key = SLOW_FILE;
					break;
				case CLICK:
					sound = click;
					key = CLICK_FILE;
					break;
				case TELEPORT:
					sound = teleport;
					key = TELEPORT_FILE;
					break;
			}
			if (!cugl::AudioChannels::get()->isActiveEffect(key)) {
				cugl::AudioChannels::get()->playEffect(key, sound, false);
				cugl::AudioChannels::get()->setEffectPan(key, 0);
			}
		}
	}

	/**
	 * Register an event ending.
	 *
	 *
	 */
	void endEvent(Effect e, int id) {
		if (activeEffects[{e, id}]) {
			activeEffects[{e, id}] = false;
		}
	}

	/**
	 * Clears all active events
	 */
	void reset() { activeEffects.clear(); }

	/**
	 * Deactivates and disposes of this sound effect controller.
	 */
	~SoundEffectController() = default;

	/**
	 * Deactivates and disposes of the instance, if it exists. Note that subsequent calls to {@link
	 * getInstance} will automatically reinitialize the class.
	 */
	static void cleanup() { instance = nullptr; }

#pragma endregion
};

#endif /* __SOUND_EFFECT_CONTROLLER_H__ */
