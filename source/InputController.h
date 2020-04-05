﻿#ifndef __INPUT_CONTROLLER_H__
#define __INPUT_CONTROLLER_H__
#include <cugl/cugl.h>

/**
 * This class represents player input.
 *
 * This input handler uses the CUGL input API.  It uses the polling API for
 * keyboard, but the callback API for touch.  This demonstrates a mix of ways
 * to handle input, and the reason for hiding it behind an abstraction like
 * this class.
 *
 * Unlike CUGL input devices, this class is not a singleton.  It must be
 * allocated before use.  However, you will notice that we do not do any
 * input initialization in the constructor.  This allows us to allocate this
 * controller as a field without using pointers. We simply add the class to the
 * header file of its owner, and delay initialization (via the method init())
 * until later. This is one of the main reasons we like to avoid initialization
 * in the constructor.
 */
class InputController {
   private:
	/** Whether or not this input is active */
	bool active;

	// KEYBOARD EMULATION
	/** Whether the reset key is down */
	bool keyReset;

	// TOUCH SUPPORT
	/** The initial touch location for the current gesture */
	cugl::Vec2 dtouch;
	/** The timestamp for the beginning of the current gesture */
	cugl::Timestamp timestamp;
	/** The ID of the last touch event */
	cugl::TouchID touchID;

	// Input results
	/** Whether the reset action was chosen. */
	bool resetPressed;
	/**
	 * How much the player is trying to roll
	 * -1 for way left, 0 for not rolling, 1 for way right
	 */
	float rollAmount;
	/**
	 * Most Recent Tap location to pass down the scenegraph
	 */
	cugl::Vec2 tapLoc;
	/**
	 * Whether a tap recently occurred
	 */
	bool tapped;
	/**
	 * Starting location of last tap.
	 */
	cugl::Vec2 tapStart;
	/**
	 * Ending location of last tap.
	 */
	cugl::Vec2 tapEnd;

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new input controller.
	 *
	 * This constructor does NOT do any initialzation.  It simply allocates the
	 * object. This makes it safe to use this class without a pointer.
	 */
	InputController(); // Don't initialize.  Allow stack based

	/**
	 * Disposes of this input controller, releasing all listeners.
	 */
	~InputController() { dispose(); }

	/**
	 * Deactivates this input controller, releasing all listeners.
	 *
	 * This method will not dispose of the input controller. It can be reused
	 * once it is reinitialized.
	 */
	void dispose();

	/**
	 * Initializes the input control
	 *
	 * This method works like a proper constructor, initializing the input
	 * controller, allocating memory and attaching listeners.
	 *
	 * @return true if the controller was initialized successfully
	 */
	bool init();

#pragma mark -
#pragma mark Input Detection
	/**
	 * Returns true if the input handler is currently active
	 *
	 * @return true if the input handler is currently active
	 */
	bool isActive() const { return active; }

	/**
	 * Processes the currently cached inputs.
	 *
	 * This method is used to to poll the current input state.  This will poll
	 * the keyboad and accelerometer.
	 *
	 * This method also gathers the delta difference in the touches. Depending
	 * on the OS, we may see multiple updates of the same touch in a single
	 * animation frame, so we need to accumulate all of the data together.
	 */
	void update(float dt);

	/**
	 * Clears any buffered inputs so that we may start fresh.
	 */
	void clear();

#pragma mark -
#pragma mark Input Results
	/**
	 * Returns the current roll amount.
	 *
	 * On keyboard, this will be -1, 0, or 1. With accelerometer on mobile, this can take on any
	 * value in the range [-1, 1].
	 *
	 * @return The roll amount. -1 is all left, 1 is all right, 0 is neutral.
	 */
	const float getRoll() { return rollAmount; }

	/**
	 * Returns the most recent tap location if a tap recently happened.
	 *
	 * @return The tap location. cugl::Vec2 of x,y screen coordinates. Returns Vec2::ZERO if no
	 * recent tap.
	 */
	const cugl::Vec2 getTapLoc() {
		if (tapped) {
			tapped = false;
			return tapLoc;
		} else {
			return cugl::Vec2::ZERO;
		}
	}

	/**
	 * Returns where the finger / mouse is currently pressed, or Vec2::ZERO if unpressed.
	 */
	const cugl::Vec2 getCurrTapLoc() const;

	/** Whether information about a new tap is available to read */
	const bool isTapEndAvailable() const { return !tapEnd.isZero(); }

	/** Returns the start and end locations of the last tap iff isTapEndAvailable is true, otherwise
	 * undefined */
	const std::tuple<cugl::Vec2, cugl::Vec2> getTapEndLoc() {
		std::tuple<cugl::Vec2, cugl::Vec2> r = std::make_tuple(tapStart, tapEnd);
		tapStart.setZero();
		tapEnd.setZero();
		return r;
	}

#pragma mark -
#pragma mark Touch Callbacks
	/**
	 * Callback for the beginning of a touch event
	 *
	 * @param t     The touch information
	 * @param event The associated event
	 */
	void touchBeganCB(const cugl::TouchEvent& event, bool focus);

	/**
	 * Callback for the end of a touch event
	 *
	 * @param t     The touch information
	 * @param event The associated event
	 */
	void touchEndedCB(const cugl::TouchEvent& event, bool focus);

#pragma mark -
#pragma mark Click Callbacks
	/**
	 * Callback for the beginning of a click event
	 *
	 * @param t     The click information
	 * @param event The associated event
	 */
	void clickBeganCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);

	/**
	 * Callback for the end of a click event
	 *
	 * @param t     The click information
	 * @param event The associated event
	 */
	void clickEndedCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus);
};

#endif /* __INPUT_CONTROLLER_H__ */
