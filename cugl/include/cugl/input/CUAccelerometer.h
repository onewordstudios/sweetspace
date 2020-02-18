//
//  CUAcclerometer.h
//  Cornell University Game Library (CUGL)
//
//  This class provides basic acclerometer support. It uses the joystick
//  subsystem, which is guaranteed to work on both iOS and Android.
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
//
#ifndef __CU_ACCELEROMETER_H__
#define __CU_ACCELEROMETER_H__

#include <cugl/input/CUInput.h>
#include <cugl/math/CUVec3.h>

namespace cugl {

/**
 * This simple class is a struct to hold acceleration event information
 */
class AccelerationEvent {
public:
    /** The time of the acceleration event */
    Timestamp timestamp;
    /** The 3-axes of acceleration during this event */
    Vec3 axis;
    /** The acceleration delta from the last recorded value */
    Vec3 delta;
    
    /**
     * Constructs a new acceleration event with the default values
     */
    AccelerationEvent() { }
    
    /**
     * Constructs a new touch event with the given values
     *
     * @param roll      The 3-axes of acceleration
     * @param diff      The acceleration delta from the last recorded value
     * @param stamp     The timestamp for the event
     */
    AccelerationEvent(const Vec3& roll, const Vec3& diff, const Timestamp& stamp) {
        axis = roll; delta = diff; timestamp = stamp;
    }
    
};
    
#pragma mark -

/**
 * This class is an input device representing the accelerometer
 *
 * This input device a three-axis accelerometer. It measures the affects of
 * gravity on each of the three axis, allowing you to measure the rotational
 * orientation of the device.
 *
 * As with most devices, we provide support for both listeners and polling
 * the mouse.  Polling the accelerometer will query the rotational axes at 
 * the start of the frame. The advantage of listeners is that they are a 
 * lot less frequent.  If the acceleration does not change significantly 
 * from frame to frame, no event will be generated.  See the method
 * {@link Accelerometer#getThreshold()} for more information.
 *
 * Listeners are guaranteed to catch acceleration changes, as long as they
 * are detected by the OS.  However, listeners are not called as soon as the
 * event happens.  Instead, the events are queued and processed at the start
 * of the animation frame, before the method {@link Application#update(float)}
 * is called.
 */
class Accelerometer : public InputDevice {
public:
#pragma mark Listener
    /**
     * @typedef Listener
     *
     * This type represents a listener for the {@link Accelerometer} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * An event is delivered whenever the accleration delta (the difference between
     * the current and previous value) exceeds the device threshold.  See the
     * method {@link Accelerometer#getThreshold()} for more information.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While accleration listeners do not traditionally require focus like a
     * keyboard does, we have included that functionality. While only one listener
     * can have focus at a time, all listeners will receive input from the
     * Acclerometer device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const std::string& value, Timestamp stamp, bool focus)>
     *
     * @param event     The acceleration event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const AccelerationEvent& event, bool focus)> Listener;

private:
    /** The SDL joystick for the accelerometer */
    SDL_Joystick* _input;
    /** The threshold for reporting acceleratomer events */
    float _threshold;
    
    /** A filter to handle the fact that we get three events for the same axis */
    bool _update;
    /** The 3-axis acceleration value for the current animation frame */
    Vec3 _current;
    /** The 3-axis acceleration value for the previous animation frame */
    Vec3 _previous;
    /** The 3-axis acceleration value for the previously generated event */
    Vec3 _anchor;
    
    /** The SDL index of the x-axis */
    int _xaxis;
    /** The SDL index of the y-axis */
    int _yaxis;
    
    /** The set of listeners called whenever we cross the threshold */
    std::unordered_map<Uint32, Listener> _listeners;

#pragma mark Constructor
    /**
     * Creates and initializes a new accelerometer device.
     *
     * WARNING: Never allocate a accelerometer device directly.  Always use the
     * {@link Input#activate()} method instead.
     */
    Accelerometer();
    
    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~Accelerometer() { dispose(); }
    
