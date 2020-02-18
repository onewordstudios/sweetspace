//
//  CUSlider.h
//  Cornell University Game Library (CUGL)
//
//	This module provides support for a slider, which allows the user to drag
//  a knob to select a value.  The slider can be spartan (a circle on a line),
//  or it can have custom images.
//
//  The slider can track its own state, relieving you of having to manually
//  check mouse presses. However, it can only do this when the slider is part
//  of a scene graph, as the scene graph maps mouse coordinates to screen
//  coordinates.
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
#ifndef __CU_SLIDER_H__
#define __CU_SLIDER_H__

#include <cugl/2d/CUNode.h>
#include <cugl/2d/CUButton.h>

namespace cugl {

/** The default minimum value for a slider */
#define DEFAULT_MIN       0
/** The default maximum value for a slider */
#define DEFAULT_MAX     100
/** The default knob radius */
#define DEFAULT_RADIUS   20

#pragma mark -
#pragma mark Slider

/**
 * This class represents a slider, allowing the user to drag a knob to select a value.
 *
 * A slider is defined by a knob and path.  Each of these are a distinct scene
 * graph node.  If these are not specified, the class constructs a simple slider
 * with a circle on a line.
 *
 * The most important attribute for a slider is the bounds attribute.  This
 * rectangle defines the slideable region inside of the path node.  This
 * allows us to have complex path nodes with tick marks and other features that
 * prevent ths slider from being centered in the node.  It also allows us to
 * define sliders with any orientation.  The bottom left corner of the bounds
 * rectangle is the minimum value while the top right is the maximum.
 *
 * The user can also specify a tick interval that allows the slider to snap
 * to predefined values.  This is useful in preventing the slider values from
 * being resolution dependent.  However, the class will not automatically
 * disolay tick marks.  If you wish to display tick marks, you must add them
 * to the path node.
 *
 * The slider can track its own state, via the {@link activate(Uint32)} method,
 * relieving you of having to manually check presses and drags. However,
 * the appropriate input device must be active before you can activate the
 * slider, as it needs to attach internal listeners.
 */
class Slider : public Node {
public:
#pragma mark Listener
    
    /**
     * @typedef Listener
     *
     * This type represents a listener for a value change in {@link Slider}.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. For simplicity, Slider nodes only support a
     * single listener. If you wish for more than one listener, then your listener
     * should handle its own dispatch.
     *
     * The function type is equivalent to
     *
     *      std::function<void (const std::string& name, float value)>
     *
     * @param name          The alider name
     * @param value            Changed value of slider
     */
    typedef std::function<void(const std::string& name, float value)> Listener;
    
protected:
#pragma mark -
#pragma mark Values
    /** The current value of the slider */
    float _value;
	/** The value range of the slider (x is min and y is max) */
    Vec2 _range;

    /** The knob widget for this slider */
    std::shared_ptr<Button> _knob;
	/** The background widget for this slider */
	std::shared_ptr<Node> _path;
    /** The slider path, defined relative to the background widget  */
    Rect _bounds;
    /** The adjusted slider path, if padding is necessary  */
    Rect _adjust;
    
    /** The (optional) )tick period for this slider */
    float _tick;
    /** Whether to snap the slider to a tick value */
    bool  _snap;
    
	/** Whether the slider is actively checking input */
	bool _active;
    /** Whether we are using the mouse (as opposed to the touch screen) */
	bool _mouse;
    /** The anchoring touch or mouse position in a drag */
    Vec2 _dragpos;
    /** The listener key when the text field is checking for events*/
	Uint32 _inputkey;
    /** Listener for this slider, which will be called when value is changed*/
    Listener _listener;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates an uninitialized slider. You must initialize it before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
     * heap, use one of the static constructors instead.
     */
    Slider();

    /**
     * Deletes this slider, disposing all resources
     *
     * It is unsafe to call this on a slider that is still currently
     * inside of a scene graph.
     */
	~Slider() { dispose(); }

    /**
     * Disposes all of the resources used.
     *
     * A disposed slider can be safely reinitialized. Any child will
     * be released. They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a slider that is still currently
     * inside of a scene graph.
     */
	virtual void dispose() override;

