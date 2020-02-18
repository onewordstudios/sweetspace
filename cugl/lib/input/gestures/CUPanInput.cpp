//
//  CUPanInput.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides basic support for multifinger pan gestures.  SDL blurs
//  pinches, rotations, and pans all into a single input event.  Therefore, you
//  need to set the sensitivity threshold to distinguish them.
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
#include <cugl/base/CUBase.h>
#include <cugl/base/CUApplication.h>
#include <cugl/input/gestures/CUPanInput.h>
#include <algorithm>

/** The default distance threshold for recognizing pans. */
#define DEFAULT_THRESHOLD   0.0005f

using namespace cugl;

#pragma mark Constructor
/**
 * Creates and initializes a new pan input device.
 *
 * WARNING: Never allocate a pan input device directly.  Always use the
 * {@link Input#activate()} method instead.
 */
PanInput::PanInput() : InputDevice(),
_screen(false),
_active(false),
_fingery(true),
_threshold(DEFAULT_THRESHOLD) {
    _event.fingers = 0;
    _event.position = Vec2::ZERO;
    _event.delta = Vec2::ZERO;
    _event.pan = Vec2::ZERO;
#if defined CU_TOUCH_SCREEN
    _screen = true;
    Size size = Application::get()->getDisplaySize();
    float factor = std::min(size.width,size.height);
    _threshold *= factor*factor;
#endif
}

/**
 * Deletes this input device, disposing of all resources
 */
void PanInput::dispose() {
    _active = false;
    _screen = false;
}

/**
 * Returns true if this device is a touch screen.
 *
 * This device is not guaranteed to be a touch screen.  For example, the
 * trackpad on MacBooks support pinches. We do try to make our best guess
 * about whether or not a device is a touch screen, but on some devices
 * this may need to be set manually.
 *
 * If this value is true, all pinch information will scale with the display.
 * Otherwise, the pinch will be normalized to a unit square, where the
 * top left corner of the touch device is (0,0) and the lower right is
 * (1,1). You may want to set this value to false for true cross-platform
 * gesture support.
 *
 * @return true if this device is a touch screen.
 */
void PanInput::setTouchScreen(bool flag) {
    if (_screen != flag) {
        _event.position = Vec2::ZERO;
        _event.delta = Vec2::ZERO;
        _event.pan = Vec2::ZERO;
        
        // Get the threshold adjustment
        Size size = Application::get()->getDisplaySize();
        float factor = std::min(size.width,size.height);
        if (flag) {
            _threshold *= factor*factor;
        } else {
            _threshold /= factor*factor;
        }
    }
    _screen = flag;
}

/**
 * Sets the distance threshold for pan events.
 *
 * SDL treats pinches, rotations, and pans as all the same gesture.  The
 * only way to distinguish them is with the treshold factor. A pan that
 * covers less distance than the threshold will not be recorded. This tells
 * the system to ignore small gestures.
 *
 * The correct value for this threshold depends on the attribute touch screen
 * attribute.  If this device is a touch screen, it will be measured in points.
 * Otherwise, it is measured as if the touch device where a unit square.
 *
 * @param threshold The distance threshold for pan events.
 */