    /**
     * Initializes this device, acquiring any necessary resources
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;

public:
#pragma mark Data Polling
    /**
     * Returns the event reporting threshold of this accelerometer.
     *
     * We only report accerelation events when the device orientation changes
     * significantly.  By significantly, we mean that the difference between
     * the current acceleration and the last generated event (measured as the
     * square of the Euclidean distance) is above this threshold.  
     *
     * By default, this value is 0.1f, which is good enough for most applications.
     * If you want reporting every animation frame, set this value to 0.
     *
     * @return the event reporting threshold of this accelerometer.
     */
    float getThreshold() const { return _threshold; }
    
    /**
     * Sets the event reporting threshold of this accelerometer.
     *
     * We only report accerelation events when the device orientation changes
     * significantly.  By significantly, we mean that the difference between
     * the current acceleration and the last generated event (measured as the
     * square of the Euclidean distance) is above this threshold.
     *
     * By default, this value is 0.1f, which is good enough for most applications.
     * If you want reporting every animation frame, set this value to 0.
     *
     * @param value The event reporting threshold of this accelerometer.
     */
    void setThreshold(float value) const;
    
    /**
     * Returns the x-axis acceleration.
     *
     * This value will be updated every animation frame, regardless of the
     * value of threshold.
     *
     * @return the x-axis acceleration.
     */
    float getAccelerationX() const { return _current.x; }

    /**
     * Returns the y-axis acceleration.
     *
     * This value will be updated every animation frame, regardless of the
     * value of threshold.
     *
     * @return the y-axis acceleration.
     */
    float getAccelerationY() const { return _current.y; }

    /**
     * Returns the z-axis acceleration.
     *
     * This value will be updated every animation frame, regardless of the
     * value of threshold.
     *
     * @return the z-axis acceleration.
     */
    float getAccelerationZ() const { return _current.z; }
    
    /**
     * Returns all three axes of acceleration.
     *
     * This value will be updated every animation frame, regardless of the
     * value of threshold.
     *
     * @return all three axes of acceleration.
     */
    const Vec3& getAcceleration() const { return _current; }

    /**
     * Returns the x-axis change in rotation.
     *
     * This change is measured against the previous animation frame, not the
     * previously recorded value.  The polling methods are always updated 
     * and ignore the threshold.
     *
     * @return the x-axis change in rotation.
     */
    float getDeltaX() const { return _current.x-_previous.x; }
    
    /**
     * Returns the y-axis change in rotation.
     *
     * This change is measured against the previous animation frame, not the
     * previously recorded value.  The polling methods are always updated
     * and ignore the threshold.
     *
     * @return the y-axis change in rotation.
     */
    float getDeltaY() const { return _current.y-_previous.y; }
    
    /**
     * Returns the z-axis change in rotation.
     *
     * This change is measured against the previous animation frame, not the
     * previously recorded value.  The polling methods are always updated
     * and ignore the threshold.
     *
     * @return the z-axis change in rotation.
     */
    float getDeltaZ() const { return _current.z-_previous.z; }
    
    /**
     * Returns all three axes of the change in rotation.
     *
     * This change is measured against the previous animation frame, not the
     * previously recorded value.  The polling methods are always updated
     * and ignore the threshold.
     *
     * @return all three axes of the change in rotation.
     */
    const Vec3 getDelta() const { return _current-_previous; }

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
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    
    /**
     * Returns the acceleration listener for the given object key
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the accleration listener for the given object key
     */
    const Listener getListener(Uint32 key) const;
    
    /**
     * Adds an acceleration listener for the given object key
     *
     * There can only be one listener for a given key.  If there is already
     * a listener for the key, the method will fail and return false.  You
     * must remove a listener before adding a new one for the same key.
     *
     * @param key       The identifier for the listener
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addListener(Uint32 key, Listener listener);
    
    /**
     * Removes the acceleration listener for the given object key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removeListener(Uint32 key);
    
    
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

#endif /* __CU_ACCELEROMETER_H__ */
