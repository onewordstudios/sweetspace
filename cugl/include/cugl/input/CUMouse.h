//
//  CUMouse.h
//  Cornell University Game Library (CUGL)
//
//  This class provides basic mouse support. While SDL combines mouse and touch
//  support into the same interface, we do not.  All touches are handled by
//  the Touchscreen class.
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

#ifndef __CU_MOUSE_H__
#define __CU_MOUSE_H__

#include <cugl/input/CUInput.h>
#include <cugl/math/CURect.h>
#include <cugl/math/CUVec2.h>

/** This represents the extent of all buttons */
#define SDL_BUTTON_ALLMASK ((1 << 6) - 1)

namespace cugl {
    
#pragma mark -

/**
 * This class is a bit vector representing a mouse and a set of mouse buttons
 *
 * As this class is intended to be used as a bit set, it supports bit-wise 
 * operations for combining mouse states together.
 *
 * The exact meaning of a ButtonState depends on the context.  It could be a
 * set of buttons held down. It could be a set of buttons recently released.
 * It is simply a way to record a set of buttons.
 */
class ButtonState {
private:
    /** 
     * The set of buttons as a bit vector.
     *
     * This value is a collection of SDL button masks (e.g. SDL_BUTTON_LMASK,
     * SDL_BUTTON_RMASK, etc.).  However, this value should be accessed through
     * the button methods so that this detail is unnecessary.
     */
    Uint32 _state;

public:
#pragma mark Constructors
    /**
     * Creates an empty mouse state with no buttons
     */
    ButtonState() : _state(0) { }

    /**
     * Creates a mouse state from the given collection of masks.
     *
     * @param state     A collection of SDL button masks.
     */
    ButtonState(Uint32 state) { _state = state; }

#pragma mark Buttons
    /**
     * Returns true if this state has the left button included
     *
     * @return true if this state has the left button included
     */
    bool hasLeft() const    { return (_state & SDL_BUTTON_LMASK) != 0;  }

    /**
     * Returns true if this state has the middle button included
     *
     * @return true if this state has the middle button included
     */
    bool hasMiddle() const  { return (_state & SDL_BUTTON_MMASK) != 0;  }

    /**
     * Returns true if this state has the right button included
     *
     * @return true if this state has the right button included
     */
    bool hasRight() const   { return (_state & SDL_BUTTON_RMASK) != 0;  }

    /**
     * Returns true if this state has the first extra button included
     *
     * This button only appears on mice that have four or more
     * buttons. It is occasionally mapped to the scroll wheel buton.
     *
     * @return true if this state has the first extra button included
     */
    bool hasX1() const      { return (_state & SDL_BUTTON_X1MASK) != 0; }
    
    /**
     * Returns true if this state has the second extra button included
     *
     * This button only appears on mice that have five or more
     * buttons.
     *
     * @return true if this state has the first extra button included
     */
    bool hasX2() const      { return (_state & SDL_BUTTON_X2MASK) != 0; }
    
    /**
     * Sets whether this state has the left button included
     *
     * @param value     Whether this state has the left button included
     */
    void setLeft(bool value)    { toggle(SDL_BUTTON_LMASK,value);   }

    /**
     * Sets whether this state has the middle button included
     *
     * @param value     Whether this state has the middle button included
     */
    void setMiddle(bool value)  { toggle(SDL_BUTTON_MMASK,value);   }

    /**
     * Sets whether this state has the right button included
     *
     * @param value     Whether this state has the right button included
     */
    void setRight(bool value)   { toggle(SDL_BUTTON_RMASK,value);   }
    
    /**
     * Sets whether this state has the first extra button included
     *
     * This button only appears on mice that have four or more
     * buttons. It is occasionally mapped to the scroll wheel buton.
     *
     * @param value     Whether this state has the first extra button included
     */
    void setX1(bool value)      { toggle(SDL_BUTTON_X1MASK,value);  }

