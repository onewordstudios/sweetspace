//
//  CUScene.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the root node of a (2d) scene graph.  After
//  much debate, we have decided to decouple this from the application class
//  (which is different than Cocos2d).  However, scenes are still permitted
//  to contain controller code.  They are in a sense a "subapplication".
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

#ifndef __CU_SCENE_H__
#define __CU_SCENE_H__

#include <cugl/math/cu_math.h>
#include <cugl/2d/CUNode.h>
#include <cugl/renderer/CUOrthographicCamera.h>

namespace cugl {
    
/**
 * This class provides the root node of a scene graph.
 *
 * The Scene class is very similar to {@link Node} and shares many methods
 * in common.  The major differences are that it has no parent and it has no
 * position (so it cannot be transformed). Instead, the Scene is defined by
 * an attached {@link OrthographicCamera}.
 *
 * Rendering happens by traversing the the scene graph using an "Pre-Order" tree
 * traversal algorithm ( https://en.wikipedia.org/wiki/Tree_traversal#Pre-order ).
 * That means that parents are always draw before (and behind children).  The
 * children of each sub tree are ordered by z-value (or by the order added).
 */
class Scene {
#pragma mark Values
protected:
    /** The name of this scene */
    std::string _name;
    /** The camera for this scene */
    std::shared_ptr<OrthographicCamera> _camera;
    /** The array of internal nodes */
    std::vector<std::shared_ptr<Node>> _children;
    /** The default tint for this scene */
    Color4 _color;
    /** Indicates whether the z-order is currently violated */
    bool _zDirty;
    /** Indicates whether auto-sorting is active */
    bool _zSort;
  
    /** The blending equation for this scene */
    GLenum _blendEquation;
    /** The source factor for the blend function */
    GLenum _srcFactor;
    /** The destination factor for the blend function */
    GLenum _dstFactor;

    /** Whether or note this scene is still active */
    bool _active;

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new degenerate Scene on the stack.
     *
     * The scene has no camera and must be initialized.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on 
     * the heap, use one of the static constructors instead.
     */
    Scene();
    
    /**
     * Deletes this scene, disposing all resources
     */
    ~Scene() { dispose(); }
    
    /**
     * Disposes all of the resources used by this scene.
     *
     * A disposed Scene can be safely reinitialized. Any children owned by this
     * scene will be released.  They will be deleted if no other object owns them.
     */
    virtual void dispose();
    
    /**
     * Initializes a Scene with the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param size      The viewport size
     *
     * @return true if initialization was successful.
     */
    virtual bool init(const Size& size) {
        return init(0,0,size.width,size.height);
    }
    
    /**
     * Initializes a Scene with the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param width     The viewport width
     * @param height    The viewport height
     *
     * @return true if initialization was successful.
     */
    bool init(float width, float height) {
        return init(0,0,width,height);
    }
    
    /**
     * Initializes a Scene with the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     * of the viewport in a larger canvas.
     *
     * @param rect      The viewport bounding box
     *
     * @return true if initialization was successful.
     */
    virtual bool init(const Rect& rect) {
        return init(rect.origin.x,rect.origin.y,rect.size.width, rect.size.height);
    }
    
