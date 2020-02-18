//
//  CUMouse.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides basic mouse support.  It can also be used for (single)
//  touches, though it is not recommended.
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
//  Version: 7/9/16


#include <cugl/input/CUMouse.h>

using namespace cugl;

/**
 * Deletes this input device, disposing of all resources
 */
void Mouse::dispose() {
    _pressListeners.clear();
    _releaseListeners.clear();
    _dragListeners.clear();
    _moveListeners.clear();
    _wheelListeners.clear();
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
bool Mouse::requestFocus(Uint32 key) {
    if (isListener(key)) {
        _focus = key;
        return true;
    }
    return false;
}

/**
 * Returns true if key represents a listener object
 *
 * An object is a listener if it is a listener for any of the five actions:
 * button press, button release, mouse drag, mouse motion, or wheel motion.
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool Mouse::isListener(Uint32 key) const {
    bool result = _pressListeners.find(key) != _pressListeners.end();
    result = result || _releaseListeners.find(key) != _releaseListeners.end();
    result = result || _dragListeners.find(key) != _dragListeners.end();
    result = result || _moveListeners.find(key) != _moveListeners.end();
    result = result || _wheelListeners.find(key) != _wheelListeners.end();
    return result;
}

/**
 * Returns the mouse press listener for the given object key
 *
 * This listener is invoked when a mouse button is pressed.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the mouse press listener for the given object key
 */
const Mouse::ButtonListener Mouse::getPressListener(Uint32 key) const {
    if (_pressListeners.find(key) != _pressListeners.end()) {
        return (_pressListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the mouse release listener for the given object key
 *
 * This listener is invoked when a mouse button is released.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the mouse release listener for the given object key
 */
const Mouse::ButtonListener Mouse::getReleaseListener(Uint32 key) const {
    if (_releaseListeners.find(key) != _releaseListeners.end()) {
        return (_releaseListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the mouse drag listener for the given object key
 *
 * This listener is invoked when the mouse is moved while any button is
 * held dowh.
 *
 * If there is no listener for the given key, it returns nullptr.
 * Just because there is a listener does not mean it is active. This
 * listener is only active is the pointer awareness is DRAG or ALWAYS.
 *
 * @param key   The identifier for the listener
 *
 * @return the mouse drag listener for the given object key
 */
const Mouse::MotionListener Mouse::getDragListener(Uint32 key) const {
    if (_dragListeners.find(key) != _dragListeners.end()) {
        return (_dragListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the mouse motion listener for the given object key
 *
 * This listener is invoked when the mouse is moved (with or without any
 * button held down).
 *
 * If there is no listener for the given key, it returns nullptr.
 * Just because there is a listener does not mean it is active. This
 * listener is only active is the pointer awareness is ALWAYS.
 *
 * @param key   The identifier for the listener
 *
 * @return the mouse motion listener for the given object key
 */
const Mouse::MotionListener Mouse::getMotionListener(Uint32 key) const {
    if (_moveListeners.find(key) != _moveListeners.end()) {
        return (_moveListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the mouse wheel listener for the given object key
 *
 * This listener is invoked when the mouse wheel moves
 *
 * @param key   The identifier for the listener
 *
 * @return the mouse wheel listener for the given object key
 */
const Mouse::WheelListener Mouse::getWheelListener(Uint32 key) const {
    if (_wheelListeners.find(key) != _wheelListeners.end()) {
        return (_wheelListeners.at(key));
    }
    return nullptr;
}

/**
 * Adds a mouse press listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when a mouse button is pressed.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Mouse::addPressListener(Uint32 key, Mouse::ButtonListener listener) {
    if (_pressListeners.find(key) == _pressListeners.end()) {
        _pressListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a mouse release listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when a mouse button is released.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Mouse::addReleaseListener(Uint32 key, Mouse::ButtonListener listener) {
    if (_releaseListeners.find(key) == _releaseListeners.end()) {
        _releaseListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a mouse release listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when the mouse is moved while any button is
 * held dowh. This method will fail and return false if the pointer
 * awareness is not DRAG or ALWAYS.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Mouse::addDragListener(Uint32 key, Mouse::MotionListener listener) {
    if (_awareness != PointerAwareness::BUTTON && _dragListeners.find(key) == _dragListeners.end()) {
        _dragListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a mouse motion listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when the mouse is moved (with or without any
 * button held down). This method will fail and return false if the
 * pointer awareness is not ALWAYS.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Mouse::addMotionListener(Uint32 key, Mouse::MotionListener listener) {
    if (_awareness == PointerAwareness::ALWAYS && _moveListeners.find(key) == _moveListeners.end()) {
        _moveListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a mouse wheel listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when the mouse wheel moves
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Mouse::addWheelListener(Uint32 key, Mouse::WheelListener listener) {
    if (_wheelListeners.find(key) == _wheelListeners.end()) {
        _wheelListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Removes the key down listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when a key is pressed.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Mouse::removePressListener(Uint32 key) {
    if (_pressListeners.find(key) != _pressListeners.end()) {
        _pressListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the mouse release listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when a mouse button is released.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Mouse::removeReleaseListener(Uint32 key) {
    if (_releaseListeners.find(key) != _releaseListeners.end()) {
        _releaseListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the mouse drag listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false. This method will succeed if there is a drag listener for
 * the given key, even if the pointer awareness if BUTTON.
 *
 * This listener is invoked when the mouse is moved while any button is
 * held dowh.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Mouse::removeDragListener(Uint32 key) {
    if (_dragListeners.find(key) != _dragListeners.end()) {
        _dragListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the mouse motion listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false. This method will succeed if there is a motion listener
 * for the given key, even if the pointer awareness if BUTTON or DRAG.
 *
 * This listener is invoked when the mouse is moved (with or without any
 * button held down).
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Mouse::removeMotionListener(Uint32 key) {
    if (_moveListeners.find(key) != _moveListeners.end()) {
        _moveListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the mouse wheel listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when the mouse wheel moves
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Mouse::removeWheelListener(Uint32 key) {
    if (_wheelListeners.find(key) != _wheelListeners.end()) {
        _wheelListeners.erase(key);
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
void Mouse::clearState() {
    _lastState = _currState;
    _lastPoint = _currPoint;
    _wheelOffset.setZero();
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
bool Mouse::updateState(const SDL_Event& event, const Timestamp& stamp) {
    switch (event.type) {
        case SDL_MOUSEBUTTONUP:
            if (event.button.which != SDL_TOUCH_MOUSEID) {
				MouseEvent mevent(SDL_BUTTON(event.button.button), Vec2((float)event.button.x, (float)event.button.y), stamp);
                _currPoint  = mevent.position;
                _currState -= mevent.buttons;
                for(auto it = _releaseListeners.begin(); it != _releaseListeners.end(); ++it) {
                    it->second(mevent,event.button.clicks,it->first == _focus);
                }
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.which != SDL_TOUCH_MOUSEID) {
                MouseEvent mevent(SDL_BUTTON(event.button.button),Vec2((float)event.button.x, (float)event.button.y),stamp);
                _currPoint  = mevent.position;
                _currState |= mevent.buttons;
                for(auto it = _pressListeners.begin(); it != _pressListeners.end(); ++it) {
                    it->second(mevent,event.button.clicks,it->first == _focus);
                }
            }
            break;
        case SDL_MOUSEMOTION:
            if (event.motion.which != SDL_TOUCH_MOUSEID) {
                if (_awareness == PointerAwareness::DRAG && event.motion.state > 0) {
                    MouseEvent mevent(SDL_BUTTON(event.button.button),Vec2((float)event.button.x, (float)event.button.y),stamp);
                    Vec2 previous((float)(event.motion.x-event.motion.xrel),(float)(event.motion.y-event.motion.yrel));
                    _currPoint = mevent.position;
                    for(auto it = _dragListeners.begin(); it != _dragListeners.end(); ++it) {
                        it->second(mevent,previous,it->first == _focus);
                    }
                } else if (_awareness == PointerAwareness::ALWAYS) {
                    MouseEvent mevent(SDL_BUTTON(event.button.button),Vec2((float)event.button.x, (float)event.button.y),stamp);
                    Vec2 previous((float)(event.motion.x-event.motion.xrel),(float)(event.motion.y-event.motion.yrel));
                    _currPoint = mevent.position;
                    for(auto it = _dragListeners.begin(); it != _dragListeners.end(); ++it) {
                        it->second(mevent,previous,it->first == _focus);
                    }
                    for(auto it = _moveListeners.begin(); it != _moveListeners.end(); ++it) {
                        it->second(mevent,previous,it->first == _focus);
                    }
                }
            }
            break;
        case SDL_MOUSEWHEEL:
            if (event.wheel.which != SDL_TOUCH_MOUSEID) {
                MouseWheelEvent mevent(Vec2((float)event.wheel.x, (float)event.wheel.y),stamp,event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
                _wheelOffset += (mevent.flipped ? mevent.direction : -mevent.direction);
                for(auto it = _wheelListeners.begin(); it != _wheelListeners.end(); ++it) {
                    it->second(mevent, it->first == _focus);
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
void Mouse::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_MOUSEMOTION);
    eventset.push_back((Uint32)SDL_MOUSEBUTTONUP);
    eventset.push_back((Uint32)SDL_MOUSEBUTTONDOWN);
    eventset.push_back((Uint32)SDL_MOUSEWHEEL);
}
