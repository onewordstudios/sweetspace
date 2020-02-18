//
//  CUNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a (2d) scene graph node.  It uses an anchor
//  based approach, much like Cocos2d.  However, it is much more streamlined
//  and removes a lot of the Cocos2d nonsense when it comes to transforms.
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
//  Version: 6/27/16

#ifndef __CU_NODE_H__
#define __CU_NODE_H__

#include <cugl/math/cu_math.h>
#include <cugl/renderer/CUSpriteBatch.h>
#include <cugl/util/CUDebug.h>
#include <cugl/assets/CUJsonValue.h>
#include <vector>
#include <string>

namespace cugl {
    
/** Forward references for scene loading support */
class Scene;
class SceneLoader;
class Layout;
    
/**
 * This class provides a 2d scene graph node.
 *
 * A base node is a rectangular space that can contain other (transformed)
 * nodes.  Each Node forms a its own coordinate space.  All rendering takes
 * place inside of this coordinate space. By default, a Node does not render
 * anything, but it does provide rendering support for subclasses via the
 * draw() method.
 *
 * Rendering happens by traversing the the scene graph using an "Pre-Order" tree
 * traversal algorithm ( https://en.wikipedia.org/wiki/Tree_traversal#Pre-order ).
 * That means that parents are always draw before (and behind children).  The
 * children of each sub tree are ordered by z-value (or by the order added).
 * 
 * One of the important pitfalls of the Node class is that the position of a
 * a Node does not necessarily define the position of its Node-space origin.
 * that means that transforms applied to the Node do not necessarily leave the
 * Node position unaffected.
 *
 * That is why it is important to understand the difference between the anchor
 * and the origin.  Each Node is a rectangular bounding-box, and its origin is
 * the bottom left corner.  However, the anchor can be any point inside of the
 * bounding box.  The scale and angle are applied to the anchor point, NOT the
 * Node origin. If you want to apply a traditional transform to the Node origin,
 * you will need to use the setAlternateTransform() option.
 * 
 * In addition, when a Node is resized, the resizing takes place relative to 
 * the anchor point. That means that the position is unchanged, but the node 
 * origin may move.  However, if the anchor is altered, the Node itself does
 * not move while the position changes.  While this may seem counter-intuitive,
 * this is traditional sprite behavior, and scene graph nodes are just a 
 * generalization of a sprite.
 */
class Node {
#pragma mark Values
protected:
    /**
     * The position of the node.
     *
     * This position is specified in the coordinate system defined by the parent.
     * Together with the anchor point, it species the origin of the node space.
     */
    Vec2   _position;
    
    /**
     * The anchor point of the node.
     *
     * The anchor point is a percentage of the node bounding box.  Together with
     * the position, it species the origin of the node space.
     */
    Vec2  _anchor;
    
    /** The (untransformed) size of this node. */
    Size   _contentSize;
    
    /** The color to tint this node. This color is white by default. */
    Color4 _tintColor;
    /** Whether to blend our color with that of our parent. */
    bool  _hasParentColor;
    /** Whether this node is visible */
    bool  _isVisible;
    
    /**
     * The scale of this node.
     *
     * The scale determines the transform from node space about the anchor. It
     * is applied before any rotation. The node coordinate space is unchanged.
     */
    Vec2  _scale;

    /**
     * The rotation angle of this node.
     *
     * The angle is measured in degrees and is counter-clockwise from the
     * x-axis. The rotation is about the anchor point, not the node origin.
     * It is applied after the scale, but before the post-transform. The node 
     * coordinate space is unchanged.
     */
    float  _angle;
    
    /**
     * The alternate transform of this node.
     *
     * This value allows you to perform more arbitrary transformations of the
     * node in parent space.  Unlike scaling and rotation, this transform is
     * applied to the node space directly (not with respect to the anchor).
     * Hence this transform is applied before any other ones.
     */
    Mat4  _transform;
    
    /** Whether or not to use the alternate transform */
    bool _useTransform;
    
    /**
     * The cached local transform matrix.
     *
     * This matrix specifes the transform from node space to parent space.
     * Depending on the settings, it is either the scale and rotation or the
     * alternate transform.
     */
    Mat4  _combined;
    
    /** The array of children nodes */
    std::vector<std::shared_ptr<Node>> _children;

    /** A weaker pointer to the parent (or null if root) */
    Node* _parent;
    /** A weaker pointer to the scene (or null if not in a scene) */
    Scene* _graph;
    /** A layout manager for complex scene graphs */
    std::shared_ptr<Layout> _layout;

    /** The (current) child offset of this node (-1 if root) */
    int _childOffset;

    /**
     * An identifying tag.
     *
     * This value is used to quickly identify the child of a node.  To work
     * properly, a tag should be unique within a scene graph.  It is 0 if
     * undefined.
     */
    unsigned int _tag;
    
    /**
     * A descriptive identifying tag.
     *
     * Like tag, this value is used to quickly identify the child of a node.
     * However, it is more descriptive, and so is useful in debugging.
     */
    std::string _name;
    
    /**
     * A cached has value of _name.
     *
     * This value is used to speed up look-ups by string.
     */
    size_t _hashOfName;
    
    /** The z-order of this node */
    int  _zOrder;
    /** Indicates whether or not the z-order is currently violated */
    bool _zDirty;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized node.
     *
     * You must initialize this Node before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the 
     * heap, use one of the static constructors instead.
     */
    Node();
    
    /**
     * Deletes this node, disposing all resources
     */
    ~Node() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed Node can be safely reinitialized. Any children owned by this
     * node will be released.  They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a Node that is still currently inside of
     * a scene graph.
     */
    virtual void dispose();

    /**
     * Initializes a node at the world origin.
     *
     * The node has both position and size (0,0).
     *
     * @return true if initialization was successful.
     */
    virtual bool init() {
        return initWithPosition(0, 0);
    }

    
    /**
     * Initializes a node at the given position.
     *
     * The node has size (0,0).  As a result, the position is identified with
     * the origin of the node space.
     *
     * @param pos   The origin of the node in parent space
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithPosition(const Vec2& pos);
    
    /**
     * Initializes a node at the given position.
     *
     * The node has size (0,0).  As a result, the position is identified with
     * the origin of the node space.
     *
     * @param x     The x-coordinate of the node in parent space
     * @param y     The y-coordinate of the node in parent space
     *
     * @return true if initialization was successful.
     */
    bool initWithPosition(float x, float y) {
        return initWithPosition(Vec2(x,y));
    }

