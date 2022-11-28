#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include <cugl/cugl.h>

#include "ButtonManager.h"

class PauseMenu : public cugl::Node {
   private:
	/** Wrapper node for pause menu */
	std::shared_ptr<cugl::Node> menu;
	/** Needle pointing to number of players */
	std::shared_ptr<cugl::Node> needle;
	/** Main pause button node */
	std::shared_ptr<cugl::Button> pauseBtn;
	/** Close menu button */
	std::shared_ptr<cugl::Button> closeBtn;
	/** Leave button */
	std::shared_ptr<cugl::Button> leaveBtn;
	/** Music mute button */
	std::shared_ptr<cugl::Button> musicMuteBtn;
	/** SFX mute button */
	std::shared_ptr<cugl::Button> sfxMuteBtn;

	/** Current frame of animation */
	size_t currFrame;

	/** Whether the pause menu is open */
	bool menuOpen;

	/** Button manager for all the buttons */
	ButtonManager btns;

   public:
	explicit PauseMenu(const std::shared_ptr<cugl::AssetManager> &assets);
	virtual ~PauseMenu();

	/**
	 * Initialize this win screen with assets from the pointed asset manager.
	 *
	 * @param assets Asset manager to load win screen assets from
	 */
	bool init(const std::shared_ptr<cugl::AssetManager> &assets);

	/**
	 * Cleanup and dispose of all assets pointed to by this node
	 */
	void dispose() override;

	/**
	 * Process all buttons in the pause menu. Returns true when leave game is pressed.
	 *
	 * @param tapData Tap data from Input Controller
	 *
	 * @return True iff the user wishes to leave the game
	 */
	bool manageButtons(const std::tuple<cugl::Vec2, cugl::Vec2> &tapData);

	/**
	 * Update the animation for this node. Should be called once every frame.
	 */
	void update();
};

#endif // PAUSE_MENU_H
