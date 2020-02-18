//
//  CUKeyboard.cpp
//  Cornell University Game Library (CUGL)
//
//  This class provides basic keyboard support.  It is intended for low-level,
//  WASD-like control.  It is not to be used for gather text.  That is the
//  purpose of the TextInput device.
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

#include <cugl/input/CUKeyboard.h>

using namespace cugl;

/**
 * Deletes this input device, disposing of all resources
 */
void Keyboard::dispose() {
    _upListeners.clear();
    _downListeners.clear();
    _previous.clear();
    _current.clear();
}

/**
 * Returns a list of the keys currently held down.
 *
 * This list contains the codes for all of the keys currently held down. This
 * list is a copy; modifying it has not effect on the poller.
 *
 * @return a list of the keys currently held down.
 */
const std::vector<KeyCode> Keyboard::keySet() const {
    return std::vector<KeyCode>(_current.begin(),_current.end());
}

#pragma mark -
#pragma mark Categories

/**
 * Returns the category of this event
 *
 * See {@link KeyCategory} for information on categories.
 *
 * @return the category of this event
 */
KeyCategory KeyEvent::keyCategory() {
    return Keyboard::keyCategory(keycode);
}

/**
 * Returns the category of this key code
 *
 * See {@link KeyCategory} for information on categories.
 *
 * @param code   The key code to check
 *
 * @return the category of this key code
 */
