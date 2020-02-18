//
//  CUObstacleSelector.h
//  Cornell University Game Library (CUGL)
//
//  This class implements a selection tool for dragging physics objects with a
//  mouse. It is essentially an instance of b2MouseJoint, but with an API that
//  makes it a lot easier to use. As with all instances of b2MouseJoint, there
//  will be some lag in the drag (though this is true on touch devices in general).
//  You can adjust the degree of this lag by adjusting the force.  However,
//  larger forces can cause artifacts when dragging an obstacle through other
//  obstacles.
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
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White
//  Version: 11/6/16
//
#ifndef __CU_OBSTACLE_SELECTOR_H__
#define __CU_OBSTACLE_SELECTOR_H__

#include <Box2D/Dynamics/Joints/b2MouseJoint.h>
#include <Box2D/Dynamics/b2Fixture.h>
//#include "CUObstacleWorld.h"

/** The default size of the mouse selector */
#define DEFAULT_MSIZE       0.2f
/** The default update frequence (in Hz) of the joint */
#define DEFAULT_FREQUENCY  10.0f
/** The default damping force of the joint */
#define DEFAULT_DAMPING     0.7f
/** The default force multiplier of the selector */
#define DEFAULT_FORCE    1000.0f

namespace cugl {

// Forward reference to the ObstacleWorld
class ObstacleWorld;
    
#pragma mark -
#pragma mark Obstacle Selector
/**
 * Selection tool to move and drag physics obstacles
 *
 * This class is essentially an instance of b2MouseJoint, but with an API that 
 * makes it a lot easier to use. It must be attached to a WorldController on 
 * creation, and this controller can never change.  If you want a selector for 
 * a different ObstacleWorld, make a new instance.
 *
 * As with all instances of b2MouseJoint, there will be some lag in the drag 
 * (though this is true on touch devices in general).  You can adjust the degree 
 * of this lag by adjusting the force.  However, larger forces can cause 
 * artifacts when dragging an obstacle through other obstacles.
 */
class ObstacleSelector {
protected:
    /** The ObstacleWorld associated with this selection */
    std::shared_ptr<ObstacleWorld> _controller;
    
    /** The location in world space of this selector */
    Vec2 _position;
    /** The size of the selection region (for accuracy) */
    Size _size;
    /** The amount to multiply by the mass to move the object */
    float _force;
    
    /** The current fixture selected by this tool (may be nullptr) */
    b2Fixture*  _selection;
    /** A default body used as the other half of the mouse joint */
    b2Body*     _ground;
    
    /** A reusable definition for creating a mouse joint */
    b2MouseJointDef   _jointDef;
    /** The current mouse joint, if an item is selected */
    b2MouseJoint*   _mouseJoint;
    
    /** The wireframe parent for debugging. */
    std::shared_ptr<Node> _scene;
    /** The wireframe node for debugging. */
    std::shared_ptr<WireNode> _hatch;
    /** The wireframe node for the connection */
    std::shared_ptr<WireNode> _connect;
    /** The conversion rate between physics units and drawing units */
    //Vec2 _drawScale;
    /** Whether or not to display the debug wireframe. */
    bool _dvisible;
    /** The wireframe color for debugging */
    Color4 _dcolor;
    
#pragma mark -
#pragma mark Scene Graph Internals
    /**
     * Creates the outline of the physics fixtures in the debug wireframe
     *
     * The debug wireframe is use to outline the fixtures attached to this
     * selector.  It is useful when you want to visualize the relationship
     * between the mouse and the selected shape.
     */
    void resetDebug();
    
    /**
     * Repositions the debug wireframe so that it agrees with the physics object.
     *
     * The debug wireframe is use to outline the fixtures attached to this 
     * selector.  It is useful when you want to visualize the relationship
     * between the mouse and the selected shape.
     */
    void updateDebug();
    
    /**
     * Repositions the scene node so that it agrees with the physics object.
     *
     * By default, the position of a node should be the body position times
     * the draw scale.  However, for some obstacles (particularly complex
     * obstacles), it may be desirable to turn the default functionality
     * off.  Hence we have made this virtual.
     */
    void updateTarget(Obstacle* obstacle);

    /**
     * Returns a new polygon for the mouse hatch
     *
     * @return a new polygon for the mouse hatch
     */
    Poly2 hatchPoly() const;
    
public:
#pragma mark Constructors
    /**
     * Creates a new ObstacleSelector
     *
     * The selector created is not usable.  This constructor only initializes 
     * default values.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ObstacleSelector() :
        _controller(nullptr), _selection(nullptr), _ground(nullptr),
        _mouseJoint(nullptr), _scene(nullptr), _hatch(nullptr),
        _connect(nullptr) {}
    
    /**
     * Disposes of this selector, releasing all resources.
     */
    ~ObstacleSelector() { dispose(); }
    
    /**
     * Disposes all of the resources used by this selector.
     *
     * A disposed selector can be safely reinitialized.
     */
    void dispose();
    