    /**
     * Initializes a node with the given size.
     *
     * The size defines the content size. The bounding box of the node is 
     * (0,0,width,height). Hence the node is anchored in the center and has 
     * position (width/2,height/2) in the parent space.  The node origin is
     * the (0,0) at the bottom left corner of the bounding box.
     *
     * @param size  The size of the node in parent space
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithBounds(const Size& size);

    /**
     * Initializes a node with the given size.
     *
     * The size defines the content size. The bounding box of the node is
     * (0,0,width,height). Hence the node is anchored in the center and has
     * position (width/2,height/2) in the parent space.  The node origin is
     * the (0,0) at the bottom left corner of the bounding box.
     *
     * @param width     The width of the node in parent space
     * @param height    The height of the node in parent space
     *
     * @return true if initialization was successful.
     */
    bool initWithBounds(float width, float height) {
        return initWithBounds(Size(width,height));
    }

    /**
     * Initializes a node with the given bounds.
     *
     * The rectangle origin is the bottom left corner of the node in parent
     * space, and corresponds to the origin of the Node space. The size defines 
     * its content width and height. The node is anchored in the center and has 
     * position origin-(width/2,height/2) in parent space.
     *
     * Because the bounding box is explicit, this is the preferred initializer
     * for Nodes that will explicitly contain other Nodes.
     *
     * @param rect  The bounds of the node in parent space
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithBounds(const Rect& rect);
    
    /**
     * Initializes a node with the given bounds.
     *
     * The rectangle origin is the bottom left corner of the node in parent
     * space, and corresponds to the origin of the Node space. The size defines
     * its content width and height. The node is anchored in the center and has
     * position origin-(width/2,height/2) in parent space.
     *
     * Because the bounding box is explicit, this is the preferred initializer
     * for Nodes that will explicitly contain other Nodes.
     *
     * @param x         The x-coordinate of the node origin in parent space
     * @param y         The y-coordinate of the node origin in parent space
     * @param width     The width of the node in parent space
     * @param height    The height of the node in parent space
     *
     * @return true if initialization was successful.
     */
    bool initWithBounds(float x, float y, float width, float height) {
        return initWithBounds(Rect(x,y,width,height));
    }

    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports the
     * following attribute values:
     *
     *      "position": A two-element number array
     *      "size":     A two-element number array
     *      "anchor":   A two-element number array representing the anchor point
     *      "color":    A four-element integer array.  Values should be 0..255
     *      "scale":    Either a two-element number array or a single number
     *      "angle":    A number, representing the rotation in DEGREES, not radians
     *      "visible":  A boolean value, representing if the node is visible
     *
     * All attributes are optional.  There are no required attributes.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data);

    /**
     * Performs a shallow copy of this Node into dst.
     *
     * No children from this node are copied, and no children of dst are 
     * modified. In addition, the parents of both Nodes are unchanged. However,
     * all other attributes of this node are copied.
     *
     * @param dst   The Node to copy into
     *
     * @return A reference to dst for chaining.
     */
    virtual Node* copy(Node* dst);

    
#pragma mark -
#pragma mark Static Constructors
    
