#ifndef __BUTTON_MANAGER_H__
#define __BUTTON_MANAGER_H__

#include <cugl/cugl.h>

/**
 * This is a helper class whose job it is to switch buttons between being up and down.
 */
class ButtonManager {
   private:
	/** Vector of all the buttons being managed */
	std::vector<std::shared_ptr<cugl::Button>> buttons;

	/** The buttons that were pressed down */
	std::vector<std::shared_ptr<cugl::Button>> downBtns;
	/** Whether the touch just started */
	bool wasDown;

   public:
	ButtonManager() { wasDown = false; }

	/**
	 * Register a button to be managed.
	 */
	void registerButton(std::shared_ptr<cugl::Button> button);

	/**
	 * Process input for a frame and update the state of all the buttons.
	 */
	void process();
};
#endif /* __BUTTON_MANAGER_H__ */