    /**
     * Sets whether this state has the second extra button included
     *
     * This button only appears on mice that have five or more
     * buttons.
     *
     * @param value     Whether this state has the second extra button included
     */
    void setX2(bool value)      { toggle(SDL_BUTTON_X2MASK,value);  }

    
#pragma mark Bitwise Operations
    /**
     * Assigns the intersection of these buttons with those in mouse.
     *
     * This method will do nothing if mouse is a different device from this 
     * one.  If the are the same, this mouse state will now contain those
     * buttons that are in both the state this object and mouse.
     *
     * @param mouse The mouse state to interset with
     *
     * @return This mouse state after modification, for chaining
     */
    ButtonState& operator&=(ButtonState mouse) {
        _state &= mouse._state;
        return *this;
    }

    /**
     * Assigns the union of these buttons with those in mouse.
     *
     * This method will do nothing if mouse is a different device from this
     * one.  If the are the same, this mouse state will now contain those
     * buttons that are in either the state of this object or in mouse.
     *
     * @param mouse The mouse state to union with
     *
     * @return This mouse state after modification, for chaining
     */
    ButtonState& operator|=(ButtonState mouse) {
        _state |= mouse._state;
        return *this;
    }

    /**
     * Assigns the symmetric difference of these buttons with those in mouse.
     *
     * This method will do nothing if mouse is a different device from this
     * one.  If the are the same, this mouse state will now contain those
     * buttons that are in either the state of this object or in mouse, but
     * not in both.
     *
     * @param mouse The mouse state to symmetric difference with
     *
     * @return This mouse state after modification, for chaining
     */
    ButtonState& operator^=(ButtonState mouse) {
        _state ^= mouse._state;
        return *this;
    }
    
    /**
     * Assigns the set difference of these buttons exluding those in mouse.
     *
     * This method will do nothing if mouse is a different device from this
     * one.  If the are the same, this mouse state will now contain those
     * buttons that are in the state of this object, but not in mouse.
     *
     * @param mouse The mouse state to exclude
     *
     * @return This mouse state after modification, for chaining
     */
    ButtonState& operator-=(ButtonState mouse) {
        _state &= ~mouse._state;
        return *this;
    }

    /**
     * Returns the intersection of these buttons with those in mouse.
     *
     * The new state will be equivalent to this one if mouse is a different
     * device than this one. If they are the same, the new state will contain 
     * those buttons that are in both the state of this object and mouse.
     *
     * @param mouse The mouse state to interset with
     *
     * @return the intersection of these buttons with those in mouse.
     */
    const ButtonState operator&(ButtonState mouse) const {
        return ButtonState(_state & mouse._state);
    }

    /**
     * Returns the union of these buttons with those in mouse.
     *
     * The new state will be equivalent to this one if mouse is a different
     * device than this one. If they are the same, the new state will contain
     * those buttons that are in either the state of this object or in mouse.
     *
     * @param mouse The mouse state to union with
     *
     * @return the union of these buttons with those in mouse.
     */
    const ButtonState operator|(ButtonState mouse) const {
        return ButtonState(_state | mouse._state);
    }

    /**
     * Returns the symmetric difference of these buttons with those in mouse.
     *
     * The new state will be equivalent to this one if mouse is a different
     * device than this one. If they are the same, the new state will contain
     * those buttons that are in either the state of this object or in mouse, 
     * but not in both.
     *
     * @param mouse The mouse state to symmetric difference with
     *
     * @return the symmetric difference of these buttons with those in mouse.
     */
    const ButtonState operator^(ButtonState mouse) const {
        return ButtonState(_state ^ mouse._state);
    }

    /**
     * Returns the set difference of these buttons exluding those in mouse.
     *
     * The new state will be equivalent to this one if mouse is a different
     * device than this one. If they are the same, the new state will contain
     * those buttons that are in this state of this object, but not in mouse.
     *
     * @param mouse The mouse state to exclude
     *
     * @return the set difference of these buttons exluding those in mouse.
     */
    const ButtonState operator-(ButtonState mouse) const {
        return ButtonState(_state & ~mouse._state);
    }

