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
#include "SDInput.h"

using namespace cugl;

#define RANGE_CLAMP(x,y,z)  (x < y ? y : (x > z ? z : x))

#pragma mark -
#pragma mark Input Factors

/** Historical choice from Marmalade */
#define INPUT_MAXIMUM_FORCE         1000.0f
/** Adjustment factor for touch input */
#define X_ADJUST_FACTOR             500.0f
#define Y_ADJUST_FACTOR             50.0f
/** Adjustment factor for accelerometer input (found experimentally) */
#define ACCELEROM_X_FACTOR          5.0f
#define ACCELEROM_Y_FACTOR          200.0f
/** Adjustment factors for keyboard input */
#define KEYBOARD_INITIAL_FORCE      10.0f
#define KEYBOARD_FORCE_INCREMENT    10.0f
#define KEYBOARD_ACCELERATION       1.2f
#define KEYBOARD_DAMPEN_AMOUNT      0.75f

/** Whether to active the accelerometer (this is TRICKY!) */
#define USE_ACCELEROMETER           false
/** The key to use for reseting the game */
#define RESET_KEY                   KeyCode::R
/** How the time necessary to process a double tap (in milliseconds) */
#define EVENT_DOUBLE_CLICK          400
/** The key for the event handlers */
#define LISTENER_KEY                1

#pragma mark -
#pragma mark Ship Input
/**
 * Creates a new input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
ShipInput::ShipInput() :
_active(false),
_keyReset(false),
_resetPressed(false),
_forceLeft(0.0f),
_forceRight(0.0f),
_forceUp(0.0f),
_forceDown(0.0f)
{
}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void ShipInput::dispose() {
    if (_active) {
#ifndef CU_TOUCH_SCREEN
        Input::deactivate<Keyboard>();
#else
        if (USE_ACCELEROMETER) {
            Input::deactivate<Accelerometer>();
        }
        Touchscreen* touch = Input::get<Touchscreen>();
        touch->removeBeginListener(LISTENER_KEY);
        touch->removeEndListener(LISTENER_KEY);
        _active = false;
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
bool ShipInput::init() {
    _timestamp.mark();
    bool success = true;
    
// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    success = Input::activate<Keyboard>();
#else
    if (USE_ACCELEROMETER) {
        success = Input::activate<Accelerometer>();
    }
    Touchscreen* touch = Input::get<Touchscreen>();

    touch->addBeginListener(LISTENER_KEY,[=](const cugl::TouchEvent& event, bool focus) {
        this->touchBeganCB(event,focus);
    });
    touch->addEndListener(LISTENER_KEY,[=](const cugl::TouchEvent& event, bool focus) {
        this->touchEndedCB(event,focus);
    });
#endif
    
    _active = success;
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
void ShipInput::update(float dt) {

// Only process keyboard on desktop
#ifndef CU_TOUCH_SCREEN
    Keyboard* keys = Input::get<Keyboard>();
    _keyReset  = keys->keyPressed(RESET_KEY);

    // Forces increase the longer you hold a key.
    if (keys->keyDown(KeyCode::ARROW_LEFT)) {
        _forceLeft += KEYBOARD_FORCE_INCREMENT;
    } else {
        _forceLeft  = 0.0f;
    }
    if (keys->keyDown(KeyCode::ARROW_RIGHT)) {
        _forceRight += KEYBOARD_FORCE_INCREMENT;
    } else {
        _forceRight  = 0.0f;
    }
    if (keys->keyDown(KeyCode::ARROW_DOWN)) {
        _forceDown += KEYBOARD_FORCE_INCREMENT;
    } else {
        _forceDown  = 0.0f;
    }
    if (keys->keyDown(KeyCode::ARROW_UP)) {
        _forceUp += KEYBOARD_FORCE_INCREMENT;
    } else {
        _forceUp  = 0.0f;
    }
    
    // Clamp everything so it does not fly off to infinity.
    _forceLeft  = (_forceLeft  > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : _forceLeft);
    _forceRight = (_forceRight > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : _forceRight);
    _forceDown  = (_forceDown  > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : _forceDown);
    _forceUp    = (_forceUp    > INPUT_MAXIMUM_FORCE ? INPUT_MAXIMUM_FORCE : _forceUp);

    // Update the keyboard thrust.  Result is cumulative.
    _keybdThrust.x += _forceRight;
    _keybdThrust.x -= _forceLeft;
    _keybdThrust.y += _forceUp;
    _keybdThrust.y -= _forceDown;
    _keybdThrust.x = RANGE_CLAMP(_keybdThrust.x, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);
    _keybdThrust.y = RANGE_CLAMP(_keybdThrust.y, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);
    
    // Transfer to main thrust. This keeps us from "adding" to accelerometer or touch.
    _inputThrust.x = _keybdThrust.x/X_ADJUST_FACTOR;
    _inputThrust.y = _keybdThrust.y/Y_ADJUST_FACTOR;
#else
    // MOBILE CONTROLS
    if (USE_ACCELEROMETER) {
        Vec3 acc = Input::get<Accelerometer>()->getAcceleration();
   
        // Apply to thrust directly.
        _inputThrust.x =  acc.x*ACCELEROM_X_FACTOR;
        _inputThrust.y = acc.y*ACCELEROM_Y_FACTOR;
    }
    // Otherwise, uses touch
#endif

    _resetPressed = _keyReset;
    if (_keyReset) {
        _inputThrust = Vec2::ZERO;
    }
    
#ifdef CU_TOUCH_SCREEN
    _keyReset = false;
#endif
}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void ShipInput::clear() {
    _resetPressed = false;
    _inputThrust = Vec2::ZERO;
    _keybdThrust = Vec2::ZERO;
    
    _forceLeft  = 0.0f;
    _forceRight = 0.0f;
    _forceUp    = 0.0f;
    _forceDown  = 0.0f;
    
    _dtouch = Vec2::ZERO;
    _timestamp.mark();
}

#pragma mark -
#pragma mark Touch Callbacks
/**
 * Callback for the beginning of a touch event
 *
 * @param t     The touch information
 * @param event The associated event
 */
 void ShipInput::touchBeganCB(const cugl::TouchEvent& event, bool focus) {
     // Update the touch location for later gestures
     _dtouch.set(event.position);
}
 
 /**
  * Callback for the end of a touch event
  *
  * @param t     The touch information
  * @param event The associated event
  */
 void ShipInput::touchEndedCB(const cugl::TouchEvent& event, bool focus) {
     // Check for a double tap.
     _keyReset =  event.timestamp.ellapsedMillis(_timestamp) <= EVENT_DOUBLE_CLICK;
     _timestamp = event.timestamp;
     
     // If we reset, do not record the thrust
     if (_keyReset) {
         return;
     }
     
     // Move the ship in this direction
     Vec2 finishTouch = event.position-_dtouch;
     finishTouch.x = RANGE_CLAMP(finishTouch.x, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);
     finishTouch.y = RANGE_CLAMP(finishTouch.y, -INPUT_MAXIMUM_FORCE, INPUT_MAXIMUM_FORCE);
     
     // Go ahead and apply to thrust now.
     _inputThrust.x = finishTouch.x/X_ADJUST_FACTOR;
     _inputThrust.y = finishTouch.y/-Y_ADJUST_FACTOR;  // Touch coords
 }
