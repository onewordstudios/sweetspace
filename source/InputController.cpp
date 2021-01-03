#include "InputController.h"

using namespace cugl;

constexpr float RANGE_CLAMP(float x, float y, float z) { return (x < y ? y : (x > z ? z : x)); }

std::shared_ptr<InputController> InputController::instance; // NOLINT (clang-tidy bug)

#pragma region Input Factors

/** The key to use for reseting the game */
constexpr KeyCode JUMP_KEY = KeyCode::SPACE;

/** The key for the event handlers */
constexpr unsigned int LISTENER_KEY = 1;

#pragma endregion
#pragma region Class Setup
/**
 * Creates a new input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
InputController::InputController()
	: active(false), rollAmount(0.0f), jumped(false), backPressed(false) {
	touchID = -1;
	bool success = Input::activate<Keyboard>();

#ifndef CU_TOUCH_SCREEN

	Input::activate<Mouse>();
	auto* mouse = Input::get<Mouse>();
	mouse->setPointerAwareness(cugl::Mouse::PointerAwareness::ALWAYS);
	mouse->addPressListener(LISTENER_KEY,
							[=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
								this->clickBeganCB(event, clicks, focus);
							});
	mouse->addReleaseListener(LISTENER_KEY,
							  [=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
								  this->clickEndedCB(event, clicks, focus);
							  });

#else
	Input::activate<Touchscreen>();

	success = Input::activate<Accelerometer>();
	Touchscreen* touch = Input::get<Touchscreen>();

	touch->addBeginListener(LISTENER_KEY, [=](const cugl::TouchEvent& event, bool focus) {
		this->touchBeganCB(event, focus);
	});
	touch->addEndListener(LISTENER_KEY, [=](const cugl::TouchEvent& event, bool focus) {
		this->touchEndedCB(event, focus);
	});
#endif
	jumped = false;
	active = success;
}

InputController::~InputController() {
	if (active) {
		Input::deactivate<Keyboard>();
#ifndef CU_TOUCH_SCREEN

		auto* mouse = Input::get<Mouse>();
		mouse->removePressListener(LISTENER_KEY);
		mouse->removeReleaseListener(LISTENER_KEY);

		Input::deactivate<Mouse>();
#else
		Input::deactivate<Touchscreen>();

		Input::deactivate<Accelerometer>();
		Touchscreen* touch = Input::get<Touchscreen>();
		touch->removeBeginListener(LISTENER_KEY);
		touch->removeEndListener(LISTENER_KEY);
#endif
		jumped = false;
		active = false;
	}
}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void InputController::clear() {
	jumped = false;
	backPressed = false;
	rollAmount = 0.0f;
	tapStart.setZero();
	tapEnd.setZero();
	touchID = -1;
}

#pragma endregion
#pragma region Update

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
void InputController::update(float  /*dt*/) {
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
	auto* keys = Input::get<Keyboard>();
	if (keys->keyPressed(JUMP_KEY)) {
		jumped = true;
	} else if (keys->keyPressed(KeyCode::ESCAPE)) {
		backPressed = true;
		CULog("Escape key pressed");
	}

	// Forces increase the longer you hold a key.
	if (keys->keyDown(KeyCode::ARROW_LEFT)) {
		rollAmount = -1.0f;
	} else if (keys->keyDown(KeyCode::ARROW_RIGHT)) {
		rollAmount = 1.0f;
	} else {
		rollAmount = 0.0f;
	}
#else
	Keyboard* keys = Input::get<Keyboard>();
	if (keys->keyPressed(KeyCode::ANDROID_BACK)) {
		CULog("Android back button pressed");
		backPressed = true;
	}

	// MOBILE CONTROLS
	Vec3 acc = Input::get<Accelerometer>()->getAcceleration();

	// Apply to thrust directly.
	rollAmount = acc.x;

#endif
}

#pragma endregion
#pragma region Getters

bool InputController::hasJumped() {
	if (jumped) {
		jumped = false;
		return true;
	}
	return false;
}

cugl::Vec2 InputController::getCurrTapLoc() {
#ifndef CU_TOUCH_SCREEN
	auto* mouse = Input::get<Mouse>();
	if (mouse->buttonDown().hasLeft()) {
		return mouse->pointerPosition();
	}
#else
	Touchscreen* touch = Input::get<Touchscreen>();
	if (touch->touchDown(touchID)) {
		return touch->touchPosition(touchID);
	}
#endif
	return cugl::Vec2::ZERO;
}

bool InputController::hasPressedBack() {
	if (backPressed) {
		backPressed = false;
		return true;
	}
	return false;
}

#pragma endregion
#pragma region Callbacks
/**
 * Callback for the beginning of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchBeganCB(const cugl::TouchEvent& event, bool  /*focus*/) {
	tapStart.set(event.position);
	touchID = event.touch;
	jumped = true;
}

/**
 * Callback for the end of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::touchEndedCB(const cugl::TouchEvent& event, bool  /*focus*/) {
	tapEnd.set(event.position);
}

/**
 * Callback for the beginning of a click event
 *
 * @param t     The touch information
 * @param event The associated event
 */
void InputController::clickBeganCB(const cugl::MouseEvent& event, Uint8  /*clicks*/, bool  /*focus*/) {
	tapStart.set(event.position);
	jumped = true;
}

/**
 * Callback for the end of a click event
 *
 * @param t     The click information
 * @param event The associated event
 */
void InputController::clickEndedCB(const cugl::MouseEvent& event, Uint8  /*clicks*/, bool  /*focus*/) {
	tapEnd.set(event.position);
}
#pragma endregion