    /**
     * Returns the complement of this set of buttons.
     *
     * The new mouse state will contain those buttons that are not in the state
     * of this object.
     *
     * @return the complement of this set of buttons.
     */
    const ButtonState operator~() const {
        return ButtonState(~_state & SDL_BUTTON_ALLMASK);
    }

private:
    /**
     * Toggles the bitmask in _state according to vale
     *
     * If value is true, it adds the bitmask.  Otherwise, it removes it.
     *
     * @param bmask The button bitmask
     * @param value The toggle value
     */
    void toggle(Uint32 bmask, bool value) {
        _state = (value ? _state | bmask : _state & ~bmask);
    }
    
    // Allow MouseEvent direct access
    friend class MouseEvent;
};
    
#pragma mark -
    
/**
 * This simple class is a struct to hold mouse event information
 */
class MouseEvent {
public:
    /** The time of the mouse event */
    Timestamp timestamp;
    /** The current button set; meaning depends on the event */
    ButtonState buttons;
    /** The current mouse position in screen coordinates */
    Vec2 position;
    
    /**
     * Constructs a new mouse event with the default values
     */
    MouseEvent() {}
    
    /**
     * Constructs a new mouse event with the given values
     *
     * @param state     The state of the button set
     * @param point     The current mouse position
     * @param stamp     The timestamp for the event
     */
    MouseEvent(Uint32 state, const Vec2& point, const Timestamp& stamp) {
        buttons._state = state; timestamp = stamp; position = point;
    }
};

#pragma mark -

/**
 * This simple class is a struct to hold mouse wheel information
 *
 * The mouse wheel movement is an integer vector with positive meaning up/right 
 * and negative meaning down/left. This directions will be reversed if flipped 
 * is true.
 */
class MouseWheelEvent {
public:
    /** The time of the mouse wheel event */
    Timestamp timestamp;
    /** The movement of the mouse wheel position */
    Vec2 direction;
    /** Whether the direction of the mouse wheel is flipped */
    bool flipped;
    
    /**
     * Constructs a new mouse wheel event with the default values
     */
    MouseWheelEvent() : flipped(false) {}
    
