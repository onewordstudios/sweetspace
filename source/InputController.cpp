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

/** Historical choice from Marmalade */
constexpr float INPUT_MAXIMUM_FORCE = 1000.0f;
/** Adjustment factor for touch input */
constexpr float X_ADJUST_FACTOR = 500.0f;
/** Adjustment factor for accelerometer input (found experimentally) */
constexpr float ACCELEROM_X_FACTOR = 5.0f;
constexpr float ACCELEROM_Y_FACTOR = 200.0f;
/** Adjustment factors for keyboard input */
constexpr float KEYBOARD_FORCE_INCREMENT = 10.0f;

/** Whether to active the accelerometer (this is TRICKY!) */
constexpr bool USE_ACCELEROMETER = false;
/** The key to use for reseting the game */
constexpr KeyCode RESET_KEY = KeyCode::R;
/** How the time necessary to process a double tap (in milliseconds) */
constexpr unsigned int EVENT_DOUBLE_CLICK = 400;

#pragma mark -
#pragma mark Ship Input
/**
 * Creates a new input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
InputController::InputController()
	: active(false),
	  keyReset(false),
	  forceLeft(0.0f),
	  forceRight(0.0f),
	  keybdThrust(0.0f),
	  resetPressed(false),
	  rollAmount(0.0f) {}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void InputController::dispose() {
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
bool InputController::init() {
	timestamp.mark();
	bool success = true;

// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
	success = Input::activate<Keyboard>();
#else
	if (USE_ACCELEROMETER) {
		success = Input::activate<Accelerometer>();
	}
	Touchscreen* touch = Input::get<Touchscreen>();

	touch->addBeginListener(LISTENER_KEY, [=](const cugl::TouchEvent& event, bool focus) {
		this->touchBeganCB(event, focus);
	});
	touch->addEndListener(LISTENER_KEY, [=](const cugl::TouchEvent& event, bool focus) {
		this->touchEndedCB(event, focus);
	});
#endif

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
		forceLeft += KEYBOARD_FORCE_INCREMENT;
	} else {
		forceLeft = 0.0f;
	}
	if (keys->keyDown(KeyCode::ARROW_RIGHT)) {
		forceRight += KEYBOARD_FORCE_INCREMENT;
	} else {
		forceRight = 0.0f;
	}

	// Clamp everything so it does not fly off to infinity.
	forceLeft = (forceLeft > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : forceLeft);
	forceRight = (forceRight > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : forceRight);

	// Update the keyboard thrust.  Result is cumulative.
	keybdThrust += forceRight;
	keybdThrust -= forceLeft;
	keybdThrust = RANGE_CLAMP(keybdThrust, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);

	// Transfer to main thrust. This keeps us from "adding" to accelerometer or touch.
	rollAmount = keybdThrust;
#else
	// MOBILE CONTROLS
	if (USE_ACCELEROMETER) {
		Vec3 acc = Input::get<Accelerometer>()->getAcceleration();

		// Apply to thrust directly.
		rollAmount = acc.x * ACCELEROM_X_FACTOR;
	}
	// Otherwise, uses touch
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
	rollAmount = 0.0f;
	keybdThrust = 0.0f;

	forceLeft = 0.0f;
	forceRight = 0.0f;

	dtouch = Vec2::ZERO;
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
	// Update the touch location for later gestures
	dtouch.set(event.position);
}

/**
 * Callback for the end of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchEndedCB(const cugl::TouchEvent& event, bool focus) {
	// Check for a double tap.
	keyReset = event.timestamp.ellapsedMillis(timestamp) <= EVENT_DOUBLE_CLICK;
	timestamp = event.timestamp;

	// If we reset, do not record the thrust
	if (keyReset) {
		return;
	}

	// Move the ship in this direction
	Vec2 finishTouch = event.position - dtouch;
	finishTouch.x = RANGE_CLAMP(finishTouch.x, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);
	finishTouch.y = RANGE_CLAMP(finishTouch.y, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);

	// Go ahead and apply to thrust now.
	rollAmount = finishTouch.x / X_ADJUST_FACTOR;
}
