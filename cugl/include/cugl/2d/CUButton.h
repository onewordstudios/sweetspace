//
//  CUButton.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a simple clickable button. The button may
//  either be represented by two images (one up and one down), or a single
//  image and two different color tints.
//
//  The button can track its own state, relieving you of having to manually
//  check mouse presses. However, it can only do this when the button is part
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
//  Author: Walker White
//  Version: 10/28/18
//
#ifndef __CU_BUTTON_H__
#define __CU_BUTTON_H__
#include <cugl/assets/CUJsonValue.h>
#include <cugl/2d/CUNode.h>
#include <cugl/2d/CUPolygonNode.h>
#include <cugl/math/CUColor4.h>
#include <unordered_map>

namespace cugl {

#pragma mark -
#pragma mark Button

/**
 * This class represents a simple clickable button
 *
 * A button is either two nodes (one for up, one for down) that swap whenever
 * the button is pressed, or a single node that changes color on a press.
 * The nodes are typically either images (e.g {@link PolygonNode} or instances
 * of class {@link Label}. These nodes are stored as children of this button, 
 * which in turn should be part of a larger scene graph.
 *
 * The button can track its own state, via the {@link activate(Uint32)} method,
 * relieving you of having to manually check mouse presses/touches. However, 
 * the appropriate input device must be active before you can activate the 
 * button, as it needs to attach internal listeners.
 *
 * When a button tracks its own state, it is classified as either a normal or
 * a toggle button. A normal button is down only when it is pressed.  A toggle
 * button changes state when pressed, and retains that state until the next
 * press.
 *
 * The user can define the clickable region to be any arbitrary polygon.  This
 * allows the click response to better match complex images.
 */
class Button : public Node {
public:
#pragma mark Listener
    /**
     * @typedef Listener
     *
     * This type represents a listener for state change in the {@link Button} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. For simplicity, Button nodes only support a
     * single listener.  If you wish for more than one listener, then your listener
     * should handle its own dispatch.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const std::string& name, bool down)>
     *
     * @param name      The button name
     * @param clicks    Whether the button is now down
     */
    typedef std::function<void(const std::string& name, bool down)> Listener;

protected:
    /** Whether or not the button is currently down */
    bool _down;
    /** Whether or not the button is a toggle switch */
    bool _toggle;

    /** The node representing the button when it is up (cannot be null) */
    std::shared_ptr<Node> _upnode;
    /** The node representing the button when it is down (may be null) */
    std::shared_ptr<Node> _downnode;
    /** Layout information for the up button */
    std::shared_ptr<JsonValue> _upform;
    /** Layout information for the down button */
    std::shared_ptr<JsonValue> _downform;
    /** The button color when the button is up */
    Color4 _upcolor;
    /** The button color when the button is down */
    Color4 _downcolor;
    /** Key for the up (unclicked) child */
    std::string _upchild;
    /** Key for the down (clicked) child */
    std::string _downchild;

    /** The button bounds (for rounder buttons) */
    Poly2 _bounds;

    /** Whether the button is actively checking for state changes */
    bool _active;
    /** Whether we are using the mouse (as opposed to the touch screen) */
    bool _mouse;
    /** The listener key when the button is checking for state changes */
    Uint32 _inputkey;
    /** The listener callback for state changes */
    Listener _listener;
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized button with no size or texture information.
     *
     * You must initialize this button before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
     * heap, use one of the static constructors instead.
     */
    Button();
    
    /**
     * Deletes this button, disposing all resources
     */
    ~Button() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed button can be safely reinitialized. Any children owned by
     * this node will be released.  They will be deleted if no other object
     * owns them.
     *
     * It is unsafe to call this on a button that is still currently inside
     * of a scene graph.
     */
    virtual void dispose() override;
  
    /**
     * Deactivates the default initializer.
     *
     * This initializer may not be used for a button.  A button must have a
     * child node for the up state at the very minimum.
     *
     * @return false
     */
    virtual bool init() override {
        CUAssertLog(false,"This node does not support the empty initializer");
        return false;
    }
    
