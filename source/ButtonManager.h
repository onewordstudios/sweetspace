#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <cugl/cugl.h>

#include "SoundEffectController.h"

/**
 * This is a helper class whose job it is to switch buttons between being up and down.
 *
 * To initialize, call the {@link registerButton} method, passing it a smart pointer to each cugl
 * button. Then, every frame, call {@link process}. The manager will automatically change the button
 * state to up or down based on how the user is clicking / tapping.
 *
 * This class does NOT currently handle the actual dispatching of button clicks, just the visual
 * update of the button's up or down state.
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
	ButtonManager() : wasDown(false) {}

	/**
	 * Register a button to be managed.
	 */
	void registerButton(const std::shared_ptr<cugl::Button>& button);

	/**
	 * Process input for a frame and update the state of all the buttons.
	 *
	 * Also plays the sound effect for a button click.
	 */
	void process();

	/**
	 * Returns true iff a button was properly tapped (the tap event both started and ended on the
	 * button)
	 *
	 * @param button The button
	 * @param tapData The start and end locations provided by the input controller
	 */
	static bool tappedButton(const std::shared_ptr<cugl::Button>& button,
							 const std::tuple<cugl::Vec2, cugl::Vec2>& tapData);

	/**
	 * Deregisters all buttons
	 */
	void clear();
};
#endif /* BUTTON_MANAGER_H */
