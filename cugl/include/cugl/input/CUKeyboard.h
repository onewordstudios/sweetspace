//
//  CUKeyboard.h
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
#ifndef __CU_KEYBOARD_H__
#define __CU_KEYBOARD_H__

#include <cugl/input/CUInput.h>
#include <cugl/util/CUTimestamp.h>
#include <functional>

namespace cugl {
 
#pragma mark KeyCode
/**
 * This is enum represents a key on a keyboard. 
 *
 * The keys available are subset of the full range of available keys.  Because
 * of our goal of cross-platform support, we only support keys that are found
 * on most common keyboards.
 *
 * The Input dispatcher will convert from an SDL keycode this enum.  That means
 * it is possible to receive a KeyCode that is not present in the enum.  Any
 * switch statement for this enum should have a default value.
 */
enum class KeyCode : int {
    /** The 0 key */
    NUM_0 = SDLK_0,
    /** The 1 key */
    NUM_1 = SDLK_1,
    /** The 2 key */
    NUM_2 = SDLK_2,
    /** The 3 key */
    NUM_3 = SDLK_3,
    /** The 4 key */
    NUM_4 = SDLK_4,
    /** The 5 key */
    NUM_5 = SDLK_5,
    /** The 6 key */
    NUM_6 = SDLK_6,
    /** The 7 key */
    NUM_7 = SDLK_7,
    /** The 8 key */
    NUM_8 = SDLK_8,
    /** The 9 key */
    NUM_9 = SDLK_9,
    
    /** The A key */
    A = SDLK_a,
    /** The B key */
    B = SDLK_b,
    /** The C key */
    C = SDLK_c,
    /** The D key */
    D = SDLK_d,
    /** The E key */
    E = SDLK_e,
    /** The F key */
    F = SDLK_f,
    /** The G key */
    G = SDLK_g,
    /** The H key */
    H = SDLK_h,
    /** The I key */
    I = SDLK_i,
    /** The J key */
    J = SDLK_j,
    /** The K key */
    K = SDLK_k,
    /** The L key */
    L = SDLK_l,
    /** The M key */
    M = SDLK_m,
    /** The N key */
    N = SDLK_n,
    /** The O key */
    O = SDLK_o,
    /** The P key */
    P = SDLK_p,
    /** The Q key */
    Q = SDLK_q,
    /** The R key */
    R = SDLK_r,
    /** The S key */
    S = SDLK_s,
    /** The T key */
    T = SDLK_t,
    /** The U key */
    U = SDLK_u,
    /** The V key */
    V = SDLK_v,
    /** The W key */
    W = SDLK_w,
    /** The X key */
    X = SDLK_x,
    /** The Y key */
    Y = SDLK_y,
    /** The Z key */
    Z = SDLK_z,
    
    /** The down arrow key */
    ARROW_DOWN = SDLK_DOWN,
    /** The Left arrow key */
    ARROW_LEFT = SDLK_LEFT,
    /** The right arrow key */
    ARROW_RIGHT = SDLK_RIGHT,
    /** The up arrow key */
    ARROW_UP = SDLK_UP,
    
    /** The ' (apostrophe) key */
    QUOTE = SDLK_QUOTE,
    /** The \ (backslash) key */
    BACKSLASH = SDLK_BACKSLASH,
    /** The , (comma) key */
    COMMA = SDLK_COMMA,
    /** The = (equals) key */
    EQUALS = SDLK_EQUALS,
    /** The ` (grave accent) key */
    BACKQUOTE = SDLK_BACKQUOTE,
    /** The [ (left bracket) ley */
    LEFT_BRACKET = SDLK_LEFTBRACKET,
    /** The - (minus) key */
    MINUS = SDLK_MINUS,
    /** The . (period) key */
    PERIOD = SDLK_PERIOD,
    /** The ] (right bracket) key */
    RIGHT_BRACKET = SDLK_RIGHTBRACKET,
    /** The ; (semicolon) key */
    SEMICOLON = SDLK_SEMICOLON,
    /** The / (slash) key */
    SLASH = SDLK_SLASH,
    /** The backspace key */
    BACKSPACE = SDLK_BACKSPACE,
    /** The spacebar */
    SPACE = SDLK_SPACE,
    /** The tab key */
    TAB = SDLK_TAB,
    
    /** The Delete key */
    DEL = SDLK_DELETE,
    /** The End key */
    END = SDLK_END,
    /** The Esc key */
    ESCAPE = SDLK_ESCAPE,
    /** The Home key */
    HOME = SDLK_HOME,
    /** The Help key */
    HELP = SDLK_HELP,
    /** The PageDown key */
    PAGE_DOWN = SDLK_PAGEDOWN,
    /** The PageUp key */
    PAGE_UP = SDLK_PAGEUP,
    /** The Pause/Break key */
    PAUSE = SDLK_PAUSE,
    /** The Return key */
    RETURN = SDLK_RETURN,
    /** The Enter key */
    ENTER = SDLK_RETURN2,
    
