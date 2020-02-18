//
//  CUInput.h
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

#ifndef __CU_INPUT_H__
#define __CU_INPUT_H__

#include <cugl/base/CUBase.h>
#include <cugl/util/CUTimestamp.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <typeindex>

#ifndef UINT32_MAX
#define UINT32_MAX  (0xffffffff)
#endif

namespace cugl {
    
class InputDevice;

#pragma mark -
/**
 * This class is a dispatcher that provides access to the active input devices.
 *
 * No input devices are active by default.  This is to cut down on the overhead
 * of processing a large number of input events.  To use an input device, you
 * must first activate it.  Once active, you can access the devices from anywhere
 * in the code via this singleton.
 *
 * Activation happens via a templated syntax.  You take the class of any input
 * device that you want, and provide it to the activate() method.  For example,
 * if you want to activate the {@link Keyboard}, you use the syntax
 *
 *      bool success = Input::activate<Keyboard>();
 *
 * You get and deactivate an input device in the same way. 
 * 
 * This is the only way to access an input device.  All input devices have
 * protected constructors and cannot be allocated by the user.
 */
class Input {
#pragma mark Values
protected:
    /** The singleton for this service */
    static Input* _singleton;
    
    /** The reference timestamp to convert SDL time to CUGL time */
    Timestamp _reference;
    /** The reference time offset to convert SDL time to CUGL time */
    Uint32 _roffset;
    
    /** The active devices registered with this service */
    std::unordered_map<std::type_index, InputDevice*> _devices;
    
    /** For each SDL_EventType, the devices that listen to that event type */
    std::unordered_map<Uint32,std::unordered_set<std::type_index>> _subscribers;

#pragma mark Constructor
    /**
     * Creates an uninitialized instance of the Input dispatcher.
     */
    Input()  {}
    
    /**
     * Destroys the Input dispatcher, releasing any remaining devices.
     */
    ~Input() { shutdown(); }

#pragma mark Application Access
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
    static bool start();

    /**
     * Stops the Input dispatcher.
     *
     * This method (which should only be called by the {@link Application}
     * class) deallocates the singleton object.  Once it is called, the
     * untemplated get() will subsequently return nullptr.
     *
     * If the dispatcher is already stopped, this method will do nothing.
     */
    static void stop();
    
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
    static Input* get();

    /**
     * Clears the input state of all active input devices
     *
     * All {@link InputDevice} objects have a method {@link InputDevice#clearState()} that 
     * flushes any cached input from the previous animation frame.  This method 
     * (which should only be called by the {@link Application} class) invokes this
     * method for all active devices.
     */
    void clear();

    /**
     * Processes an SDL_Event by all active input devices
     *
     * All {@link InputDevice} objects have a method {@link InputDevice#updateState()} 
     * that reacts to an SDL input event. This method (which should only be 
     * called by the {@link Application} class) invokes this method for all 
     * appropriate devices. It only sends the event to devices that subscribe
     * to its event type.
     *
     * @param event The input event to process
     *
     * @return false if the input indicates that the application should quit.
     */
    bool update(SDL_Event event);
    
    // All of the above methods should only be accessed by this class
    friend class Application;

#pragma mark Internal Helpers
    /**
     * Registers the given input device with the key.
     *
     * This method places input into the _devices table with the given key. It
     * also queries the device (via the {@link InputDevice#queryEvents(std::vector<Uint32>&)}
     * method) for the associated event types. It actives these event types as
     * necessary and adds this device as a subscriber for each event type. 
     *
     * If input is nullptr, then this method will return false;
     *
     * @return true if registration was successful
     */
    bool registerDevice(std::type_index key, InputDevice* input);
    
    /**
     * Registers the input device for the given the key.
     *
     * This method removes the device for the given key from the _devices table.
     * It queries the device (via the {@link InputDevice#queryEvents(std::vector<Uint32>&)}
     * method) for the associated event types. It deactives these event types as
     * necessary and removes this device as a subscriber for each event type.        
     *
     * @return the input device, ready for deletion.
     */
    InputDevice* unregisterDevice(std::type_index key);
    
