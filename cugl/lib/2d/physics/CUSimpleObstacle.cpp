//
//  CUSimpleObstacle.cpp
//  Cornell University Game Library (CUGL)
//
//  This class serves to provide a uniform interface for all single-body objects
//  (regardless of shape).  How, it still cannot be instantiated directly, as
//  the correct instantiation depends on the shape.  See BoxObstacle and
//  CircleObstacle for concrete examples.
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
#include <cugl/2d/physics/CUSimpleObstacle.h>
#include <Box2D/Dynamics/b2World.h>

using namespace cugl;


#pragma mark -
#pragma mark Fixture Methods

/**
 * Sets the density of this body
 *
 * The density is typically measured in usually in kg/m^2. The density can
 * be zero or positive. You should generally use similar densities for all
 * your fixtures. This will improve stacking stability.
 *
 * @param value  the density of this body
 */
void SimpleObstacle::setDensity(float value) {
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
 * The friction parameter is usually set between 0 and 1, but can be any
 * non-negative value. A friction value of 0 turns off friction and a value
 * of 1 makes the friction strong. When the friction force is computed
 * between two shapes, Box2D must combine the friction parameters of the
 * two parent fixtures. This is done with the geometric mean.
 *
 * @param value  the friction coefficient of this body
 */
void SimpleObstacle::setFriction(float value) {
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
 * The friction parameter is usually set between 0 and 1, but can be any
 * non-negative value. A friction value of 0 turns off friction and a value
 * of 1 makes the friction strong. When the friction force is computed
 * between two shapes, Box2D must combine the friction parameters of the
 * two parent fixtures. This is done with the geometric mean.
 *
 * @param value  the restitution of this body
 */
void SimpleObstacle::setRestitution(float value) {
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
 * Sometimes game logic needs to know when two entities overlap yet there
 * should be no collision response. This is done by using sensors. A sensor
 * is an entity that detects collision but does not produce a response.
 *
 * @param value  whether this object is a sensor.
 */
void SimpleObstacle::setSensor(bool value) {
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
 * Collision filtering allows you to prevent collision between fixtures. For
 * example, say you make a character that rides a bicycle. You want the
 * bicycle to collide with the terrain and the character to collide with
 * the terrain, but you don't want the character to collide with the bicycle
 * (because they must overlap). Box2D supports such collision filtering
 * using categories and groups.
 *
 * A value of null removes all collision filters.
 *
 * @param value  the filter data for this object
 */
void SimpleObstacle::setFilterData(b2Filter value) {
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
 * Implementations of this method should NOT retain ownership of the
 * Box2D world. That is a tight coupling that we should avoid.
 *
 * @param world Box2D world to store body
 *
 * @return true if object allocation succeeded
 */
bool SimpleObstacle::activatePhysics(b2World& world) {
    // Make a body, if possible
    _bodyinfo.active = true;
    _body = world.CreateBody(&_bodyinfo);
    _body->SetUserData((void*)this);
    
    // Only initialize if a body was created.
    if (_body != nullptr) {
        createFixtures();
        return true;
    }
    
    _bodyinfo.active = false;
    return false;
}

/**
 * Destroys the physics Body(s) of this object if applicable.
 *
 * This removes the body from the Box2D world.
 *
 * @param world Box2D world that stores body
 */
void SimpleObstacle::deactivatePhysics(b2World& world) {
    // Should be good for most (simple) applications.
    if (_body != nullptr) {
        releaseFixtures(); // Have to remove these first.
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
 * should not be used to process actions or any other gameplay information.
 * Its primary purpose is to adjust changes to the fixture, which have to
 * take place after collision.
 *
 * In other words, this is the method that updates the scene graph.  If you
 * forget to call it, it will not draw your changes.
 *
 * @param dt Timing values from parent loop
 */
void SimpleObstacle::update(float delta) {
    Obstacle::update(delta);
    // Recreate the fixture object if dimensions changed.
    if (isDirty()) {
        createFixtures();
    }
}


#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Repositions the scene node so that it agrees with the physics object.
 *
 * By default, the position of a node should be the body position times
 * the draw scale.  However, for some obstacles (particularly complex
 * obstacles), it may be desirable to turn the default functionality
 * off.  Hence we have made this virtual.
 */
void SimpleObstacle::updateDebug() {
    CUAssertLog(_scene, "Attempt to reposition a wireframe with no parent");
    Vec2 pos = getPosition();
    float angle = getAngle();
    
    // Positional snap
    if (_posSnap >= 0) {
        pos.x = floor((pos.x*_posFact+0.5f)/_posFact);
        pos.y = floor((pos.y*_posFact+0.5f)/_posFact);
    }
    // Rotational snap
    if (_angSnap >= 0) {
        angle = (float)(180*angle/M_PI);
        angle = floor((angle*_angFact+0.5f)/_angFact); // Formula is for degrees
        angle = (float)(M_PI*angle/180);
    }
    
    _debug->setPosition(pos);
    _debug->setAngle(angle);
}

