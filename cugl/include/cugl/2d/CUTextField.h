//
//  CUTextField.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a single line text field. It is useful
//  for providing input forms for your application, such as saved games or
//  player settings.  Because it is only a single line, it is a subclass of
//  label.  A multiline text input would be a TextArea, and that is not
//  currently supported.
//
//  To make use of a TextField, BOTH Keyboard and TextInput input devices
//  must be activated.  In particular, TextInput allows the support of
//  virtual keyboards on mobile devices.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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
//  Author: Walker White and Enze Zhou
//  Version: 1/8/18
//
#ifndef __CU_TEXT_FIELD_H__
#define __CU_TEXT_FIELD_H__

#include <cugl/2d/CULabel.h>
#include <cugl/input/CUTextInput.h>
#include <cugl/input/CUKeyboard.h>
#include <string>

namespace cugl {

#pragma mark -
#pragma mark TextField

/**
 * This class represents a single line text field.
 *
 * A text field is a subclass of {link Label} that permits the user to edit
 * it when focused (e.g. when it is selected to receive keyboard events).
 * A focused field displays a blinking cursor with the current edit position.
 * There is no cursor displayed when the field does not have focus.
 *
 * The button can track its own state, via the {@link activate(Uint32)} method,
 * relieving you of having to manually the keyboard.  In addition, it also
 * reponds to mouse/touch input, allowing you to reposition the cursor and
 * either focus or unfocus the text field. However, the appropriate input
 * devices must be active before you can activate the text fields, as it needs
 * to attach internal listeners.  These devices include {@link TextInput},
 * {@link Keyboard}, and either {@link Mouse} or {@link Touchscreen}.
 *
 * The text field supports two category of listeners.  The first tracks any
 * changes to the text.  The second only updates when the field loses focus,
 * such as when the user presses return.
 */
class TextField : public Label {
public:
#pragma mark TextFieldListener
    
    /**
     * @typedef Listener
     *
     * This type represents a listener for text change in the {@link TextField} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. For simplicity, TextField nodes only support a
     * single listener. If you wish for more than one listener, then your listener
     * should handle its own dispatch.
     *
     * The function type is equivalent to
     *
     *      std::function<void (const std::string& name, const std::string& current)>
     *
     * @param name          The text field name
     * @param current       Text after editing
     */
    typedef std::function<void (const std::string& name, const std::string& current)> Listener;
    
protected:
#pragma mark -
#pragma mark Values
    /** The current cursor rectangle, */
    Rect _cursor;
    /** Timer for blinking the cursor. */
    int _cursorBlink;
    /** Cursor position indexed from the end of text. 0 means the end. */
    int _cursorIndex;
    /** Actual length of text, used to accelerate cursor placement. */
    float _textLength;
    
    
    /** Whether the field is actively checking for state changes */
    bool _active;
    /** Whether the field is actively receiving keyboad events */
    bool _focused;
    /** Whether we are using the mouse (as opposed to the touch screen) */
    bool _mouse;
    /** The listener key when the text field is checking for events*/
	Uint32 _inputKey;
    
    /** The listener callback for text changes */
	Listener _typeListener;
    /** The listener callback for loss of focus */
    Listener _exitListener;

	/** Whether the Alt key is down (used for word level editing) */
	bool _altDown;
	/** Whether the Meta key is down (used for line level editing) */
	bool _metaDown;
	/** Whether the backspace key is held down (used for continuous deleting) */
	bool _backDown;
    /** A timer to safely implement continuous deletion */
    Uint32 _backCount;


public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates an uninitialized text field with no size or font.
     *
     * You must initialize this field before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
     * heap, use one of the static constructors instead.
     */
    TextField();

    /**
     * Deletes this text field, disposing all resources
     *
     * It is unsafe to call this on a text field that is still currently
     * inside of a scene graph.
     */
	~TextField() { dispose(); }