    /**
     * Constructs a new mouse wheel event with the given values
     *
     * @param offset    The amount the mouse wheel was moved
     * @param stamp     The timestamp for the event
     * @param flip      Whether the direction of the mouse wheel is flipped
     */
    MouseWheelEvent(const Vec2& offset, const Timestamp& stamp, bool flip=false) {
        direction = offset; timestamp = stamp; flipped = flip;
    }

};

#pragma mark -
#pragma mark Mouse
/**
 * This class is an input device representing the mouse.
 *
 * This input device represents a standard mouse. Unlike the SDL api, it does
 * not support touch events. If you want access to touch events, you should 
 * use the device {@link Touchscreen} instead.
 *
 * As with most devices, we provide support for both listeners and polling
 * the mouse.  Polling the mouse will query the mouse state at the start of the 
 * frame, but it may miss those case in there are multiple mouse events in a
 * single animation frame. This is a real concern for mouse motion events, as
 * SDL will occasionally record more than one of these a frame.
 *
 * Listeners are also the preferred way to react to mouse wheel events. Mouse
 * wheel events are relative and hard to accumulate in a polling framework.
 *
 * Listeners are guaranteed to catch all presses and releases, as long as they
 * are detected by the OS.  However, listeners are not called as soon as the
 * event happens.  Instead, the events are queued and processed at the start
 * of the animation frame, before the method {@link Application#update(float)}
 * is called.
 *
 * Motion listeners are not active by default. They must be activated by the
 * method {@link setPointerAwareness(PointerAwareness)}.
 */
class Mouse : public InputDevice {
public:
    /**
     * This enum is used to represent how sensative this device is to movement.
     *
     * Movement events can be extremely prolific, especially if they do not 
     * require a button press. This enum is used limit how often these events 
     * received. By default, a mouse position is only recorded on a mouse press
     * or release.
     */
    enum class PointerAwareness {
        /** Mouse position is only recorded on a press or a release */
        BUTTON,
        /** Mouse position is only recorded while dragging */
        DRAG,
        /** Mouse position is always recorded */
        ALWAYS
    };
    
#pragma mark Listeners
    /**
     * @typedef ButtonListener
     *
     * This type represents a listener for button presses/releases in the {@link Mouse} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * This type of listener only responds to button presses and releases, not mouse
     * movement. Listeners are guaranteed to be called at the start of an animation
     * frame, before the method {@link Application#update(float) }.
     *
     * The listener does not receive any information indicating whether the event is
     * a press or a release. That is handled when the listener is registered.  On
     * the other hand, the listener will get a counter if the press/release is a
     * sequence of rapid clicks.  This is a way of detecting double or even triple
     * clicks.  The click counter will continue to increment as long as there is
     * a click every .5 seconds.
     *
     * While mouse listeners do not traditionally require focus like a keyboard does,
     * we have included that functionality. While only one listener can have focus
     * at a time, all listeners will receive input from the Mouse device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const MouseEvent& event, Uint8 clicks, bool focus)>
     *
     * @param event     The mouse event for this press/release
     * @param clicks    The number of recent clicks, including this one
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const MouseEvent& event, Uint8 clicks, bool focus)> ButtonListener;
    
    /**
     * @typedef MotionListener
     *
     * This type represents a listener for movement in the {@link Mouse} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * This type of listener only responds to mouse movement, not button presses
     * or releases.  Listeners are guaranteed to be called at the start of an
     * animation frame, before the method {@link Application#update(float) }.
     *
     * In addition the the mouse event, the listener will provide the previously
     * registered mouse location.  This will allow you to determin the relative
     * mouse movement.
     *
     * While mouse listeners do not traditionally require focus like a keyboard does,
     * we have included that functionality. While only one listener can have focus
     * at a time, all listeners will receive input from the Mouse device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const MouseEvent& event, const Vec2& previous, bool focus)>
     *
     * @param event     The mouse event for this movement
     * @param previous  The previous position of the mouse
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const MouseEvent& event, const Vec2& previous, bool focus)> MotionListener;
    
    /**
     * @typedef WheelListener
     *
     * This type represents a listener for the mouse wheel in the {@link Mouse} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * This type of listener only responds to the wheel mouse, not any other buttons
     * or mouse movement. Listeners are guaranteed to be called at the start of an
     * animation frame, before the method {@link Application#update(float) }.
     *
     * While mouse listeners do not traditionally require focus like a keyboard does,
     * we have included that functionality. While only one listener can have focus
     * at a time, all listeners will receive input from the Mouse device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const MouseWheelEvent& event, bool focus)>
     *
     * @param event     The mouse event for this wheel motion
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const MouseWheelEvent& event, bool focus)> WheelListener;
    
protected:
    /** The current awareness for pointer movement */
    PointerAwareness _awareness;

    /** The mouse buttons held down the previous animation frame */
    ButtonState _lastState;
    /** The mouse buttons held down the current animation frame */
    ButtonState _currState;
    
    /** The mouse position for the previous animation frame */
    Vec2 _lastPoint;
    /** The mouse position for the current animation frame */
    Vec2 _currPoint;

    /** The amount of wheel movement this animation frame */
    Vec2 _wheelOffset;
    
    /** The set of listeners called whenever a mouse is pressed */
    std::unordered_map<Uint32, ButtonListener> _pressListeners;
    /** The set of listeners called whenever a mouse is released */
    std::unordered_map<Uint32, ButtonListener> _releaseListeners;
    /** The set of listeners called whenever a mouse is moved */
    std::unordered_map<Uint32, MotionListener> _moveListeners;
    /** The set of listeners called whenever a mouse is dragged */
    std::unordered_map<Uint32, MotionListener> _dragListeners;
    /** The set of listeners called whenever a mouse wheel is moved */
    std::unordered_map<Uint32, WheelListener> _wheelListeners;

#pragma mark Constructor
    /**
     * Creates and initializes a new mouse device.
     *
     * The mouse device will ignore all movement events until the method
     * {@link setPointerAwareness(PointerAwareness)} is called.
     *
     * WARNING: Never allocate a mouse device directly.  Always use the
     * {@link Input#activate()} method instead.
     */
    Mouse() : InputDevice(), _awareness(PointerAwareness::BUTTON) {}
    
    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~Mouse() { }

    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;

public:
    /**
     * Returns the current pointer awareness of this device
     *
     * Movement events can be extremely prolific, especially if they do not
     * require a button press. This enum is used limit how often these events
     * received. By default, a mouse position is only recorded on a mouse press
     * or release.
     *
     * @return the current pointer awareness of this device
     */
    PointerAwareness getPointerAwareness() const { return _awareness; }
    
