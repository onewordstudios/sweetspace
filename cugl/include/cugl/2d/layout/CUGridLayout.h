//
//  CUGridLayout.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an support for a grid layout. A grid layout subdivides
//  the node into equal sized grid regions.  Each grid region may receive a
//  a single child.  A grid region behaves like an AnchoredLayout for the
//  rules on placing the child.  The result is a slightly more flexible
//  layout manager than the grid layout in Java.
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
#ifndef __CU_GRID_LAYOUT_H__
#define __CU_GRID_LAYOUT_H__
#include <cugl/2d/layout/CULayout.h>
#include <unordered_map>

namespace cugl {
    
/**
 * This class provides a grid layout manager.
 *
 * A grid layout subdivides the node into equal sized grid regions.  Each grid
 * region may receive its own child (and can receive more than one).  A grid
 * region behaves like an {@link AnchoredLayout} for the rules of how to place
 * the child. The result is a slightly more flexible layout manager than the
 * grid layout in Java.
 *
 * Layout information is indexed by key.  To look up the layout information
 * of a scene graph node, we use the name of the node.  This requires all
 * nodes to have unique names.  The {@link SceneLoader} prefixes all child
 * names by the parent name, so this is the case in any well-defined JSON file.
 */
class GridLayout : public Layout {
protected:
    /**
     * This inner class stores the layout information as a struct.
     *
     * The x and y values must be valid.  They are verified when assigned
     * and when the grid size changes.
     */
    class Entry {
    public:
        /** The x offset from the anchor in absolute or relative units */
        Uint32 x;
        /** The y offset from the anchor in absolute or relative units */
        Uint32 y;
        /** The anchor rule to place it in the grid square. */
        Anchor anchor;
    };
    
    /** The map of keys to layout information */
    std::unordered_map<std::string,Entry> _entries;
    /** The number of columns of grid regions */
    Uint32 _gwidth;
    /** The number of rows of grid regions */
    Uint32 _gheight;
    
#pragma mark Constructors
public:
    /**
     * Creates a degenerate layout manager with no data.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    GridLayout();
    
    /**
     * Deletes this layout manager, disposing of all resources.
     */
    ~GridLayout() { dispose(); }
    
    /**
     * Initializes a new layout manager with the given JSON specificaton.
     *
     * In addition to the 'type' attribute (which must be "float"), the JSON
     * specification supports the following attribute values:
     *
     *      "width":    An integer with the number of columns in the grid
     *      "height":   An integer with the number of rows in the grid
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
    virtual void dispose() override { _entries.clear(); }
    
    /**
     * Returns a newly allocated layout manager.
     *
     * The layout manager is initially empty.  Before using it to perform a
     * layout, layout information must be registered throught the {@link add}
     * method interface.
     *
     * @return a newly allocated layout manager.
     */
    static std::shared_ptr<GridLayout> alloc() {
        std::shared_ptr<GridLayout> result = std::make_shared<GridLayout>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated layout manager with the given JSON specificaton.
     *
     * In addition to the 'type' attribute (which must be "float"), the JSON
     * specification supports the following attribute values:
     *
     *      "width":    An integer with the number of columns in the grid
     *      "height":   An integer with the number of rows in the grid
     *
     * All attributes other than 'type' are optional.
     *
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated layout manager with the given JSON specificaton.
     */
    static std::shared_ptr<GridLayout> allocWithData(const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<GridLayout> result = std::make_shared<GridLayout>();
        return (result->initWithData(data) ? result : nullptr);
    }
    
#pragma mark Layout
    /**
     * Returns the grid size of this layout
     *
     * The size must have non-zero width and height.  Despite the return type,
     * the width and height must be integers.
     *
     * @return the grid size of this layout
     */
    Size getGridSize() const { return Size((float)_gwidth, (float)_gheight); }
    
    /**
     * Sets the grid size of this layout
     *
     * The size must have non-zero width and height.  Despite the parameter type,
     * the width and height must be integers.  The values will be cast to the
     * nearest integer.
     *
     * @param size  The grid size of this layout
     */
    void setGridSize(const Size& size) { setGridSize((Uint32)size.width, (Uint32)size.height); }

    /**
     * Sets the grid size of this layout
     *
     * The size must have non-zero width and height.
     *
     * @param width     The number of columns in the grid
     * @param height    The number of rows in the grid
     */
    void setGridSize(Uint32 width, Uint32 height);

    /**
     * Assigns layout information for a given key.
     *
     * The JSON object may contain any of the following attribute values:
     *
     *      "x_index":  An integer with the horizontal grid index
     *      "y_index":  An integer with the vertical grid index
     *      "x_anchor": One of 'left', 'center', 'right', or 'fill'
     *      "y_anchor": One of 'bottom', 'middle', 'top', or 'fill'
     *
     * The specified grid region it treated like an {@link AnchoredLayout}
     * when placing the child for the given key. There is no limit on the
     * number of children that may share a grid region.
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
     * Assigns the layout position for a given key.
     *
     * The specified grid region it treated like an {@link AnchoredLayout}
     * when placing the child for the given key. There is no limit on the
     * number of children that may share a grid region.
     *
     * To look up the layout information of a scene graph node, we use the name
     * of the node.  This requires all nodes to have unique names. The
     * {@link SceneLoader} prefixes all child names by the parent name, so
     * this is the case in any well-defined JSON file. If the key is already
     * in use, this method will fail.
     *
     * @param key       The key identifying the layout information
     * @param x         The column of the grid region
     * @param y         The row of the grid region
     * @param anchor    The anchor rule for the grid region
     *
     * @return true if the priority was assigned to that key
     */
    bool addPosition(const std::string key, unsigned int x, unsigned int y, Anchor anchor);

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
     * A grid layout subdivides the node into equal sized grid regions.  Each grid
     * region may receive its own child (and can receive more than one).  A grid
     * region behaves like an {@link AnchoredLayout} for the rules of how to place
     * the child. The result is a slightly more flexible layout manager than the
     * grid layout in Java.
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
    virtual void layout(Node* node) override;
    
#pragma mark Internal Helpers
protected:
    /**
     * Returns true if (width, height) is a valid grid size
     *
     * If the layout manager is resized, it must be done in such a way that
     * none of the children are out of bounds.  This method returns false
     * if this happens.  This method is used by {@link setGridSize} before
     * changing the actual grid size.
     *
     * @param width     The number of columns in the grid
     * @param height    The number of rows in the grid
     *
     * @return true if (width, height) is a valid grid size
     */
    bool validate(Uint32 width, Uint32 height);
};

}

#endif /* __CU_GRID_LAYOUT_H__ */
