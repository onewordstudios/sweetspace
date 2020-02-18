//
//  CUTextInput.cpp
//  Cornell University Game Library (CUGL)
//
//  This class is an object-oriented interface to the SDL text input system.
//  We have tried to keep this class as minimal as possible to make it as
//  flexible as possible.
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
//  Version: 7/8/16

#include <cugl/input/CUTextInput.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

#pragma mark Constructors
/**
 * Creates and initializes a new text input device.
 *
 * WARNING: Never allocate a text input device directly.  Always use the
 * {@link Input#activate()} method instead.
 */
TextInput::TextInput() : InputDevice(),
_active(false),
_updated(false),
_validator(nullptr) {}

/**
 * Deletes this input device, disposing of all resources
 */
void TextInput::dispose() {
    _buffer.clear();
    _validator = nullptr;
    _listeners.clear();
}

#pragma mark -
#pragma mark Activation
/**
 * Start accepting text with this device
 *
 * Until this method is called, no input will ever be added to the buffer.
 * Once the method is called, input will continue to be added to the buffer
 * until the method {@link end()} is called.
 *
 * Calling this method will clear any text that was previously in the
 * buffer.
 */
void TextInput::begin() {
    _buffer.clear();
    _active = true;
    SDL_StartTextInput();
}

/**
 * Stop accepting text with this device
 *
 * Once the method is called, no more text will be added to the buffer.
 * However, the buffer itself will remain so that the text can be read.
 */
void TextInput::end() {
    _active = false;
    SDL_StopTextInput();
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
bool TextInput::requestFocus(Uint32 key) {
    if (isListener(key)) {
        _focus = key;
        return true;
    }
    return false;
}

/**
 * Sets the current validator for this input device.
 *
 * A validator checks whether intermediate input should be appended to the
 * buffer. There may only be one validator at a time.
 *
 * @param validator The text validator for this input device
 */
void TextInput::setValidator(TextInput::Validator validator) {
    _validator = validator;
}

/**
 * Returns true if key represents a listener object
 *
 * @param key   The identifier for the listener
 *
 * @return true if key represents a listener object
 */
bool TextInput::isListener(Uint32 key) const {
    return _listeners.find(key) != _listeners.end();
}

/**
 * Returns the text input listener for the given object key
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the text input listener for the given object key
 */
const TextInput::Listener TextInput::getListener(Uint32 key) const {
    if (isListener(key)) {
        return (_listeners.at(key));
    }
    return nullptr;
}

/**
 * Adds a text input listener for the given object key
 *
 * There can only be one listener for a given key.  If there is already
 * a listener for the key, the method will fail and return false.  You
 * must remove a listener before adding a new one for the same key.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully added
 */
bool TextInput::addListener(Uint32 key, TextInput::Listener listener) {
    if (!isListener(key)) {
        _listeners[key] = listener;
        return true;
    }
    return false;
}

/**
 * Removes the text input listener for the given object key
 *
 * If there is no active listener for the given key, this method fails and
 * returns false.
 *
 * @param key   The identifier for the listener
 *
 * @return true if the listener was succesfully removed
 */
bool TextInput::removeListener(Uint32 key) {
    if (isListener(key)) {
        _listeners.erase(key);
        return true;
    }
    return false;
}

/**
 * Validates the value and appends it to the buffer if appropriate
 *
 * This method calls on the active TextInputValidator to test the value
 * before appending it.  If there is no validator, the data is appended
 * automatically.
 *
 * @param value The text to validate
 * @param stamp The event timestamp in CUGL time
 */
void TextInput::validate(const std::string& value, const Timestamp& stamp) {
    bool append = true;
    if (_validator) {
        append = _validator(value);
    }
    if (append) {
        _buffer.append(value);
        _updated = true;
        TextInputEvent tevent(_buffer,value,stamp);
        for(auto it = _listeners.begin(); it != _listeners.end(); ++it) {
            it->second(tevent,it->first == _focus);
        }
        
    }
}

#pragma mark -
#pragma mark Input Device
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
bool TextInput::updateState(const SDL_Event& event, const Timestamp& stamp) {
    switch (event.type) {
        case SDL_TEXTEDITING:
            // Swallow the editor.
            break;
        case SDL_TEXTINPUT:
            validate(event.text.text,stamp);
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
void TextInput::queryEvents(std::vector<Uint32>& eventset) {
    eventset.push_back((Uint32)SDL_TEXTEDITING);
    eventset.push_back((Uint32)SDL_TEXTINPUT);
}