    /**
     * Initializes a button with the given up node.
     *
     * The button will look exactly like the given node when it is not pressed.
     * When pressed, it will darken the up node a bit.
     *
     * @param up    The button when it is not pressed
     *
     * @return true if the button is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Node>& up) {
        return init(up,up->getColor()*Color4::GRAY);
    }
    
    /**
     * Initializes a button with the given node and color
     *
     * The button will look exactly like the given node when it is not pressed.
     * When pressed, it will tint the up node by the specified color.
     *
     * @param up    The button when it is not pressed
     * @param down  The button tint when it is pressed
     *
     * @return true if the button is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Node>& up, Color4 down);
    
    /**
     * Initializes a button with the given nodes
     *
     * The button will look exactly like the up node when it is not pressed.
     * It will look like the down node when it is pressed.  The size of this
     * button is the size of the larger of the two nodes.
     *
     * @param up    The button when it is not pressed
     * @param down  The button when it is pressed
     *
     * @return true if the button is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Node>& up, const std::shared_ptr<Node>& down);
    
    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "up":       A JSON object defining a scene graph node
     *      "down":     A JSON object defining a scene graph node OR
     *                  a 4-element integer array with values from 0..255
     *      "pushable": An even array of polygon vertices (numbers)
     *
     * The attribute 'up' is REQUIRED.  All other attributes are optional.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) override;

#pragma mark Static Constructors
    /**
     * Returns a newly allocated button with the given up node.
     *
     * The button will look exactly like the given node when it is not pressed.
     * When pressed, it will darken the up node a bit.
     *
     * @param up    The button when it is not pressed
     *
     * @return a newly allocated button with the given up node.
     */
    static std::shared_ptr<Button> alloc(const std::shared_ptr<Node>& up) {
        std::shared_ptr<Button> node = std::make_shared<Button>();
        return (node->init(up) ? node : nullptr);
    }

    /**
     * Returns a newly allocated button with the given node and color
     *
     * The button will look exactly like the given node when it is not pressed.
     * When pressed, it will tint the up node by the specified color.
     *
     * @param up    The button when it is not pressed
     * @param down  The button tint when it is pressed
     *
     * @return a newly allocated button with the given node and color
     */
    static std::shared_ptr<Button> alloc(const std::shared_ptr<Node>& up, Color4 down) {
        std::shared_ptr<Button> node = std::make_shared<Button>();
        return (node->init(up,down) ? node : nullptr);
    }

    /**
     * Returns a newly allocated button with the given nodes
     *
     * The button will look exactly like the up node when it is not pressed.
     * It will look like the down node when it is pressed.  The size of this
     * button is the size of the larger of the two nodes.
     *
     * @param up    The button when it is not pressed
     * @param down  The button when it is pressed
     *
     * @return a newly allocated button with the given nodes
     */
    static std::shared_ptr<Button> alloc(const std::shared_ptr<Node>& up, const std::shared_ptr<Node>& down) {
        std::shared_ptr<Button> node = std::make_shared<Button>();
        return (node->init(up,down) ? node : nullptr);
    }

    /**
     * Returns a newly allocated node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *      "up":       A JSON object defining a scene graph node
     *      "down":     A JSON object defining a scene graph node OR
     *                  a 4-element integer array with values from 0..255
     *      "pushable": An even array of polygon vertices (numbers)
     *
     * The attribute 'up' is REQUIRED.  All other attributes are optional.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<Button> result = std::make_shared<Button>();
        if (!result->initWithData(loader,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<Node>(result);
    }
    
#pragma mark Button Attributes
    /**
     * Sets the color tinting this node.
     *
     * This color will be multiplied with the parent (this node on top) if
     * hasRelativeColor() is true.
     *
     * The default color is white, which means that all children have their
     * natural color.
     *
     * @param color the color tinting this node.
     */
    virtual void setColor(Color4 color) override;

    /**
     * Returns the region responding to mouse clicks.
     *
     * The pushable region is the area of this node that responds to mouse
     * clicks.  By allowing it to be an arbitrary polygon, we are capable of
     * defining buttons with complex shapes. The polygon must have SOLID type.
     *
     * @return the region responding to mouse clicks.
     */
    const Poly2& getPushable() const { return _bounds; }
    
    /**
     * Sets the region responding to mouse clicks.
     *
     * The pushable region is the area of this node that responds to mouse
     * clicks.  By allowing it to be an arbitrary polygon, we are capable of
     * defining buttons with complex shapes. The polygon must have SOLID type.
     *
     * @param bounds    The region responding to mouse clicks.
     */
    void setPushable(const Poly2& bounds);
    
    /**
     * Sets the region responding to mouse clicks.
     *
     * The pushable region is the area of this node that responds to mouse
     * clicks.  By allowing it to be an arbitrary polygon, we are capable of
     * defining buttons with complex shapes. The vertices will be converted
     * into a polygon using {@link SimpleTriangulator}.
     *
     * @param vertices  The region responding to mouse clicks.
     */
    void setPushable(const std::vector<Vec2>& vertices);
    
    /**
     * Arranges the child of this node using the layout manager.
     *
     * This process occurs recursively and top-down. A layout manager may end
     * up resizing the children.  That is why the parent must finish its layout
     * before we can apply a layout manager to the children.
     */
    virtual void doLayout() override;
    
#pragma mark Button State
    /**
     * Returns true if this button contains the given screen point
     *
     * This method is used to manually check for mouse presses/touches.  It
     * converts a point in screen coordinates to the node coordinates and
     * checks if it is in the bounds of the button.
     *
     * @param point The point in screen coordinates
     *
     * @return true if this button contains the given screen point
     */
    bool containsScreen(const Vec2& point);
    
