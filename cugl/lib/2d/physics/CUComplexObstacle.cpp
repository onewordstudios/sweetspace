//
//  CUComplexObstacle.cpp
//  Cornell Extensions to Cocos2D
//
//  This module provides a physics object that supports mutliple Bodies.
//  This is the base class for objects that are tied together with joints.
//
//  This class does not provide Shape information, and cannot be instantiated
//  directly.  There are no default complex objects.  You will need to create
//  your own subclasses to use this class.  It is very similar to Lab 4
//  from CS 3152.
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
#include <cugl/2d/physics/CUComplexObstacle.h>
#include <Box2D/Dynamics/b2World.h>

using namespace cugl;


#pragma mark -
#pragma mark Fixture Methods
/**
 * Sets the density of this body
 *
 * The density is typically measured in usually in kg/m^2. The density can be zero or
 * positive. You should generally use similar densities for all your fixtures. This
 * will improve stacking stability.
 *
 * This method affects the root body of this composite structure only.  If you want
 * to set the value for any of the child obstacles, iterate over the children.
 *
 * @param value  the density of this body
 */
void ComplexObstacle::setDensity(float value) {
    _fixture.density = value;
    if (_body != nullptr) {
        for (b2Fixture* f = _body->GetFixtureList(); f; f = f->GetNext()) {
            f->SetDensity(value);
        }
        if (!_masseffect) {
            _body->ResetMassData();
        }
    }
}

/**
 * Sets the friction coefficient of this body
 *
 * The friction parameter is usually set between 0 and 1, but can be any non-negative
 * value. A friction value of 0 turns off friction and a value of 1 makes the friction
 * strong. When the friction force is computed between two shapes, Box2D must combine
 * the friction parameters of the two parent fixtures. This is done with the geometric
 * mean.
 *
 * This method affects the root body of this composite structure only.  If you want
 * to set the value for any of the child obstacles, iterate over the children.
 *
 * @param value  the friction coefficient of this body
 */
void ComplexObstacle::setFriction(float value) {
    _fixture.friction = value;
    if (_body != nullptr) {
        for (b2Fixture* f = _body->GetFixtureList(); f; f = f->GetNext()) {
            f->SetFriction(value);
        }
    }
}

/**
 * Sets the restitution of this body
 *
 * Restitution is used to make objects bounce. The restitution value is usually set
 * to be between 0 and 1. Consider dropping a ball on a table. A value of zero means
 * the ball won't bounce. This is called an inelastic collision. A value of one means
 * the ball's velocity will be exactly reflected. This is called a perfectly elastic
 * collision.
 *
 * This method affects the root body of this composite structure only.  If you want
 * to set the value for any of the child obstacles, iterate over the children.
 *
 * @param value  the restitution of this body
 */
void ComplexObstacle::setRestitution(float value) {
    _fixture.restitution = value;
    if (_body != nullptr) {
        for (b2Fixture* f = _body->GetFixtureList(); f; f = f->GetNext()) {
            f->SetRestitution(value);
        }
    }
}

/**
 * Sets whether this object is a sensor.
 *
 * Sometimes game logic needs to know when two entities overlap yet there should be
 * no collision response. This is done by using sensors. A sensor is an entity that
 * detects collision but does not produce a response.
 *
 * This method affects the root body of this composite structure only.  If you want
 * to set the value for any of the child obstacles, iterate over the children.
 *
 * @param value  whether this object is a sensor.
 */
void ComplexObstacle::setSensor(bool value) {
    _fixture.isSensor = value;
    if (_body != nullptr) {
        for (b2Fixture* f = _body->GetFixtureList(); f; f = f->GetNext()) {
            f->SetSensor(value);
        }
    }
}

/**
 * Sets the filter data for this object
 *
 * Collision filtering allows you to prevent collision between fixtures. For example,
 * say you make a character that rides a bicycle. You want the bicycle to collide
 * with the terrain and the character to collide with the terrain, but you don't want
 * the character to collide with the bicycle (because they must overlap). Box2D
 * supports such collision filtering using categories and groups.
 *
 * A value of null removes all collision filters. This method affects ALL of the
 * bodies in this composite structure.  For fine-grain control, you will need to
 * loop over all elements in the composite structure.
 *
 * @param value  the filter data for this object
 */
void ComplexObstacle::setFilterData(b2Filter value) {
    _fixture.filter = value;
    if (_body != nullptr) {
        for (b2Fixture* f = _body->GetFixtureList(); f; f = f->GetNext()) {
            f->SetFilterData(value);
        }
    }
}


#pragma mark -
#pragma mark Physics Methods

/**
 * Creates the physics Body(s) for this object, adding them to the world.
 *
 * Implementations of this method should NOT retain a reference to World.
 * That is a tight coupling that we should avoid.
 *
 * @param world Box2D world to store body
 *
 * @return true if object allocation succeeded
 */
