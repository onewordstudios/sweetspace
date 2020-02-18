//
//  CUScene.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the root node of a (2d) scene graph.  After
//  much debate, we have decided to decouple this from the application class
//  (which is different than Cocos2d).
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
//  Version: 7/1/16

#include <cugl/2d/CUScene.h>
#include <cugl/util/CUStrings.h>
#include <sstream>
#include <algorithm>

using namespace cugl;

/**
 * Creates a new degenerate Scene on the stack.
 *
 * The scene has no camera and must be initialized.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
Scene::Scene() :
_camera(nullptr),
_name(""),
_color(Color4::WHITE),
_blendEquation(GL_FUNC_ADD),
_srcFactor(GL_SRC_ALPHA),
_dstFactor(GL_ONE_MINUS_SRC_ALPHA),
_zDirty(false),
_zSort(false),
_active(false)
{}

/**
 * Disposes all of the resources used by this scene.
 *
 * A disposed Scene can be safely reinitialized. Any children owned by this
 * scene will be released.  They will be deleted if no other object owns them.
 */
void Scene::dispose() {
    removeAllChildren();
    _camera = nullptr;
    _name = "";
    _color = Color4::WHITE;
    _zDirty = false;
    _zSort  = false;
    _active = false;
}

/**
 * Initializes a Scene with the given viewport.
 *
 * Offseting the viewport origin has little affect on the Scene in general.
 * It only affects the coordinate conversion methods {@link Camera#project()}
 * and {@link Camera#unproject()}. It is supposed to represent the offset
 * of the viewport in a larger canvas.
 *
 * @param x         The viewport x offset
 * @param y         The viewport y offset
 * @param width     The viewport width
 * @param height    The viewport height
 *
 * @return true if initialization was successful.
 */
bool Scene::init(float x, float y, float width, float height) {
    _camera = OrthographicCamera::allocOffset(x, y, width, height);
    _active = _camera != nullptr;
    return _active;
}


#pragma mark -
#pragma mark Attributes

/**
 * Returns the camera for this scene.
 *
 * @return the camera for this scene.
 */
std::shared_ptr<Camera> Scene::getCamera() {
    return std::dynamic_pointer_cast<Camera>(_camera);
}

/**
 * Returns the camera for this scene.
 *
 * @return the camera for this scene.
 */
const std::shared_ptr<Camera> Scene::getCamera() const {
    return std::dynamic_pointer_cast<Camera>(_camera);
}

/**
 * Returns a string representation of this scene for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this scene for debuggging purposes.
 */
std::string Scene::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Scene(name:" : "(name:");
    ss << _name << ")";
    return ss.str();
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
std::shared_ptr<Node> Scene::getChild(unsigned int pos) {
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
const std::shared_ptr<Node>& Scene::getChild(unsigned int pos) const {
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
std::shared_ptr<Node> Scene::getChildByTag(unsigned int tag) const  {
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
std::shared_ptr<Node> Scene::getChildByName(const std::string& name) const {
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
void Scene::addChild(const std::shared_ptr<Node>& child, int zval) {
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
    child->setParent(nullptr);
    child->pushScene(this);
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
void Scene::swapChild(const std::shared_ptr<Node>& child1, const std::shared_ptr<Node>& child2,
                      bool inherit) {
    _children[child1->_childOffset] = child2;
    child2->_childOffset = child1->_childOffset;
    child2->setParent(nullptr);
    child1->setParent(nullptr);
    child2->pushScene(this);
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
void Scene::removeChild(unsigned int pos) {
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
void Scene::removeChild(const std::shared_ptr<Node>& child) {
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
void Scene::removeChildByTag(unsigned int tag) {
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
void Scene::removeChildByName(const std::string &name) {
    std::shared_ptr<Node> child = getChildByName(name);
    if (child != nullptr) {
        removeChild(child->_childOffset);
    }
}


/**
 * Removes all children from this Node.
 */
void Scene::removeAllChildren() {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->setParent(nullptr);
        (*it)->_childOffset = -1;
        (*it)->pushScene(nullptr);
    }
    _children.clear();
    _zDirty = false;
}

#pragma mark -
#pragma mark Z-order
/**
 * Resorts the children of this node according to z-value.
 *
 * If two children have the same z-value, their relative order is
 * preserved to what it was before the sort. This method should be
 * called before rendering.
 */
void Scene::sortZOrder() {
    if (_zDirty) {
        std::sort(_children.begin(),_children.end(),Node::compareNodeSibs);
        // Fix the offsets
        int ii = 0;
        for(auto it = _children.begin(); it != _children.end(); ++it ) {
            (*it)->_childOffset = ii++;
        }
        _zDirty = false;
        for(auto it = _children.begin(); it != _children.end(); ++it ) {
            (*it)->sortZOrder();
        }
    }
}

#pragma mark -
#pragma mark Rendering
/**
 * Draws all of the children in this scene with the given SpriteBatch.
 *
 * This method assumes that the sprite batch is not actively drawing.
 * It will call both begin() and end().
 *
 * Rendering happens by traversing the the scene graph using an "Pre-Order"
 * tree traversal algorithm ( https://en.wikipedia.org/wiki/Tree_traversal#Pre-order ).
 * That means that parents are always draw before (and behind children). The
 * children of each sub tree are ordered by z-value (or by the order added).
 *
 * @param batch     The SpriteBatch to draw with.
 */
void Scene::render(const std::shared_ptr<SpriteBatch>& batch) {
    if (_zSort && _zDirty) {
        sortZOrder();
    }
    
    batch->begin(_camera->getCombined());
    
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->render(batch, Mat4::IDENTITY, _color);
    }

    batch->end();
    batch->setBlendFunc(_srcFactor, _dstFactor);
    batch->setBlendEquation(_blendEquation);
}
