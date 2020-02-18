//
//  CUGridLayout.cpp
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
#include <cugl/2d/layout/CUGridLayout.h>

using namespace cugl;

#pragma mark Constructors
/**
 * Creates a degenerate layout manager with no data.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
GridLayout::GridLayout() :
_gwidth(1),
_gheight(1) {
}

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
bool GridLayout::initWithData(const std::shared_ptr<JsonValue>& data) {
    _gwidth  = data->getInt("width",1);
    _gheight = data->getInt("height",1);
    return true;
}


#pragma mark -
#pragma mark Layout
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
bool GridLayout::add(const std::string key, const std::shared_ptr<JsonValue>& data) {
    std::string horz = data->getString("x_anchor","center");
    std::string vert = data->getString("y_anchor","middle");
    Anchor anchor = getAnchor(horz, vert);
    
    unsigned int x = data->getInt("x_index",0);
    unsigned int y = data->getInt("y_index",0);
    return addPosition(key,x,y,anchor);
}

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
bool GridLayout::addPosition(const std::string key, unsigned int x, unsigned int y, Anchor anchor) {
    auto last = _entries.find(key);
    if (last != _entries.end()) {
        return false;
    }
    
    Entry entry;
    entry.anchor = anchor;
    entry.x = x;
    entry.y = y;
    _entries[key] = entry;
    return true;
}

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
bool GridLayout::remove(const std::string key) {
    auto entry = _entries.find(key);
    if (entry != _entries.end()) {
        _entries.erase(entry);
        return true;
    }
    return false;
}

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
void GridLayout::layout(Node* node) {
    auto kids = node->getChildren();
    Size size = node->getContentSize();
    Size grid = Size(size.width/_gwidth,size.height/_gheight);
    for(auto it = kids.begin(); it != kids.end(); ++it) {
        auto jt = _entries.find((*it)->getName());
        if (jt != _entries.end()) {
            Entry entry = jt->second;
            Rect bounds(Vec2(entry.x*grid.width,entry.y*grid.height),grid);
            reanchor(it->get(), entry.anchor);
            placeNode(it->get(), entry.anchor, bounds, Vec2::ZERO);
        }
    }
}


#pragma mark -
#pragma mark GridSize
/**
 * Returns the grid size of this layout
 *
 * The size must have non-zero width and height.  Despite the return type,
 * the width and height must be integers.
 *
 * @return the grid size of this layout
 */
void GridLayout::setGridSize(Uint32 width, Uint32 height) {
    if (validate(width,height)) {
        _gwidth  = width;
        _gheight = height;
    }
}

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
bool GridLayout::validate(Uint32 width, Uint32 height) {
    for(auto it = _entries.begin(); it != _entries.end(); ++it) {
        Uint32 x = it->second.x;
        Uint32 y = it->second.y;
        if (x >= width || y >= height) {
            CUAssertLog(false, "Grid location (%d,%d) is invalidated by new grid size (%d,%d)",x,y,width,height);
            return false;
        }
    }
    return true;
}