bool ComplexObstacle::activatePhysics(b2World& world) {
    // Make a body, if possible
    _bodyinfo.active = true;
    _body = world.CreateBody(&_bodyinfo);
    _body->SetUserData((void*)this);
    
    // Only initialize if a body was created.
    bool success = true;
    if (_body == nullptr) {
        _bodyinfo.active = false;
        return false;
    }
    createFixtures();
    
    // Active all other bodies.
    for(auto it = _bodies.begin(); it != _bodies.end(); ++it) {
        success = success && (*it)->activatePhysics(world);
    }
    success = success && createJoints(world);
    
    // Clean up if we failed
    if (!success) {
        deactivatePhysics(world);
    }
    return success;
}

/**
 * Destroys the physics Body(s) of this object if applicable,
 * removing them from the world.
 *
 * @param world Box2D world that stores body
 */
void ComplexObstacle::deactivatePhysics(b2World& world) {
    // Should be good for most (simple) applications.
    if (_body != nullptr) {
        for(auto it = _joints.begin(); it!= _joints.end(); ++it) {
            world.DestroyJoint(*it);
        }
        _joints.clear();
        for(auto it = _bodies.begin(); it!= _bodies.end(); ++it) {
            (*it)->deactivatePhysics(world);
        }
        
        releaseFixtures();
        
        // Snapshot the values
        setBodyState(*_body);
        world.DestroyBody(_body);
        _body = nullptr;
        _bodyinfo.active = false;
    }
}

/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * This method is called AFTER the collision resolution state. Therefore, it
 * should not be used to process actions or any other gameplay information.  Its
 * primary purpose is to adjust changes to the fixture, which have to take place
 * after collision.
 *
 * In other words, this is the method that updates the scene graph.  If
 * you forget to call it, it will not draw your changes.
 *
 * @param dt Timing values from parent loop
 */
void ComplexObstacle::update(float delta) {
    // Recreate the fixture object if dimensions changed.
    if (isDirty()) {
        createFixtures();
    }
    
    updateDebug();
    // Update the children
    for(auto it = _bodies.begin(); it!= _bodies.end(); ++it) {
        (*it)->update(delta);
    }
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
void ComplexObstacle::setDebugColor(Color4 color) {
    Obstacle::setDebugColor(color);
    for(auto it = _bodies.begin(); it!= _bodies.end(); ++it) {
        (*it)->setDebugColor(color);
    }
}

/**
 * Sets the color of the debug wireframe.
 *
 * The default color is white, which means that the objects will be shown
 * with a white wireframe.
 *
 * @param color     the color of the debug wireframe.
 * @param cascade   whether to cascade the color to the component objects
 */
void ComplexObstacle::setDebugColor(Color4 color, bool cascade) {
    Obstacle::setDebugColor(color);
    if (cascade) {
        for(auto it = _bodies.begin(); it!= _bodies.end(); ++it) {
            (*it)->setDebugColor(color);
        }
    }
}

/**
 * Sets the parent scene graph node for the debug wireframe
 *
 * The given node is the parent coordinate space for drawing physics.
 * All debug nodes for physics objects are drawn within this coordinate
 * space.  Setting the visibility of this node to false will disable
 * any debugging.  Similarly, setting this value to nullptr will
 * disable any debugging.
 *
 * This scene graph node is intended for debugging purposes only.  If
 * you want a physics body to update a proper texture image, you should
 * either use the method {@link update(float)} for subclasses or
 * {@link setListener} for decoupled classes.
 *
 * @param node  he parent scene graph node for the debug wireframe
 */
void ComplexObstacle::setDebugScene(const std::shared_ptr<Node>& node) {
    Obstacle::setDebugScene(node);
    for(auto it = _bodies.begin(); it != _bodies.end(); it++) {
        (*it)->setDebugScene(node);
    }
}

/**
 * Creates the outline of the physics fixtures in the debug wireframe
 *
 * The debug wireframe is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 */
void ComplexObstacle::resetDebug() {
    if (_debug == nullptr) {
        _debug = WireNode::alloc();
        _debug->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        _debug->setPosition(Vec2::ZERO);
        _debug->setColor(_dcolor);
    }
    _scene->addChild(_debug);
}

/**
 * Repositions the debug wireframe so that it agrees with the physics object.
 *
 * The debug wireframe is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 */
void ComplexObstacle::updateDebug() { }


#pragma mark -
#pragma mark Memory Management

/**
 * Deletes this physics object and all of its resources.
 *
 * We have to make the destructor public so that we can polymorphically
 * delete physics objects.
 *
 * The purpose of this destructor is to warn us if we delete an object
 * pre-maturely.
 */
ComplexObstacle::~ComplexObstacle() {
    CUAssertLog(_body == nullptr, "You must deactive physics before deleting an object");
    setDebugScene(nullptr);
    _bodies.clear();
}