    /**
     * Returns a newly allocated node at the world origin.
     *
     * The node has both position and size (0,0).
     *
     * @return a newly allocated node at the world origin.
     */
    static std::shared_ptr<Node> alloc() {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node at the given position.
     *
     * The node has size (0,0).  As a result, the position is identified with
     * the origin of the node space.
     *
     * @param pos   The origin of the node in parent space
     *
     * @return a newly allocated node at the given position.
     */
    static std::shared_ptr<Node> allocWithPosition(const Vec2& pos) {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->initWithPosition(pos) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node at the given position.
     *
     * The node has size (0,0).  As a result, the position is identified with
     * the origin of the node space.
     *
     * @param x     The x-coordinate of the node in parent space
     * @param y     The y-coordinate of the node in parent space
     *
     * @return a newly allocated node at the given position.
     */
    static std::shared_ptr<Node> allocWithPosition(float x, float y) {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->initWithPosition(x,y) ? result : nullptr);
    }

    /**
     * Returns a newly allocated node with the given size.
     *
     * The size defines the content size. The bounding box of the node is
     * (0,0,width,height). Hence the node is anchored in the center and has
     * position (width/2,height/2) in the parent space.  The node origin is
     * the (0,0) at the bottom left corner of the bounding box.
     *
     * @param size  The size of the node in parent space
     *
     * @return a newly allocated node with the given size.
     */
    static std::shared_ptr<Node> allocWithBounds(const Size& size) {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->initWithBounds(size) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given size.
     *
     * The size defines the content size. The bounding box of the node is
     * (0,0,width,height). Hence the node is anchored in the center and has
     * position (width/2,height/2) in the parent space.  The node origin is
     * the (0,0) at the bottom left corner of the bounding box.
     *
     * @param width     The width of the node in parent space
     * @param height    The height of the node in parent space
     *
     * @return a newly allocated node with the given size.
     */
    static std::shared_ptr<Node> allocWithBounds(float width, float height) {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->initWithBounds(width,height) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given bounds.
     *
     * The rectangle origin is the bottom left corner of the node in parent
     * space, and corresponds to the origin of the Node space. The size defines
     * its content width and height. The node is anchored in the center and has
     * position origin-(width/2,height/2) in parent space.
     *
     * Because the bounding box is explicit, this is the preferred constructor
     * for Nodes that will explicitly contain other Nodes.
     *
     * @param rect  The bounds of the node in parent space
     *
     * @return a newly allocated node with the given bounds.
     */
    static std::shared_ptr<Node> allocWithBounds(const Rect& rect) {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->initWithBounds(rect) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given bounds.
     *
     * The rectangle origin is the bottom left corner of the node in parent
     * space, and corresponds to the origin of the Node space. The size defines
     * its content width and height. The node is anchored in the center and has
     * position origin-(width/2,height/2) in parent space.
     *
     * Because the bounding box is explicit, this is the preferred constructor
     * for Nodes that will explicitly contain other Nodes.
     *
     * @param x         The x-coordinate of the node origin in parent space
     * @param y         The y-coordinate of the node origin in parent space
     * @param width     The width of the node in parent space
     * @param height    The height of the node in parent space
     *
     * @return a newly allocated node with the given bounds.
     */
    static std::shared_ptr<Node> allocWithBounds(float x, float y, float width, float height) {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->initWithBounds(x,y,width,height) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specification.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link SceneLoader}.  This JSON format supports the
     * following attribute values:
     *
     *      "position": A two-element number array
     *      "size":     A two-element number array
     *      "anchor":   A two-element number array representing the anchor point
     *      "color":    A four-element integer array.  Values should be 0..255
     *      "scale":    A two-element number array
     *      "angle":    A number, representing the rotation in DEGREES, not radians
     *      "visible":  A boolean value, representing if the node is visible
     *
     * All attributes are optional.  There are no required attributes.
     *
     * @param loader    The scene loader passing this JSON file
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specification.
     */
    static std::shared_ptr<Node> allocWithData(const SceneLoader* loader,
                                               const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<Node> result = std::make_shared<Node>();
        return (result->initWithData(loader,data) ? result : nullptr);
    }

#pragma mark -
#pragma mark Identifiers
    /**
     * Returns a tag that is used to identify the node easily.
     *
     * This tag is used to quickly access a child node, since child position
     * may change. To work properly, a tag should be unique within a scene 
     * graph.  It is 0 if undefined.
     *
     * @return a tag that is used to identify the node easily.
     */
    unsigned int getTag() const { return _tag; }
    
    /**
     * Sets a tag that is used to identify the node easily.
     *
     * This tag is used to quickly access a child node, since child position
     * may change. To work properly, a tag should be unique within a scene
     * graph.  It is 0 if undefined.
     *
     * @param tag   A tag that is used to identify the node easily.
     */
    void setTag(unsigned int tag) { _tag = tag; }
    
    /**
     * Returns a string that is used to identify the node.
     *
     * This name is used to access a child node, since child position may 
     * change. In addition, the name is useful for debugging. To work properly, 
     * a name should be unique within a scene graph. It is empty if undefined.
     *
     * @return a string that is used to identify the node.
     */
    const std::string& getName() const { return _name; }
    
    /**
     * Sets a string that is used to identify the node.
     *
     * This name is used to access a child node, since child position may
     * change. In addition, the name is useful for debugging. To work properly,
     * a name should be unique within a scene graph. It is empty if undefined.
     *
     * @param name  A string that is used to identify the node.
     */
    void setName(const std::string& name) {
        _name = name;
        _hashOfName = std::hash<std::string>()(_name);
    }

    /**
     * Returns a string representation of this node for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this node for debuggging purposes.
     */
    virtual std::string toString(bool verbose = false) const;
    
    /** Cast from a Node to a string. */
    operator std::string() const { return toString(); }
    
    
#pragma mark -
#pragma mark Position
    /**
     * Returns the position of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system.  The relationship between the position and Node space is 
     * determined by the anchor point.  See {@link getAnchor()} for more 
     * details.
     *
     * @return the position of the node in its parent's coordinate system.
     */
    const Vec2& getPosition() const { return _position; }
    
    /**
     * Sets the position of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system.  The relationship between the position and Node space is
     * determined by the anchor point.  See {@link getAnchor()} for more
     * details.
     *
     * @param position  The position of the node in its parent's coordinate system.
     */
    void setPosition(const Vec2 &position) { setPosition(position.x,position.y); }
    
    /**
     * Sets the position of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system.  The relationship between the position and Node space is
     * determined by the anchor point.  See {@link getAnchor()} for more
     * details.
     *
     * @param  x    The x-coordinate of the node in its parent's coordinate system.
     * @param  y    The x-coordinate of the node in its parent's coordinate system.
     */
    void setPosition(float x, float y);
    
    /**
     * Returns the x-coordinate of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system.  The relationship between the position and Node space is
     * determined by the anchor point.  See {@link getAnchor()} for more
     * details.
     *
     * @return the x-coordinate of the node in its parent's coordinate system.
     */
    float getPositionX(void) const { return getPosition().x; }
    
    /**
     * Sets the x-coordinate of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system.  The relationship between the position and Node space is
     * determined by the anchor point.  See {@link getAnchor()} for more
     * details.
     *
     * @param  x    The x-coordinate of the node in its parent's coordinate system.
     */
    void setPositionX(float x) { setPosition(x,getPositionY()); }
    
    /**
     * Returns the y-coordinate of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system.  The relationship between the position and Node space is
     * determined by the anchor point.  See {@link getAnchor()} for more
     * details.
     *
     * @return the y-coordinate of the node in its parent's coordinate system.
     */
    float getPositionY(void) const { return getPosition().y; }

    /**
     * Sets the y-coordinate of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system.  The relationship between the position and Node space is
     * determined by the anchor point.  See {@link getAnchor()} for more
     * details.
     *
     * @param  y    The y-coordinate of the node in its parent's coordinate system.
     */
    void setPositionY(float y) { setPosition(getPositionX(),y); }
    
    /**
     * Returns the position of the anchor point node in OpenGL space.
     *
     * Note that this is the position of the anchor point.  This is not the
     * same as the location of the node origin in world space.
     *
     * @return the position of this node in OpenGL space.
     */
    Vec2 getWorldPosition() const {
        return nodeToWorldCoords(_anchor*getContentSize());
    }


#pragma mark -
#pragma mark Size
    /**
     * Returns the untransformed size of the node.
     *
     * The content size remains the same no matter how the node is scaled or
     * rotated. All nodes must have a size, though it may be degenerate (0,0).
     *
     * @return the untransformed size of the node.
     */
    const Size& getContentSize() const { return _contentSize; }

    /**
     * Sets the untransformed size of the node.
     *
     * The content size remains the same no matter how the node is scaled or
     * rotated. All nodes must have a size, though it may be degenerate (0,0).
     *
     * Changing the size of a rectangle will not change the position of the 
     * node.  However, if the anchor is not the bottom-left corner, it will
     * change the origin.  The Node will grow out from an anchor on an edge,
     * and equidistant from an anchor in the center.
     *
     * @param size  The untransformed size of the node.
     */
    virtual void setContentSize(const Size& size);

    /**
     * Sets the untransformed size of the node.
     *
     * The content size remains the same no matter how the node is scaled or
     * rotated. All nodes must have a size, though it may be degenerate (0,0).
     *
     * Changing the size of a rectangle will not change the position of the
     * node.  However, if the anchor is not the bottom-left corner, it will
     * change the origin.  The Node will grow out from an anchor on an edge,
     * and equidistant from an anchor in the center.
     *
     * @param width     The untransformed width of the node.
     * @param height    The untransformed height of the node.
     */
    virtual void setContentSize(float width, float height) {
        setContentSize(Size(width, height));
    }
    
    /**
     * Returns the untransformed width of the node.
     *
     * The content width remains the same no matter how the node is scaled or
     * rotated. All nodes must have a width, though it may be degenerate (0).
     *
     * @return the untransformed width of the node.
     */
    float getContentWidth() const { return getContentSize().width; }
    
    /**
     * Sets the untransformed width of the node.
     *
     * The content width remains the same no matter how the node is scaled or
     * rotated. All nodes must have a width, though it may be degenerate (0).
     *
     * Changing the size of a rectangle will not change the position of the
     * node.  However, if the anchor is not the bottom-left corner, it will
     * change the origin.  The Node will grow out from an anchor on an edge,
     * and equidistant from an anchor in the center.
     *
     * @param width     The untransformed width of the node.
     */
    void setContentWidth(float width) { setContentSize(width,getContentHeight()); }
    
    /**
     * Returns the untransformed height of the node.
     *
     * The content height remains the same no matter how the node is scaled or
     * rotated. All nodes must have a height, though it may be degenerate (0).
     *
     * @return the untransformed height of the node.
     */
    float getContentHeight() const { return getContentSize().height; }

    /**
     * Sets the untransformed height of the node.
     *
     * The content height remains the same no matter how the node is scaled or
     * rotated. All nodes must have a height, though it may be degenerate (0).
     *
     * Changing the size of a rectangle will not change the position of the
     * node.  However, if the anchor is not the bottom-left corner, it will
     * change the origin.  The Node will grow out from an anchor on an edge,
     * and equidistant from an anchor in the center.
     *
     * @param height    The untransformed height of the node.
     */
    void setContentHeight(float height) { setContentSize(getContentWidth(),height); }
    
    /**
     * Returns the transformed size of the node.
     *
     * This method returns the size of the axis-aligned bounding box that
     * contains the transformed node in its parents coordinate system.  If the
     * node is rotated, this may not be a perfect fit of the transformed 
     * contents.
     *
     * @return the transformed size of the node.
     */
    Size getSize() const;
    
    /**
     * Returns the transformed width of the node.
     *
     * This method returns the width of the axis-aligned bounding box that
     * contains the transformed node in its parents coordinate system.  If the
     * node is rotated, this may not be a perfect fit of the transformed
     * contents.
     *
     * @return the transformed width of the node.
     */
    float getWidth() const { return getSize().width; }

    /**
     * Returns the transformed height of the node.
     *
     * This method returns the height of the axis-aligned bounding box that
     * contains the transformed node in its parents coordinate system.  If the
     * node is rotated, this may not be a perfect fit of the transformed
     * contents.
     *
     * @return the transformed height of the node.
     */
    float getHeight() const { return getSize().height; }
    
    /**
     * Returns an AABB (axis-aligned bounding-box) in the parent's coordinates.
     *
     * This method returns the minimal axis-aligned bounding box that contains 
     * the transformed node in its parents coordinate system.  If the node is 
     * rotated, this may not be a perfect fit of the transformed contents.
     *
     * @return An AABB (axis-aligned bounding-box) in the parent's coordinates.
     */
    Rect getBoundingBox() const {
        return getNodeToParentTransform().transform(Rect(Vec2::ZERO, getContentSize()));
    }
    
#pragma mark -
#pragma mark Anchors
    /**
     * Sets the anchor point in percentages.
     *
     * The anchor point defines the relative origin of Node with respect to its
     * parent.  It is a "pin" where the Node is attached to its parent.  In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * The anchorPoint is normalized, like a percentage. (0,0) means the 
     * bottom-left corner and (1,1) means the top-right corner. There are many
     * anchor point constants defined in {@link Vec2}.  However, there is
     * nothing preventing an anchor point higher than (1,1) or lower than (0,0).
     *
     * The default anchorPoint is (0.5,0.5), so it starts in the center of
     * the node. Changing the anchor will not move the contents of the node in
     * the parent space, but it will change the value of the Node position.
     *
     * @param anchor    The anchor point of node.
     */
    virtual void setAnchor(const Vec2& anchor);
    
    /**
     * Sets the anchor point in percentages.
     *
     * The anchor point defines the relative origin of Node with respect to its
     * parent.  It is a "pin" where the Node is attached to its parent.  In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * The anchorPoint is normalized, like a percentage. (0,0) means the
     * bottom-left corner and (1,1) means the top-right corner. There are many
     * anchor point constants defined in {@link Vec2}.  However, there is
     * nothing preventing an anchor point higher than (1,1) or lower than (0,0).
     *
     * The default anchorPoint is (0.5,0.5), so it starts in the center of
     * the node. Changing the anchor will not move the contents of the node in
     * the parent space, but it will change the value of the Node position.
     *
     * @param x     The horizontal anchor percentage.
     * @param y     The vertical anchor percentage.
     */
    void setAnchor(float x, float y) { setAnchor(Vec2(x,y)); }
    
    /**
     * Returns the anchor point in percent.
     *
     * The anchor point defines the relative origin of Node with respect to its
     * parent.  It is a "pin" where the Node is attached to its parent.  In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * The anchorPoint is normalized, like a percentage. (0,0) means the
     * bottom-left corner and (1,1) means the top-right corner. There are many
     * anchor point constants defined in {@link Vec2}.  However, there is
     * nothing preventing an anchor point higher than (1,1) or lower than (0,0).
     *
     * The default anchorPoint is (0.5,0.5), so it starts in the center of
     * the node. Changing the anchor will not move the contents of the node in
     * the parent space, but it will change the value of the Node position.
     *
     * @return The anchor point in percent.
     */
    const Vec2& getAnchor() const { return _anchor; }

    /**
     * Returns the anchor point in pixels.
     *
     * The anchor point defines the relative origin of Node with respect to its
     * parent.  It is a "pin" where the Node is attached to its parent.  In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * This version of the anchor point is multiplied by the size of the Node
     * bounding box. Hence (0,0) means the bottom-left corner and (width,height) 
     * means the top-right corner.
     *
     * @return The anchor point in percent.
     */
    Vec2 getAnchorInPixels() { return _anchor * _contentSize; }

    
#pragma mark -
#pragma mark Visibility
    /**
     * Returns the color tinting this node.
     *
     * This color will be multiplied with the parent (this node on top) if
     * hasRelativeColor() is true.
     *
     * The default color is white, which means that all children have their
     * natural color.
     *
     * @return the color tinting this node.
     */
    Color4 getColor() const { return _tintColor; }
    
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
    virtual void setColor(Color4 color) { _tintColor = color; }

    /**
     * Returns the absolute color tinting this node.
     *
     * If hasRelativeColor() is true, this value is the base color multiplied by
     * the absolute color of its parent. If hasRelativeColor() is false, it is
     * the same as the tint color.
     *
     * @return the absolute color tinting this node.
     */
    Color4 getAbsoluteColor();

    /**
     * Returns true if the node is visible.
     *
     * If a node is not visible, then it is not drawn.  This means that its
     * children are not visible as well, regardless of their visibility settings.
     * The default value is true, making the node visible.
     *
     * @return true if the node is visible.
     */
    bool isVisible() const { return _isVisible; }
    
    /**
     * Sets whether the node is visible.
     *
     * If a node is not visible, then it is not drawn.  This means that its
     * children are not visible as well, regardless of their visibility settings.
     * The default value is true, making the node visible.
     *
     * @param visible   true if the node is visible.
     */
    void setVisible(bool visible) { _isVisible = visible; }
    
    /**
     * Returns true if this node is tinted by its parent.
     *
     * If this value is true, the base color is multiplied with the absolute
     * color of its parent when rendering happens.  Otherwise, the base color 
     * is used alone.
     *
     * @return true if this node is tinted by its parent.
     */
    bool hasRelativeColor() { return _hasParentColor; }
    
    /**
     * Sets whether this node is tinted by its parent.
     *
     * If this value is true, the base color is multiplied with the absolute
     * color of its parent when rendering happens.  Otherwise, the base color
     * is used alone.
     *
     * @param flag  Whether this node is tinted by its parent.
     */
    void setRelativeColor(bool flag) { _hasParentColor = flag; }
    
    
#pragma mark -
#pragma mark Transforms
    /**
     * Returns the non-uniform scaling factor for this node about the anchor.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the 
     * origin is in the bottom left corner.  Scaling is first, before any other
     * transforms.
     *
     * @return the non-uniform scaling factor for this node about the anchor
     */
    const Vec2& getScale() const  { return _scale;   }
    
    /**
     * Returns the x-axis scaling factor for this node.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the
     * origin is in the bottom left corner.  Scaling is first, before any other
     * transforms.
     *
     * @return the x-axis scaling factor for this node.
     */
    float getScaleX() const { return _scale.x; }
    
    /**
     * Returns the y-axis scaling factor for this node.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the
     * origin is in the bottom left corner.  Scaling is first, before any other
     * transforms.
     *
     * @return the y-axis scaling factor for this node.
     */
    float getScaleY() const { return _scale.y; }

    /**
     * Sets the uniform scaling factor for this node.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the
     * origin is in the bottom left corner.  Scaling is first, before any other
     * transforms.
     *
     * @param scale the uniform scaling factor.
     */
    void setScale(float scale) {
        _scale.set(scale,scale);
        if (!_useTransform) updateTransform();
    }

    /**
     * Sets the non-uniform scaling factor for this node.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the
     * origin is in the bottom left corner.  Scaling is first, before any other
     * transforms.
     *
     * @param vec   the non-uniform scaling factor.
     */
    void setScale(const Vec2& vec) {
        _scale = vec;
        if (!_useTransform) updateTransform();
    }

    /**
     * Sets the non-uniform scaling factor for this node.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the
     * origin is in the bottom left corner.  Scaling is first, before any other
     * transforms.
     *
     * @param sx    the x-axis scaling factor.
     * @param sy    the y-axis scaling factor.
     */
    void setScale(float sx, float sy) {
        _scale.set(sx,sy);
        if (!_useTransform) updateTransform();
    }
    
    /**
     * Returns the rotation angle of this node.
     *
     * This value rotates the node about the anchor, with the anchor unmoved.  
     * The angle is measured in radians , counter-clockwise from the x-axis.
     * Rotation is applied after scaling.
     *
     * @return the rotation angle of this node.
     */
    float getAngle() { return _angle; }

    /**
     * Sets the rotation angle of this node.
     *
     * This value rotates the node about the anchor, with the anchor unmoved.
     * The angle is measured in radians , counter-clockwise from the x-axis.
     * Rotation is applied after scaling.
     *
     * @param angle the rotation angle of this node.
     */
    void setAngle(float angle) {
        _angle = angle;
        if (!_useTransform) updateTransform();
    }
    
    /**
     * Returns the alternate transform of this node.
     *
     * Unlike the built-in scaling and rotation, this transform is applied
     * to the coordinate space of the Node (e.g. with the origin in the bottom
     * left corner).
     *
     * Scaling/rotation and the alternate transform do no play nice with each
     * other.  It does not make sense to stack them on top of one another in
     * either direction.  Hence, you should only use of of the two.  See the
     * method {@link chooseAlternateTransform(bool) } to choose.
     *
     * @return the alternate transform of this node.
     */
    const Mat4& getAlternateTransform() { return _transform; }
    
    /**
     * Sets the alternate transform of this node.
     *
     * Unlike the built-in scaling and rotation, this transform is applied
     * to the coordinate space of the Node (e.g. with the origin in the bottom
     * left corner).
     *
     * Scaling/rotation and the alternate transform do no play nice with each
     * other.  It does not make sense to stack them on top of one another in
     * either direction.  Hence, you should only use of of the two.  See the
     * method {@link chooseAlternateTransform(bool) } to choose.
     *
     * @param transform the alternate transform of this node.
     */
    void setAlternateTransform(const Mat4& transform) {
        _transform = transform;
        updateTransform();
    }
    
    /**
     * Returns true if the Node is using the alternate transform.
     *
     * Unlike the built-in scaling and rotation, the alternate transform is
     * applied to the coordinate space of the Node (e.g. with the origin in 
     * the bottom left corner).
     *
     * Scaling/rotation and the alternate transform do no play nice with each
     * other.  It does not make sense to stack them on top of one another in
     * either direction.  Hence, you should only use of of the two. This method
     * returns true if the alternate transform is in use.
     *
     * @return true if the Node is using the alternate transform.
     */
    bool withAlternateTransform() { return _useTransform; }

    /**
     * Set whether the Node is using the alternate transform.
     *
     * Unlike the built-in scaling and rotation, the alternate transform is
     * applied to the coordinate space of the Node (e.g. with the origin in
     * the bottom left corner).
     *
     * Scaling/rotation and the alternate transform do no play nice with each
     * other.  It does not make sense to stack them on top of one another in
     * either direction.  Hence, you should only use of of the two. This method
     * determines whether the alternate transform is in use.
     *
     * @param active    Whether the Node is using the alternate transform.
     */
    void chooseAlternateTransform(bool active) {
        _useTransform = active; updateTransform();
    }

    /**
     * Returns the matrix transforming node space to parent space.
     *
     * This value is the node's transform.  It is either computed from the scale
     * and rotation about the anchor, or the alternate transform, as determined
     * by defined by  {@link chooseAlternateTransform(bool) }.
     *
     * @return the matrix transforming node space to parent space.
     */
    const Mat4& getNodeToParentTransform() const { return _combined; }
    
    /**
     * Returns the matrix transforming parent space to node space.
     *
     * This value is the inverse that node's transform.  It is either computed
     * from the scale and rotation about the anchor, or the alternate transform, 
     * as determined by defined by  {@link chooseAlternateTransform(bool) }.
     *
     * @return the matrix transforming parent space to node space.
     */
    Mat4 getParentToNodeTransform() const { return _combined.getInverse(); }
    
    /**
     * Returns the matrix transforming node space to world space.
     *
     * This matrix is used to convert node coordinates into OpenGL coordinates.
     * It is the recursive (left-multiplied) node-to-parent transforms of all 
     * of its ancestors.
     *
     * @return the matrix transforming node space to world space.
     */
    Mat4 getNodeToWorldTransform() const;
    
    /**
     * Returns the matrix transforming node space to world space.
     *
     * This matrix is used to convert OpenGL coordinates into node coordinates.
     * This method is useful for converting global positions like touches
     * or mouse clicks. It is the recursive (right-multiplied) parent-to-node
     * transforms of all of its ancestors.
     *
     * @return the matrix transforming node space to world space.
     */
    Mat4 getWorldToNodeTransform() const {
        return getNodeToWorldTransform().getInverse();
    }
    
    /**
     * Converts a screen position to node (local) space coordinates.
     *
     * This method is useful for converting global positions like touches
     * or mouse clicks, which are represented in screen coordinates. Screen
     * coordinates typically have the origin in the top left.
     *
     * The screen coordinate system is defined by the scene's camera. The method
     * uses the camera to convert into world space, and then converts from world
     * space into not (local) space.
     *
     * This method returns the original point if there is no active scene.
     *
     * @param screenPoint   An position on the screen
     *
     * @return A point in node (local) space coordinates.
     */
    Vec2 screenToNodeCoords(const Vec2& screenPoint) const;

    
    /**
     * Converts an OpenGL position to node (local) space coordinates.
     *
     * See getWorldtoNodeTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param worldPoint    An OpenGL position.
     *
     * @return A point in node (local) space coordinates.
     */
    Vec2 worldToNodeCoords(const Vec2& worldPoint) const {
        return getWorldToNodeTransform().transform(worldPoint);
    }

    /**
     * Converts an node (local) position to screen coordinates.
     *
     * This method is useful for converting back to global positions like
     * touches or mouse clicks, which are represented in screen coordinates. 
     * Screen coordinates typically have the origin in the top left.
     *
     * The screen coordinate system is defined by the scene's camera. The method
     * converts the node point into world space, and then uses the camera to
     * convert into screen space.
     *
     * This method returns the original point if there is no active scene.
     *
     * @param nodePoint     A local position.
     *
     * @return A point in screen coordinates.
     */
    Vec2 nodeToScreenCoords(const Vec2& nodePoint) const;
    
    /**
     * Converts an node (local) position to OpenGL coordinates.
     *
     * See getNodeToWorldTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param nodePoint     A local position.
     *
     * @return A point in OpenGL coordinates.
     */
    Vec2 nodeToWorldCoords(const Vec2& nodePoint) const {
        return getNodeToWorldTransform().transform(nodePoint);
    }
    
    /**
     * Converts an parent space position to node (local) space coordinates.
     *
     * See getParentToNodeTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param parentPoint   A parent position.
     *
     * @return A point in node (local) space coordinates.
     */
    Vec2 parentToNodeCoords(const Vec2& parentPoint) const {
        return getParentToNodeTransform().transform(parentPoint);
    }
    
    /**
     * Converts an node (local) space position to parent coordinates.
     *
     * See getNodeToParentTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param nodePoint     A local position.
     *
     * @return A point in parent space coordinates.
     */
    Vec2 nodeToParentCoords(const Vec2& nodePoint) const {
        return getNodeToParentTransform().transform(nodePoint);
    }

#pragma mark -
#pragma mark Z-order
    /**
     * Sets the value used to 'sort' a node relative to its siblings.
     *
     * The z-order determines the drawing order of the children.  If two nodes
     * have the same z-order, they are drawn in the order that they were added
     * to their parent.
     *
     * The z-order is not an actual z-value. Attempting to interpret it other
     * wise would result in an interleaving of children, which is not what
     * we want in a scene graph.
     *
     * Sorting does not happen automatically (except within a {@link Scene}). 
     * It is the responsibility of a user to sort the z-order before a call to 
     * render. Otherwise, render order will be in the unsorted order.
     *
     * @param z The local Z order value.
     */
    void setZOrder(int z);
    
    /**
     * Returns the value used to 'sort' a node relative to its siblings.
     *
     * The z-order determines the drawing order of the children.  If two nodes
     * have the same z-order, they are drawn in the order that they were added
     * to their parent.
     *
     * The z-order is not an actual z-value. Attempting to interpret it other
     * wise would result in an interleaving of children, which is not what
     * we want in a scene graph.
     *
     * Sorting does not happen automatically (except within a {@link Scene}).
     * It is the responsibility of a user to sort the z-order before a call to
     * render. Otherwise, render order will be in the unsorted order.
     *
     * @return the value used to 'sort' a node relative to its siblings.
     */
    int getZOrder() const { return _zOrder; }
    
    /**
     * Returns whether the children of this node needs resorting.
     *
     * The children of a node may need to be resorted whenever a child is added,
     * or whenever the z-value of a child is changed.
     *
     * This value satisfies the following invariant: if a Node is dirty and 
     * needs resorting, then so are all of its ancestors (including any 
     * associated {@link Scene}). Our methods guarantee this invariant, so 
     * that the method isZDirty() always returns the correct value.
     *
     * Sorting does not happen automatically (except within a {@link Scene}).
     * It is the responsibility of a user to sort the z-order before a call to
     * render. Otherwise, render order will be in the unsorted order.
     *
     * @return whether the children of this node needs resorting.
     */
    bool isZDirty() const { return _zDirty; }

    /**
     * Resorts the children of this node according to z-value.
     *
     * If two children have the same z-value, their relative order is
     * preserved to what it was before the sort. This method should be
     * called before rendering.
     *
     * Resorting is done recursively down a tree for each child that is dirty
     * and needs resorting.  We guarantee that Nodes will not be resorted
     * unless necessary, because of the following invariant: if a Node is dirty
     * and needs resorting, then so are all of its ancestors (including any
     * associated {@link Scene}).
     *
     * Sorting does not happen automatically (except within a {@link Scene}).
     * It is the responsibility of a user to call this method before rendering. 
     * Otherwise, render order will be in the unsorted order.
     */
    void sortZOrder();
    
    
#pragma mark -
#pragma mark Scene Graph
    /**
     * Returns the number of children of this node.
     *
     * @return The number of children of this node.
     */
    size_t getChildCount() const { return _children.size(); }

    /**
     * Returns the child at the given position.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * @param pos   The child position.
     *
     * @return the child at the given position.
     */
    std::shared_ptr<Node> getChild(unsigned int pos);

    /**
     * Returns the child at the given position.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * @param pos   The child position.
     *
     * @return the child at the given position.
     */
    const std::shared_ptr<Node>& getChild(unsigned int pos) const;
    
    /**
     * Returns the child at the given position, typecast to a shared T pointer.
     *
     * This method is provided to simplify the polymorphism of a scene graph.
     * While all children are a subclass of type Node, you may want to access
     * them by their specific subclass.  If the child is not an instance of 
     * type T (or a subclass), this method returns nullptr.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * @param pos   The child position.
     *
     * @return the child at the given position, typecast to a shared T pointer.
     */
    template <typename T>
    inline std::shared_ptr<T> getChild(unsigned int pos) const {
        return std::dynamic_pointer_cast<T>(getChild(pos));
    }

    /**
     * Returns the (first) child with the given tag.
     *
     * If there is more than one child of the given tag, it returns the first
     * one that is found. For the base Node class, children are always 
     * enumerated in the order that they are added.  However, this is not 
     * guaranteed for subclasses of Node. Hence it is very important that
     * tags be unique.
     *
     * @param tag   An identifier to find the child node.
     *
     * @return the (first) child with the given tag.
     */
    std::shared_ptr<Node> getChildByTag(unsigned int tag) const;
    
    /**
     * Returns the (first) child with the given tag, typecast to a shared T pointer.
     *
     * This method is provided to simplify the polymorphism of a scene graph.
     * While all children are a subclass of type Node, you may want to access
     * them by their specific subclass.  If the child is not an instance of
     * type T (or a subclass), this method returns nullptr.
     *
     * If there is more than one child of the given tag, it returns the first
     * one that is found. For the base Node class, children are always
     * enumerated in the order that they are added.  However, this is not
     * guaranteed for subclasses of Node. Hence it is very important that
     * tags be unique.
     *
     * @param tag   An identifier to find the child node.
     *
     * @return the (first) child with the given tag, typecast to a shared T pointer.
     */
    template <typename T>
    inline std::shared_ptr<T> getChildByTag(unsigned int tag) const {
        return std::dynamic_pointer_cast<T>(getChildByTag(tag));
    }
    
    /**
     * Returns the (first) child with the given name.
     *
     * If there is more than one child of the given name, it returns the first
     * one that is found. For the base Node class, children are always
     * enumerated in the order that they are added.  However, this is not
     * guaranteed for subclasses of Node. Hence it is very important that
     * names be unique.
     *
     * @param name  An identifier to find the child node.
     *
     * @return the (first) child with the given name.
     */
    std::shared_ptr<Node> getChildByName(const std::string& name) const;
    
    /**
     * Returns the (first) child with the given name, typecast to a shared T pointer.
     *
     * This method is provided to simplify the polymorphism of a scene graph.
     * While all children are a subclass of type Node, you may want to access
     * them by their specific subclass.  If the child is not an instance of
     * type T (or a subclass), this method returns nullptr.
     *
     * If there is more than one child of the given name, it returns the first
     * one that is found. For the base Node class, children are always
     * enumerated in the order that they are added.  However, this is not
     * guaranteed for subclasses of Node. Hence it is very important that
     * names be unique.
     *
     * @param name  An identifier to find the child node.
     *
     * @return the (first) child with the given name, typecast to a shared T pointer.
     */
    template <typename T>
    inline std::shared_ptr<T> getChildByName(const std::string& name) const {
        return std::dynamic_pointer_cast<T>(getChildByName(name));
    }

    /**
     * Returns the list of the node's children.
     *
     * @return the list of the node's children.
     */
    std::vector<std::shared_ptr<Node>> getChildren() { return _children; }

    /**
     * Returns the list of the node's children.
     *
     * @return the list of the node's children.
     */
    const std::vector<std::shared_ptr<Node>>& getChildren() const { return _children; }
    
    /**
     * Adds a child to this node.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * @param child A child node.
     */
    void addChild(std::shared_ptr<Node> child) {
        addChild(child,child->_zOrder);
    }
    
    /**
     * Adds a child to this node with the given z-order.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * The z-order overrides what z-value was previously in the child node.
     *
     * @param child A child node.
     * @param zval  The (new) child z-order.
     */
    virtual void addChild(const std::shared_ptr<Node>& child, int zval);
    
    /**
     * Adds a child to this node with the given tag.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * @param child A child node.
     * @param tag   An integer to identify the node easily.
     */
    void addChildWithTag(const std::shared_ptr<Node>& child, unsigned int tag) {
        addChild(child);
        child->setTag(tag);
    }
    
    /**
     * Adds a child to this node with the given tag and z-order
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * The z-order overrides what z-value was previously in the child node.
     *
     * @param child A child node.
     * @param tag   An integer to identify the node easily.
     * @param zval  The (new) child z-order.
     */
    void addChildWithTag(const std::shared_ptr<Node>& child, unsigned int tag, int zval) {
        addChild(child,zval);
        child->setTag(tag);
    }

    /**
     * Adds a child to this node with the given name.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * @param child A child node.
     * @param name  A string to identify the node.
     */
    void addChildWithName(const std::shared_ptr<Node>& child, const std::string &name) {
        addChild(child);
        child->setName(name);
    }
    
    /**
     * Adds a child to this node with the given name and z-order
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * The z-order overrides what z-value was previously in the child node.
     *
     * @param child A child node.
     * @param name  A string to identify the node.
     * @param zval  The (new) child z-order.
     */
    void addChildWithName(const std::shared_ptr<Node>& child, const std::string &name, int zval) {
        addChild(child,zval);
        child->setName(name);
    }
    
    /**
     * Swaps the current child child1 with the new child child2.
     *
     * If inherit is true, the children of child1 are assigned to child2 after
     * the swap; this value is false by default.  The purpose of this value is 
     * to allow transition nodes in the middle of the scene graph.
     *
     * This method is undefined if child1 is not a child of this node.
     *
     * @param child1    The current child of this node
     * @param child2    The child to swap it with.
     * @param inherit   Whether the new child should inherit the children of child1.
     */
    void swapChild(const std::shared_ptr<Node>& child1, const std::shared_ptr<Node>& child2, bool inherit=false);
    
    /**
     * Returns a (weak) pointer to the parent node.
     *
     * The purpose of this pointer is to climb back up the scene graph tree.
     * No child asserts ownership of its parent.
     *
     * @return a (weak) pointer to the parent node.
     */
    Node* getParent() { return _parent; }
    
    /**
     * Returns a (weak) pointer to the parent node.
     *
     * The purpose of this pointer is to climb back up the scene graph tree.
     * No child asserts ownership of its parent.
     *
     * @return a (weak) pointer to the parent node.
     */
    const Node* getParent() const { return _parent; }
    
    /**
     * Returns a (weak) pointer to the scene graph.
     *
     * The purpose of this pointer is to climb back up to the root of the
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @return a (weak) pointer to the scene graph.
     */
    Scene* getScene() { return _graph; }
    
    /**
     * Returns a (weak) pointer to the scene graph.
     *
     * The purpose of this pointer is to climb back up to the root of the 
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @return a (weak) pointer to the scene graph.
     */
    const Scene* getScene() const { return _graph; }

    /**
     * Removes this node from its parent node.
     * 
     * If the node has no parent, nothing happens.
     */
    void removeFromParent() { if (_parent) { _parent->removeChild(_childOffset); } }
    
    /**
     * Removes the child at the given position from this Node.
     *
     * Removing a child alters the position of every child after it.  Hence
     * it is unsafe to cache child positions.
     *
     * @param pos   The position of the child node which will be removed.
     */
    virtual void removeChild(unsigned int pos);
    
    /**
     * Removes a child from this Node. 
     *
     * Removing a child alters the position of every child after it.  Hence
     * it is unsafe to cache child positions.
     *
     * If the child is not in this node, nothing happens.
     *
     * @param child The child node which will be removed.
     */
    void removeChild(const std::shared_ptr<Node>& child);
    
    /**
     * Removes a child from the Node by tag value.
     *
     * If there is more than one child of the given tag, it removes the first
     * one that is found. For the base Node class, children are always
     * enumerated in the order that they are added.  However, this is not
     * guaranteed for subclasses of Node. Hence it is very important that
     * tags be unique.
     *
     * @param tag   An integer to identify the node easily.
     */
    void removeChildByTag(unsigned int tag);
    
    /**
     * Removes a child from the Node by name.
     *
     * If there is more than one child of the given name, it removes the first
     * one that is found. For the base Node class, children are always
     * enumerated in the order that they are added.  However, this is not
     * guaranteed for subclasses of Node. Hence it is very important that
     * names be unique.
     *
     * @param name  A string to identify the node.
     */
    void removeChildByName(const std::string &name);
    
    /**
     * Removes all children from this Node.
     */
    virtual void removeAllChildren();
    
    
#pragma mark -
#pragma mark Rendering
    /**
     * Draws this Node and all of its children with the given SpriteBatch.
     *
     * You almost never need to override this method.  You should override the
     * method draw(shared_ptr<SpriteBatch>,const Mat4&,Color4) if you need to
     * define custom drawing code.
     *
     * @param batch     The SpriteBatch to draw with.
     * @param transform The global transformation matrix.
     * @param tint      The tint to blend with the Node color.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint);

    /**
     * Draws this Node and all of its children with the given SpriteBatch.
     *
     * You almost never need to override this method.  You should override the
     * method draw(shared_ptr<SpriteBatch>,const Mat4&,Color4) if you need to
     * define custom drawing code.
     *
     * @param batch     The SpriteBatch to draw with.
     */
    virtual void render(const std::shared_ptr<SpriteBatch>& batch) {
        render(batch,Mat4::IDENTITY,Color4::WHITE);
    }

    /**
     * Draws this Node via the given SpriteBatch.
     *
     * This method only worries about drawing the current node.  It does not
     * attempt to render the children.
     *
     * This is the method that you should override to implement your custom
     * drawing code.  You are welcome to use any OpenGL commands that you wish.
     * You can even skip use of the SpriteBatch.  However, if you do so, you
     * must flush the SpriteBatch by calling end() at the start of the method.
     * in addition, you should remember to call begin() at the start of the
     * method.
     *
     * This method provides the correct transformation matrix and tint color.
     * You do not need to worry about whether the node uses relative color.  
     * This method is called by render() and these values are guaranteed to be 
     * correct.  In addition, this method does not need to check for visibility, 
     * as it is guaranteed to only be called when the node is visible.
     *
     * @param batch     The SpriteBatch to draw with.
     * @param transform The global transformation matrix.
     * @param tint      The tint to blend with the Node color.
     */
    virtual void draw(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) {}
    
    
#pragma mark -
#pragma mark Layout Automation
    /**
     * Returns the layout manager for this node
     *
     * We had originally intended to completely decouple layout managers from
     * nodes.  However, nodes (including layout assignemnts) are typically
     * built bottom-up, while layout must happen top down to correctly resize
     * elements.  Therefore, we do allow the addition of an optional layout
     * manager.
     *
     * @return the layout manager for this node
     */
    const std::shared_ptr<Layout>& getLayout() const { return _layout; }
    
    /**
     * Sets the layout manager for this node
     *
     * We had originally intended to completely decouple layout managers from
     * nodes.  However, nodes (including layout assignemnts) are typically
     * built bottom-up, while layout must happen top down to correctly resize
     * elements.  Therefore, we do allow the addition of an optional layout
     * manager.
     *
     * Changing the layout manager does not reperform layout.  You must call
     * {@link doLayout()} to do this.
     *
     * @param layout	The layout manager for this node
     */
    void setLayout(const std::shared_ptr<Layout>& layout) { _layout = layout; }
    
    /**
     * Arranges the child of this node using the layout manager.
     *
     * This process occurs recursively and top-down. A layout manager may end
     * up resizing the children.  That is why the parent must finish its layout
     * before we can apply a layout manager to the children.
     */
    virtual void doLayout();

private:
#pragma mark -
#pragma mark Internal Helpers
    /**
     * Sets whether the children of this node needs resorting.
     *
     * The children of a node may need to be resorted whenever a child is added,
     * or whenever the z-value of a child is changed.
     *
     * This value satisfies the following invariant: if a Node is dirty and
     * needs resorting, then so are all of its ancestors (including any
     * associated {@link Scene}). Our methods guarantee this invariant, so
     * that the method isZDirty() always returns the correct value.
     *
     * Sorting does not happen automatically (except within a {@link Scene}).
     * It is the responsibility of a user to sort the z-order before a call to
     * render. Otherwise, render order will be in the unsorted order.
     *
     * @param value Whether the children of this node needs resorting.
     */
    void setZDirty(bool value);

    /**
     * Sets the parent node.
     *
     * The purpose of this pointer is to climb back up the scene graph tree.
     * No child asserts ownership of its parent.
     *
     * @param parent    A pointer to the parent node.
     */
    void setParent(Node* parent) { _parent = parent; }

    /**
     * Sets the scene graph.
     *
     * The purpose of this pointer is to climb back up to the root of the
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @param scene    A pointer to the scene graph.
     */
    void setScene(Scene* scene) { _graph = scene; }

    /**
     * Recursively sets the scene graph for this node and all its children.
     *
     * The purpose of this pointer is to climb back up to the root of the
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @param scene    A pointer to the scene graph.
     */
    void pushScene(Scene* scene);

    /**
     * Returns true if sibling a is less than b in sorted z-order.
     *
     * This method is used by std::sort to sort the children.
     *
     * @param a The first child
     * @param b The second child
     *
     * @return true if sibling a is less than b in sorted z-order.
     */
    static bool compareNodeSibs(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b);
    
    /**
     * Updates the node to parent transform.
     *
     * This transform is defined by scaling, rotation, the post-rotation 
     * transform, and positional translation, in that order.
     */
    virtual void updateTransform();

    // Copying is only allowed via shared pointer.
    CU_DISALLOW_COPY_AND_ASSIGN(Node);
    
    friend class Scene;
};


}
#endif /* __CU_NODE_H__ */
