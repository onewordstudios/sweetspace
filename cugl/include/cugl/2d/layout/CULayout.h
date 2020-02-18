//
//  CULayout.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an abstract class interface for Java-style Layout
//  managers.  This gives us more flexibility for creating scene graphs
//  on different resolution devices.
//
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
#ifndef __CU_LAYOUT_H__
#define __CU_LAYOUT_H__
#include <cugl/2d/CUNode.h>
#include <cugl/assets/CUJsonValue.h>

namespace  cugl {


/**
 * This class is an abstract interface for a layout manager.
 *
 * A layout manager associates layout information with scene graph nodes, much
 * like a map interface. When a layout manager is asked to layout a Node, it
 * searches for those children that are registered with the layout manager.
 * For those children, it repositions and/or resizes them according to the
 * layout information.
 *
 * Layout information is indexed by key.  To look up the layout information
 * of a scene graph node, we use the name of the node.  This requires all
 * nodes to have unique names.  The {@link SceneLoader} prefixes all child
 * names by the parent name, so this is the case in any well-defined JSON file.
 *
 * All layout managers extend this class, providing implementations for
 * the {@link add}, {@link remove}, and {@link layout} methods.
 *
 * Several layout managers, such as {@link AnchoredLayout} and {@link GridLayout}
 * make use of anchors.  Therefore, we provide support for them in this class
 * in order to consolidate code.
 */
class Layout {
#pragma mark -
#pragma mark Constructors
public:
    /**
     * This enum represents the nine possible anchors.
     *
     * The anchor positions are similar to that of a {@link NinePatch}. The
     * anchor positions are relative to the Node bounding box.  In addition,
     * there are "fill" anchors, which stretch the node to fill the available
     * space.
     */
    enum class Anchor : int {
        /** The bottom left corner, or position (0,0) in Node coordinate space. */
        BOTTOM_LEFT     = 0,
        /** The left side, or position (0,height/2) in Node coordinate space. */
        MIDDLE_LEFT     = 1,
        /** The top left corner, or position (0,height) in Node coordinate space. */
        TOP_LEFT        = 2,
        /** The bottom side, or position (width/2,0) in Node coordinate space. */
        BOTTOM_CENTER   = 3,
        /** The middle region, or position (width/2,height/2) in Node coordinate space. */
        CENTER          = 4,
        /** The top side, or position (width/2,height) in Node coordinate space. */
        TOP_CENTER      = 5,
        /** The bottom right corner, or position (width,0) in Node coordinate space. */
        BOTTOM_RIGHT    = 6,
        /** The right side, or position (width,height/2) in Node coordinate space. */
        MIDDLE_RIGHT    = 7,
        /** The top left corner, or position (width,height) in Node coordinate space. */
        TOP_RIGHT       = 8,
        /** Anchors at y=0, but stretches the width to fill the parent. */
        BOTTOM_FILL     = 9,
        /** Anchors at y=height/2, but stretches the width to fill the parent. */
        MIDDLE_FILL     = 10,
        /** Anchors at y=height, but stretches the width to fill the parent. */
        TOP_FILL        = 11,
        /** Anchors at x=0, but stretches the height to fill the parent. */
        LEFT_FILL       = 12,
        /** Anchors at x=width/2, but stretches the height to fill the parent. */
        CENTER_FILL     = 13,
        /** Anchors at x=width, but stretches the height to fill the parent. */
        RIGHT_FILL      = 14,
        /** Stretches the width and height to fill the entire parent */
        TOTAL_FILL      = 15,
        /** No anchor.  The layout will not adjust this child */
        NONE            = 16
    };
    
    /**
     * Creates a degenerate layout manager with no data.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Layout() {}
    
    /**
     * Deletes this layout manager, disposing of all resources.
     */
    ~Layout() { dispose(); }
    
    /**
     * Deletes the layout resources and resets all attributes.
     *
     * A disposed layout manager can be safely reinitialized.
     */
    virtual void dispose() {}
    
