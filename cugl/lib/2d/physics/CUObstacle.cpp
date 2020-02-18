//
//  CUObstacle.h
//  Cornell University Game Library (CUGL)
//
//  Box2D is an excellent physics engine in how it decouples collision and
//  geometry from rigid body dynamics.  However, there are some times in which
//  coupling is okay for convenience reasons (particularly when we have the
//  option to uncouple).  This module is such an example; it couples the
//  bodies and fixtures from Box2d into a single class, making the physics
//  easier to use (in most cases).
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
#include <cugl/2d/physics/CUObstacle.h>
#include <memory>
#include <iostream>
#include <sstream>

using namespace cugl;

#pragma mark -
#pragma mark Constructor
/*
 * Creates a new physics object at the origin.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead (in this case, in
 * one of the subclasses).
 */
Obstacle::Obstacle() :
_scene(nullptr),
_debug(nullptr),
_listener(nullptr)
{ }

/**
 * Deletes this physics object and all of its resources.
 *
 * We have to make the destructor public so that we can polymorphically
 * delete physics objects.
 *
 * A non-default destructor is necessary since we must release all
 * claims on scene graph nodes.
 */
Obstacle::~Obstacle() {
    setDebugScene(nullptr);
}

/**
 * Initializes a new physics object at the given point
 *
 * @param  vec  Initial position in world coordinates
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool Obstacle::init(const Vec2& vec) {
    // Object has yet to be deactivated
    _remove = false;
    
    // Allocate the body information
    _bodyinfo.awake  = true;
    _bodyinfo.allowSleep = true;
    _bodyinfo.gravityScale = 1.0f;
    _bodyinfo.position.Set(vec.x,vec.y);
    // Objects are physics objects unless otherwise noted
    _bodyinfo.type = b2_dynamicBody;
    
    // Turn off the mass information
    _masseffect = false;
    
    return true;
}

/**
 * Copies the state from the given body to the body def.
 *
 * This is important if you want to save the state of the body before removing
 * it from the world.
 */
void Obstacle::setBodyState(const b2Body& body) {
    _bodyinfo.type   = body.GetType();
    _bodyinfo.angle  = body.GetAngle();
    _bodyinfo.active = body.IsActive();
    _bodyinfo.awake  = body.IsAwake();
    _bodyinfo.bullet = body.IsBullet();
    _bodyinfo.position.Set(body.GetPosition().x,body.GetPosition().y);
    _bodyinfo.linearVelocity.Set(body.GetLinearVelocity().x,body.GetLinearVelocity().y);
    _bodyinfo.allowSleep = body.IsSleepingAllowed();
    _bodyinfo.fixedRotation = body.IsFixedRotation();
    _bodyinfo.gravityScale  = body.GetGravityScale();
    _bodyinfo.angularDamping = body.GetAngularDamping();
    _bodyinfo.linearDamping  = body.GetLinearDamping();
}


#pragma mark -
#pragma mark MassData Methods

/**
 * Sets the center of mass for this physics body
 *
 * @param x  the x-coordinate of the center of mass for this physics body
 * @param y  the y-coordinate of the center of mass for this physics body
 */
void Obstacle::setCentroid(float x, float y) {
    if (!_masseffect) {
        _masseffect = true;
        _massdata.I = getInertia();
        _massdata.mass = getMass();
    }
    _massdata.center.Set(x,y);
}

/**
 * Sets the rotational inertia of this body
 *
 * For static bodies, the mass and rotational inertia are set to zero. When
 * a body has fixed rotation, its rotational inertia is zero.
 *
 * @param value  the rotational inertia of this body
 */
void Obstacle::setInertia(float value) {
    if (!_masseffect) {
        _masseffect = true;
        _massdata.center.Set(getCentroid().x,getCentroid().y);
        _massdata.mass = getMass();
    }
    _massdata.I = value;
}

/**
 * Sets the mass of this body
 *
 * The value is usually in kilograms.
 *
 * @param value  the mass of this body
 */
void Obstacle::setMass(float value) {
    if (!_masseffect) {
        _masseffect = true;
        _massdata.center.Set(getCentroid().x,getCentroid().y);
        _massdata.I = getInertia();
    }
    _massdata.mass = value;
}


#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Sets the color of the debug wireframe.
 *
 * The default color is white, which means that the objects will be shown
 * with a white wireframe.
 *
 * @param color the color of the debug wireframe.
 */
void Obstacle::setDebugColor(Color4 color) {
    _dcolor = color;
    if (_debug) {
        _debug->setColor(color);
    }
}

/**
 * Sets the scene graph node for drawing purposes.
 *
 * The scene graph is completely decoupled from the physics system.  The node
 * does not have to be the same size as the physics body. We only guarantee
 * that the node is positioned correctly according to the drawing scale.
 *
 * @param value  the scene graph node for drawing purposes.
 *
 * @retain  a reference to this scene graph node
 * @release the previous debug graph node used by this object
 */
void Obstacle::setDebugScene(const std::shared_ptr<Node>& node) {
    // Release the node if we have one previously
    if (_scene != nullptr) {
        if (_debug != nullptr && _debug->getParent() != nullptr) {
            _scene->removeChild(_debug);
        }
        _scene = nullptr;
    }
    if (node != nullptr) {
        _scene = node;
        resetDebug();
        updateDebug();
    }
}

/**
 * Repositions the scene node so that it agrees with the physics object.
 *
 * By default, the position of a node should be the body position times
 * the draw scale.  However, for some obstacles (particularly complex
 * obstacles), it may be desirable to turn the default functionality
 * off.  Hence we have made this virtual.
 */
void Obstacle::updateDebug() {
    CUAssertLog(_scene, "Attempt to reposition a wireframe with no parent");
    Vec2 pos = getPosition();
    float angle = getAngle();
    _debug->setPosition(pos);
    _debug->setAngle(angle);
}


#pragma mark -
#pragma Debugging Methods

/**
 * Returns a string representation of this physics object.
 *
 * This method converts the physics object into a string for debugging.  By
 * default it shows the tag and position.  Other physics objects may want to
 * override this method for more detailed information.
 *
 * @return a string representation of this physics object
 */
std::string Obstacle::toString() const {
    std::stringstream ss;
    Vec2 p = getPosition();
    ss << "[Obstacle " << _tag << ": (" << p.x << "," << p.y << "), ";
    ss << (isActive() ? "active ]" : "inactive ]");
    return ss.str();
}

/**
 * Outputs this physics object to the given output stream.
 *
 * This function uses the toString() method to convert the physics object
 * into a string
 *
 * @param  os   the output stream
 * @param  obj  the physics object to ouput
 *
 * @return the output stream
 */
std::ostream& operator<<(std::ostream& os, const Obstacle& obj) {
    os << obj.toString();
    return os;
}