    /**
     * Shutdown and deregister any active input devices.
     *
     * This method is emergency clean-up in case the user forgot to manually
     * stop any active devices.
     */
    void shutdown();


#pragma mark Service Access
public:
    /**
     * Returns the input device singleton for the given class T.
     *
     * If the input device is not active, it returns nullptr.
     *
     * @tparam T    The input device type
     *
     * @return the input device singleton for the given class T.
     */
    template <typename T>
    static T* get() {
        if (!_singleton) { return nullptr; }
        std::type_index indx = std::type_index(typeid(T));
        auto it = _singleton->_devices.find(indx);
        if (it != _singleton->_devices.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }
    
    /**
     * Activates the input device singleton for the given class T.
     *
     * This method immediately registers the device, making it available for
     * use. If T is not a valid input device, it returns false.
     *
     * @tparam T    The input device type
     *
     * @return true if the input device was successfully activated.
     */
    template <typename T>
    static inline bool activate() {
        if (!_singleton) { return false; }
        std::type_index indx = std::type_index(typeid(T));
        auto it = _singleton->_devices.find(indx);
        // Return true if we are already active
        if (it != _singleton->_devices.end()) {
            return true;
        } else {
            T* alloc = new T();
            // Fail gracefully if the user gave us a bad class.
            if (alloc->init() && _singleton->registerDevice(indx,dynamic_cast<InputDevice*>(alloc))) {
                return true;
            } else {
                delete alloc;
            }
        }
        return false;
    }
    
    /**
     * Activates the input device singleton for the given class T.
     *
     * This method immediately unregisters the device, making it no longer safe
     * for use. If T is not an active input device, it returns false.
     *
     * @tparam T    The input device type
     *
     * @return true if the input device was successfully deactivated.
     */
    template <typename T>
    static bool deactivate() {
        if (!_singleton) { return false; }
        std::type_index indx = std::type_index(typeid(T));
        auto it = _singleton->_devices.find(indx);
        // Fail if we are not active
        if (it != _singleton->_devices.end()) {
            delete static_cast<T*>(_singleton->unregisterDevice(indx));
            return true;
        }
        return false;
    }

};


#pragma mark -
/**
 * This class is the abstract base class of every input device.
 *
 * Most of the methods of this class are pure virtual and are to be implemented
 * in the specific input device.  Hence this class works like a Java interface.
 *
 * The only exception to this rule is focus. Many input devices are designed to 
 * send input to a focused object (e.g. a text field or other UI widget). We
 * ask each such object to identify itself by a unique key and use that key
 * to resolve focus.
 */
class InputDevice {
protected:
    /** The key identifying the object with focus */
    Uint32 _focus;

public:
    /** No object is allowed to use this key, which is the same as UINT32_MAX */
    static const Uint32 RESERVED_KEY = UINT32_MAX;
    
    /**
     * Returns the index of the object with current focus
     *
     * @return the index of the object with current focus
     */
    Uint32 currentFocus() const { return _focus; }
    
    /**
     * Sets the current focus to that of the given object key.
     *
     * In some input devices, this method may fail if the key is not recognized
     * as valid.  See the notes for each input device.
     *
     * @return true if the object for key successfully acquired focus.
     */
    virtual bool requestFocus(Uint32 key) { _focus = key; return true; }
    
    /**
     * Releases the current focus so that no object key is assigned.
     */
    void releaseFocus() { _focus = RESERVED_KEY; }

protected:
    /**
     * Creates and initializes a new input device.
     *
     * WARNING: Never allocate an input device directly.  Always use the
     * {@link Input#activate()} method instead.
     */
    InputDevice() : _focus(UINT32_MAX) {}

    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~InputDevice() { }

    /**
     * Initializes this device, acquiring any necessary resources
     *
     * @return true if initialization was successful
     */
    virtual bool init() { return true; }

    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() {}
    
    /**
     * Clears the state of this input device, readying it for the next frame.
     *
     * Many devices keep track of what happened "this" frame.  This method is
     * necessary to advance the frame.
     */
    virtual void clearState() = 0;
    
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
    virtual bool updateState(const SDL_Event& event, const Timestamp& stamp) = 0;
    
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
    virtual void queryEvents(std::vector<Uint32>& eventset) = 0;
    
    // Only Input can access these methods.
    friend class Input;
};
    
}

#endif /* __CU_INPUT_H__ */
