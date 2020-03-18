//
//  SDInput.h
//  Ship Demo
//
//  This input controller is primarily designed for keyboard control.  On mobile
//  you will notice that we use gestures to emulate keyboard commands. They even
//  use the same variables (though we need other variables for internal keyboard
//  emulation).  This simplifies our design quite a bit.
//
//  Author: Walker White
//  Version: 1/10/17
//
#include "InputController.h"

using namespace cugl;

constexpr float RANGE_CLAMP(float x, float y, float z) { return (x < y ? y : (x > z ? z : x)); }

#pragma mark -
#pragma mark Input Factors

/** The key to use for reseting the game */
constexpr KeyCode RESET_KEY = KeyCode::R;

/** The key for the event handlers */
constexpr unsigned int LISTENER_KEY = 1;

#pragma mark -
#pragma mark Ship Input
/**
 * Creates a new input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
InputController::InputController()
	: active(false), keyReset(false), resetPressed(false), rollAmount(0.0f), tapped(false) {}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void InputController::dispose() {
	if (active) {
		Input::deactivate<Keyboard>();
#ifndef CU_TOUCH_SCREEN

		Mouse* mouse = Input::get<Mouse>();
		mouse->removePressListener(LISTENER_KEY);
		mouse->removeReleaseListener(LISTENER_KEY);
#else
		Input::deactivate<Accelerometer>();
		Touchscreen* touch = Input::get<Touchscreen>();
		touch->removeBeginListener(LISTENER_KEY);
		touch->removeEndListener(LISTENER_KEY);
#endif
		Input::deactivate<TextInput>();
		tapped = false;
		active = false;
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
bool InputController::init() {
	timestamp.mark();
	bool success = true;
	// Process keyboard on all?
	success = Input::activate<Keyboard>();
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN

	Mouse* mouse = Input::get<Mouse>();
	mouse->addPressListener(LISTENER_KEY,
							[=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
								this->clickBeganCB(event, clicks, focus);
							});
	mouse->addReleaseListener(LISTENER_KEY,
							  [=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
								  this->clickEndedCB(event, clicks, focus);
							  });

#else
	success = Input::activate<Accelerometer>();
	Touchscreen* touch = Input::get<Touchscreen>();

	touch->addBeginListener(LISTENER_KEY, [=](const cugl::TouchEvent& event, bool focus) {
		this->touchBeganCB(event, focus);
	});
	touch->addEndListener(LISTENER_KEY, [=](const cugl::TouchEvent& event, bool focus) {
		this->touchEndedCB(event, focus);
	});
#endif
	success = success && Input::activate<TextInput>();
	tapped = false;
	active = success;
	return success;
}

/**
 * Processes the currently cached inputs.
 *
 * This method is used to to poll the current input state.  This will poll the
 * keyboad and accelerometer.
 *
 * This method also gathers the delta difference in the touches. Depending on
 * the OS, we may see multiple updates of the same touch in a single animation
 * frame, so we need to accumulate all of the data together.
 */
void InputController::update(float dt) {
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
	Keyboard* keys = Input::get<Keyboard>();
	keyReset = keys->keyPressed(RESET_KEY);

	// Forces increase the longer you hold a key.
	if (keys->keyDown(KeyCode::ARROW_LEFT)) {
		rollAmount = -1.0f;
	} else if (keys->keyDown(KeyCode::ARROW_RIGHT)) {
		rollAmount = 1.0f;
	} else {
		rollAmount = 0.0f;
	}
#else
	// MOBILE CONTROLS
	Vec3 acc = Input::get<Accelerometer>()->getAcceleration();

	// Apply to thrust directly.
	rollAmount = acc.x;

#endif

	resetPressed = keyReset;
	if (keyReset) {
		rollAmount = 0;
	}

#ifdef CU_TOUCH_SCREEN
	keyReset = false;
#endif
}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void InputController::clear() {
	resetPressed = false;
	tapped = false;
	rollAmount = 0.0f;

	dtouch = Vec2::ZERO;
	tapLoc = Vec2::ZERO;
	timestamp.mark();
}

#pragma mark -
#pragma mark Touch Callbacks
/**
 * Callback for the beginning of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchBeganCB(const cugl::TouchEvent& event, bool focus) {
	// Update the tap location for jump
	tapLoc.set(event.position);
	tapped = true;
}

/**
 * Callback for the end of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchEndedCB(const cugl::TouchEvent& event, bool focus) {}

#pragma mark -
#pragma mark Click Callbacks
/**
 * Callback for the beginning of a click event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::clickBeganCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
	// Update the click location for jump
	tapLoc.set(event.position);
	tapped = true;
}

/**
 * Callback for the end of a click event
 *
 * @param t     The click information
 * @param event The associated event
 */
void InputController::clickEndedCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {}