KeyCategory Keyboard::keyCategory(KeyCode code) {
    switch (code) {
        case KeyCode::NUM_0:
        case KeyCode::NUM_1:
        case KeyCode::NUM_2:
        case KeyCode::NUM_3:
        case KeyCode::NUM_4:
        case KeyCode::NUM_5:
        case KeyCode::NUM_6:
        case KeyCode::NUM_7:
        case KeyCode::NUM_8:
        case KeyCode::NUM_9:
            return KeyCategory::NUMBER;
        case KeyCode::A:
        case KeyCode::B:
        case KeyCode::C:
        case KeyCode::D:
        case KeyCode::E:
        case KeyCode::F:
        case KeyCode::G:
        case KeyCode::H:
        case KeyCode::I:
        case KeyCode::J:
        case KeyCode::K:
        case KeyCode::L:
        case KeyCode::M:
        case KeyCode::N:
        case KeyCode::O:
        case KeyCode::P:
        case KeyCode::Q:
        case KeyCode::R:
        case KeyCode::S:
        case KeyCode::T:
        case KeyCode::U:
        case KeyCode::V:
        case KeyCode::W:
        case KeyCode::X:
        case KeyCode::Y:
        case KeyCode::Z:
            return KeyCategory::LETTER;
        case KeyCode::ARROW_DOWN:
        case KeyCode::ARROW_LEFT:
        case KeyCode::ARROW_RIGHT:
        case KeyCode::ARROW_UP:
            return KeyCategory::ARROW;
        case KeyCode::QUOTE:
        case KeyCode::BACKSLASH:
        case KeyCode::COMMA:
        case KeyCode::EQUALS:
        case KeyCode::BACKQUOTE:
        case KeyCode::LEFT_BRACKET:
        case KeyCode::MINUS:
        case KeyCode::PERIOD:
        case KeyCode::RIGHT_BRACKET:
        case KeyCode::SEMICOLON:
        case KeyCode::SLASH:
        case KeyCode::BACKSPACE:
        case KeyCode::SPACE:
        case KeyCode::TAB:
            return KeyCategory::PUNCTUATION;
        case KeyCode::DEL:
        case KeyCode::END:
        case KeyCode::ESCAPE:
        case KeyCode::HOME:
        case KeyCode::HELP:
        case KeyCode::PAGE_DOWN:
        case KeyCode::PAGE_UP:
        case KeyCode::PAUSE:
        case KeyCode::RETURN:
            return KeyCategory::SPECIAL;
        case KeyCode::CAPS_LOCK:
        case KeyCode::LEFT_ALT:
        case KeyCode::LEFT_CTRL:
        case KeyCode::LEFT_SHIFT:
        case KeyCode::LEFT_META:
        case KeyCode::RIGHT_ALT:
        case KeyCode::RIGHT_CTRL:
        case KeyCode::RIGHT_META:
        case KeyCode::RIGHT_SHIFT:
        case KeyCode::NUMLOCK:
            return KeyCategory::MODIFIER;
        case KeyCode::KEYPAD_0:
        case KeyCode::KEYPAD_1:
        case KeyCode::KEYPAD_2:
        case KeyCode::KEYPAD_3:
        case KeyCode::KEYPAD_4:
        case KeyCode::KEYPAD_5:
        case KeyCode::KEYPAD_6:
        case KeyCode::KEYPAD_7:
        case KeyCode::KEYPAD_8:
        case KeyCode::KEYPAD_9:
        case KeyCode::KEYPAD_CLEAR:
        case KeyCode::KEYPAD_EQUALS:
        case KeyCode::KEYPAD_DIVIDE:
        case KeyCode::KEYPAD_MULTIPLY:
        case KeyCode::KEYPAD_MINUS:
        case KeyCode::KEYPAD_PLUS:
        case KeyCode::KEYPAD_ENTER:
            return KeyCategory::KEYPAD;
        default:
            break;
    }
    return KeyCategory::UNKNOWN;
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
bool Keyboard::requestFocus(Uint32 key) {
    if (isListener(key)) {
        _focus = key;
        return true;
    }
    return false;
}

/**
 * Returns true if key represents a listener object
 *
 * An object is a listener if it is either a key down or a key up
 * listener.
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool Keyboard::isListener(Uint32 key) const {
    bool result = _upListeners.find(key) != _upListeners.end();
    return result || _downListeners.find(key) != _downListeners.end();
}

/**
 * Returns the key down listener for the given object key
 *
 * This listener is invoked when a key is pressed.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the key down listener for the given object key
 */
const Keyboard::Listener Keyboard::getKeyDownListener(Uint32 key) const {
    if (_downListeners.find(key) != _downListeners.end()) {
        return (_downListeners.at(key));
    }
    return nullptr;
}

/**
 * Returns the key up listener for the given object key
 *
 * This listener is invoked when a key is released.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the key up listener for the given object key
 */
const Keyboard::Listener Keyboard::getKeyUpListener(Uint32 key) const {
    if (_upListeners.find(key) != _upListeners.end()) {
        return (_upListeners.at(key));
    }
    return nullptr;
}

/**
 * Adds a key down listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when a key is pressed.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Keyboard::addKeyDownListener(Uint32 key, Keyboard::Listener listener) {
    if (_downListeners.find(key) == _downListeners.end()) {
        _downListeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Adds a key up listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * This listener is invoked when a key is released.
 *
 * @param key       The identifier for the listener
 * @param listern   The listener to add
 *
 * @return true if the listener was succesfully added
 */
bool Keyboard::addKeyUpListener(Uint32 key, Keyboard::Listener listener) {
    if (_upListeners.find(key) == _upListeners.end()) {
        _upListeners[key] = listener;
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
bool Keyboard::removeKeyDownListener(Uint32 key) {
    if (_downListeners.find(key) != _downListeners.end()) {
        _downListeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Removes the key up listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * This listener is invoked when a key is released.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool Keyboard::removeKeyUpListener(Uint32 key) {
    if (_upListeners.find(key) != _upListeners.end()) {
        _upListeners.erase(key);
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
void Keyboard::clearState() {
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
bool Keyboard::updateState(const SDL_Event& event, const Timestamp& stamp) {
    KeyCode code = static_cast<KeyCode>(event.key.keysym.sym);
    switch (event.type) {
        case SDL_KEYDOWN:
        {
            // Apparently SDL is forgetful
            if (_current.find(code) == _current.end()) {
                KeyEvent event(code,stamp);
                _current.emplace(code);
                for(auto it = _downListeners.begin(); it != _downListeners.end(); ++it) {
                    it->second(event,it->first == _focus);
                }
            }
        }
            break;
        case SDL_KEYUP:
        {
            // Apparently SDL is forgetful
            if (_current.find(code) != _current.end()) {
                KeyEvent event(code,stamp);
                _current.erase(code);
                for(auto it = _upListeners.begin(); it != _upListeners.end(); ++it) {
                    it->second(event,it->first == _focus);
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
void Keyboard::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_KEYDOWN);
    eventset.push_back((Uint32)SDL_KEYUP);
}
