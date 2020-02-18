//
//  CUAcclerometer.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides basic acclerometer support. It uses the joystick
//  subsystem, which is guaranteed to work on both iOS and Android.
//
//  This class is a singleton and should never be allocated directly.  It
//  should only be accessed via the Input dispatcher.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 7/12/16
#include <cugl/input/CUAccelerometer.h>
#include <cugl/base/CUDisplay.h>
#include <limits.h>

/** This value should be good enough for most applications */
#define DEFAULT_THRESHOLD 0.1f

using namespace cugl;

/** Force multiplier to calibrate the accelerometers */
#if defined (__IPHONEOS__)
    #define SDL_MAX_GFORCE 5.0f
#else
//	If we aren't on an iPhone, then we need to weaken it a bit
    #define SDL_MAX_GFORCE 1.0f
#endif

#ifndef INT16_MAX
    #define INT16_MAX (0x7fff)
#endif

/**
 * Creates and initializes a new accelerometer device.
 *
 * WARNING: Never allocate a accelerometer device directly.  Always use the
 * {@link Input#activate()} method instead.
 */
Accelerometer::Accelerometer() : InputDevice(),
_xaxis(0),
_yaxis(1),
_input(nullptr),
_update(false),
_threshold(DEFAULT_THRESHOLD) {}

/**
 * Initializes this device, acquiring any necessary resources
 *
 * @return true if initialization was successful
 */
bool Accelerometer::init() {
    // Check for joystick
    int limit = SDL_NumJoysticks();
    for(int ii = 0; _input == nullptr && ii < limit; ii++) {
        std::string name(SDL_JoystickNameForIndex(ii));

        // This is guaranteed to work for Android and iOS
        if (name.substr(name.find_last_of(' ')+1) == "Accelerometer") {
            _input = SDL_JoystickOpen(ii);
        }
    }
    
#if defined (__IPHONEOS__)
    // Meaning of xaxis and yaxis depends on whether or not we are in landscape mode
    if (Display::get()->getAspectRatio() > 1) {
        _xaxis = 1; _yaxis = 0;
    }
    // Android does not appear to have this issue.
#endif
    
    return _input != nullptr;
}

/**
 * Unintializes this device, returning it to its default state
 *
 * An uninitialized device may not work without reinitialization.
 */
void Accelerometer::dispose() {
    if (_input != nullptr) {
        SDL_JoystickClose(_input);
        _input = nullptr;
    }
    _threshold = DEFAULT_THRESHOLD;
    _update = false;
    _current.setZero();
    _previous.setZero();
}

#pragma mark -
#pragma mark Listeners
/**
 * Requests focus for the given identifier
 *
 * Only a listener can have focus.  This method returns false if key
 * does not refer to an active listener
 *
 * @param key   The identifier for the focus object
 *
 * @return false if key does not refer to an active listener
 */
bool Accelerometer::requestFocus(Uint32 key) {
    if (isListener(key)) {
        _focus = key;
        return true;
    }
    return false;
}

/**
 * Returns true if key represents a listener object
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool Accelerometer::isListener(Uint32 key) const {
    return _listeners.find(key) != _listeners.end();
}

/**
 * Returns the acceleration listener for the given object key
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the accleration listener for the given object key
 */
const Accelerometer::Listener Accelerometer::getListener(Uint32 key) const {
    if (isListener(key)) {
        return (_listeners.at(key));
    }
    return nullptr;
}

/**
 * Adds an acceleration listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Accelerometer::addListener(Uint32 key, Accelerometer::Listener listener) {
    if (!isListener(key)) {
        _listeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Removes the acceleration listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Accelerometer::removeListener(Uint32 key) {
    if (isListener(key)) {
        _listeners.erase(key);
        return true;
    }
    return false;
}


#pragma mark -
#pragma mark Input Device
/**
 * Clears the state of this input device, readying it for the next frame.
 *
 * Many devices keep track of what happened "this" frame.  This method is
 * necessary to advance the frame.
 */
void Accelerometer::clearState() {
    _previous = _current;
    _update = false;
}

/**
 * Processes an SDL_Event
 *
 * The dispatcher guarantees that an input device only receives events that
 * it subscribes to.
 *
 * @param event The input event to process
 * @param stamp The event timestamp in CUGL time
 *
 * @return false if the input indicates that the application should quit.
 */
bool Accelerometer::updateState(const SDL_Event& event, const Timestamp& stamp) {
    // Apparently SDL sends events for each axis.  We filter this noise.
    if (_update || event.type != SDL_JOYAXISMOTION) { return  true; }
    _current.set(SDL_JoystickGetAxis(_input, _xaxis),SDL_JoystickGetAxis(_input, _yaxis),SDL_JoystickGetAxis(_input, 2));
    _current *= SDL_MAX_GFORCE / INT16_MAX;
#if defined (__IPHONEOS__)
	_current.y = -_current.y;
#endif
    _update = true;
    if (_current.distanceSquared(_anchor) >= _threshold) {
        AccelerationEvent aevent(_current,_current-_anchor,stamp);
        _anchor = _current;
        for(auto it=_listeners.begin(); it != _listeners.end(); ++it) {
            it->second(aevent,it->first == _focus);
        }
    }
    return true;
}

/**
 * Determine the SDL events of relevance and store there types in eventset.
 *
 * An SDL_EventType is really Uint32.  This method stores the SDL event
 * types for this input device into the vector eventset, appending them
 * to the end. The Input dispatcher then uses this information to set up
 * subscriptions.
 *
 * @param eventset  The set to store the event types.
 */
void Accelerometer::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_JOYAXISMOTION);
}


