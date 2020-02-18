//
//  CURotationInput.h
//  Cornell University Game Library (CUGL)
//
//  This class provides basic support for rotation gestures.  SDL blurs pinches,
//  rotations, and pans all into a single input event.  Therefore, you need to
//  set the sensitivity threshold to distinguish them.
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
//  Version: 12/20/16
//
#include <cugl/base/CUApplication.h>
#include <cugl/input/gestures/CURotationInput.h>

/** The default angular threshold for recognizing rotations. */
#define DEFAULT_THRESHOLD 0.00001f

using namespace cugl;

#pragma mark Constructor
/**
 * Creates and initializes a new rotation input device.
 *
 * WARNING: Never allocate a rotation input device directly.  Always use the
 * {@link Input#activate()} method instead.
 */
RotationInput::RotationInput() : InputDevice(),
_screen(false),
_active(false),
_threshold(DEFAULT_THRESHOLD) {
    _event.fingers = 0;
    _event.position = Vec2::ZERO;
    _event.rotation = 0.0f;
    _event.delta = 0.0f;
#if defined CU_TOUCH_SCREEN
    _screen = true;
#endif
}

/**
 * Deletes this input device, disposing of all resources
 */
void RotationInput::dispose() {
    _active = false;
}

/**
 * Sets whether this device is a touch screen.
 *
 * This device is not guaranteed to be a touch screen.  For example, the
 * trackpad on MacBooks support rotations. We do try to make our best guess
 * about whether or not a device is a touch screen, but on some devices
 * this may need to be set manually.
 *
 * If this value is true, all rotation information will scale with the
 * display. Otherwise, the rotation angle will be normalized to a unit
 * square, where the top left corner of the touch device is (0,0) and the
 * lower right is (1,1). You may want to set this value to false for true
 * cross-platform gesture support.
 *
 * @param flag  Whether this device is a touch screen.
 */
void RotationInput::setTouchScreen(bool flag) {
    if (_screen != flag) {
        _event.position = Vec2::ZERO;
        _event.rotation = 0.0f;
        _event.delta = 0.0f;
    }
    _screen = flag;
}

/**
 * Sets the angular threshold for rotation events.
 *
 * SDL treats pinches, rotations, and pans as all the same gesture.  The
 * only way to distinguish them is with the treshold factor. A rotation that
 * covers a smaller angle than the threshold will not be recorded. This
 * tells the system to ignore small gestures.
 *
 * @param threshold The angular threshold for rotation events.
 */
void RotationInput::setThreshold(float threshold) {
    CUAssertLog(threshold >= 0, "Attempt to use negative threshold %.3f",threshold);
    _threshold = threshold;
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
bool RotationInput::requestFocus(Uint32 key) {
    if (isListener(key)) {
        _focus = key;
        return true;
    }
    return false;
}

/**
 * Returns true if key represents a listener object
 *
 * An object is a listener if it is a listener for any of the three actions:
 * rotation begin, rotation end, or rotation change.
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool RotationInput::isListener(Uint32 key) const {
    bool result = _beginListeners.find(key) != _beginListeners.end();
    result = result || _finishListeners.find(key) != _finishListeners.end();
    result = result || _changeListeners.find(key) != _changeListeners.end();
    return result;
}

/**
 * Returns the rotational begin listener for the given object key
 *
 * This listener is invoked when the rotation crosses the angular threshold.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the rotational begin listener for the given object key
 */
const RotationInput::Listener RotationInput::getBeginListener(Uint32 key) const {
    if (_beginListeners.find(key) != _beginListeners.end()) {
        return (_beginListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the rotational end listener for the given object key
 *
 * This listener is invoked when all (but one) fingers in an active rotation
 * are released.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the rotational end listener for the given object key
 */
const RotationInput::Listener RotationInput::getEndListener(Uint32 key) const {
    if (_finishListeners.find(key) != _finishListeners.end()) {
        return (_finishListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the rotational change listener for the given object key
 *
 * This listener is invoked when the rotation angle changes.
 *
 * @param key   The identifier for the listener
 *
 * @return the rotational change listener for the given object key
 */
const RotationInput::Listener RotationInput::getChangeListener(Uint32 key) const {
    if (_changeListeners.find(key) != _changeListeners.end()) {
        return (_changeListeners.at(key));
    }
    return nullptr;
}

/**
 * Adds a rotational begin listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when the rotation crosses the angular threshold.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool RotationInput::addBeginListener(Uint32 key, RotationInput::Listener listener) {
    if (_beginListeners.find(key) == _beginListeners.end()) {
        _beginListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a rotational end listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when all (but one) fingers in an active pinch
 * are released.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool RotationInput::addEndListener(Uint32 key, RotationInput::Listener listener) {
    if (_finishListeners.find(key) == _finishListeners.end()) {
        _finishListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a rotational change listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when the rotation angle changes.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool RotationInput::addChangeListener(Uint32 key, RotationInput::Listener listener) {
    if (_changeListeners.find(key) == _changeListeners.end()) {
        _changeListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Removes the rotational begin listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when the rotation crosses the angular threshold.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool RotationInput::removeBeginListener(Uint32 key) {
    if (_beginListeners.find(key) != _beginListeners.end()) {
        _beginListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the rotational end listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when all (but one) fingers in an active rotation
 * are released.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool RotationInput::removeEndListener(Uint32 key) {
    if (_finishListeners.find(key) != _finishListeners.end()) {
        _finishListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the rotational change listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when the rotation angle changes.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool RotationInput::removeChangeListener(Uint32 key) {
    if (_changeListeners.find(key) != _changeListeners.end()) {
        _changeListeners.erase(key);
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
void RotationInput::clearState() {
    // This is all event driven.  Do nothing.
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
bool RotationInput::updateState(const SDL_Event& event, const Timestamp& stamp) {
    switch (event.type) {
        case SDL_FINGERDOWN:
            _event.fingers++;
            break;
        case SDL_FINGERUP:
        {
            _event.fingers--;
            if (_active && _event.fingers <= 1) {
                _event.timestamp = stamp;
                for(auto it = _finishListeners.begin(); it != _finishListeners.end(); ++it) {
                    it->second(_event,it->first == _focus);
                }
                _event.delta = 0.0f;
                _event.rotation = 0.0f;
                _event.position = Vec2::ZERO;
                _active = false;
            }
        }
            break;
        case SDL_MULTIGESTURE:
        {
            _event.position.set(event.mgesture.x,event.mgesture.y);
            if (_screen) {
                Size size = Application::get()->getDisplaySize();
                _event.position *= size;
                _event.position += Application::get()->getDisplayBounds().origin;
            }
            _event.delta = event.mgesture.dTheta;
            _event.rotation += _event.delta;
            _event.timestamp = stamp;
            if (_active) {
                for(auto it = _changeListeners.begin(); it != _changeListeners.end(); ++it) {
                    it->second(_event,it->first == _focus);
                }
            } else if (_event.rotation > _threshold || _event.rotation < -_threshold) {
                _active = true;
                for(auto it = _beginListeners.begin(); it != _beginListeners.end(); ++it) {
                    it->second(_event,it->first == _focus);
                }
            }
        }
            break;
        default:
            // Do nothing
            break;
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
void RotationInput::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_FINGERDOWN);
    eventset.push_back((Uint32)SDL_FINGERUP);
    eventset.push_back((Uint32)SDL_FINGERMOTION);
    eventset.push_back((Uint32)SDL_MULTIGESTURE);
}