    /**
     * Disposes all of the resources used by this text field.
     *
     * A disposed text field can be safely reinitialized. Any child will
     * be released. They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a text field that is still currently
     * inside of a scene graph.
     */
    virtual void dispose() override;


#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated text field with the given size and font.
     *
     * The text is empty and may be set later with {@link setText()}
     * or by the user.
     *
     * @param size		The size of the text field.
     * @param font		The font of this text field.
     *
     * @return a newly allocated button with the given size and font.
     */
    static std::shared_ptr<TextField> alloc(const Size& size, const std::shared_ptr<Font>& font) {
        std::shared_ptr<TextField> result = std::make_shared<TextField>();
        return (result->Label::init(size,font) ? result : nullptr);
    }

    /**
     * Returns a newly allocated text field with the given text and font.
     *
     * The label will be sized to fit the rendered text exactly. That is, the
     * height will be the maximum height of the font, and the width will be the
     * sum of the advance of the rendered characters.  That means that there
     * may be some natural spacing around the characters.
     *
     * All unprintable characters will be removed from the string.  This
     * includes tabs and newlines. They will be replaced by spaces. If any
     * glyphs are missing from the font atlas, they will not be rendered.

     * @param text		The initial text of this text field.
     * @param font		The font of this text field.
     *
     * @return a newly allocated button with the given text and font.
     */
    static std::shared_ptr<TextField> alloc(const std::string& text, const std::shared_ptr<Font>& font) {
        std::shared_ptr<TextField> result = std::make_shared<TextField>();
        return (result->initWithText(text,font) ? result : nullptr);
    }

