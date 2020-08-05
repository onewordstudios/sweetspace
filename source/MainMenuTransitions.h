#ifndef __MAIN_MENU_TRANSITIONS_H__
#define __MAIN_MENU_TRANSITIONS_H__
#include <cugl/cugl.h>

#include "MainMenuMode.h"

class MainMenuMode::MainMenuTransitions {
   private:
	MainMenuMode* parent;

	/** Helper object to do scene graph animations */
	AnimationManager animations;

	/**
	 * Play animation that leaves the main menu
	 */
	void animateOutMainMenu();

	/**
	 * Play animation that returns to the main menu
	 */
	void returnToMainMenu();

   public:
	MainMenuTransitions(MainMenuMode* parent);
	void init(const std::shared_ptr<AssetManager>& assets);
	void go(MatchState destination);
	bool step();
	void reset();
};
#endif /* __MAIN_MENU_TRANSITIONS_H__ */