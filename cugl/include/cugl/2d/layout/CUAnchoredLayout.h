//
//  CUAnchoredLayout.h
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
#ifndef __CU_ANCHORED_LAYOUT_H__
#define __CU_ANCHORED_LAYOUT_H__
#include <cugl/2d/layout/CULayout.h>
#include <unordered_map>

namespace  cugl {
/**
 * This class provides an anchored layout manager.
 *
 * An anchored layout attaches a child node to one of nine "anchors" in the
 * parent (corners, sides or middle), together with a percentage (or absolute)
 * offset.  As the parent grows or shinks, the child will move according to its
 * anchor.  For example, nodes in the center will stay centered, while nodes on
 * the left side will move to keep the appropriate distance from the left side.
 * In fact, the stretching behavior is very similar to that of a
 * {@link NinePatch}.
 *
 * Layout information is indexed by key.  To look up the layout information
 * of a scene graph node, we use the name of the node.  This requires all
 * nodes to have unique names.  The {@link SceneLoader} prefixes all child
 * names by the parent name, so this is the case in any well-defined JSON file.
 */
class AnchoredLayout : public Layout {
protected:
    /**
     * This inner class stores the layout information as a struct.
     *
     * Offsets may either be absolute or relative.  A relative offset is
     * expressed as a percentage of width or height.  An absolute offset is
     * expressed in terms of Node coordinates.
     */
    class Entry {
    public:
        /** The x offset from the anchor in absolute or relative units */
        float x_offset;
        /** The y offset from the anchor in absolute or relative units */
        float y_offset;
        /** The associated anchor point */
        Anchor anchor;
        /** Whether to use an absolute offset instead of a relative (percentage) one */
        bool absolute;
    };
    
    /** The map of keys to layout information */
    std::unordered_map<std::string,Entry> _entries;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degenerate layout manager with no data.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    AnchoredLayout() {}
    
    /**
     * Deletes this layout manager, disposing of all resources.
     */
    ~AnchoredLayout() { dispose(); }
    
    /**
     * Initializes a new layout manager with the given JSON specificaton.
     *
     * The JSON specification format is simple. It only supports one (required)
     * attribute: 'type'.  The type should specify "anchored".
     *
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const std::shared_ptr<JsonValue>& data) override { return true; }
    
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
    static std::shared_ptr<AnchoredLayout> alloc() {
        std::shared_ptr<AnchoredLayout> result = std::make_shared<AnchoredLayout>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated layout manager with the given JSON specificaton.
     *
     * The JSON specification format is simple. It only supports one (required)
     * attribute: 'type'.  The type should specify "anchored".
     *
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated layout manager with the given JSON specificaton.
     */
    static std::shared_ptr<AnchoredLayout> allocWithData(const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<AnchoredLayout> result = std::make_shared<AnchoredLayout>();
        return (result->initWithData(data) ? result : nullptr);
    }
    
#pragma mark Layout
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
    virtual bool add(const std::string key, const std::shared_ptr<JsonValue>& data) override;
    
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
    bool addAbsolute(const std::string key, Anchor anchor, const Vec2& offset);

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
    bool addRelative(const std::string key, Anchor anchor, const Vec2& offset);

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
    virtual void layout(Node* node) override;

};

}
#endif /* __CU_ANCHORED_LAYOUT_H__ */