void PanInput::setThreshold(float threshold) {
    CUAssertLog(threshold >= 0, "Attempt to use negative threshold %.3f",threshold);
    _threshold = threshold*threshold;
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
bool PanInput::requestFocus(Uint32 key) {
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
 * pinch begin, pinch end, or pinch change.
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool PanInput::isListener(Uint32 key) const {
    bool result = _beginListeners.find(key) != _beginListeners.end();
    result = result || _finishListeners.find(key) != _finishListeners.end();
    result = result || _motionListeners.find(key) != _motionListeners.end();
    return result;
}

/**
 * Returns the pinch begin listener for the given object key
 *
 * This listener is invoked when pinch crosses the distance threshold.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the pinch begin listener for the given object key
 */
const PanInput::Listener PanInput::getBeginListener(Uint32 key) const {
    if (_beginListeners.find(key) != _beginListeners.end()) {
        return (_beginListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the pinch end listener for the given object key
 *
 * This listener is invoked when all (but one) fingers in an active pinch
 * are released.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the pinch end listener for the given object key
 */
const PanInput::Listener PanInput::getEndListener(Uint32 key) const {
    if (_finishListeners.find(key) != _finishListeners.end()) {
        return (_finishListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the pinch change listener for the given object key
 *
 * This listener is invoked when the pinch distance changes.
 *
 * @param key   The identifier for the listener
 *
 * @return the pinch change listener for the given object key
 */
const PanInput::Listener PanInput::getMotionListener(Uint32 key) const {
    if (_motionListeners.find(key) != _motionListeners.end()) {
        return (_motionListeners.at(key));
    }
    return nullptr;
}

/**
 * Adds a pinch begin listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when pinch crosses the distance threshold.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool PanInput::addBeginListener(Uint32 key, PanInput::Listener listener) {
    if (_beginListeners.find(key) == _beginListeners.end()) {
        _beginListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a pinch end listener for the given object key
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
bool PanInput::addEndListener(Uint32 key, PanInput::Listener listener) {
    if (_finishListeners.find(key) == _finishListeners.end()) {
        _finishListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a pinch change listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when the pinch distance changes.
 *
 * @param key       The identifier for the listener
 * @param listener  The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool PanInput::addMotionListener(Uint32 key, PanInput::Listener listener) {
    if (_motionListeners.find(key) == _motionListeners.end()) {
        _motionListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Removes the pinch begin listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when pinch crosses the distance threshold.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool PanInput::removeBeginListener(Uint32 key) {
    if (_beginListeners.find(key) != _beginListeners.end()) {
        _beginListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the pinch end listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when all (but one) fingers in an active pinch
 * are released.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool PanInput::removeEndListener(Uint32 key) {
    if (_finishListeners.find(key) != _finishListeners.end()) {
        _finishListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the pinch change listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when the pinch distance changes.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool PanInput::removeMotionListener(Uint32 key) {
    if (_motionListeners.find(key) != _motionListeners.end()) {
        _motionListeners.erase(key);
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
void PanInput::clearState() {
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
bool PanInput::updateState(const SDL_Event& event, const Timestamp& stamp) {
    switch (event.type) {
        case SDL_FINGERDOWN:
            // At least anchor the point even if not started yet.
            if (_event.fingers < 2) {
                _event.position = getScaledPosition(event.tfinger.x,event.tfinger.y);
            } else if (_active && _fingery) {
                // Reboot events when we add fingers
                for(auto it = _finishListeners.begin(); it != _finishListeners.end(); ++it) {
                    it->second(_event,it->first == _focus);
                }
                for(auto it = _beginListeners.begin(); it != _beginListeners.end(); ++it) {
                    it->second(_event,it->first == _focus);
                }
            }
            _event.fingers++;
            break;
        case SDL_FINGERUP:
        {
            _event.fingers--;
            if (_active) {
                if (_event.fingers <= 1) {
                    _event.timestamp = stamp;
                    for(auto it = _finishListeners.begin(); it != _finishListeners.end(); ++it) {
                        it->second(_event,it->first == _focus);
                    }
                    _event.pan = Vec2::ZERO;
                    _event.delta = Vec2::ZERO;
                    _event.position = Vec2::ZERO;
                    _active = false;
                } else if (_fingery) {
                    // Reboot events when we remove fingers
                    for(auto it = _finishListeners.begin(); it != _finishListeners.end(); ++it) {
                        it->second(_event,it->first == _focus);
                    }
                    for(auto it = _beginListeners.begin(); it != _beginListeners.end(); ++it) {
                        it->second(_event,it->first == _focus);
                    }
                }
            }
        }
            break;
        case SDL_MULTIGESTURE:
        {
            Vec2 current = getScaledPosition(event.mgesture.x,event.mgesture.y);
            _event.delta = current-_event.position;
            _event.position = current;
            _event.timestamp = stamp;
 
            if (_active) {
                _event.pan += _event.delta;
                for(auto it = _motionListeners.begin(); it != _motionListeners.end(); ++it) {
                    it->second(_event,it->first == _focus);
                }
            } else if (_event.delta.lengthSquared() > _threshold) {
                _active = true;
                _event.pan = Vec2::ZERO;
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
 * Returns the scale/unscaled touch position.
 *
 * The value returned depends on the value of attribute _screen.  If this
 * attribute is false, the position is normalized to the unit square.
 * Otherwise it is scaled to the touch screen.
 *
 * @return the scale/unscaled touch position.
 */
Vec2 PanInput::getScaledPosition(float x, float y) {
    Vec2 result(x,y);
    if (_screen) {
        Size size = Application::get()->getDisplayBounds().size;
        result *= size;
        result += Application::get()->getDisplayBounds().origin;
    }
    return result;
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
void PanInput::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_FINGERDOWN);
    eventset.push_back((Uint32)SDL_FINGERUP);
    eventset.push_back((Uint32)SDL_FINGERMOTION);
    eventset.push_back((Uint32)SDL_MULTIGESTURE);
}