    /**
     * Initializes a slider with the default values.
     *
     * This initializer will create a horizontal slider from 0 to 100.  It will
     * assign one pixel to each value.  The knob radius will be 20.
     *
     * @return true if the button is initialized properly, false otherwise.
     */
    virtual bool init() override {
        return init(Vec2(DEFAULT_MIN,DEFAULT_MAX),Rect(DEFAULT_MIN,DEFAULT_RADIUS,DEFAULT_MAX,0));
    }

    /**
     * Initializes a slider with given bounds.
     *
     * The slider visuals will be interpretted from bounds.  The knob will be
     * a circle whose radius is the maximum of x and y, where (x,y) is the
     * bounds origin.  The path will be a simple line, but it will be surrounded
     * by a transparent "track" which tightly fits the knob.
     *
     * The range is the slider value range.  The x value is the minimum value
     * (corresponding to the bottom left corner of bounds) and the y value is
     * the maximum value (corresponding to the top right corner of bounds).
     * The slider will start at the middle value.
     *
     * @param range	    The slider value range
     * @param bounds    The slider path
     *
     * @return true if the slider is initialized properly, false otherwise.
     */
    bool init(const Vec2& range, const Rect& bounds);

    /**
     * Initializes a slider with given scene graph nodes.
     *
     * The slider visuals will be taken from the scene graph nodes knob and path.
     * The rectangle bounds should define an interior region of path.  The
     * knob graph node can be slid from the origing of bounds to the top right
     * corner.
     *
     * The range is the slider value range.  The x value is the minimum value
     * (corresponding to the bottom left corner of bounds) and the y value is
     * the maximum value (corresponding to the top right corner of bounds).
     * The slider will start at the middle value.
     *
     * @param range     The slider value range
     * @param bounds    The slider path
     * @param path      The scene graph node for the path
     * @param knob      The scene graph node for the knob
     *
     * @return true if the slider is initialized properly, false otherwise.
     */
    bool initWithUI(const Vec2& range, const Rect& bounds,
                    const std::shared_ptr<Node>& path,
                    const std::shared_ptr<Button>& knob);

    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "bounds":   A 4-element array of numbers (x,y,width,height)
     *      "range":    A 2-element array of numbers (min,max)
     *      "value':    A number representing the initial value
     *      "tick':     A number greater than 0, representing the tick period
     *      "snap":     A boolean indicating whether to snap to a nearest tick
     *      "knob":     A JSON object defining a scene graph node
     *      "path":     A JSON object defining a scene graph node OR
     *
     * The attribute 'bounds' is REQUIRED.  All other attributes are optional.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) override;

    
#pragma mark -
#pragma mark Static Constructors

    /**
     * Returns a newly allocated slider with the default values.
     *
     * This initializer will create a horizontal slider from 0 to 100.  It will
     * assign one pixel to each value.  The knob radius will be 20.
     *
     * @return a newly allocated  slider with the default values.
     */
    static std::shared_ptr<Slider> alloc() {
        std::shared_ptr<Slider> node = std::make_shared<Slider>();
        return (node->init() ? node : nullptr);
    }