    /** The Caps Lock key */
    CAPS_LOCK = SDLK_CAPSLOCK,
    /** The left Alt/Option key */
    LEFT_ALT = SDLK_LALT,
    /** The left Ctrl key */
    LEFT_CTRL = SDLK_LCTRL,
    /** The left Shift key */
    LEFT_SHIFT = SDLK_LSHIFT,
    /** The left Windows/Apple/Meta key */
    LEFT_META = SDLK_LGUI,
    /** The right Alt/Option key */
    RIGHT_ALT = SDLK_RALT,
    /** The right Ctrl key */
    RIGHT_CTRL = SDLK_RCTRL,
    /** The right Shift key */
    RIGHT_SHIFT = SDLK_RSHIFT,
    /** The right Windows/Apple/Meta key */
    RIGHT_META = SDLK_RGUI,
    /** The Numlock/Clear key */
    NUMLOCK = SDLK_NUMLOCKCLEAR,
    
    /** Tthe 0 key (numeric keypad) */
    KEYPAD_0 = SDLK_KP_0,
    /** The 1 key (numeric keypad) */
    KEYPAD_1 = SDLK_KP_1,
    /** The 2 key (numeric keypad) */
    KEYPAD_2 = SDLK_KP_2,
    /** The 3 key (numeric keypad) */
    KEYPAD_3 = SDLK_KP_3,
    /** The 4 key (numeric keypad) */
    KEYPAD_4 = SDLK_KP_4,
    /** The 5 key (numeric keypad) */
    KEYPAD_5 = SDLK_KP_5,
    /** The 6 key (numeric keypad) */
    KEYPAD_6 = SDLK_KP_6,
    /** The 7 key (numeric keypad) */
    KEYPAD_7 = SDLK_KP_7,
    /** The 8 key (numeric keypad) */
    KEYPAD_8 = SDLK_KP_8,
    /** The 9 key (numeric keypad) */
    KEYPAD_9 = SDLK_KP_9,
    /** The Clear key (numeric keypad) */
    KEYPAD_CLEAR = SDLK_KP_CLEAR,
    /** The = [equals] key (numeric keypad) */
    KEYPAD_EQUALS = SDLK_KP_EQUALS,
    /** The / [divide] key (numeric keypad) */
    KEYPAD_DIVIDE = SDLK_KP_DIVIDE,
    /** The * [multiply] key (numeric keypad) */
    KEYPAD_MULTIPLY = SDLK_KP_MULTIPLY,
    /** The - [minus] key (numeric keypad) */
    KEYPAD_MINUS = SDLK_KP_MINUS,
    /** The + [plus] key (numeric keypad) */
    KEYPAD_PLUS = SDLK_KP_PLUS,
    /** The Enter key (numeric keypad) */
    KEYPAD_ENTER = SDLK_KP_ENTER,
    
    /** We have no idea what this key is */
    UNKNOWN = SDLK_POWER
};
    
/**
 * This function is a hash code function for keyboard key codes
 */
struct KeyCodeHasher {
	/**
	 * The hashing operator
	 *
	 * @param k	The key code to hash
	 */
    std::size_t operator()(const KeyCode& k) const {
        return (std::hash<int>()(static_cast<int>(k)));
    }
};


#pragma mark -
/**! \public
 * This enum represents the category of the key
 *
 * Categories are used to group key codes in rough groups
 */
enum class KeyCategory {
    /** This key code is a number 0 to 9 */
    NUMBER,
    /** This key code is letter in the roman alphabet */
    LETTER,
    /** This key code is an arrow key */
    ARROW,
    /** This key code is a punctuation (or space) marker */
    PUNCTUATION,
    /** This key code is a special character, including return or enter */
    SPECIAL,
    /** This key code is a modier like shift or control */
    MODIFIER,
    /** This key code is a character from the keypad */
    KEYPAD,
    /** This key code is not supported by CUGL */
    UNKNOWN
};
    
#pragma mark -
    
/**
 * This simple class is a struct to hold key event information
 */
class KeyEvent {
public:
    /** The time that the key was pressed/released */
    Timestamp timestamp;
    /** The code for the key */
    KeyCode keycode;
    
    /**
     * Constructs a new key event with the default values
     */
    KeyEvent() : keycode(KeyCode::UNKNOWN) {}
    
    /**
     * Constructs a new key event with the given values
     *
     * @param code  The code for the key pressed/released
     * @param stamp The timestamp for the event
     */
    KeyEvent(KeyCode code, const Timestamp& stamp) {
        keycode = code; timestamp = stamp;
    }
        
