//
//  CUTouchscreen.h
//  Cornell University Game Library (CUGL)
//
//  This class provides basic touch support. While it supports multitouch, it
//  only receives one touch per event.  For complex multitouch events (such as
//  gestures) you should use GestureInput instead.
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

#ifndef __CU_TOUCHSCREEN_H__
#define __CU_TOUCHSCREEN_H__

#include "CUInput.h"
#include <cugl/math/CURect.h>

namespace cugl {

/** This is a reserved value to indicate that there is no touch */
#define CU_INVALID_TOUCH    -1

#pragma mark -
    
/** This is the type representing a finger or touch */
typedef Sint64 TouchID;

/**
 * This simple class is a struct to hold touch event information.
 */
class TouchEvent {
public:
    /** The time of the touch event */
    Timestamp timestamp;
    /** The associated finger for this touch event */
    TouchID touch;
    /** The current touch position in screen coordinates */
    Vec2 position;
    /** The current touch pressure (this is a normalized value 0..1) */
    float pressure;
    
    /**
     * Constructs a new touch event with the default values
     */
    TouchEvent() : touch(-1), pressure(0) {}
    
    /**
     * Constructs a new touch event with the given values
     *
     * @param finger    The finger generating this event
     * @param point     The current touch position
     * @param force     The current touch pressure
     * @param stamp     The timestamp for the event
     */
    TouchEvent(TouchID finger, const Vec2& point, float force, const Timestamp& stamp) {
        touch = finger; position = point; pressure = force; timestamp = stamp;
    }
};



#pragma mark -
/**
 * This class is an input device representing the touch screen.
 *
 * This input device represents a screen that supports multiple simultaneous
 * touches. Why multitouch is possible, each touch is registered as a separate
 * event.  This is ideal when you wish to treat each finger as a separate mouse
 * pointer.  However, it can be tricky when you want to recognize complex
 * actions like gestures.  For gesture support, we recomment that you use the
 * class (currently unimplemented) GestureInput.
 *
 * Note that a device may support multitouch without actually having a touch
 * screen. MacBooks or other devices with gesture-enabled trackpads are an
 * example of there.  This class is not safe for those devices as it will 
 * inappropriately attempt to convert the touch to a screen position.
 *
 * As with most devices, we provide support for both listeners and polling
 * the mouse.  Polling the device will query the touch screen at the start of
 * the frame, but it may miss those case in there are multiple mouse events in
 * a single animation frame. This is a real concern for touch motion events, as
 * SDL will occasionally record more than one of these a frame.
 *
 * This device is much more suited for listeners than polling.  Because touch
 * ids are changing all the time, we purge any touch data once the finger is
 * lifted.  This means that there is no way to purge a finger's last position.
 * In addition, listeners are the only way to determine pressure.  There is
 * no polling functionality for touch pressure.
 *
 * Listeners are guaranteed to catch all presses and releases, as long as they
 * are detected by the OS.  However, listeners are not called as soon as the
 * event happens.  Instead, the events are queued and processed at the start
 * of the animation frame, before the method {@link Application#update(float)}
 * is called.
 *
 * Unlike {@link Mouse}, the motion listeners are active by default.
 */
class Touchscreen : public InputDevice {
public:
#pragma mark Listeners
    /**
     * @typedef ContactListener
     *
     * This type represents a listener for a press/release in the {@link Touchscreen} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * This type of listener only responds to button presses and releases, not touch
     * movement. Listeners are guaranteed to be called at the start of an animation
     * frame, before the method {@link Application#update(float) }.
     *
     * The listener does not receive any information indicating whether the event
     * is a press or a release. That is handled when the listener is registered.
     *
     * While touch listeners do not traditionally require focus like a keyboard does,
     * we have included that functionality. While only one listener can have focus
     * at a time, all listeners will receive input from the Touchscreen device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const TouchEvent& event, bool focus)>
     *
     * @param event     The touch event for this press/release
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const TouchEvent& event, bool focus)> ContactListener;
    
    /**
     * @typedef MotionListener
     *
     * This type represents a listener for movement in the {@link Touchscreen} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * This type of listener only responds to touch movement, not presses or
     * releases.  Listeners are guaranteed to be called at the start of an
     * animation frame, before the method {@link Application#update(float) }.
     *
     * In addition the the touch event, the listener will provide the previously
     * registered touch location.  This will allow you to determin the relative
     * touch movement.
     *
     * While touch listeners do not traditionally require focus like a keyboard does,
     * we have included that functionality. While only one listener can have focus
     * at a time, all listeners will receive input from the Touchscreen device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const TouchEvent& event, const Vec2& previous, bool focus)>
     *
     * @param event     The touch event for this movement
     * @param previous  The previous position of the touch
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const TouchEvent& event, const Vec2& previous, bool focus)> MotionListener;

protected:
    /** The touch position for the previous animation frame */
    std::unordered_map<TouchID,Vec2> _previous;
    /** The touch position for the previous animation frame */
    std::unordered_map<TouchID,Vec2> _current;
    
