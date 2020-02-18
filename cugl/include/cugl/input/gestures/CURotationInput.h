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
#ifndef __CU_ROTATION_INPUT_H__
#define __CU_ROTATION_INPUT_H__
#include <cugl/input/CUInput.h>
#include <cugl/math/CUVec2.h>

namespace cugl {

#pragma mark -
#pragma mark RotationEvent

/**
 * This simple class is a struct to hold rotation event information.
 */
class RotationEvent {
public:
    /** The time of the rotation event */
    Timestamp timestamp;
    /** The normalized center of this rotation */
    Vec2 position;
    /** The number of fingers involved in this rotation */
    int fingers;
    /** The cumulative rotation angle */
    float rotation;
    /** The rotation delta since the last animation frame */
    float delta;
    
    /**
     * Constructs a new touch event with the default values
     */
    RotationEvent() : fingers(0), rotation(0.0f), delta(0.0f) {}
    
    /**
     * Constructs a new pinch event with the given values
     *
     * @param point     The normalized rotation center
     * @param down      The number of fingers in the rotation
     * @param angle     The delta angle of the rotation
     * @param stamp     The timestamp for the event
     */
    RotationEvent(const Vec2& point, int down, float angle, const Timestamp& stamp) {
        position = point; fingers = down; rotation = angle; delta = angle; timestamp = stamp;
    }
};

    
#pragma mark -
#pragma mark RotationInput

/**
 * This class is an input device recognizing rotation events.
 *
 * A rotation is a gesture where two or more fingers are rotated about a single
 * point on the device. Rotations and pinches often start with the same
 * initial gesture; the only difference is how the gesture changes over time.
 * For clarity, we have separated these two gestures.
 *
 * This input device is a touch device that supports multitouch gestures.
 * This is often the screen itself, but this is not always guaranteed.  For
 * example, the trackpad on MacBooks support pinches.  For that reason, we
 * cannot guarantee that the touches scale with the display.  Instead, all
 * gesture information is normalized, with the top left corner of the touch
 * device being (0,0) and the lower right being (1,1).
 *
 * If you know that the touch device is the screen, and would like to measure
 * the rotational anchor in screen coordinates, you should set the screen 
 * attribute to true with {@link setTouchScreen}.  In this case, the rotation
 * position will be scaled to its poisition on the screen. In those cases where 
 * the device is known to be the screen (Android, iOS devices), this value 
 * starts out as true.
 *
 * SDL treats pinches, rotations, and pans as all the same gesture.  The only
 * way to distinguish them is with the treshold factor. This tells the system
 * to ignore small gestures.
 *
 * As with most devices, we provide support for both listeners and polling
 * the mouse.  Polling the device will query the touch screen at the start of
 * the frame, but it may miss those case in there are multiple pinch changes in
 * a single animation frame.
 *
 * Listeners are guaranteed to catch all changes in the rotation angle, as long 
 * as they are detected by the OS.  However, listeners are not called as soon as
 * the event happens.  Instead, the events are queued and processed at the start
 * of the animation frame, before the method {@link Application#update(float)}
 * is called.
 */
class RotationInput : public InputDevice {
public:
#pragma mark Listener
    /**
     * @typedef Listener
     *
     * This type represents a listener for a rotation in the {@link RotationInput} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * While rotation listeners do not traditionally require focus like a keyboard
     * does, we have included that functionality. While only one listener can have
     * focus at a time, all listeners will receive input from the RotationInput device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const RotationInput& event, bool focus)>
     *
     * @param event     The touch event for this pinch/zoom
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const RotationEvent& event, bool focus)> Listener;
    
protected:
    /** Whether or not this input device is a touch screen */
    bool  _screen;
    /** Whether or not there is an active pinch being processed */
    bool  _active;
    /** The movement threshold for generating a pinch event */
    float _threshold;
    /** The pinch event data (stored whether or not there is an event) */
    RotationEvent _event;
    