    /**
     * Returns true if this button contains the given screen point
     *
     * This method is used to manually check for mouse presses/touches.  It
     * converts a point in screen coordinates to the node coordinates and
     * checks if it is in the bounds of the button.
     *
     * @param x The x-value in screen coordinates
     * @param y The y-value in screen coordinates
     *
     * @return true if this button contains the given screen point
     */
    bool containsScreen(float x, float y) {
        return containsScreen(Vec2(x,y));
    }

    /**
     * Return true if this button is currently down.
     *
     * Buttons only have two states: up and down.  The default state is up.
     *
     * Changing this value will change how the button is displayed on the
     * screen.  It will also invoke the {@link Listener} if one is
     * currently attached.
     *
     * @return true if this button is currently down.
     */
    bool isDown() const { return _down; }
    
    /**
     * Sets whether this button is currently down.
     *
     * Buttons only have two states: up and down.  The default state is up.
     *
     * Changing this value will change how the button is displayed on the
     * screen.  It will also invoke the {@link Listener} if one is
     * currently attached.
     *
     * @param down  Whether this button is currently down.
     */
    void setDown(bool down);

    /**
     * Returns true if this is a toggle button
     *
     * A normal button is down only when it is pressed.  A toggle button
     * changes state when pressed, and retains that state until the next
     * press.
     *
     * This attribute is only relevant for activated buttons
     *
     * @return true if this is a toggle button
     */
    bool isToggle() const { return _toggle; }

    /**
     * Sets whether this is a toggle button
     *
     * A normal button is down only when it is pressed.  A toggle button
     * changes state when pressed, and retains that state until the next
     * press.
     *
     * This attribute is only relevant for activated buttons
     *
     * @param value whether this is a toggle button
     */
    void setToggle(bool value) { _toggle = value; }

#pragma mark Listeners
    /**
     * Returns true if this button has a listener
     *
     * This listener is invoked when the button state changes (up or down).
     *
     * A button may only have one listener at a time.
     *
     * @return true if this button has a listener
     */
    bool hasListener() const { return _listener != nullptr; }
    
    /**
     * Returns the listener (if any) for this button
     *
     * This listener is invoked when the button state changes (up or down).
     *
     * A button may only have one listener at a time. If there is no listener,
     * this method returns nullptr.
     *
     * @return the listener (if any) for this button
     */
    const Listener getListener() const { return _listener; }
    
    /**
     * Sets the listener for this button.
     *
     * This listener is invoked when the button state changes (up or down).
     *
     * A button may only have one listener at a time.  If this button already
     * has a listener, this method will replace it for the once specified.
     *
     * @param listener  The listener to use
     */
    void setListener(Listener listener) { _listener = listener; }
    
    /**
     * Removes the listener for this button.
     *
     * This listener is invoked when the button state changes (up or down).
     *
     * A button may only have one listener at a time.  If this button does not
     * have a listener, this method will fail.
     *
     * @return true if the listener was succesfully removed
     */
    bool removeListener();
    
    /**
     * Activates this button to listen for mouse/touch events.
     *
     * This method attaches a listener to either the {@link Mouse} or 
     * {@link Touchscreen} inputs to monitor when the button is pressed and/or
     * released. The button will favor the mouse, but will use the touch screen
     * if no mouse input is active.  If neither input is active, this method
     * will fail.
     *
     * When active, the button will change its state on its own, without
     * requiring the user to use {@link setDown(bool)}.  If there is a
     * {@link Listener} attached, it will call that function upon any
     * state changes.
     *
     * @param key   The listener key for the input device
     *
     * @return true if the button was successfully activated
     */
    bool activate(Uint32 key);

    /**
     * Deactivates this button, ignoring future mouse/touch events.
     *
     * This method removes its internal listener from either the {@link Mouse}
     * or {@link Touchscreen} inputs to monitor when the button is pressed and/or
     * released.  The input affected is the one that received the listener
     * upon activation.
     *
     * When deactivated, the button will no longer change its state on its own.
     * However, the user can still change the state manually with the 
     * {@link setDown(bool)} method.  In addition, any {@link Listener}
     * attached will still respond to manual state changes.
     *
     * @return true if the button was successfully deactivated
     */
    bool deactivate();
    
    /**
     * Returns true if this button has been activated.
     *
     * @return true if this button has been activated.
     */
    bool isActive() const { return _active; }
};

}

#endif /* __CU_BUTTON_H__ */
