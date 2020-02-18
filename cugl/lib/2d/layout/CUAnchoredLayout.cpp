//
//  CUAnchoredLayout.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an support for an anchored layout.  An anchored layout
//  attaches a child node to one of nine "anchors" in the parent (corners, sides
//  or middle), together with a percentage (or absolute) offset.  As the parent
//  grows or shinks, the child will move according to its anchor.  For example,
//  nodes in the center will stay centered, while nodes on the left side will
//  move to keep the appropriate distance from the left side.
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
#include <cugl/2d/layout/CUAnchoredLayout.h>

using namespace cugl;

/**
 * Assigns layout information for a given key.
 *
 * The JSON object may contain any of the following attribute values:
 *
 *      "x_anchor": One of 'left', 'center', 'right', or 'fill'
 *      "y_anchor": One of 'bottom', 'middle', 'top', or 'fill'
 *      "absolute": Whether to use absolute instead of relative (percentage) offsets
 *      "x_offset": A number indicating the horizontal offset from the anchor.
 *                  If "absolute" is true, this is the distance in coordinate space.
 *                  Otherwise it is a percentage of the width.
 *      "y_offset": A number indicating the vertical offset from the anchor.
 *                  If "absolute" is true, this is the distance in coordinate space.
 *                  Otherwise it is a percentage of the height.
 *
 * All attributes are optional.  There are no required attributes.
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
bool AnchoredLayout::add(const std::string key, const std::shared_ptr<JsonValue>& data) {
    std::string horz = data->getString("x_anchor","center");
    std::string vert = data->getString("y_anchor","middle");
    Anchor anchor = getAnchor(horz, vert);

    Vec2 offset;
    offset.x = data->getFloat("x_offset",0.0f);
    offset.y = data->getFloat("y_offset",0.0f);
    
    
    bool absolute =  data->getBool("absolute",false);
    
    return absolute ? addAbsolute(key,anchor,offset) : addRelative(key, anchor, offset);
}

/**
 * Assigns layout information for a given key.
 *
 * This method specifies the anchor offset in absolute terms.  That is,
 * offset is the distance from the anchor in Node coordinate space.
 *
 * To look up the layout information of a scene graph node, we use the name
 * of the node.  This requires all nodes to have unique names. The
 * {@link SceneLoader} prefixes all child names by the parent name, so
 * this is the case in any well-defined JSON file. If the key is already
 * in use, this method will fail.
 *
 * @param key       The key identifying the layout information
 * @param anchor    The anchor point to use
 * @param offset    The offset from the anchor point
 *
 * @return true if the layout information was assigned to that key
 */
bool AnchoredLayout::addAbsolute(const std::string key, Anchor anchor, const Vec2& offset) {
    auto last = _entries.find(key);
    if (last != _entries.end()) {
        return false;
    }

    Entry entry;
    entry.anchor = anchor;
    entry.x_offset = offset.x;
    entry.y_offset = offset.y;
    entry.absolute = true;
    _entries[key] = entry;
    return true;
}

/**
 * Assigns layout information for a given key.
 *
 * This method specifies the anchor offset in relative terms.  That is,
 * offset is the percentage of the width and height of the Node.
 *
 * To look up the layout information of a scene graph node, we use the name
 * of the node.  This requires all nodes to have unique names. The
 * {@link SceneLoader} prefixes all child names by the parent name, so
 * this is the case in any well-defined JSON file. If the key is already
 * in use, this method will fail.
 *
 * @param key       The key identifying the layout information
 * @param anchor    The anchor point to use
 * @param offset    The offset from the anchor point
 *
 * @return true if the layout information was assigned to that key
 */
bool AnchoredLayout::addRelative(const std::string key, Anchor anchor, const Vec2& offset) {
    auto last = _entries.find(key);
    if (last != _entries.end()) {
        return false;
    }
    
    Entry entry;
    entry.anchor = anchor;
    entry.x_offset = offset.x;
    entry.y_offset = offset.y;
    entry.absolute = false;
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
bool AnchoredLayout::remove(const std::string key) {
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
 * This layout manager will searches for those children that are registered
 * with it. For those children, it repositions and/or resizes them according
 * to the layout information.
 *
 * This manager attaches a child node to one of nine "anchors" in the parent
 * (corners, sides or middle), together with a percentage (or absolute)
 * offset.  As the parent grows or shinks, the child will move according to
 * its anchor.  For example, nodes in the center will stay centered, while
 * nodes on the left side will move to keep the appropriate distance from
 * the left side. In fact, the stretching behavior is very similar to that
 * of a {@link NinePatch}.
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
void AnchoredLayout::layout(Node* node) {
    auto kids = node->getChildren();
    Size size = node->getContentSize();
    for(auto it = kids.begin(); it != kids.end(); ++it) {
        auto jt = _entries.find((*it)->getName());
        if (jt != _entries.end()) {
            Entry entry = jt->second;
            Vec2 offset;
            offset.x = entry.absolute ? entry.x_offset : entry.x_offset*size.width;
            offset.y = entry.absolute ? entry.y_offset : entry.y_offset*size.height;
            placeNode(it->get(), entry.anchor, Rect(Vec2::ZERO,size), offset);
        }
    }
}