    /**
     * Sets the current pointer awareness of this device
     *
     * Movement events can be extremely prolific, especially if they do not
     * require a button press. This enum is used limit how often these events
     * received. By default, a mouse position is only recorded on a mouse press
     * or release.
     *
     * If this value is changed from a permission value (e.g. ALWAYS) to a 
     * more restrictive one (e.g. BUTTON), then any associated listeners will
     * be deactivated.  However, the listeners will not be deleted.
     *
     * @param awareness The pointer awareness for this device
     */
    void setPointerAwareness(PointerAwareness awareness) { _awareness = awareness; }
    

#pragma mark Data Polling
    /**
     * Returns the collection of buttons currently held down
     *
     * @return the collection of buttons currently held down
     */
    ButtonState buttonDown() const { return _currState; }
    
    /**
     * Returns the collection of buttons not currently held down
     *
     * @return the collection of buttons not currently held down
     */
    ButtonState buttonUp() const { return ~_currState; }

    /**
     * Returns the collection of buttons pressed this animation frame
     *
     * @return the collection of buttons pressed this animation frame
     */
    ButtonState buttonPressed() const { return _currState-_lastState; }

    /**
     * Returns the collection of buttons released this animation frame
     *
     * @return the collection of buttons released this animation frame
     */
    ButtonState buttonReleased() const { return _lastState-_currState; }

    /**
     * Returns the current position of the mouse this animation frame
     *
     * @return the current position of the mouse this animation frame
     */
    Vec2 pointerPosition() const { return _currPoint; }

    /**
     * Returns the directional amount the mouse moved this animation frame
     *
     * This will be (0,0) if the mouse did not move
     *
     * @return the directional amount the mouse moved this animation frame
     */
    Vec2 pointerOffset() const { return _currPoint-_lastPoint; }

    /**
     * Returns the amount the mouse wheel moved this animation frame.
     *
     * This will be (0,0) if the mouse wheel did not move
     *
     * @return the amount the mouse wheel moved this animation frame.
     */
    Vec2 wheelDirection() const { return _wheelOffset; }
    
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
     * An object is a listener if it is a listener for any of the five actions:
     * button press, button release, mouse drag, mouse motion, or wheel motion.
     *
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    
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
    const ButtonListener getPressListener(Uint32 key) const;
    
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
    const ButtonListener getReleaseListener(Uint32 key) const;

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
    const MotionListener getDragListener(Uint32 key) const;

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
    const MotionListener getMotionListener(Uint32 key) const;

    /**
     * Returns the mouse wheel listener for the given object key
     *
     * This listener is invoked when the mouse wheel moves
     *
     * @param key   The identifier for the listener
     *
     * @return the mouse wheel listener for the given object key
     */
    const WheelListener getWheelListener(Uint32 key) const;

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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addPressListener(Uint32 key, ButtonListener listener);

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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addReleaseListener(Uint32 key, ButtonListener listener);

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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addDragListener(Uint32 key, MotionListener listener);

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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addMotionListener(Uint32 key, MotionListener listener);

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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addWheelListener(Uint32 key, WheelListener listener);

    /**
     * Removes the mouse press listener for the given object key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * This listener is invoked when a mouse button is pressed.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removePressListener(Uint32 key);

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
    bool removeReleaseListener(Uint32 key);

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
    bool removeDragListener(Uint32 key);

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
    bool removeMotionListener(Uint32 key);

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
    bool removeWheelListener(Uint32 key);

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

#endif /* __CU_MOUSE_H__ */
