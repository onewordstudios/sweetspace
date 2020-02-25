#include "GMController.h"

using namespace cugl;

#pragma mark -
#pragma mark GM Factors

/** The maximum number of events on ship at any one time. This will probably need to scale with the
 * number of players*/
constexpr unsigned int MAX_EVENTS = 3;

#pragma mark -
#pragma mark GM
/**
 * Creates a new GM controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
GMController::GMController() : active(false), numEvents(0) {}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void GMController::dispose() {
	if (active) {
#ifndef CU_TOUCH_SCREEN
		Input::deactivate<Keyboard>();
#else
		if (USE_ACCELEROMETER) {
			Input::deactivate<Accelerometer>();
		}
		Touchscreen* touch = Input::get<Touchscreen>();
		touch->removeBeginListener(LISTENER_KEY);
		touch->removeEndListener(LISTENER_KEY);
		active = false;
#endif
	}
}

/**
 * Initializes the input control for the given drawing scale.
 *
 * This method works like a proper constructor, initializing the input
 * controller and allocating memory.  However, it still does not activate
 * the listeners.  You must call start() do that.
 *
 * @return true if the controller was initialized successfully
 */
bool GMController::init() {
	bool success = true;
	active = success;
	return success;
}

/**
 * Processes the GM.
 *
 * This method is used to generate and manage current ship events
 */
void GMController::update(float dt) {}

/**
 * Clears all events
 */
void GMController::clear() { numEvents = 0; }