    /**
     * Initializes a new layout manager.
     *
     * The layout manager is initially empty.  Before using it to perform a
     * layout, layout information must be registered throught the {@link add}
     * method interface.
     *
     * @return true if initialization is successful.
     */
    virtual bool init() { return true; }
    
    /**
     * Initializes a new layout manager with the given JSON specificaton.
     *
     * The JSON specification format is implementation specific. However, all
     * layout managers require a 'type' attribute that specifies the name of
     * the layout manager.
     *
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const std::shared_ptr<JsonValue>& data) { return false; }

#pragma mark Layout
    /**
     * Assigns layout information for a given key.
     *
     * To look up the layout information of a scene graph node, we use the name
     * of the node.  This requires all nodes to have unique names. The
     * {@link SceneLoader} prefixes all child names by the parent name, so
     * this is the case in any well-defined JSON file.
     *
     * The format of the JSON object is layout manager specific.
     *
     * If the key is already in use, this method will fail.
     *
     * @param key   The key identifying the layout information
     * @param data  A JSON object with the layout information
     *
     * @return true if the layout information was assigned to that key
     */
    virtual bool add(const std::string key, const std::shared_ptr<JsonValue>& data) { return false; }
    
    /**
     * Removes the layout information for a given key.
     *
     * To look up the layout information of a scene graph node, we use the name
     * of the node.  This requires all nodes to have unique names. The
     * {@link SceneLoader} prefixes all child names by the parent name, so
     * this is the case in any well-defined JSON file.
     *
     * If the key is not in use, this method will fail.
     *
     * @param key   The key identifying the layout information
     *
     * @return true if the layout information was removed for that key
     */
    virtual bool remove(const std::string key) { return false; }
    
    /**
     * Performs a layout on the given node.
     *
     * This layout manager will searches for those children that are registered
     * with it. For those children, it repositions and/or resizes them according
     * to the layout information.
     *
     * To look up the layout information of a scene graph node, this method uses
     * the name of the node.  This requires all nodes to have unique names. The
     * {@link SceneLoader} prefixes all child names by the parent name, so
     * this is the case in any well-defined JSON file.
     *
     * Children not registered with this layout manager are not affected.
     *
     * @param node  The scene graph node to rearrange
     */
    virtual void layout(Node* node) {}
    
#pragma mark Layout Helpers
    /**
     * Returns the anchor for the given text values
     *
     * This method is used to get an anchor object from a JSON directory.  The
     * x_anchor should be one of 'left', 'center', 'right', or 'fill'.  The
     * y_anchor should be one of 'bottom', 'middle', 'top', or 'fill'.
     *
     * @param x_anchor  The horizontal anchor setting
     * @param y_anchor  The vertical anchor setting
     *
     * @return the anchor for the given text values
     */
    static Anchor getAnchor(const std::string& x_anchor, const std::string& y_anchor);
    
    /**
     * Repositions the given node according the rules of its anchor.
     *
     * The repositioning is done relative to bounds, not the parent node. This
     * allows us to apply anchors to a subregion, like we do in {@link GridLayout}.
     * The value offset should be in coordinates, and not percentages.
     *
     * @param node      The node to reposition
     * @param anchor    The anchor rule for this node
     * @param bounds    The area to compute the position from
     * @param offset    The offset from the computed anchor position
     */
    static void placeNode(Node* node, Anchor anchor, const Rect& bounds, const Vec2& offset);
    
    /**
     * Resets the node anchor to agree with the layout anchor
     *
     * For some layout managers, the layout anchor (which is an enum) may
     * disagree with the node anchor (which is a percentage vector).  This
     * method allows a layout manager to "disable" the node anchor in favor
     * of the layout anchor.
     *
     * @param node      The node to reanchor
     * @param anchor    The layout anchor to use
     */
    static void reanchor(Node* node, Anchor anchor);

};

}
#endif /* __CU_LAYOUT_H__ */