    /** The set of listeners called whenever a touch begins */
    std::unordered_map<Uint32, ContactListener> _beginListeners;
    /** The set of listeners called whenever a touch ends */
    std::unordered_map<Uint32, ContactListener> _finishListeners;
    /** The set of listeners called whenever a touch is moved */
    std::unordered_map<Uint32, MotionListener> _moveListeners;

#pragma mark Constructors
    /**
     * Creates and initializes a new touch screen device.
     *
     * WARNING: Never allocate a touch screen device directly.  Always use the
     * {@link Input#activate()} method instead.
     */
    Touchscreen() : InputDevice() {}
    
    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~Touchscreen() {}
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;

#pragma mark Data Polling
public:
    /**
     * Returns true if touch is a finger currenly held down on the screen
     *
     * If this value returns false, it is unsafe to call either of the methods
     * {@link touchPosition()} or {@link touchOffset()}.
     *
     * @param touch An identifier for a touch/finger
     *
     * @return true if touch is a finger currenly held down on the screen
     */
    bool touchDown(TouchID touch) const {
        return _current.find(touch) != _current.end();
    }
    
    /**
     * Returns true if touch is a finger pressed this animation frame
     *
     * @param touch An identifier for a touch/finger
     *
     * @return true if touch is a finger pressed this animation frame
     */
    bool touchPressed(TouchID touch) const {
        return (_current.find(touch) != _current.end() &&
                _previous.find(touch) == _previous.end());
    }
    
    /**
     * Returns true if touch is a finger released this animation frame
     *
     * The identifer touch will not be in the set {@link touchSet()} and
     * it will be unsafe to call either {@link touchPosition()} or 
     * {@link touchOffset()}.
     *
     * @param touch An identifier for a touch/finger
     *
     * @return true if touch is a finger released this animation frame
     */
    bool touchReleased(TouchID touch) {
        return (_current.find(touch) == _current.end() &&
                _previous.find(touch) != _previous.end());
    }
    
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
    Vec2 touchPosition(TouchID touch) const;
    
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
    Vec2 touchOffset(TouchID touch) const;
    
    /**
     * Returns the number of fingers currently held down.
     *
     * @return the number of fingers currently held down.
     */
    unsigned int touchCount() const { return (unsigned int)_current.size(); }

    /**
     * Returns the set of identifiers for the fingers currently held down.
     *
     * @return the set of identifiers for the fingers currently held down.
     */
    const std::vector<TouchID> touchSet() const;
    
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
    virtual bool requestFocus(Uint32 key) override;
    
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
    bool isListener(Uint32 key) const;
    
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
    const ContactListener getBeginListener(Uint32 key) const;
    
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
    const ContactListener getEndListener(Uint32 key) const;
    
    /**
     * Returns the touch motion listener for the given object key
     *
     * This listener is invoked when the touch is moved across the screen.
     *
     * @param key   The identifier for the listener
     *
     * @return the touch motion listener for the given object key
     */
    const MotionListener getMotionListener(Uint32 key) const;
    
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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addBeginListener(Uint32 key, ContactListener listener);
    
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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addEndListener(Uint32 key, ContactListener listener);
    
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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addMotionListener(Uint32 key, MotionListener listener);

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
    bool removeBeginListener(Uint32 key);
    
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
    bool removeEndListener(Uint32 key);
    
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
    bool removeMotionListener(Uint32 key);
    
protected:
#pragma mark Input Device
    /**
     * Clears the state of this input device, readying it for the next frame.
     *
     * Many devices keep track of what happened "this" frame.  This method is
     * necessary to advance the frame.
     */
    virtual void clearState() override;
    
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
    virtual bool updateState(const SDL_Event& event, const Timestamp& stamp) override;
    
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
    virtual void queryEvents(std::vector<Uint32>& eventset) override;
    
    // Apparently friends are not inherited
    friend class Input;
};

};

#endif /* __CU_TOUCHSCREEN_H__ */
