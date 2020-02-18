//
//  CUFloatLayout.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an support for a float layout.  Children in a float
//  layout are arranged in order, according to the layout orientation (horizontal
//  or vertical).  If there is not enough space in the Node for the children
//  to all be in the same row or column (depending on orientation), then the
//  later children wrap around to a new row or column.  This is the same way
//  that float layouts work in Java.
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
#ifndef __CU_FLOAT_LAYOUT_H__
#define __CU_FLOAT_LAYOUT_H__
#include <cugl/2d/layout/CULayout.h>
#include <vector>
#include <unordered_map>

namespace cugl {
    
/**
 * This class provides a float layout manager.
 *
 * Children in a float layout are arranged in order, according to the layout
 * orientation (horizontal or vertical).  If there is not enough space in the
 * Node for the children to all be in the same row or column (depending on
 * orientation), then the later children wrap around to a new row or column.
 * New rows are added downwards and new columns are added to the right.  This
 * is the same way that float layouts work in Java.
 *
 * Any children that cannot fit (non-overlapping) into the Node are dropped.
 * Once a child is dropped, no further children will be placed.  So an exceptional
 * large child can block the rest of the layout.
 *
 * Layout information is indexed by key.  To look up the layout information
 * of a scene graph node, we use the name of the node.  This requires all
 * nodes to have unique names.  The {@link SceneLoader} prefixes all child
 * names by the parent name, so this is the case in any well-defined JSON file.
 */
class FloatLayout : public Layout {
public:
    /**
     * This enum represents the possible layout alignments.
     *
     * Alignment is independent of an orientation.  It specifies how to align
     * each line or column with respect to each other, as well as how to
     * anchor all of them in the larger Node space.
     *
     * While alignment looks superficially similar to anchors, we do not currently
     * allow fill alignments for a float layout.  In addition, the semantics
     * of alignment are very different than anchors.  So we express them as
     * different layouts.
     */
    enum class Alignment : int {
        /**
         * In horizontal orientation, this left justifies all of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their bottom, and the bottom line will be flush with the bottom
         * of the Node.
         *
         * In vertical orientation, this bottom justifies all of the individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their left, and the left column will be flush with the left
         * of the Node.
         */
        BOTTOM_LEFT     = 0,
        /**
         * In horizontal orientation, this left justifies all of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their middle, and the layout will be centered in the Node.
         *
         * In vertical orientation, this centers each of individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their left, and the left column will be flush with the left
         * of the Node.
         */
        MIDDLE_LEFT     = 1,
        /**
         * In horizontal orientation, this left justifies all of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their top, and the top line will be flush with the bottom
         * of the Node.
         *
         * In vertical orientation, this top justifies all of the individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their left, and the left column will be flush with the left
         * of the Node.
         */
        TOP_LEFT        = 2,
        /**
         * In horizontal orientation, this centers each of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their bottom, and the bottom line will be flush with the bottom
         * of the Node.
         *
         * In vertical orientation, this bottom justifies all of the individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their center, and the layout will be centered in the Node.
         */
        BOTTOM_CENTER   = 3,
        /**
         * In horizontal orientation, this centers each of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their center, and the layout will be centered in the Node.
         *
         * In vertical orientation, this centers each of the individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their center, and the layout will be centered in the Node.
         */
        CENTER          = 4,
        /**
         * In horizontal orientation, this centers each of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their top, and the top line will be flush with the bottom
         * of the Node.
         *
         * In vertical orientation, this top justifies all of the individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their center, and the layout will be centered in the Node.
         */
        TOP_CENTER      = 5,
        /**
         * In horizontal orientation, this right justifies all of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their bottom, and the bottom line will be flush with the bottom
         * of the Node.
         *
         * In vertical orientation, this bottom justifies all of the individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their right, and the right column will be flush with the right
         * of the Node.
         */
        BOTTOM_RIGHT    = 6,
        /**
         * In horizontal orientation, this right justifies all of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their middle, and the layout will be centered in the Node.
         *
         * In vertical orientation, this centers each of individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their right, and the right column will be flush with the right
         * of the Node.
         */
        MIDDLE_RIGHT    = 7,
        /**
         * In horizontal orientation, this right justifies all of the individual
         * lines. In addition, all Nodes in a single line will be aligned
         * by their top, and the top line will be flush with the bottom
         * of the Node.
         *
         * In vertical orientation, this top justifies all of the individual
         * columns. In addition, all Nodes in a single column will be aligned
         * by their right, and the right column will be flush with the right
         * of the Node.
         */
        TOP_RIGHT       = 8
    };
    
protected:
    /** The child priority */
    std::vector<std::string> _priority;
    /** To ensure key uniqueness */
    std::unordered_map<std::string,size_t> _keyset;
    
    /** Whether the layout is horizontal or vertical */
    bool _horizontal;
    /** The layout aligment */
    Alignment _alignment;
    
#pragma mark Internal Helpers
    /**
     * Performs a horizontal layout on the given node.
     *
     * This method is identical to {@link layout(Node*)} except that it overrides
     * the orientation settings of the layout manager; it always lays out the
     * children horizontally.
     *
     * @param node  The scene graph node to rearrange
     */
    void layoutHorizontal(Node* node);
    
    /**
     * Performs a vertical layout on the given node.
     *
     * This method is identical to {@link layout(Node*)} except that it overrides
     * the orientation settings of the layout manager; it always lays out the
     * children vertically.
     *
     * @param node  The scene graph node to rearrange
     */
    void layoutVertical(Node* node);

    
#pragma mark Constructors
public:
    /**
     * Creates a degenerate layout manager with no data.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    FloatLayout();
    
    /**
     * Deletes this layout manager, disposing of all resources.
     */
    ~FloatLayout() { dispose(); }
    
