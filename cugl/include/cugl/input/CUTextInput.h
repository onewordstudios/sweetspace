//
//  CUTextInput.h
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

#ifndef __CU_TEXT_INPUT_H__
#define __CU_TEXT_INPUT_H__

#include <cugl/input/CUInput.h>
#include <cugl/util/CUTimestamp.h>
#include <functional>

namespace cugl {
    
#pragma mark TextInputEvent
    
/**
 * This simple class is a struct to hold text input information
 *
 * The buffer includes the suffix appended by this event.  To get the buffer 
 * before the event, compute the appropriate substring.
 */
class TextInputEvent {
public:
    /** The time of the text input event */
    Timestamp timestamp;
    /** The current buffer for the text input */
    std::string buffer;
    /** The substring added by this specific event */
    std::string added;

    /**
     * Constructs a new text input event with the default values
     */
    TextInputEvent() {}
    
    /**
     * Constructs a new text input event with the given values
     *
     * @param text      The current text input buffer
     * @param suffix    The substring added by this specific event
     * @param stamp     The timestamp for the event
     */
    TextInputEvent(const std::string& text, const std::string& suffix, const Timestamp& stamp) {
        buffer = text; added = suffix; timestamp = stamp;
    }
};

#pragma mark -
/**
 * This class is a service that extracts UTF8 text from typing.
 *
 * You never want to use a keyboard device to gather text.  That is because
 * complex unicode characters can correspond to several keystrokes.  This device
 * abstracts this process, to make it easier to gather text for password fields,
 * text boxes, or the like.
 *
 * This class is an object-oriented abstraction build on top of the SDL Text
 * Input API.  For a tutorial of this API see
 *
 *      https://wiki.libsdl.org/Tutorials/TextInput
 *
 * While this class abstracts aways the SDL calls, the process remains the same.
 * First you start a text input sequence with the method {@link begin()}.
 * While the user types, it is stored into the buffer, which can be queried at
 * any time.  You can retrieve the buffer via polling, or via a listener that
 * is called every time the input updates.
 *
 * The buffer will continue to fill until either the method {@link end()} is
 * called, At that point, no more text is received by this device.  However, 
 * the buffer is still present and can be queried to get the final result. The
 * buffer is not erased until the method {@link begin()} is called again.
 *
 * Listeners are guaranteed to be called at the start of an animation frame,
 * before the method {@link Application#update(float)} is called.
 *
 * Unlike {@link Keyboard}, this class is fine to use with mobile devices.
 * On many devices, calling the method {@link begin()} will create a virtual
 * keyboard to input text.
 */
class TextInput : public InputDevice {
public:
#pragma mark Listeners
    /**
     * @typedef Listener
     *
     * This type represents a listener for the {@link TextInput} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * A TextInput is designed to send input to a focused object (e.g. a text
     * field or other UI widget).  While only one listener can have focus at a
     * time, all listeners will receive input from the TextInput.
     *
     * This listener is called whenever text is appends to the buffer.
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const TextInputEvent& event, bool focus)>
     *
     * @param event     The input event for this append to the buffer
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const TextInputEvent& event, bool focus)> Listener;
    
    /**
     * @typedef Validator
     *
     * This type represents a listener for validating text input. A validator
     * checks whether intermediate input should be appended to the buffer. There
     * may only be one validator at a time.
     *
     * This function type is equivalent to
     *
     *      std::function<bool(const std::string& value)>
     *
     * @param value     The character or string to append to the buffer
     *
     * @return true if the value should be appended to the buffer
     */
    typedef std::function<bool(const std::string& value)> Validator;
    
#pragma mark Values
protected:
    /** The input buffer for this device */
    std::string _buffer;

    /** Whether the input device is actively receiving text input */
    bool _active;
    /** Whether we have appended data to the buffer this animation frame */
    bool _updated;
    
    /** The validator to check that text is acceptable before appending it */
    Validator _validator;
    /** The set of listeners called whenever we append to the input buffer */
    std::unordered_map<Uint32, Listener> _listeners;

#pragma mark -
#pragma mark Constructor
    /**
     * Creates and initializes a new text input device.
     *
     * WARNING: Never allocate a text input device directly.  Always use the
     * {@link Input#activate()} method instead.
     */
    TextInput();

    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~TextInput() {}
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;

public:
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
    void begin();
    
    /**
     * Stop accepting text with this device
     *
     * Once the method is called, no more text will be added to the buffer.
     * However, the buffer itself will remain so that the text can be read.
     */
    void end();

    /**
     * Returns true if this device is actively receiving input.
     *
     * This method will return true after {@link begin()} is called, but
     * before {@link end()} is called.
     *
     * @return true if this device is actively receiving input.
     */
    bool isActive() const { return _active; }
    
#pragma mark -
#pragma mark Data Access
    /**
     * Returns the current input buffer of this text input device
     *
     * This buffer is cleared whenever {@link begin()} is called.
     *
     * @return the current input buffer of this text input device
     */
    const std::string& getBuffer() const { return _buffer; }

    /**
     * Returns true if the buffer updated this animation frame
     *
     * This value resets every animation frame.  It is useful if you are 
     * keeping track of input via polling instead of a listener.
     *
     * @return true if the buffer updated this animation frame
     */
    bool didUpdate() const { return _updated; }
    
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
    virtual bool requestFocus(Uint32 key) override;

    /**
     * Sets the current validator for this input device.
     *
     * A validator checks whether intermediate input should be appended to the 
     * buffer. There may only be one validator at a time.
     *
     * @param validator The text validator for this input device
     */
    void setValidator(Validator validator);

    /**
     * Returns the current validator for this input device.
     *
     * A validator checks whether intermediate input should be appended to the
     * buffer. There may only be one validator at a time.
     *
     * @return the current validator for this input device.
     */
    const Validator getValidator() const { return _validator; }

    /**
     * Returns true if key represents a listener object
     *
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    
    /**
     * Returns the text input listener for the given object key
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the text input listener for the given object key
     */
    const Listener getListener(Uint32 key) const;
    
    /**
     * Adds a text input listener for the given object key
     *
     * There can only be one listener for a given key.  If there is already
     * a listener for the key, the method will fail and return false.  You
     * must remove a listener before adding a new one for the same key.
     *
     * @param key   	The identifier for the listener
     * @param listener	The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addListener(Uint32 key, Listener listener);

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
    bool removeListener(Uint32 key);

protected:
    /**
     * Validates the value and appends it to the buffer if appropriate
     *
     * This method calls on the active {@link Validator} to test the value
     * before appending it.  If there is no validator, the data is appended
     * automatically.
     *
     * @param value The text to validate
     * @param stamp The timestamp for validation
     */
    void validate(const std::string& value, const Timestamp& stamp);


#pragma mark -
#pragma mark Input Device
    /**
     * Clears the state of this input device, readying it for the next frame.
     *
     * Many devices keep track of what happened "this" frame.  This method is
     * necessary to advance the frame.
     */
    virtual void clearState() override { _updated = false; }
    
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

#endif /* __CU_TEXT_INPUT_H__ */