    /**
     * Returns a newly allocated text field with the given text and font.
     *
     * The label will be sized to fit the rendered text exactly. That is, the
     * height will be the maximum height of the font, and the width will be the
     * sum of the advance of the rendered characters.  That means that there
     * may be some natural spacing around the characters.
     *
     * All unprintable characters will be removed from the string.  This
     * includes tabs and newlines. They will be replaced by spaces. If any
     * glyphs are missing from the font atlas, they will not be rendered.

     * @param text		The initial text of this text field, in C string format.
     * @param font		The font of this text field.
     *
     * @return a newly allocated button with the given text and font.
     */
    static std::shared_ptr<TextField> alloc(const char* text, const std::shared_ptr<Font>& font) {
        std::shared_ptr<TextField> result = std::make_shared<TextField>();
        return (result->initWithText(text,font) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "font":         The name of a previously loaded font asset
     *      "text":         The initial label text
     *      "foreground":   A four-element integer array. Values should be 0..255
     *      "background":   A four-element integer array. Values should be 0..255
     *      "padding":      A two-element float array.
     *      "halign":       One of 'left', 'center', 'right', 'hard left',
     *                      'true center' and 'hard right'.
     *      "valign":       One of 'top', 'middle', 'bottom', 'hard top',
     *                      'true middle' and 'hard bottom'.
     *
     * The attribute 'font' is REQUIRED.  All other attributes are optional.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue> data) {
        std::shared_ptr<TextField> node = std::make_shared<TextField>();
        if (!node->initWithData(loader,data)) { node = nullptr; }
        return std::dynamic_pointer_cast<Node>(node);
    }


#pragma mark -
#pragma mark Listener
    /**
     * Returns true if this text field has a a type listener
     *
     * This listener is invoked when the text changes.
     *
     * A text field may only have one type listener at a time.
     *
     * @return true if this text field has a type listener
     */
	bool hasTypeListener() const { return _typeListener != nullptr; }

   /**
	 * Returns the type listener (if any) for this text field
	 *
	 * This listener is invoked when the text changes.
	 *
	 * A text field may only have one type listener at a time. If there is no
     * type listener, this method returns nullptr.
	 *
	 * @return the type listener (if any) for this text field
	 */
    const Listener getTypeListener() const { return _typeListener; }

    /**
     * Sets the type listener for this text field.
     *
     * This listener is invoked when the text changes.
     *
     * A text field may only have one type listener at a time.  If this text
     * field already has a type listener, this method will replace it.
     *
     * @param listener  The type listener to use
     */
    void setTypeListener(Listener listener) { _typeListener = listener; }

    /**
     * Removes the type listener for this text field.
     *
     * This listener is invoked when the text changes.
     *
     * A text field may only have one type listener at a time.  If this text
     * field does not have a type listener, this method will fail.
     *
     * @return true if the type listener was successfully removed
     */
    bool removeTypeListener();
    
    /**
     * Returns true if this text field has an exit listener
     *
     * This listener is invoked when the field loses focus.
     *
     * A text field may only have one exit listener at a time.
     *
     * @return true if this text field has an exit listener
     */
    bool hasExitListener() const { return _exitListener != nullptr; }
    
    /**
     * Returns the exit listener (if any) for this text field
     *
     * This listener is invoked when the field loses focus.
     *
     * A text field may only have one exit listener at a time. If there is no
     * exit listener, this method returns nullptr.
     *
     * @return the exit listener (if any) for this text field
     */
    const Listener getExitListener() const { return _exitListener; }
    
    /**
     * Sets the exit listener for this text field.
     *
     * This listener is invoked when the field loses focus.
     *
     * A text field may only have one exit listener at a time.  If this text
     * field already has an exit listener, this method will replace it.
     *
     * @param listener  The exit listener to use
     */
    void setExitListener(Listener listener) { _exitListener = listener; }
    
    /**
     * Removes the exit listener for this text field.
     *
     * This listener is invoked when the field loses focus.
     *
     * A text field may only have one exit listener at a time.  If this text
     * field does not have an exit listener, this method will fail.
     *
     * @return true if the exit listener was successfully removed
     */
    bool removeExitListener();

    
#pragma mark -
#pragma mark Editing
    /**
     * Sets the text for this label.
     *
     * All unprintable characters will be removed from the string.  This
     * includes tabs and newlines. They will be replaced by spaces.
     *
     * The string must be in either ASCII or UTF8 format.  No other string
     * encodings are supported.  As all ASCII strings are also UTF8, you can
     * this effectively means that the text must be UTF8.
     *
     * If the font is missing glyphs in this string, the characters in the text
     * may be different than those displayed.
     *
     * If this text field has a type listener, it will be invoked when this
     * method is called.
     *
     * @param text      The text for this label.
     * @param resize    Whether to resize the label to fit the new text.
     */
    virtual void setText(const std::string& text, bool resize=false) override;
    
    /**
     * Activates this text field to enable editing.
     *
     * This method attaches a listener to either the {@link Mouse} or
     * {@link Touchscreen} inputs to monitor when the text field is pressed
     * and/or released. The text field will favor the mouse, but will use the
     * touch screen if no mouse input is active. If neither input is active,
     * this method will fail.
     *
     * It will also attach a listener to {@link TextInput} to provide access
     * to a (possibly virtual) keyboard and collect user typing.  Finally, it
     * attaches a liatener to {@link Keyboard} to monitor special keys such
     * as Alt, Meta, and the arrow keys.
     *
     * Notice that activating a text field and requesting focus is separate.
     * The field will request focus if selected by a touch or press, but it
     * cannot be editted until it has focus.
     *
     * @param key   The listener key for the input device
     *
     * @return true if the text field was successfully activated
     */
    bool activate(Uint32 key);

   /**
    * Deactivates this text field, ignoring any future input.
    *
    * This method removes its internal listener from either the {@link Mouse}
    * or {@link Touchscreen}, and from {@link Keyboard} and {@link TextInput}.
    *
    * When deactivated, the text field will no longer change its text on its own.
    * However, the user can still change manually with the {@link setText()} method. 
    * Futhermore, the appropriate type listener will be called when the text changes. 
    * However, any attempts to manually acquire focus will fail.
    *
    * @param dispose    Whether this request is the result of a dispose action
    *
    * @return true if the text field was successfully deactivated
    */
    bool deactivate(bool dispose=false);

    /**
     * Returns true if this text field has been activated.
     *
     * @return true if this text field has been activated.
     */
    bool isActive() const { return _active; }
    
    /**
     * Requests text input focus for this text field.
     *
     * When a text field is activated, it does not immediately have focus.
     * A text field without focus cannot be editted.  By either clicking
     * on the field or calling thus function, you can acquire focus and edit
     * the field.
     *
     * This method will fail if the text field is not active.
     *
     * @return true if successfully requested focus.
     */
 	bool requestFocus();

    /**
     * Releases text input focus for this text field.
     *
     * When the focus is released, the label can no longer be editting.
     * Typically this means that the user has input the final value,
     * which is why the exit listener (if any exists) is called.
     *
     * In addition to calling this method manually, a user can release focus
     * either by pressing RETURN or clicking somewhere outside of the field.
     *
     * @return true if successfully released focus.
     */
 	bool releaseFocus();

    /**
     * Returns true if this text field has focus.
     *
     * @return true if this button has has focus.
     */
    bool hasFocus() const { return _focused; }
    
    
protected:
#pragma mark -
#pragma mark Rendering
    /**
     * Draws this text field via the given SpriteBatch.
     *
     * This method only worries about drawing the current text field.  It does not
     * attempt to render the children.
     *
     * This method provides the correct transformation matrix and tint color.
     * You do not need to worry about whether the node uses relative color.
     * This method is called by render() and these values are guaranteed to be
     * correct.  In addition, this method does not need to check for visibility,
     * as it is guaranteed to only be called when the node is visible.
     *
     * This method overrides the one from {@link Label}. It adds the drawing of
     * a blinking cursor that indicates the edit position.
     *
     * @param batch     The SpriteBatch to draw with.
     * @param transform The global transformation matrix.
     * @param tint      The tint to blend with the Node color.
     */
    virtual void draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) override;

    
#pragma mark -
#pragma mark Internal Helpers
    /**
     * Returns true if value is a valid input character.
     *
     * This method is used by {@link TextInput} to validate input.  If a character
     * is not valid, it is dropped.
     *
     * @param value The (UTF8) character to validate
     *
     * @return true if value is a valid input character.
     */
    bool validate(const std::string& value);
    