    /** The set of listeners called whenever a pinch begins */
    std::unordered_map<Uint32, Listener> _beginListeners;
    /** The set of listeners called whenever a pinch ends */
    std::unordered_map<Uint32, Listener> _finishListeners;
    /** The set of listeners called whenever a pinch is moved */
    std::unordered_map<Uint32, Listener> _changeListeners;
    
    
#pragma mark Constructor
    /**
     * Creates and initializes a new rotation input device.
     *
     * WARNING: Never allocate a rotation input device directly.  Always use the
     * {@link Input#activate()} method instead.
     */
    RotationInput();
    
    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~RotationInput() {}
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;
    
#pragma mark Device Attributes
public:
    /**
     * Returns true if this device is a touch screen.
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
     * @return true if this device is a touch screen.
     */
    bool isTouchScreen() const { return _screen; }
    
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
    void setTouchScreen(bool flag);
    
    /**
     * Returns the angular threshold for rotation events.
     *
     * SDL treats pinches, rotations, and pans as all the same gesture.  The
     * only way to distinguish them is with the treshold factor. A rotation that
     * covers a smaller angle than the threshold will not be recorded. This
     * tells the system to ignore small gestures.
     *
     * @return the angular threshold for rotation events.
     */
    float getThreshold() const { return _threshold; }
    
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
    void setThreshold(float threshold);
    
    
#pragma mark Data Polling
    /**
     * Returns true if the device is in the middle of an active rotation.
     *
     * If the device is not an in active rotation, all other polling methods
     * will return the default value.
     *
     * @return true if the device is in the middle of an active rotation.
     */
    bool isActive() const { return _active; }
    
    /**
     * Returns the change in the rotation angle since the last animation frame.
     *
     * This value is positive if the rotation is clockqwise, and negative if
     * it is counter-clockwise.  All values are in radians.
     *
     * @return the change in the rotation angle since the last animation frame.
     */
    float getDelta() const { return _active ? _event.delta : 0.0f; }
    
    /**
     * Returns the cumulative angular change since the gesture began.
     *
     * This value is positive if the rotation is clockqwise, and negative if 
     * it is counter-clockwise.  All values are in radians.
     *
     * @return the cumulative pinch distance since the gesture began.
     */
    float getRotation() const { return _active ? _event.rotation : 0.0f; }
    
    /**
     * Returns the number of fingers involved in the rotation gesture.
     *
     * This value may change over the course of the rotation gesture.  However,
     * there are always guaranteed to be at least two fingers.
     *
     * @return the number of fingers involved in the rotation gesture.
     */
    int getFingers() const { return _active ? _event.fingers : 0; }
    
    /**
     * Returns the normalized center of the pinch.
     *
     * This value may change over the course of the pinch gesture.
     *
     * @return the normalized center of the pinch.
     */
    const Vec2& getPosition() const { return _active ? _event.position : Vec2::ZERO; }
    
    
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
     * rotation begin, rotation end, or rotation change.
     *
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    
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
    const Listener getBeginListener(Uint32 key) const;
    
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
    const Listener getEndListener(Uint32 key) const;
    
    /**
     * Returns the rotational change listener for the given object key
     *
     * This listener is invoked when the rotation angle changes.
     *
     * @param key   The identifier for the listener
     *
     * @return the rotational change listener for the given object key
     */
    const Listener getChangeListener(Uint32 key) const;
    
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
    bool addBeginListener(Uint32 key, Listener listener);
    
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
    bool addEndListener(Uint32 key, Listener listener);
    
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
    bool addChangeListener(Uint32 key, Listener listener);
    
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
    bool removeBeginListener(Uint32 key);
    
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
    bool removeEndListener(Uint32 key);
    
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
    bool removeChangeListener(Uint32 key);
    
    
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

}
#endif /* __CU_ROTATION_INPUT_H__ */