    /**
     * Returns the category of this event
     *
     * See {@link KeyCategory} for information on categories.
     *
     * @return the category of this event
     */
    KeyCategory keyCategory();
};
    
#pragma mark -
/**
 * This class is an input device representing the keyboard.
 *
 * This device is used when you want low-level monitoring of the keys, like
 * traditional WASD control.  It is not appropriate for mobile devices, which
 * must use virtual keyboards.  If you want to get text from the user, you 
 * should not use this device.  Use {@link TextInput} instead.
 *
 * As with most devices, we provide support for both listeners and polling
 * the keyboard.  Polling the keyboard will query the key state at the 
 * start of the frame, but it may miss those case in which a user presses
 * and releases a key in a single animation frame.
 *
 * Listeners are guaranteed to catch all presses and releases, as long as they
 * are detected by the OS.  However, listeners are not called as soon as the
 * event happens.  Instead, the events are queued and processed at the start
 * of the animation frame, before the method {@link Application#update(float)} 
 * is called. 
 */
class Keyboard : public InputDevice {
public:
#pragma mark Listener
    /**
     * @typedef Listener
     *
     * This type represents a listener for the {@link Keyboard} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * A Keyboard is designed to send input to a focused object (e.g. a text
     * field or other UI widget). While only one listener can have focus at a
     * time, all listeners will receive input from the Keyboard.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const KeyEvent& event, bool focus)>
     *
     * @param code  The code for this key
     * @param stamp The timestamp for this event
     * @param focus Whether the listener currently has focus
     */
    typedef std::function<void(const KeyEvent& event, bool focus)> Listener;
    
#pragma mark Values
protected:
    /** The keys pressed in the previous animation frame */
    std::unordered_set<KeyCode,KeyCodeHasher> _previous;
    /** The keys pressed in the current animation frame */
    std::unordered_set<KeyCode,KeyCodeHasher> _current;
    
    /** The set of listeners called whenever a key is pressed */
    std::unordered_map<Uint32, Listener> _downListeners;
    /** The set of listeners called whenever a key is released */
    std::unordered_map<Uint32, Listener> _upListeners;

#pragma mark Constructor
    /**
     * Creates and initializes a new keyboard device.
     *
     * WARNING: Never allocate a keyboard device directly.  Always use the
     * {@link Input#activate()} method instead.
     */
    Keyboard() : InputDevice() {}
    
    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~Keyboard() {}
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;

public:
#pragma mark Data Polling
    /**
     * Returns true if the key is currently held down.
     *
     * @param  code the keyboard key to test
     *
     * @return true if the key is currently held down.
     */
    bool keyDown(KeyCode code) const {
        return _current.find(code) != _current.end();
    }
    
    /**
     * Returns true if the key was pressed this animation frame.
     *
     * A key press occurs if the key is down this animation frame, but was not
     * down the previous animation frame.
     *
     * @param  code the keyboard key to test
     *
     * @return true if the key is currently held down.
     */
    bool keyPressed(KeyCode code) const {
        return _current.find(code) != _current.end() && _previous.find(code) == _previous.end();
    }
    
    /**
     * Returns true if the key was released this animation frame.
     *
     * A key release occurs if the key is up this animation frame, but was not
     * up the previous animation frame.
     *
     * @param  code the keyboard key to test
     *
     * @return true if the key is currently held down.
     */
    bool keyReleased(KeyCode code) {
        return _current.find(code) == _current.end() && _previous.find(code) != _previous.end();
    }
    
    /**
     * Returns the number of keys currently held down.
     *
     * @return the number of keys currently held down.
     */
    unsigned int keyCount() const { return (unsigned int)_current.size(); }
    
    /**
     * Returns a list of the keys currently held down.
     *
     * This list contains the codes for all of the keys currently held down. This
     * list is a copy; modifying it has not effect on the poller.
     *
     * @return a list of the keys currently held down.
     */
    const std::vector<KeyCode> keySet() const;

    /**
     * Returns the category of the given key code
     *
     * See {@link KeyCategory} for information on categories.
     *
     * @param code   The key code to check
     *
     * @return the category of the given key code
     */
    static KeyCategory keyCategory(KeyCode code);
    
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
     * An object is a listener if it is either a key down or a key up
     * listener.
     *
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    
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
    const Listener getKeyDownListener(Uint32 key) const;
    
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
    const Listener getKeyUpListener(Uint32 key) const;
    
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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addKeyDownListener(Uint32 key, Listener listener);
    
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
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addKeyUpListener(Uint32 key, Listener listener);

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
    bool removeKeyDownListener(Uint32 key);

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
    bool removeKeyUpListener(Uint32 key);
    
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

#endif /* __CU_KEYBOARD_H__ */
