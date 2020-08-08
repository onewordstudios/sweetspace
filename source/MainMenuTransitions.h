#ifndef __MAIN_MENU_TRANSITIONS_H__
#define __MAIN_MENU_TRANSITIONS_H__
#include <cugl/cugl.h>

#include "MainMenuMode.h"

class MainMenuMode::MainMenuTransitions {
   private:
	MainMenuMode* parent;

	/** Helper object to do scene graph animations */
	AnimationManager animations;

	/** Play animation that leaves the main menu */
	void mainMenuOut();

	/** Play animation that returns to the main menu */
	void mainMenuIn();

   public:
	/**
	 * Create a main menu transition implementation object (you should only ever need one).
	 *
	 * @param parent A pointer to the main menu mode itself
	 */
	MainMenuTransitions(MainMenuMode* parent);

	/**
	 * Start the transition into the main menu mode.
	 *
	 * @param assets The asset manager with the main menu's assets
	 */
	void init(const std::shared_ptr<AssetManager>& assets);

	/**
	 * Transition to a new mode
	 *
	 * @param destination The new mode to transition to
	 */
	void to(MatchState destination);

	/**
	 * Step the animation forward one frame.
	 *
	 * @returns True iff an animation is in progress
	 */
	bool step();

	/**
	 * Cleanup this object and reset all scene graph objects to their starting positions.
	 */
	void reset();
};
#endif /* __MAIN_MENU_TRANSITIONS_H__ */