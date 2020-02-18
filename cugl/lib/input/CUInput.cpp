//
//  CUInput.cpp
//  Cornell University Game Library (CUGL)
//
//  This class is an event dispatcher that works as a singleton service.  That
//  is, it is singleton that allows us to access a modular collection of other
//  singletons (in this case input devices) that implement a common interface.
//  This makes sense for singletons that need flexible functionality like input
//  devices and asset managers.
//
//  We use templates to completely decouple the input devices from this class.
//  That is, this class does not need to know the type of any new input device.
//  Instead, you attach the devices by a template, which hashes the device by
//  the type id.  When the user requests a device, the type of the device is
//  hashed to retrieve the singleton.
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

#include <cugl/input/CUInput.h>
#include <cugl/input/CUTextInput.h>
#include <cugl/util/CUDebug.h>

using namespace cugl;

/** The singleton for this service */
Input* Input::_singleton = nullptr;

#pragma mark Application Methods

/**
 * Attempts to start the Input dispatcher, returning true on success.
 *
 * This method (which should only be called by the {@link Application}
 * class) allocates the singleton object.  It it returns true, then
 * the untemplated get() should no longer return nullptr.
 *
 * If the dispatcher is already started, this method will return false.
 *
 * @return true if the dispatcher successfully initialized.
 */
bool Input::start() {
    if (_singleton != nullptr) { return false; }
    
    // LOCK IT DOWN (SDL turns everything on by default)
    SDL_EventState(SDL_KEYDOWN,SDL_DISABLE);
    SDL_EventState(SDL_KEYUP,SDL_DISABLE);
    SDL_EventState(SDL_TEXTEDITING,SDL_DISABLE);
    SDL_EventState(SDL_TEXTINPUT,SDL_DISABLE);
    SDL_EventState(SDL_KEYMAPCHANGED,SDL_DISABLE);
    SDL_EventState(SDL_MOUSEMOTION,SDL_DISABLE);
    SDL_EventState(SDL_MOUSEBUTTONDOWN,SDL_DISABLE);
    SDL_EventState(SDL_MOUSEBUTTONUP,SDL_DISABLE);
    SDL_EventState(SDL_MOUSEWHEEL,SDL_DISABLE);
    SDL_EventState(SDL_JOYAXISMOTION,SDL_DISABLE);
    SDL_EventState(SDL_JOYBALLMOTION,SDL_DISABLE);
    SDL_EventState(SDL_JOYHATMOTION,SDL_DISABLE);
    SDL_EventState(SDL_JOYBUTTONDOWN,SDL_DISABLE);
    SDL_EventState(SDL_JOYBUTTONUP,SDL_DISABLE);
    SDL_EventState(SDL_JOYDEVICEADDED,SDL_DISABLE);
    SDL_EventState(SDL_JOYDEVICEREMOVED,SDL_DISABLE);
    SDL_EventState(SDL_CONTROLLERAXISMOTION,SDL_DISABLE);
    SDL_EventState(SDL_CONTROLLERBUTTONDOWN,SDL_DISABLE);
    SDL_EventState(SDL_CONTROLLERBUTTONUP,SDL_DISABLE);
    SDL_EventState(SDL_CONTROLLERDEVICEADDED,SDL_DISABLE);
    SDL_EventState(SDL_CONTROLLERDEVICEREMOVED,SDL_DISABLE);
    SDL_EventState(SDL_CONTROLLERDEVICEREMAPPED,SDL_DISABLE);
    SDL_EventState(SDL_FINGERDOWN,SDL_DISABLE);
    SDL_EventState(SDL_FINGERUP,SDL_DISABLE);
    SDL_EventState(SDL_FINGERMOTION,SDL_DISABLE);
    SDL_EventState(SDL_DOLLARGESTURE,SDL_DISABLE);
    SDL_EventState(SDL_DOLLARRECORD,SDL_DISABLE);
    SDL_EventState(SDL_MULTIGESTURE,SDL_DISABLE);
    SDL_EventState(SDL_CLIPBOARDUPDATE,SDL_DISABLE);
    SDL_EventState(SDL_DROPFILE,SDL_DISABLE);
    SDL_EventState(SDL_AUDIODEVICEADDED,SDL_DISABLE);
    SDL_EventState(SDL_AUDIODEVICEREMOVED,SDL_DISABLE);
    SDL_EventState(SDL_RENDER_TARGETS_RESET,SDL_DISABLE);
    SDL_EventState(SDL_RENDER_DEVICE_RESET,SDL_DISABLE);
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK,"1");
    _singleton = new Input();
    return true;
}