    /**
     * Initializes a new selector for the given ObstacleWorld
     *
     * This controller can never change.  If you want a selector for a different
     * ObstacleWorld, make a new instance.
     *
     * This initializer uses the default mouse size.
     *
     * @param world     the physics controller
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<ObstacleWorld>& world) {
        return init(world, Size(DEFAULT_MSIZE, DEFAULT_MSIZE));
    }
    
    /**
     * Initializes a new selector for the given ObstacleWorld and mouse size.
     *
     * This controller can never change.  If you want a selector for a different
     * ObstacleWorld, make a new instance.  However, the mouse size can be 
     * changed at any time.
     *
     * @param world     the physics controller
     * @param mouseSize the mouse size
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<ObstacleWorld>& world, const Size& mouseSize);
    

#pragma mark Static Constructors
    /**
     * Returns a newly allocated ObstacleSelector for the given ObstacleWorld
     *
     * This controller can never change.  If you want a selector for a different
     * ObstacleWorld, make a new instance.
     *
     * This constructor uses the default mouse size.
     *
     * @param world     the physics controller
     *
     * @return a newly allocated ObstacleSelector for the given ObstacleWorld
     */
    static std::shared_ptr<ObstacleSelector> alloc(const std::shared_ptr<ObstacleWorld>& world) {
        std::shared_ptr<ObstacleSelector> result = std::make_shared<ObstacleSelector>();
        return (result->init(world) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated ObstacleSelector for the given world and mouse size.
     *
     * This controller can never change.  If you want a selector for a different
     * ObstacleWorld, make a new instance.  However, the mouse size can be changed
     * at any time.
     *
     * @param world     the physics controller
     * @param mouseSize the mouse size
     *
     * @return an autoreleased selector object
     */
    static std::shared_ptr<ObstacleSelector> alloc(const std::shared_ptr<ObstacleWorld>& world,
                                                   const Size& mouseSize) {
        std::shared_ptr<ObstacleSelector> result = std::make_shared<ObstacleSelector>();
        return (result->init(world,mouseSize) ? result : nullptr);
    }

#pragma mark Positional Methods
    /**
     * Returns the current position of this selector (in World space)
     *
     * @return the current position of this selector (in World space)
     */
    const Vec2& getPosition() { return _position; }

    /**
     * Sets the current position of this selector (in World space)
     *
     * @param x  the x-coordinate of the selector
     * @param y  the y-coordinate of the selector
     */
    void setPosition(float x, float y);

    /**
     * Sets the current position of this selector (in World space)
     *
     * @param pos the position of the selector
     */
    void setPosition(const Vec2& pos) { setPosition(pos.x,pos.y); }


    
#pragma mark Selection Methods
    /**
     * Returns true if a physics body is currently selected
     *
     * @return true if a physics body is currently selected
     */
    bool isSelected() const { return _selection != nullptr; }
    
    /**
     * Returns true if a physics body was selected at the current position.
     *
     * This method contructs and AABB the size of the mouse pointer, centered at
     * the current position.  If any part of the AABB overlaps a fixture, it is
     * selected.
     *
     * @return true if a physics body was selected at the given position.
     */
    bool select();
    
    /**
     * Deselects the physics body, discontinuing any mouse movement.
     *
     * The body may still continue to move of its own accord.
     */
    void deselect();
    
    /**
     * Returns a (weak) reference to the Obstacle selected (if any)
     *
     * Just because a physics body was selected does not mean that an Obstacle
     * was selected.  The body could be a basic Box2d body generated by other
     * means. If the body is not an Obstacle, this method returns nullptr.
     *
     * @return a (weak) reference to the Obstacle selected (if any)
     */
    Obstacle* getObstacle();
    
    /**
     * Callback function for mouse selection.
     *
     * This is the callback function used by the method queryAABB to select a
     * physics body at the current mouse location.
     *
     * @param  fixture  the fixture selected
     *
     * @return false to terminate the query.
     */
    bool onQuery(b2Fixture* fixture);
    
    
#pragma mark Attribute Properties
    /**
     * Returns the response speed of the mouse joint
     *
     * See the documentation of b2JointDef for more information on the response
     * speed.
     *
     * @return the response speed of the mouse joint
     */
    float getFrequency() const { return _jointDef.frequencyHz; }
    
    /**
     * Sets the response speed of the mouse joint
     *
     * See the documentation of b2JointDef for more information on the response
     * speed.
     *
     * @param  speed    the response speed of the mouse joint
     */
    void setFrequency(float speed) { _jointDef.frequencyHz = speed; }
    
    /**
     * Returns the damping ratio of the mouse joint
     *
     * See the documentation of b2JointDef for more information on the damping
     * ratio.
     *
     * @return the damping ratio of the mouse joint
     */
    float getDamping() const { return _jointDef.dampingRatio; }
    
    /**
     * Sets the damping ratio of the mouse joint
     *
     * See the documentation of b2JointDef for more information on the damping
     * ratio.
     *
     * @param  ratio   the damping ratio of the mouse joint
     */
    void setDamping(float ratio) { _jointDef.dampingRatio = ratio; }
    
    /**
     * Returns the force multiplier of the mouse joint
     *
     * The mouse joint will move the attached fixture with a force of this value
     * times the object mass.
     *
     * @return the force multiplier of the mouse joint
     */
    float getForce() const { return _force; }
    
    /**
     * Sets the force multiplier of the mouse joint
     *
     * The mouse joint will move the attached fixture with a force of this value
     * times the object mass.
     *
     * @param  force    the force multiplier of the mouse joint
     */
    void setForce(float force) { _force = force; }
    
    /**
     * Returns the size of the mouse pointer
     *
     * When a selection is made, this selector will create an axis-aligned 
     * bounding box centered at the mouse position.  Any fixture overlapping 
     * this box will be selected.  The size of this box is determined by this 
     * value.
     *
     * @return the size of the mouse pointer
     */
    const Size& getMouseSize() const { return _size; }
    
    /**
     * Sets the size of the mouse pointer
     *
     * When a selection is made, this selector will create an axis-aligned
     * bounding box centered at the mouse position.  Any fixture overlapping
     * this box will be selected.  The size of this box is determined by this
     * value.
     *
     * @param  size the size of the mouse pointer
     */
    void setMouseSize(const Size& size) { _size = size; resetDebug(); }
    
    
#pragma mark -
#pragma mark Scene Graph Methods
    /**
     * Returns the color of the debug wireframe.
     *
     * The default color is white, which means that the objects will be shown
     * with a white wireframe.
     *
     * @return the color of the debug wireframe.
     */
    Color4 getDebugColor() const { return _dcolor; }
    
    /**
     * Sets the color of the debug wireframe.
     *
     * The default color is white, which means that the objects will be shown
     * with a white wireframe.
     *
     * @param color the color of the debug wireframe.
     */
    void setDebugColor(Color4 color) {
        _dcolor = color;
        if (_hatch)   { _hatch->setColor(color);   }
        if (_connect) { _connect->setColor(color); }
    }
    
    /**
     * Returns the parent scene graph node for the debug wireframe
     *
     * The returned node is the parent coordinate space for drawing physics.
     * All debug nodes for physics objects are drawn within this coordinate
     * space.  Setting the visibility of this node to false will disable
     * any debugging.
     *
     * The wireframe will be drawn using physics coordinates, which is possibly
     * much smaller than your drawing coordinates (e.g. 1 Box2D unit = 1 pixel).
     * If you want the wireframes to be larger, you should scale the parent
     * parent coordinate space to match the rest of the application.
     *
     * This scene graph node is intended for debugging purposes only.  If
     * you want a physics body to update a proper texture image, you should
     * either use the method {@link Obstacle#update(float)} for subclasses or
     * {@link Obstacle#setListener} for decoupled classes.
     *
     * @return the scene graph node for the debug wireframe
     */
    Node* getDebugScene() const { return _scene.get(); }
    
    /**
     * Sets the parent scene graph node for the debug wireframe
     *
     * The given node is the parent coordinate space for drawing physics.
     * All debug nodes for physics objects are drawn within this coordinate
     * space.  Setting the visibility of this node to false will disable
     * any debugging.  Similarly, setting this value to nullptr will
     * disable any debugging.
     *
     * The wireframe will be drawn using physics coordinates, which is possibly
     * much smaller than your drawing coordinates (e.g. 1 Box2D unit = 1 pixel).
     * If you want the wireframes to be larger, you should scale the parent
     * parent coordinate space to match the rest of the application.
     *
     * This scene graph node is intended for debugging purposes only.  If
     * you want a physics body to update a proper texture image, you should
     * either use the method {@link Obstacle#update(float)} for subclasses or
     * {@link Obstacle#setListener} for decoupled classes.
     *
     * @param node  he parent scene graph node for the debug wireframe
     */
    void setDebugScene(const std::shared_ptr<Node>& node);
    
    /**
     * Sets whether the debug wireframe for this object is visible
     *
     * This method is necessary for touch screen devices, where we cannot
     * track the selector if there is no active touch.
     *
     * @param flag  whether the debug wireframe for this object is visible
     */
    void setVisible(bool flag) {
        _dvisible = flag;
        if (_hatch)   { _hatch->setVisible(flag);   }
        if (_connect) { _connect->setVisible(flag); }
    }
    
    /**
     * Returns true if the debug wireframe for this object is visible
     *
     * This method is necessary for touch screen devices, where we cannot
     * track the selector if there is no active touch.
     *
     * @return true if the debug wireframe for this object is visible
     */
    bool isVisible() { return _dvisible; }
    
    /**
     * Returns true if the obstacle has a wireframe for debugging.
     *
     * This method will return false if there is no active parent scene
     * for the wireframe.
     *
     * @return true if the obstacle has a wireframe for debugging.
     */
    bool hasDebug() { return _hatch != nullptr; }
};

}
#endif /* __CU_OBSTACLE_SELECTOR_H__ */