    /**
     * Initializes a new layout manager with the given JSON specificaton.
     *
     * In addition to the 'type' attribute (which must be "float"), the JSON
     * specification supports the following attribute values:
     *
     *      "orientation":  One of 'horizontal' or 'vertical'
     *      "x_alignment":  One of 'left', 'center', or 'right'
     *      "y_alignment":  One of 'bottom', 'middle', or 'top'
     *
     * All attributes other than 'type' are optional.
     *
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const std::shared_ptr<JsonValue>& data) override;
    
    /**
     * Deletes the layout resources and resets all attributes.
     *
     * A disposed layout manager can be safely reinitialized.
     */
    virtual void dispose() override { _priority.clear(); _keyset.clear(); }
    
    /**
     * Returns a newly allocated layout manager.
     *
     * The layout manager is initially empty.  Before using it to perform a
     * layout, layout information must be registered throught the {@link add}
     * method interface.
     *
     * @return a newly allocated layout manager.
     */
    static std::shared_ptr<FloatLayout> alloc() {
        std::shared_ptr<FloatLayout> result = std::make_shared<FloatLayout>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated layout manager with the given JSON specificaton.
     *
     * In addition to the 'type' attribute (which must be "float"), the JSON
     * specification supports the following attribute values:
     *
     *      "orientation":  One of 'horizontal' or 'vertical'
     *      "x_alignment":  One of 'left', 'center', or 'right'
     *      "y_alignment":  One of 'bottom', 'middle', or 'top'
     *
     * All attributes other than 'type' are optional.
     *
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated layout manager with the given JSON specificaton.
     */
    static std::shared_ptr<FloatLayout> allocWithData(const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<FloatLayout> result = std::make_shared<FloatLayout>();
        return (result->initWithData(data) ? result : nullptr);
    }
    
#pragma mark Layout
    /**
     * Returns true if the layout orientation is horizontal.
     *
     * All children must be laid out by the same orientation.
     *
     * @return true if the layout orientation is horizontal.
     */
    bool isHorizontal() const { return _horizontal; }

    /**
     * Sets whether the layout orientation is horizontal.
     *
     * All children must be laid out by the same orientation.
     *
     * @param value Whether the layout orientation is horizontal.
     */
    void setHorizontal(bool value) { _horizontal = value; }
    
    /**
     * Returns the alignment of this layout.
     *
     * All children must share the same alignment.
     *
     * @return the alignment of this layout.
     */
    Alignment getAlignment() const { return _alignment; }
    
    /**
     * Sets whether the layout orientation is horizontal.
     *
     * All children must be laid out by the same orientation.
     *
     * @param value The alignment of this layout.
     */
    void setAlignment(Alignment value) { _alignment = value; }

    /**
     * Assigns layout information for a given key.
     *
     * The JSON object may contain any of the following attribute value:
     *
     *      "priority":     An int indicating placement priority.
     *                      Children with lower priority go first.
     *
     * A child with no priority is put at the end. If there is already a child
     * with the given priority, then this method will fail.
     *
     * To look up the layout information of a scene graph node, we use the name
     * of the node.  This requires all nodes to have unique names. The
     * {@link SceneLoader} prefixes all child names by the parent name, so
     * this is the case in any well-defined JSON file. If the key is already
     * in use, this method will fail.
     *
     * @param key   The key identifying the layout information
     * @param data  A JSON object with the layout information
     *
     * @return true if the layout information was assigned to that key
     */
    virtual bool add(const std::string key, const std::shared_ptr<JsonValue>& data) override;
    
    /**
     * Assigns the layout priority for a given key.
     *
     * In a float layout, children with lower priority go first. If there is
     * already a child with the given priority, then this method will fail.
     *
     * To look up the layout information of a scene graph node, we use the name
     * of the node.  This requires all nodes to have unique names. The
     * {@link SceneLoader} prefixes all child names by the parent name, so
     * this is the case in any well-defined JSON file. If the key is already
     * in use, this method will fail.
     *
     * @param key       The key identifying the layout information
     * @param priority  The priority (lower is better) for this key
     *
     * @return true if the priority was assigned to that key
     */
    bool addPriority(const std::string key, size_t priority);
    
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
    virtual bool remove(const std::string key) override;
    
    /**
     * Performs a layout on the given node.
     *
     * This layout manager will searches for those children that are registered
     * with it. For those children, it repositions and/or resizes them according
     * to the layout information.
     *
     * Children in a float layout are arranged in order, according to the layout
     * orientation (horizontal or vertical).  If there is not enough space in the
     * Node for the children to all be in the same row or column (depending on
     * orientation), then the later children wrap around to a new row or column.
     * New rows are added downwards and new columns are added to the right.  This
     * is the same way that float layouts work in Java.
     *
     * Any children that cannot fit (non-overlapping) into the Node are dropped.
     * Once a child is dropped, no further children will be placed.  So an exceptional
     * large child can block the rest of the layout.
     *
     * Layout information is indexed by key.  To look up the layout information
     * of a scene graph node, we use the name of the node.  This requires all
     * nodes to have unique names.  The {@link SceneLoader} prefixes all child
     * names by the parent name, so this is the case in any well-defined JSON file.
     *
     * Children not registered with this layout manager are not affected.
     *
     * @param node  The scene graph node to rearrange
     */
    virtual void layout(Node* node) override;
    
};

}


#endif /* __CU_FLOAT_LAYOUT_H__ */
