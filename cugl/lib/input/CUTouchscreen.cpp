//
//  CUTouchscreen.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides basic touch support. While it supports multitouch, it
//  only receives one touch per event.  For complex multitouch events (such as
//  gestures) you shouls use GestureInput instead.
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

#include <cugl/input/CUTouchscreen.h>
#include <cugl/base/CUApplication.h>

using namespace cugl;

/**
 * Deletes this input device, disposing of all resources
 */
void Touchscreen::dispose() {
    _current.clear();
    _previous.clear();
    _beginListeners.clear();
    _finishListeners.clear();
    _moveListeners.clear();
}

#pragma mark -
#pragma mark Data Polling
/**
 * Returns the position of the finger touch.
 *
 * If touch is not a finger currently held down, this method will cause
 * an error.
 *
 * @param touch An identifier for a touch/finger
 *
 * @return the position of the finger touch.
 */
Vec2 Touchscreen::touchPosition(TouchID touch) const {
    CUAssertLog(_current.find(touch) != _current.end(), "There is no touch for id %lld", touch);
    return _current.at(touch);
}

/**
 * Returns the difference between the current and previous position of touch.
 *
 * If the finger was just pressed this frame, it will return the current
 * position. If touch is not a finger currently held down, this method will
 * cause an error.
 *
 * @param touch An identifier for a touch/finger
 *
 * @return the difference between the current and previous position of touch.
 */
Vec2 Touchscreen::touchOffset(TouchID touch) const {
    CUAssertLog(_current.find(touch) != _current.end(), "There is no touch for id %lld", touch);
    Vec2 offset = _current.at(touch);
    if (_previous.find(touch) == _previous.end()) {
        offset -= _previous.at(touch);
    } else {
        offset.setZero();
    }
    return offset;
}

/**
 * Returns the set of identifiers for the fingers currently held down.
 *
 * @return the set of identifiers for the fingers currently held down.
 */
const std::vector<TouchID> Touchscreen::touchSet() const {
    std::vector<TouchID> result;
    result.reserve(_current.size());
    for(auto it = _current.begin(); it != _current.end(); ++it) {
        result.push_back(it->first);
    }
    return result;
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
bool Touchscreen::requestFocus(Uint32 key) {
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
 * touch begin, touch end, or touch motion.
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool Touchscreen::isListener(Uint32 key) const {
    bool result = _beginListeners.find(key) != _beginListeners.end();
    result = result || _finishListeners.find(key) != _finishListeners.end();
    result = result || _moveListeners.find(key) != _moveListeners.end();
    return result;
}

/**
 * Returns the touch begin listener for the given object key
 *
 * This listener is invoked when a finger is first pressed.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the touch begin listener for the given object key
 */
const Touchscreen::ContactListener Touchscreen::getBeginListener(Uint32 key) const {
    if (_beginListeners.find(key) != _beginListeners.end()) {
        return (_beginListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the touch end listener for the given object key
 *
 * This listener is invoked when a finger is finally released.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the touch end listener for the given object key
 */
const Touchscreen::ContactListener Touchscreen::getEndListener(Uint32 key) const {
    if (_finishListeners.find(key) != _finishListeners.end()) {
        return (_finishListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the touch motion listener for the given object key
 *
 * This listener is invoked when the touch is moved across the screen.
 *
 * @param key   The identifier for the listener
 *
 * @return the touch motion listener for the given object key
 */
const Touchscreen::MotionListener Touchscreen::getMotionListener(Uint32 key) const {
    if (_moveListeners.find(key) != _moveListeners.end()) {
        return (_moveListeners.at(key));
    }
    return nullptr;
}

/**
 * Adds a touch begin listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when a finger is first pressed.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Touchscreen::addBeginListener(Uint32 key, Touchscreen::ContactListener listener) {
    if (_beginListeners.find(key) == _beginListeners.end()) {
        _beginListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a touch end listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when a finger is finally released.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Touchscreen::addEndListener(Uint32 key, Touchscreen::ContactListener listener) {
    if (_finishListeners.find(key) == _finishListeners.end()) {
        _finishListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a touch motion listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when the touch is moved across the screen.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Touchscreen::addMotionListener(Uint32 key, Touchscreen::MotionListener listener) {
    if (_moveListeners.find(key) == _moveListeners.end()) {
        _moveListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Removes the touch begin listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when a finger is first pressed.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Touchscreen::removeBeginListener(Uint32 key) {
    if (_beginListeners.find(key) != _beginListeners.end()) {
        _beginListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the touch end listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when a finger is finally released.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Touchscreen::removeEndListener(Uint32 key) {
    if (_finishListeners.find(key) != _finishListeners.end()) {
        _finishListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the touch motion listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when the touch is moved across the screen.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Touchscreen::removeMotionListener(Uint32 key) {
    if (_moveListeners.find(key) != _moveListeners.end()) {
        _moveListeners.erase(key);
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
void Touchscreen::clearState() {
    _previous.clear();
    _previous.insert(_current.begin(),_current.end());
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
bool Touchscreen::updateState(const SDL_Event& event, const Timestamp& stamp) {
    switch (event.type) {
        case SDL_FINGERDOWN:
        {
            TouchEvent tevent(event.tfinger.fingerId,Vec2(event.tfinger.x,event.tfinger.y),
                              event.tfinger.pressure, stamp);
            
            tevent.position *= Application::get()->getDisplayBounds().size;
            tevent.position += Application::get()->getDisplayBounds().origin;
            _current[tevent.touch] = tevent.position;
            for(auto it = _beginListeners.begin(); it != _beginListeners.end(); ++it) {
                it->second(tevent,it->first == _focus);
            }
        }
            break;
        case SDL_FINGERUP:
        {
            TouchEvent tevent(event.tfinger.fingerId,Vec2(event.tfinger.x,event.tfinger.y),
                              event.tfinger.pressure, stamp);

            tevent.position *= Application::get()->getDisplayBounds().size;
            tevent.position += Application::get()->getDisplayBounds().origin;
            _current.erase(tevent.touch);
            for(auto it = _finishListeners.begin(); it != _finishListeners.end(); ++it) {
                it->second(tevent,it->first == _focus);
            }
        }
            break;
        case SDL_FINGERMOTION:
        {
            TouchEvent tevent(event.tfinger.fingerId,Vec2(event.tfinger.x,event.tfinger.y),
                              event.tfinger.pressure, stamp);
            Vec2 previous(event.tfinger.x-event.tfinger.dx,event.tfinger.y-event.tfinger.dy);
            
            Size screen = Application::get()->getDisplayBounds().size;
            Vec2 origin = Application::get()->getDisplayBounds().origin;
            tevent.position *= screen;
            tevent.position += origin;
            previous *= screen;
            previous += origin;

            _current[tevent.touch] = tevent.position;
            for(auto it = _moveListeners.begin(); it != _moveListeners.end(); ++it) {
                it->second(tevent,previous,it->first == _focus);
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
void Touchscreen::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_FINGERDOWN);
    eventset.push_back((Uint32)SDL_FINGERUP);
    eventset.push_back((Uint32)SDL_FINGERMOTION);
}