/**
 * Stops the Input dispatcher.
 *
 * This method (which should only be called by the {@link Application}
 * class) deallocates the singleton object.  Once it is called, the
 * untemplated get() will subsequently return nullptr.
 *
 * If the dispatcher is already stopped, this method will do nothing.
 */
void Input::stop() {
    if (_singleton != nullptr) {
        delete _singleton;
        _singleton = nullptr;
    }
}

/**
 * Returns the Input dispatcher singleton.
 *
 * This method (which should only be called by the {@link Application}
 * class) provides direct access to the singleton so that events may be
 * communicated.  The user should never use this method.  They should
 * use the templated get() instead.
 *
 * This method returns nullptr if start() has not yet been called.
 *
 * @return the Input dispatcher singleton.
 */
Input* Input::get() {
    return _singleton;
}

/**
 * Clears the input state of all active input devices
 *
 * All {@link InputDevices} have a method {@link clearState()} that flushes
 * any cached input from the previous animation frame.  This method (which
 * should only be called by the {@link Application} class) invokes this
 * method for all active devices.
 */
void Input::clear() {
    // Mark the start of the input phase
    _reference.mark();
    _roffset = SDL_GetTicks();
    for(auto it = _devices.begin(); it != _devices.end(); ++it) {
        it->second->clearState();
    }
}

/**
 * Processes an SDL_Event by all active input devices
 *
 * All {@link InputDevices} have a method {@link updateState(SDL_Event&)}
 * that reacts to an SDL input event. This method (which should only be
 * called by the {@link Application} class) invokes this method for all
 * appropriate devices. It only sends the event to devices that subscribe
 * to its event type.
 *
 * @param event The input event to process
 *
 * @return false if the input indicates that the application should quit.
 */
bool Input::update(SDL_Event event) {
    bool result = true;
    //Timestamp eventtime = _reference-(_roffset-event.common.timestamp);
    Timestamp eventtime = _reference;
    if (_roffset > event.common.timestamp) {
        eventtime -= _roffset-event.common.timestamp;
    } else {
        // Something is wrong with SDL timekeeping
        eventtime += event.common.timestamp-_roffset;
    }
    auto it = _subscribers.find(event.type);
    if (it != _subscribers.end()) {
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
            result = _devices[*jt]->updateState(event,eventtime) && result;
        }
    }
    
    return result;
}

#pragma mark -
#pragma mark Internal Helpers
/**
 * Registers the given input device with the key.
 *
 * This method places input into the _devices table with the given key. It
 * also queries the device (via the {@link queryEvents(std::vector<Uint32>&)}
 * method) for the associated event types. It actives these event types as
 * necessary and adds this device as a subscriber for each event type.
 *
 * If input is nullptr, then this method will return false;
 *
 * @return true if registration was successful
 */
bool Input::registerDevice(std::type_index key, InputDevice* input) {
    if (input == nullptr) { return false; }
    
    _devices[key] = input;
    std::vector<Uint32> eventset;
    input->queryEvents(eventset);
    for(auto it = eventset.begin(); it != eventset.end(); ++it) {
        auto sset = _subscribers.find(*it);
        if (sset == _subscribers.end()) {
            _subscribers.emplace(*it,std::unordered_set<std::type_index>());
            SDL_EventState(*it, SDL_ENABLE);
            sset = _subscribers.find(*it);
        }
        sset->second.emplace(key);
    }
    return true;
}

/**
 * Registers the input device for the given the key.
 *
 * This method removes the device for the given key from the _devices table.
 * It queries the device (via the {@link queryEvents(std::vector<Uint32>&)}
 * method) for the associated event types. It deactives these event types as
 * necessary and removes this device as a subscriber for each event type.
 *
 * @return the input device, ready for deletion.
 */
InputDevice* Input::unregisterDevice(std::type_index key) {
    InputDevice* input = _devices[key];
    std::vector<Uint32> eventset;
    input->queryEvents(eventset);
    for(auto it = eventset.begin(); it != eventset.end(); ++it) {
        auto sset = _subscribers.find(*it);
        sset->second.erase(key);
        if (sset->second.size() == 0) {
            _subscribers.erase(*it);
            SDL_EventState(*it, SDL_DISABLE);
        }
        _devices.erase(key);
    }
    return input;
}

/**
 * Shutdown and deregister any active input devices.
 *
 * This method is emergency clean-up in case the user forgot to manually
 * stop any active devices.
 */
void Input::shutdown() {
    for(auto it = _devices.begin(); it != _devices.end(); ++it) {
        delete it->second;
    }
    _devices.clear();
    for(auto it = _subscribers.begin(); it != _subscribers.end(); ++it) {
        SDL_EventState(it->first, SDL_DISABLE);
    }
    _subscribers.clear();
}