    /**
     * Returns a newly allocated slider with given bounds.
     *
     * The slider visuals will be interpretted from bounds.  The knob will be
     * a circle whose radius is the maximum of x and y, where (x,y) is the
     * bounds origin.  The path will be a simple line, but it will be surrounded
     * by a transparent "track" which tightly fits the knob.
     *
     * The range is the slider value range.  The x value is the minimum value
     * (corresponding to the bottom left corner of bounds) and the y value is
     * the maximum value (corresponding to the top right corner of bounds).
     * The slider will start at the middle value.
     *
     * @param range        The slider value range
     * @param bounds    The slider path
     *
     * @return a newly allocated slider with given bounds.
     */
    static std::shared_ptr<Slider> alloc(const Vec2& range, const Rect& bounds) {
        std::shared_ptr<Slider> node = std::make_shared<Slider>();
        return (node->init(range,bounds) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated slider with given scene graph nodes.
     *
     * The slider visuals will be taken from the scene graph nodes knob and path.
     * The rectangle bounds should define an interior region of path.  The
     * knob graph node can be slid from the origing of bounds to the top right
     * corner.
     *
     * The range is the slider value range.  The x value is the minimum value
     * (corresponding to the bottom left corner of bounds) and the y value is
     * the maximum value (corresponding to the top right corner of bounds).
     * The slider will start at the middle value.
     *
     * @param range     The slider value range
     * @param bounds    The slider path
     * @param path      The scene graph node for the path
     * @param knob      The scene graph node for the knob
     *
     * @return a newly allocated slider with given scene graph nodes.
     */
    static std::shared_ptr<Slider> allocWithUI(const Vec2& range, const Rect& bounds,
                                               const std::shared_ptr<Node>& path,
                                               const std::shared_ptr<Button>& knob) {
        std::shared_ptr<Slider> node = std::make_shared<Slider>();
        return (node->initWithUI(range,bounds,path,knob) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "bounds":   A 4-element array of numbers (x,y,width,height)
     *      "range":    A 2-element array of numbers (min,max)
     *      "value':    A number representing the initial value
     *      "tick':     A number greater than 0, representing the tick period
     *      "snap":     A boolean indicating whether to snap to a nearest tick
     *      "knob":     A JSON object defining a scene graph node
     *      "path":     A JSON object defining a scene graph node OR
     *
     * The attribute 'bounds' is REQUIRED.  All other attributes are optional.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Slider> allocWithData(const SceneLoader* loader,
                                                 const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<Slider> node = std::make_shared<Slider>();
        return (node->initWithData(loader,data) ? node : nullptr);
    }


#pragma mark -
#pragma mark Slider State
    /**
     * Returns the minimum possible slider value.
     *
     * This is the value when the slider is at the bottom left corner of
     * the bounds.
     *
     * @return the minimum possible slider value.
     */
    float getMinValue() const { return _range.x; }

    /**
     * Sets the minimum possible slider value.
     *
     * This is the value when the slider is at the bottom left corner of
     * the bounds.
     *
     * @param value The minimum possible slider value.
     */
    void setMinValue(float value) { _range.x = value; reposition(); }

    /**
     * Returns the maximum possible slider value.
     *
     * This is the value when the slider is at the top right corner of the
     * bounds.
     *
     * @return the maximum possible slider value.
     */
    float getMaxValue() const { return _range.y; }

    /**
     * Sets the maximum possible slider value.
     *
     * This is the value when the slider is at the top right corner of the
     * bounds.
     *
     * @param value The maximum possible slider value.
     */
    void setMaxValue(float value) { _range.y = value; reposition(); }

    /**
     * Returns the range of possible slider values.
     *
     * The x coordinate is the minimum value while the y coordinate is the
     * maximum value.
     *
     * @return the range of possible slider values.
     */
    const Vec2& getRange() const { return _range; }

    /**
     * Sets the range of possible slider values.
     *
     * The x coordinate is the minimum value while the y coordinate is the
     * maximum value.
     *
     * @param range The range of possible slider values.
     */
    void setRange(const Vec2& range) { _range = range; reposition(); }

    /**
     * Sets the range of possible slider values.
     *
     * The minimum is the value when the slider is at the bottom left corner
     * of the bounds and the maximum is when the slider is in the top right
     * corner.
     *
     * @param min   The minimum possible slider value.
     * @param max   The maximum possible slider value.
     */
    void setRange(float min, float max) { setRange(Vec2(min,max)); }
    
    /**
     * Returns the current slider value.
     *
     * @return the current slider value.
     */
    float getValue() const { return _value; }

    /**
     * Sets the current slider value.
     *
     * If the slider is set to snap to ticks, this will assign the value to
     * the nearest tick.  In addition, if the value is out of range, it will
     * snap to the nearest value in range.
     *
     * @param value The slider value
     */
    void setValue(float value) { _value = validate(value); reposition(); }

    
#pragma mark -
#pragma mark Appearance
    /**
     * Returns the scene graph node for the knob.
     *
     * It is safe to make changes to the knob so long as you do not resize it
     * it or scale it.  Doing so will mess up the slider visuals.
     *
     * @return the scene graph node for the knob.
     */
    const std::shared_ptr<Button>& getKnob() const { return _knob; }

    /**
     * Sets the scene graph node for the knob.
     *
     * If this value is nullptr, the method will construct a default knob
     * scene graph consisting of a simple circle.
     *
     * Changing the knob may resize the bounding box of the slider.  The slider
     * tries to ensure that the knob remains inside of the bounding box no
     * matter its position.
     *
     * @param knob  The new scene graph node for the knob.
     */
    void setKnob(const std::shared_ptr<Button>& knob) { placeKnob(knob); reconfigure(); }

    /**
     * Returns the scene graph node for the path.
     *
     * It is safe to make changes to the path so long as you do not resize it
     * it or scale it.  Doing so will mess up the slider visuals.
     *
     * @return the scene graph node for the path.
     */
    const std::shared_ptr<Node>& getPath() const { return _path; }
    
    /**
     * Sets the scene graph node for the path.
     *
     * If this value is nullptr, the method will construct a default path
     * scene graph consisting of a simple line and a semi-transparent track.
     *
     * Changing the knob may resize the bounding box of the slider.  The slider
     * tries to ensure that the entire path remains inside of the bounding box.
     *
     * @param path  The new scene graph node for the path.
     */
    void setPath(const std::shared_ptr<Node>& path) { placePath(path); reconfigure(); }

    /**
     * Returns the sliding bounds
     *
     * This rectangle defines the slideable region inside of the path node. The
     * bottom left corner of the bounds rectangle is the minimum value while
     * the top right is the maximum.  While the origin should have positive
     * values either the width or height may be negative.
     *
     * The bounds should be inside of the bounding box of the path node.
     * However, this is not enforced.
     *
     * @return the sliding bounds
     */
    const Rect& getBounds() const { return _bounds; }

    /**
     * Sets the sliding bounds
     *
     * This rectangle defines the slideable region inside of the path node. The
     * bottom left corner of the bounds rectangle is the minimum value while
     * the top right is the maximum.  While the origin should have positive
     * values either the width or height may be negative.
     *
     * The bounds should be inside of the bounding box of the path node.
     * However, this is not enforced.
     *
     * @param value The new sliding bounds
     */
    void getBounds(const Rect& value) { _bounds = value; reconfigure(); }
    
#pragma mark -
#pragma mark Tick Support
    /**
     * Returns the tick period of this slider.
     *
     * The tick period is used to set a course granularity on the slider.
     * Without it, each pixel is essentially its own value.  However, the
     * tick period is irrelevant unless the slider is set to snap to a tick.
     *
     * When the slider does snap to a tick, it snaps to the nearest value
     * min + k * tick where k is an non-negative integer. If this value is
     * greater than max, then it will snap to the max.
     *
     * The tick period should be nonnegative, but this is not enforced.  A
     * negative tick value will always snap to the minimum value.
     *
     * @return the tick period of this slider.
     */
    float getTick() const { return _tick; }
    
    /**
     * Sets the tick period of this slider.
     *
     * The tick period is used to set a course granularity on the slider.
     * Without it, each pixel is essentially its own value.  However, the
     * tick period is irrelevant unless the slider is set to snap to a tick.
     *
     * When the slider does snap to a tick, it snaps to the nearest value
     * min + k * tick where k is an non-negative integer. If this value is
     * greater than max, then it will snap to the max.
     *
     * The tick period should be nonnegative, but this is not enforced.  A
     * negative tick value will always snap to the minimum value.
     *
     * @param value The tick period of this slider.
     */
    void setTick(float value) { _tick = value; reposition(); }
    
    /**
     * Returns whether the slider will snap to a tick mark.
     *
     * When the slider does snap to a tick, it snaps to the nearest value
     * min + k * tick where k is an non-negative integer. If this value is
     * greater than max, then it will snap to the max.
     *
     * @return true if the slider will snap to a tick mark; false otherwise
     */
    bool hasSnap() const { return _snap; }
    
    /**
     * Sets whether the slider will snap to a tick mark.
     *
     * When the slider does snap to a tick, it snaps to the nearest value
     * min + k * tick where k is an non-negative integer. If this value is
     * greater than max, then it will snap to the max.
     *
     * @param value Whether the slider will snap to a tick mark.
     */
    void snapTick(bool value) { _snap = value; reposition(); }

#pragma mark -
#pragma mark Listeners
    /**
     * Returns true if this slider has a listener
     *
     * This listener is invoked when value changes,
     * whether active or not.
     *
     * A slider may only have one listener at a time.
     *
     * @return true if this slider has a listener
     */
	bool hasListener() const { return _listener != nullptr; }

    /**
	 * Returns the listener (if any) for this slider
	 *
     * This listener is invoked when value changes,
     * whether active or not.
	 *
	 * A slider may only have one listener at a time.
	 * If there is no listener, this method returns nullptr.
	 *
	 * @return the listener (if any) for this slider.
	 */
	const Listener getListener() const { return _listener; }

    /**
     * Sets the listener for this slider.
     *
     * This listener is invoked when value changes,
     * whether active or not.
     *
     * A slider may only have one listener at a time. If this slider already
     * has a listener, this method will replace it.
     *
     * @param listener  The listener to use
     */
	void setListener(Listener listener) { _listener = listener; }

    /**
     * Removes the listener for this slider.
     *
     * This listener is invoked when value changes,
     * whether active or not.
     *
     * A slider may only have one listener at a time. If slider does not
     * have a listener, this method will fail.
     *
     * @return true if the listener was successfully removed
     */
	bool removeListener();

    /**
     * Activates this slider to enable dragging.
     *
     * This method attaches a listener to either the {@link Mouse} or
     * {@link Touchscreen} inputs to monitor when the slider is dragged.
     * The slider will favor the mouse, but will use the touch screen
     * if no mouse input is active. If neither input is active, this method
     * will fail.
     *
     * When active, the slider will change its value on its own, without
     * requiring the user to use {@link setValue(float)}. If there is a
     * {@link Listener} attached, it will call that function upon any
     * state changes.
     *
     * @param key   The listener key for the input device
     *
     * @return true if the slider was successfully activated
     */
	bool activate(Uint32 key);

   /**
	* Deactivates this slider, unable to drag from then on.
	*
	* This method removes its internal listener from either the {@link Mouse}
	* or {@link Touchscreen}.
	*
	* When deactivated, the slider will no longer change value on its own.
	* However, the user can still change manually with the {@link setValue(float)}
    * method. In addition, any {@link Listener} attached will still
    * respond to manual state changes.
	*
	* @return true if the slider was successfully deactivated
	*/
	bool deactivate();
    
    /**
     * Returns true if this slider has been activated.
     *
     * @return true if this slider has been activated.
     */
    bool isActive() const { return _active; }
    
    
protected:
#pragma mark -
#pragma mark Internal Helpers
    /**
     * Returns the correct value nearest the given one.
     *
     * This method is used to snap values to the grid of ticks, as well as
     * keep the value in range.
     *
     * @param value The candidate value
     *
     * @return the nearest correct value.
     */
    float validate(float value) const;
    
    /**
     * Resizes the node and arranges the position of the knob and path.
     *
     * This method is called whenever the bounds or scene graph changes.
     */
    void reconfigure();

    /**
     * Repositions the knob to reflect a change in value.
     *
     * This method is called whenever the value or its range changes.
     */
    void reposition();
    
    /**
     * Drags the knob to the given position.
     *
     * This method is called by the touch listeners and assumes that an
     * initial drag anchor has been set.  The position defines a drag
     * vector that is projected on to the sliding bounds.
     *
     * @param pos   The position to drag to (if in the sliding bounds)
     */
    void dragKnob(const Vec2& pos);
    
    /**
     * Sets the scene graph node for the knob.
     *
     * If this value is nullptr, the method will construct a default knob
     * scene graph consisting of a simple circle.
     *
     * Unlike {@link setKnob()}, this does not resize the bounding box.
     *
     * @param knob  The new scene graph node for the knob.
     */
    void placeKnob(const std::shared_ptr<Button>& knob);
    
    /**
     * Sets the scene graph node for the path.
     *
     * If this value is nullptr, the method will construct a default path
     * scene graph consisting of a simple line and a semi-transparent track.
     *
     * Unlike {@link setPath()}, this does not resize the bounding box.
     *
     * @param path  The new scene graph node for the path.
     */
    void placePath(const std::shared_ptr<Node>& path);

};

}

#endif /* __CU_SLIDER_H__ */