    /**
     * Updates the text with the given input data.
     *
     * This method is a callback for the {@link TextInput} device.
     *
     * @param event		The text input event to be handled.
     * @param focus		Whether the text field currently has text input focus.
     */
    void updateInput(const TextInputEvent& event, bool focus);

    /**
     * Updates the state of any special keys.
     *
     * This method is a callback for the {@link Keyboard} device.
     *
     * @param event		The key event to be handled.
     * @param focus		Whether the text field currently has keyboard focus
     * @param down      Whether the key is pressed down (as opposed to released)
     */
    void updateKey(const KeyEvent& event, bool focus, bool down);

    /**
     * Responds to a touch or press, changing the field focus.
     *
     * If the press is outside text field, focus will be released. If the
     * press is within the bounds, it will request focus (if not currently
     * in focus) and move the cursor to the position pressed.
     *
     * @param pos   The screen coordinate where the event happened.
     * @param focus	Whether the text field currently has keyboard focus
     */
    void updatePress(Vec2 pos, bool focus);

    /**
     * Updates the cursor position.
     *
     * This method is called whenever either the text changes or the cursor
     * moves.
     */
    void updateCursor();

    /**
     * Moves the cursor one word forward or backward.
     *
     * If there is any space between the cursor and the word in the correct
     * direction, it will move adjacent to the word.  Otherwise, it will
     * skip over the word.
     *
     * @param forward   Whether to move the cursor forward.
     *
     * @return the index of the new cursor position
     */
    int skipWord(bool forward);

    /**
     * Deletes one character before the current cursor.
     *
     * If alt is pressed, it will delete an entire word. If meta is pressed,
     * it will delete the entire field.
     */
    void deleteOne();
    
    /**
     * Deletes one character if counter is the current back counter.
     *
     * This method is used to implement continous deletion.  When the
     * backspace key is held for a long time, the field schedules this
     * function as a callback with the application.  When in invoked,
     * it will delete a single letter each frame.  It will execute until
     * the back counter changes, which happens when the key is released.
     *
     * @param counter   The timer to validate deletion
     *
     * @return true if deletion should continue
     */
    bool deleteMany(Uint32 counter);

};

}

#endif /* __CU_TEXT_FIELD_H__ */
