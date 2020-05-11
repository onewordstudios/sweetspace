//
//  CUNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a (2d) scene graph node.  It uses an anchor
//  based approach, much like Cocos2d.  However, it is much more streamlined
//  and removes a lot of the Cocos2d nonsense when it comes to transforms.
//
//  This is the base class for any scene graph node, and should be extended
//  for custom scene graph functionality.
//
//  This module is based on an original file from GamePlay3D: http://gameplay3d.org.
//  It has been modified to support the CUGL framework.
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

#include <cugl/2d/CUNode.h>
#include <cugl/2d/CUScene.h>
#include <cugl/2d/layout/CULayout.h>
#include <cugl/renderer/CUCamera.h>
#include <cugl/util/CUStrings.h>
#include <cugl/assets/CUAssetManager.h>
#include <sstream>
#include <algorithm>

using namespace cugl;

#pragma mark Constructors
/**
 * Creates an uninitialized node.
 *
 * You must initialize this Node before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
 * heap, use one of the static constructors instead.
 */
Node::Node() :
_tag(0),
_name(""),
_hashOfName(0),
_tintColor(Color4::WHITE),
_hasParentColor(true),
_isVisible(true),
_anchor(Vec2::ANCHOR_BOTTOM_LEFT),
_scale(Vec2::ONE),
_angle(0),
_useTransform(false),
_parent(nullptr),
_graph(nullptr),
_zOrder(0),
_zDirty(false),
_childOffset(-2) {}

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
bool Node::initWithPosition(const Vec2& pos) {
    CUAssertLog(_childOffset == -2, "Attempting to reinitialize a Node");
    _position = pos;
    _combined = Mat4::IDENTITY;
    _combined.m[12] = pos.x;
    _combined.m[13] = pos.y;
    _childOffset = -1;
    return true;
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
bool Node::initWithBounds(const Size& size) {
    CUAssertLog(_childOffset == -2, "Attempting to reinitialize a Node");
    _contentSize = size;
    _position = 0.5f*size;
    _combined = Mat4::IDENTITY;
    _childOffset = -1;
    return true;
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
bool Node::initWithBounds(const Rect& rect) {
    CUAssertLog(_childOffset == -2, "Attempting to reinitialize a Node");
    _position = rect.origin + 0.5f*rect.size;
    _contentSize = rect.size;
    _combined = Mat4::IDENTITY;
    _combined.m[12] = rect.origin.x;
    _combined.m[13] = rect.origin.y;
    _childOffset = -1;
    return true;
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
 *      "scale":    A two-element number array
 *      "angle":    A number, representing the rotation in DEGREES, not radians
 *      "visible":  A boolean value
 *
 * All attributes are optional.  There are no required attributes.
 *
 * @param loader    The scene loader passing this JSON file
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool Node::initWithData(const SceneLoader* loader, const std::shared_ptr<JsonValue>& data) {
    CUAssertLog(_childOffset == -2, "Attempting to reinitialize a Node");
    if (!data) {
        return initWithPosition(0, 0);
    }
    _combined = Mat4::IDENTITY;
    _childOffset = -1;
    
    // It is VERY important to do this first
    Vec2 value;
    if (data->has("anchor")) {
        JsonValue* pos = data->get("anchor").get();
        CUAssertLog(pos->size() >= 2, "'anchor' must be a two element number array");
        value.x = pos->get(0)->asFloat(0.0f);
        value.y = pos->get(1)->asFloat(0.0f);
        setAnchor(value);
    }
    
    if (data->has("position")) {
        JsonValue* pos = data->get("position").get();
        CUAssertLog(pos->size() >= 2, "'position' must be a two element number array");
        value.x = pos->get(0)->asFloat(0.0f);
        value.y = pos->get(1)->asFloat(0.0f);
        setPosition(value);
    }
    
    if (data->has("color")) {
        JsonValue* col = data->get("color").get();
        CUAssertLog(col->size() >= 4, "'color' must be a four element number array");
        _tintColor.r = col->get(0)->asInt(0);
        _tintColor.g = col->get(1)->asInt(0);
        _tintColor.b = col->get(2)->asInt(0);
        _tintColor.a = col->get(3)->asInt(0);
    }
    
    _isVisible = data->getBool("visible",true);

    bool transform = false;
    if (data->has("size")) {
        transform = true;
        JsonValue* size = data->get("size").get();
        CUAssertLog(size->size() >= 2, "'size' must be a two element number array");
        _contentSize.width  = size->get(0)->asFloat(0.0f);
        _contentSize.height = size->get(1)->asFloat(0.0f);
    }
    
    if (data->has("scale")) {
        transform = true;
        JsonValue* scale = data->get("scale").get();
        if (scale->size() > 0) {
            _scale.x = scale->get(0)->asFloat(1.0f);
            _scale.y = scale->get(1)->asFloat(1.0f);
        } else {
            _scale.x = scale->asFloat(1.0f);
            _scale.y = _scale.x;
        }
    }

    if (data->has("angle")) {
        transform = true;
        _angle = data->getFloat("angle",0.0f);
        _angle *= M_PI/180.0f;
    }
    
    if (transform && !_useTransform) updateTransform();
    
    return true;
}

/**
 * Disposes all of the resouces used by this node.
 *
 * A disposed Node can be safely reinitialized. Any children owned by this
 * node will be released.  They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a Node that is still currently inside of
 * a scene graph.
 */
void Node::dispose() {
    if (_childOffset >= 0) {
        removeFromParent();
    }
    removeAllChildren();
    _position = Vec2::ZERO;
    _anchor   = Vec2::ANCHOR_CENTER;
    _contentSize = Size::ZERO;
    _tintColor = Color4::WHITE;
    _hasParentColor = true;
    _isVisible = true;
    _scale = Vec2::ONE;
    _angle = 0;
    _transform = Mat4::IDENTITY;
    _useTransform = false;
    _combined = Mat4::IDENTITY;
    _parent = nullptr;
    _graph = nullptr;
    _childOffset = -2;
    _tag = 0;
    _name = "";
    _hashOfName = 0;
    _zOrder = 0;
    _zDirty = false;
}

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
Node* Node::copy(Node* dst) {
    dst->_position = _position;
    dst->_anchor   = _anchor;
    dst->_contentSize = _contentSize;
    dst->_tintColor = _tintColor;
    dst->_hasParentColor = _hasParentColor;
    dst->_isVisible = _isVisible;
    dst->_scale = _scale;
    dst->_angle = _angle;
    dst->_transform = _transform;
    dst->_useTransform = _useTransform;
    dst->_combined = _combined;
    dst->_tag = _tag;
    dst->_name = _name;
    dst->_hashOfName = _hashOfName;

    dst->setZOrder(_zOrder);
    return dst;
}

#pragma mark -
#pragma mark Attributes

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
void Node::setPosition(float x, float y) {
    _combined.m[12] += (x-_position.x);
    _combined.m[13] += (y-_position.y);
    _position.set(x,y);
}

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
void Node::setContentSize(const Size& size) {
    _position += _anchor*(size-_contentSize);
    _contentSize.set(size);
    if (!_useTransform) updateTransform();
    if (_layout) {
        doLayout();
    }
}

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
void Node::setAnchor(const Vec2& anchor) {
    _position += (anchor-_anchor)*_contentSize;
    _anchor = anchor;
    if (!_useTransform) updateTransform();
}

/**
 * Returns a string representation of this vector for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this vector for debuggging purposes.
 */
std::string Node::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Node(tag:" : "(tag:");
    ss <<  cugl::to_string(_tag);
    ss << ", name:" << _name;
    ss << ", children:" << cugl::to_string((Uint64)_children.size());
    ss << ")";
    if (verbose) {
        ss << "\n";
        for(auto it = _children.begin(); it != _children.end(); ++it) {
            ss << "  " << (*it)->toString(verbose);
        }
    }

    return ss.str();
}

#pragma mark -
#pragma mark Transforms
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
Size Node::getSize() const {
    Rect bounds = getBoundingBox();
    return bounds.size;
}

/**
 * Returns the matrix transforming node space to world space.
 *
 * This matrix is used to convert node coordinates into OpenGL coordinates.
 * It is the recursive (left-multiplied) transforms of all of its descendents.
 *
 * @return the matrix transforming node space to world space.
 */
Mat4 Node::getNodeToWorldTransform() const {
    Mat4 result = _combined;
    if (_parent) {
        // Multiply on left
        Mat4::multiply(result,_parent->getNodeToWorldTransform(),&result);
    }
    return result;
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
Vec2 Node::screenToNodeCoords(const Vec2& screenPoint) const {
    if (_graph == nullptr) {
        return screenPoint;
    }
    return worldToNodeCoords(_graph->getCamera()->screenToWorldCoords(screenPoint));
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
 * @param nodePoint     A local position.
 *
 * @return A point in screen coordinates.
 */
Vec2 Node::nodeToScreenCoords(const Vec2& nodePoint) const {
    if (_graph == nullptr) {
        return nodePoint;
    }
    return (_graph->getCamera()->worldToScreenCoords(nodeToWorldCoords(nodePoint)));
}


/**
 * Updates the node to parent transform.
 *
 * This transform is defined by scaling, rotation, the post-rotation
 * transform, and positional translation, in that order.
 */
void Node::updateTransform() {
    Vec2 offset = _anchor*getContentSize();
    if (_useTransform) {
        _combined = _transform;
    } else {
        Mat4::createTranslation(-offset.x, -offset.y, 0.0f, &_combined);
        _combined.scale(_scale.x, _scale.y, 1.0f);
        _combined.rotateZ(_angle);
        _combined.translate(offset.x, offset.y, 0.0f);
    }
    _combined.m[12] += _position.x-offset.x;
    _combined.m[13] += _position.y-offset.y;
}


#pragma mark -
#pragma mark Scene Graph
/**
 * Returns the child at the given position.
 *
 * For the base Node class, children are always enumerated in the order
 * that they are added.  However, this is not guaranteed for subclasses of
 * Node.  Hence you should attempt to retrieve a child by tag or by name.
 *
 * @param pos   The child position.
 *
 * @return the child at the given position.
 */
std::shared_ptr<Node> Node::getChild(unsigned int pos) {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    return _children[pos];
}

/**
 * Returns the child at the given position.
 *
 * For the base Node class, children are always enumerated in the order
 * that they are added.  However, this is not guaranteed for subclasses of
 * Node.  Hence you should attempt to retrieve a child by tag or by name.
 *
 * @param pos   The child position.
 *
 * @return the child at the given position.
 */
const std::shared_ptr<Node>& Node::getChild(unsigned int pos) const {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    return _children[pos];
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
std::shared_ptr<Node> Node::getChildByTag(unsigned int tag) const {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        if ((*it)->getTag() == tag) {
            return *it;
        }
    }
    return nullptr;
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
std::shared_ptr<Node> Node::getChildByName(const std::string& name) const {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        if ((*it)->getName() == name) {
            return *it;
        }
    }
    return nullptr;
}

/**
 * Adds a child to this node with the given z-order.
 *
 * For the base Node class, children are always enumerated in the order
 * that they are added.  However, this is not guaranteed for subclasses of
 * Node.  Hence you should attempt to retrieve a child by tag or by name.
 *
 * The z-order overrides what z-value was previously in the child node.
 *
 * @param child A child node.
 * @param zval  The (new) child z-order.
 */
void Node::addChild(const std::shared_ptr<Node>& child, int zval) {
    CUAssertLog(child->_childOffset == -1, "The child is already in a scene graph");
    CUAssertLog(child->_graph == nullptr,  "The child is already in a scene graph");
    child->_childOffset = (unsigned int)_children.size();
    child->_zOrder = zval;
    
    // Check to see if we need resorting (including if child is dirty)
    if (!_zDirty) {
        if (child->_childOffset > 0) {
            int zOther = _children.back()->getZOrder();
            setZDirty(zOther > zval || child->isZDirty());
        } else {
            setZDirty(child->isZDirty());
        }
    }
    
    // Add the child
    _children.push_back(child);
    child->setParent(this);
    child->pushScene(_graph);
    
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
void Node::swapChild(const std::shared_ptr<Node>& child1, const std::shared_ptr<Node>& child2, bool inherit) {
    _children[child1->_childOffset] = child2;
    child2->_childOffset = child1->_childOffset;
    child2->setParent(this);
    child1->setParent(nullptr);
    child2->pushScene(_graph);
    child1->pushScene(nullptr);
    
    // Check if we are dirty and/or inherit children
    bool childdirty = false;
    if (inherit) {
        std::vector<std::shared_ptr<Node>> grands = child1->getChildren();
        child1->removeAllChildren();
        for(auto it = grands.begin(); it != grands.end(); ++it) {
            child2->addChild(*it);
        }
        childdirty = child2->isZDirty();
    }
    setZDirty(_zDirty || child1->_zOrder != child2->_zOrder || childdirty);
}

/**
 * Removes the child at the given position from this Node.
 *
 * Removing a child alters the position of every child after it.  Hence
 * it is unsafe to cache child positions.
 *
 * @param pos   The position of the child node which will be removed.
 */
void Node::removeChild(unsigned int pos) {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    std::shared_ptr<Node> child = _children[pos];
    child->setParent(nullptr);
    child->pushScene(nullptr);
    child->_childOffset = -1;
    for(int ii = pos; ii < _children.size()-1; ii++) {
        _children[ii] = _children[ii+1];
        _children[ii]->_childOffset = ii;
    }
    _children.resize(_children.size()-1);
}

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
void Node::removeChild(const std::shared_ptr<Node>& child) {
    CUAssertLog(_children[child->_childOffset] == child, "The child is not in this scene graph");
    removeChild(child->_childOffset);
}

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
void Node::removeChildByTag(unsigned int tag) {
    std::shared_ptr<Node> child = getChildByTag(tag);
    if (child != nullptr) {
        removeChild(child->_childOffset);
    }
}

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
void Node::removeChildByName(const std::string &name) {
    std::shared_ptr<Node> child = getChildByName(name);
    if (child != nullptr) {
        removeChild(child->_childOffset);
    }
}

/**
 * Removes all children from this Node.
 */
void Node::removeAllChildren() {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->setParent(nullptr);
        (*it)->_childOffset = -1;
        (*it)->pushScene(nullptr);
    }
    _children.clear();
    _zDirty = false;
}

/**
 * Recursively sets the scene graph for this node and all its children.
 *
 * The purpose of this pointer is to climb back up to the root of the
 * scene graph tree. No node asserts ownership of its scene.
 *
 * @param parent    A pointer to the scene graph.
 */
void Node::pushScene(Scene* scene) {
    setScene(scene);
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->pushScene(scene);
    }
}

/**
 * Arranges the child of this node using the layout manager.
 *
 * This process occurs recursively and top-down. A layout manager may end
 * up resizing the children.  That is why the parent must finish its layout
 * before we can apply a layout manager to the children.
 */
void Node::doLayout() {
    if (_layout) {
        _layout->layout(this);
    }
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->doLayout();
    }
}

#pragma mark -
#pragma mark Z-Order

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
 * @param z The local Z order value.
 */
void Node::setZOrder(int z) {
    _zOrder = z;
    
    // Notify the parent if we have a problem.
    if (_parent != nullptr && !_parent->_zDirty) {
        int size = (int)_parent->_children.size();
        bool zd = (_childOffset > 0 && _parent->_children[_childOffset-1]->_zOrder > z);
        zd = zd || (_childOffset < size-1 && _parent->_children[_childOffset+1]->_zOrder < z);
        _parent->setZDirty(zd);
    } else if (_graph != nullptr && !_graph->isZDirty()) {
        int size = (int)_graph->getChildren().size();
        bool zd = (_childOffset > 0 && _graph->getChild(_childOffset-1)->_zOrder > z);
        zd = zd || (_childOffset < size-1 && _parent->getChild(_childOffset+1)->_zOrder < z);
        _graph->setZDirty(zd);
    }
}

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

void Node::setZDirty(bool value) {
    _zDirty = value;
    if (value) {
        if (_parent != nullptr) {
            _parent->setZDirty(value);
        } else if (_graph != nullptr) {
            _graph->setZDirty(value);
        }
    }
}

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
bool Node::compareNodeSibs(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
    return a->_zOrder < b->_zOrder ||
    (a->_zOrder == b->_zOrder && a->_childOffset < b->_childOffset);
}

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
void Node::sortZOrder() {
    if (_zDirty) {
        std::sort(_children.begin(),_children.end(),Node::compareNodeSibs);
        // Fix the offsets
        int ii = 0;
        for(auto it = _children.begin(); it != _children.end(); ++it ) {
            (*it)->_childOffset = ii++;
        }
        _zDirty = false;
        // Invariant guarantees this is the only way they are dirty
        for(auto it = _children.begin(); it != _children.end(); ++it ) {
            (*it)->sortZOrder();
        }
    }
}

#pragma mark -
#pragma mark Rendering

/**
 * Draws this Node via the given SpriteBatch.
 *
 * If you create custom Nodes, you must override this method with your
 * own draw code.  The base method simply draws all children.
 *
 * The transformation matrix will be multiplied on the right by the parent
 * transform of this Node.  In addition, if hasRelativeColor() is true, it
 * will blend the Node color with the given tint.
 *
 * @param batch     The SpriteBatch to draw with.
 * @param matrix    The global transformation matrix.
 * @param tint      The tint to blend with the Node color.
 */
void Node::render(const std::shared_ptr<SpriteBatch>& batch, const Mat4& transform, Color4 tint) {
    if (!_isVisible) { return; }
    
    Mat4 matrix;
    Mat4::multiply(_combined,transform,&matrix);
    Color4 color = _tintColor;
    if (_hasParentColor) {
        color *= tint;
    }

    draw(batch,matrix,color);
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->render(batch, matrix, color);
    }
}

/**
 * Returns the absolute color tinting this node.
 *
 * If hasRelativeColor() is true, this value is the base color multiplied by
 * the absolute color of its parent. If hasRelativeColor() is false, it is the 
 * same as the tint color.
 *
 * @return the absolute color tinting this node.
 */
Color4 Node::getAbsoluteColor() {
    Color4 result = _tintColor;
    if (_parent && _hasParentColor) {
        result *= _parent->getAbsoluteColor();
    }
    return result;
}