    /**
     * Initializes a Scene with the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     * of the viewport in a larger canvas.
     *
     * @param origin    The viewport offset
     * @param size      The viewport size
     *
     * @return true if initialization was successful.
     */
    virtual bool init(const Vec2& origin, const Size& size) {
        return init(origin.x,origin.y,size.width, size.height);
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
    bool init(float x, float y, float width, float height);

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated Scene for the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param size      The viewport size
     *
     * @return a newly allocated Scene for the given viewport.
     */
    static std::shared_ptr<Scene> alloc(const Size& size) {
        std::shared_ptr<Scene> result = std::make_shared<Scene>();
        return (result->init(size) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated Scene for the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param width     The viewport width
     * @param height    The viewport height
     *
     * @return a newly allocated Scene for the given viewport.
     */
    static std::shared_ptr<Scene> alloc(float width, float height) {
        std::shared_ptr<Scene> result = std::make_shared<Scene>();
        return (result->init(width,height) ? result : nullptr);
    }

    /**
     * Returns a newly allocated Scene for the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     *
     * @param rect      The viewport bounding box
     *
     * @return a newly allocated Scene for the given viewport.
     */
    static std::shared_ptr<Scene> alloc(const Rect& rect) {
        std::shared_ptr<Scene> result = std::make_shared<Scene>();
        return (result->init(rect) ? result : nullptr);
    }

    /**
     * Returns a newly allocated Scene for the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     *
     * @param origin    The viewport offset
     * @param size      The viewport size
     *
     * @return a newly allocated Scene for the given viewport.
     */
    static std::shared_ptr<Scene> alloc(const Vec2& origin, const Size& size) {
        std::shared_ptr<Scene> result = std::make_shared<Scene>();
        return (result->init(origin,size) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated Scene for the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     *
     * @param x         The viewport x offset
     * @param y         The viewport y offset
     * @param width     The viewport width
     * @param height    The viewport height
     *
     * @return a newly allocated Scene for the given viewport.
     */
    static std::shared_ptr<Scene> alloc(float x, float y, float width, float height) {
        std::shared_ptr<Scene> result = std::make_shared<Scene>();
        return (result->init(x,y,width,height) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns a string that is used to identify the scene.
     *
     * @return a string that is used to identify the scene.
     */
    const std::string& getName() const { return _name; }

    /**
     * Returns the string that is used to identify the scene.
     *
     * @param name  A string that is used to identify the scene.
     */
    void setName(const std::string& name) { _name = name; }

    /**
     * Returns the camera for this scene.
     *
     * @return the camera for this scene.
     */
    std::shared_ptr<Camera> getCamera();

    /**
     * Returns the camera for this scene.
     *
     * @return the camera for this scene.
     */
    const std::shared_ptr<Camera> getCamera() const;
    
    /**
     * Returns the tint color for this scene.
     *
     * During the render phase, this color will be applied to any child for 
     * which hasRelativeColor() is true.
     *
     * @return the tint color for this scene.
     */
    Color4 getColor() { return _color; }

    /**
     * Sets the tint color for this scene.
     *
     * During the render phase, this color will be applied to any child for
     * which hasRelativeColor() is true.
     *
     * @param color  The tint color for this scene.
     */
    void setColor(Color4 color) { _color = color; }
    
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
    virtual std::string toString(bool verbose = false) const;
    
    /** Cast from a Scene to a string. */
    operator std::string() const { return toString(); }

#pragma mark -
#pragma mark View Size
    /**
     * Sets this Scene to have the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param size      The viewport size
     */
    void setSize(const Size& size) {
        _camera->set(size);
    }
    
    /**
     * Sets this Scene to have the given viewport.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param width     The viewport width
     * @param height    The viewport height
     */
    void setSize(float width, float height) {
        _camera->set(width,height);
    }
    

    /**
     * Sets this Scene to have the given viewport width.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param width     The viewport width
     */
    void setWidth(float width) {
        _camera->set(width,_camera->getViewport().size.height);
    }
    
    /**
     * Sets this Scene to have the given viewport height.
     *
     * The viewport origin is assumed to be (0,0).
     *
     * @param height    The viewport height
     */

    void setHeight(float height) {
        _camera->set(_camera->getViewport().size.width,height);
    }
    
    /**
     * Sets this Scene to have the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     *
     * @param rect      The viewport bounding box
     */
    void setBounds(const Rect& rect) {
        _camera->set(rect);
    }
    
    /**
     * Sets this Scene to have the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     *
     * @param origin    The viewport offset
     * @param size      The viewport size
     */
    void setBounds(const Vec2& origin, const Size& size) {
         _camera->set(origin,size);
    }
    
    /**
     * Sets this Scene to have the given viewport.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     *
     * @param x         The viewport x offset
     * @param y         The viewport y offset
     * @param width     The viewport width
     * @param height    The viewport height
     */
    void setBounds(float x, float y, float width, float height) {
        _camera->set(x,y,width,height);
    }
    
    /**
     * Offsets the viewport origin by the given amount.
     *
     * Offseting the viewport origin has little affect on the Scene in general.
     * It only affects the coordinate conversion methods {@link Camera#project()}
     * and {@link Camera#unproject()}. It is supposed to represent the offset
     *
     * @param origin    The offset of the viewport origin
     */
    void setOffset(const Vec2& origin) {
        _camera->set(origin,_camera->getViewport().size);
    }
    
    /**
     * Returns the world space equivalent of a point in screen coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * In computing the world space coordinates, this method assumes that the
     * z-value of the original vector is the same as near, which is the
     * closest it can be the screen.
     *
     * This method is important for converting event coordinates (such as a
     * mouse click) to world coordinates.
     *
     * @param screenCoords  The point in screen coordinates
     *
     * @return the world space equivalent of a point in screen coordinates.
     */
    Vec3 screenToWorldCoords(const Vec2& screenCoords) const {
        return _camera->screenToWorldCoords(screenCoords);
    }
    
    /**
     * Returns the screen space equivalent of a point in world coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * This method is important for converting world coordinates to event
     * coordinates (such as a mouse click).
     *
     * @param worldCoords   The point in wprld coordinates
     *
     * @return the screen space equivalent of a point in world coordinates.
     */
    Vec2 worldToScreenCoords(const Vec3& worldCoords) const {
        return _camera->worldToScreenCoords(worldCoords);
    }
    
#pragma mark -
#pragma mark Scene Graph
    /**
     * Returns the number of immediate children of this scene.
     *
     * @return The number of immediate children of this scene.
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
     * one that is found. Children are not necessarily enumerated in the order
     * that they are added. For example, they may be resorted by their z-order.
     * Hence it is very important that tags be unique.
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
     * one that is found. Children are not necessarily enumerated in the order
     * that they are added. For example, they may be resorted by their z-order.
     * Hence it is very important that tags be unique.
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
     * one that is found. Children are not necessarily enumerated in the order
     * that they are added. For example, they may be resorted by their z-order.
     * Hence it is very important that names be unique.
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
     * one that is found. Children are not necessarily enumerated in the order
     * that they are added. For example, they may be resorted by their z-order.
     * Hence it is very important that names be unique.
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
     * Returns the list of the scene's immediate children.
     *
     * @return the list of the scene's immediate children.
     */
    std::vector<std::shared_ptr<Node>> getChildren() { return _children; }
    
    /**
     * Returns the list of the scene's immediate children.
     *
     * @return the list of the scene's immediate children.
     */
    const std::vector<std::shared_ptr<Node>>& getChildren() const { return _children; }
    
    /**
     * Adds a child to this scene.
     *
     * Children are not necessarily enumerated in the order that they are added.
     * For example, they may be resorted by their z-order. Hence you should
     * generally attempt to retrieve a child by tag or by name instead.
     *
     * @param child A child node.
     */
    void addChild(std::shared_ptr<Node> child) {
        addChild(child,child->getZOrder());
    }
    
    /**
     * Adds a child to this scene with the given z-order.
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
     * Adds a child to this scene with the given tag.
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
     * Adds a child to this scene with the given tag and z-order
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
     * Adds a child to this scene with the given name.
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
     * Adds a child to this scene with the given name and z-order
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
     * to allow transitions in the scene graph.
     *
     * This method is undefined if child1 is not a child of this scene.
     *
     * @param child1    The current child of this node
     * @param child2    The child to swap it with.
     * @param inherit   Whether the new child should inherit the children of child1.
     */
    void swapChild(const std::shared_ptr<Node>& child1, const std::shared_ptr<Node>& child2, bool inherit=false);
    
    /**
     * Removes the child at the given position from this Scene.
     *
     * Removing a child alters the position of every child after it.  Hence
     * it is unsafe to cache child positions.
     *
     * @param pos   The position of the child node which will be removed.
     */
    virtual void removeChild(unsigned int pos);
    
    /**
     * Removes a child from this Scene.
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
     * Removes a child from the Scene by tag value.
     *
     * If there is more than one child of the given tag, it removes the first
     * one that is found. Children are not necessarily enumerated in the order
     * that they are added. For example, they may be resorted by their z-order.
     * Hence it is very important that tags be unique.
     *
     * @param tag   An integer to identify the node easily.
     */
    void removeChildByTag(unsigned int tag);
    
    /**
     * Removes a child from the Scene by name.
     *
     * If there is more than one child of the given name, it removes the first
     * one that is found. Children are not necessarily enumerated in the order
     * that they are added. For example, they may be resorted by their z-order.
     * Hence it is very important that names be unique.
     *
     * @param name  A string to identify the node.
     */
    void removeChildByName(const std::string &name);
    
    /**
     * Removes all children from this Node.
     */
    virtual void removeAllChildren();

#pragma mark -
#pragma mark Z-order
    /**
     * Returns true if this scene automatically sorts children by z-value.
     *
     * If this value is true, the scene graph will be resorted every time that
     * render() is called.  Because of our invariant with dirty nodes, sorting
     * will be avoided if it is not necessary.
     *
     * Sorting only happens automatically is this value it true.
     * Otherwise, you will need to call {@link sortZOrder()} if you wish
     * to guarantee that the sceen graph is in z-order.
     *
     * @return true if this scene automatically sorts children by z-value.
     */
    bool isZAutoSort() { return _zSort; }
    
    /**
     * Sets whether this scene automatically sorts children by z-value.
     *
     * If this value is true, the scene graph will be resorted every time that
     * render() is called.  Because of our invariant with dirty nodes, sorting
     * will be avoided if it is not necessary.
     *
     * Sorting only happens automatically is this value it true.
     * Otherwise, you will need to call {@link sortZOrder()} if you wish
     * to guarantee that the sceen graph is in z-order.
     *
     * @param value  Whether this scene automatically sorts children by z-value.
     */
    void setZAutoSort(bool value) { _zSort = value; }
    
    /**
     * Returns whether the children of this Scene needs resorting.
     *
     * The children of a scene may need to be resorted whenever a child is added,
     * or whenever the z-value of a child is changed.
     *
     * This value satisfies the following invariant: if a Node is dirty and
     * needs resorting, then so are all of its ancestors (including this
     * associated {@link Scene}). Our methods guarantee this invariant, so
     * that the method isZDirty() always returns the correct value.
     *
     * Sorting only happens automatically is {@link isZAutoSort()} it true.
     * Otherwise, you will need to call {@link sortZOrder()} if you wish
     * to guarantee that the sceen graph is in z-order.
     *
     * @return whether the children of this node needs resorting.
     */
    bool isZDirty() const { return _zDirty; }
    
    /**
     * Resorts the children of this Scene according to z-value.
     *
     * If two children have the same z-value, their relative order is
     * preserved to what it was before the sort. This method should be
     * called before rendering.
     *
     * Resorting is done recursively down a tree for each child that is dirty
     * and needs resorting.  We guarantee that Nodes will not be resorted
     * unless necessary, because of the following invariant: if a Node is dirty
     * and needs resorting, then so are all of its ancestors (including this
     * associated {@link Scene}).
     *
     * Sorting only happens automatically is {@link isZAutoSort()} it true.
     * Otherwise, you will need to call {@link sortZOrder()} if you wish
     * to guarantee that the sceen graph is in z-order.
     */
    void sortZOrder();
    
#pragma mark -
#pragma mark Scene Logic
    /**
     * Returns true if the scene is currently active
     *
     * @return true if the scene is currently active
     */
    bool isActive( ) const { return _active; }

    /**
     * Sets whether the scene is currently active
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) { _active = value; }

    /**
     * The method called to update the scene.
     *
     * This method should be overridden with the specific scene logic.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    virtual void update(float timestep) {}

    /**
     * Resets the status of the scene to its original configuration.
     */
    virtual void reset() {}
    
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
    void render(const std::shared_ptr<SpriteBatch>& batch);
    
private:
#pragma mark -
#pragma mark Internal Helpers
    /**
     * Sets whether the children of this Scene needs resorting.
     *
     * The children of a scene may need to be resorted whenever a child is added,
     * or whenever the z-value of a child is changed.
     *
     * This value satisfies the following invariant: if a Node is dirty and
     * needs resorting, then so are all of its ancestors (including this
     * associated {@link Scene}). Our methods guarantee this invariant, so
     * that the method isZDirty() always returns the correct value.
     *
     * Sorting only happens automatically is {@link isZAutoSort()} it true.
     * Otherwise, you will need to call {@link sortZOrder()} if you wish
     * to guarantee that the sceen graph is in z-order.
     *
     * @param value Whether the children of this node needs resorting.
     */
    void setZDirty(bool value) { _zDirty = value; }
    
    // Tightly couple with Node
    friend class Node;
};

}
#endif /* __CU_SCENE_H__ */
