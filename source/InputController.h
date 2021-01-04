#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H
#include <cugl/cugl.h>

/**
 * This class represents player input.
 *
 * This input handler uses the CUGL input API.  It uses the polling API for
 * keyboard, but the callback API for touch.  This demonstrates a mix of ways
 * to handle input, and the reason for hiding it behind an abstraction like
 * this class.
 *
 * This class is a singleton. It is initialized the first time the instance is acquired.
 */
class InputController {
   private:
	/**
	 * The singleton instance of this class.
	 */
	static std::shared_ptr<InputController> instance; // NOLINT

	/** Whether or not this input is active */
	bool active;

	// TOUCH SUPPORT
	/** The ID of the last touch event */
	cugl::TouchID touchID;

	// Input results
	/**
	 * How much the player is trying to roll
	 * -1 for way left, 0 for not rolling, 1 for way right
	 */
	float rollAmount;
	/**
	 * Whether a jump recently occurred
	 */
	bool jumped;
	/**
	 * Starting location of last tap, or zero if none occurred
	 */
	cugl::Vec2 tapStart;
	/**
	 * Ending location of last tap.
	 */
	cugl::Vec2 tapEnd;
	/**
	 * Whether the escape or back button was recently pressed
	 */
	bool backPressed;

	/**
	 * Creates a new input controller.
	 *
	 * This constructor does NOT do any initialzation.  It simply allocates the
	 * object. This makes it safe to use this class without a pointer.
	 */
	InputController();

   public:
#pragma region Constructors

	/**
	 * Grab a pointer to the singleton instance of this class.
	 *
	 * If this is the first time this is called, or if the class was previously disposed, this will
	 * initialize all the input devices too.
	 */
	static std::shared_ptr<InputController> getInstance() {
		if (instance == nullptr) {
			instance = std::shared_ptr<InputController>(new InputController());
		}
		return instance;
	}

	/**
	 * Deactivates and disposes of this input controller, releasing all listeners.
	 */
	~InputController();

	/**
	 * Deactivates and disposes of the instance, if it exists. Note that subsequent calls to {@link
	 * getInstance} will automatically reinitialize the class.
	 */
	static void cleanup() { instance = nullptr; }

#pragma endregion
#pragma region Input Detection
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

#pragma endregion
#pragma region Generic Input Results

	/**
	 * Returns where the finger / mouse is currently pressed, or Vec2::ZERO if unpressed.
	 */
	static cugl::Vec2 getCurrTapLoc();

	/** Whether information about a new tap is available to read */
	bool isTapEndAvailable() const { return !tapEnd.isZero(); }

	/** Returns the start and end locations of the last tap iff isTapEndAvailable is true, otherwise
	 * undefined */
	std::tuple<cugl::Vec2, cugl::Vec2> getTapEndLoc() {
		std::tuple<cugl::Vec2, cugl::Vec2> r = std::make_tuple(tapStart, tapEnd);
		tapStart.setZero();
		tapEnd.setZero();
		return r;
	}

	/**
	 * Return whether the player has pressed back since the last time this method was queried
	 */
	bool hasPressedBack();

#pragma endregion
#pragma region Gameplay Input Results

	/**
	 * Returns the current roll amount.
	 *
	 * On keyboard, this will be -1, 0, or 1. With accelerometer on mobile, this can take on any
	 * value in the range [-1, 1].
	 *
	 * @return The roll amount. -1 is all left, 1 is all right, 0 is neutral.
	 */
	float getRoll() const { return rollAmount; }

	/**
	 * Return whether the player has jumped since the last time this method was queried
	 */
	bool hasJumped();

#pragma endregion
#pragma region Callbacks
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
#pragma endregion
};

#endif /* __INPUT_CONTROLLER_H__ */
