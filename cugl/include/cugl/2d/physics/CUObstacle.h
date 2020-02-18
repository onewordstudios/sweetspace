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
#ifndef __CU_OBSTACLE_H__
#define __CU_OBSTACLE_H__

#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <iostream>
#include <cugl/2d/CUWireNode.h>

namespace cugl {
    
#pragma mark -
#pragma mark Obstacle

/**
 * Base model class to support collisions.
 *
 * Instances represents a body and/or a group of bodies. There should be NO game
 * controlling logic code in a physics objects. That should reside in the 
 * Controllers.
 *
 * This abstract class has no Body or Shape information and should never be
 * instantiated directly. Instead, you should instantiate either SimpleObstacle 
 * or ComplexObstacle. This class only exists to unify common functionality. In
 * particular, it wraps the body and and fixture information into a single 
 * interface.
 *
 * Many of the method comments in this class are taken from the Box2d manual by
 * Erin Catto (2011).
 */
class Obstacle {
protected:
    /** Stores the body information for this shape */
    b2BodyDef _bodyinfo;
    /** Stores the fixture information for this shape */
    b2FixtureDef _fixture;
    /** The mass data of this shape (which may override the fixture) */
    b2MassData _massdata;
    /** Whether or not to use the custom mass data */
    bool _masseffect;
    
    /** The wireframe parent for debugging. */
    std::shared_ptr<Node> _scene;
    /** The wireframe node for debugging. */
    std::shared_ptr<WireNode> _debug;
    /** The wireframe color for debugging */
    Color4 _dcolor;
    /** A tag for debugging purposes */
    std::string _tag;
    
    /** (Singular) callback function for state updates */
    std::function<void(Obstacle* obstacle)> _listener;
    
#pragma mark -
#pragma mark Scene Graph Internals
    /**
     * Creates the outline of the physics fixtures in the debug wireframe
     *
     * The debug wireframe is use to outline the fixtures attached to this object.
     * This is very useful when the fixtures have a very different shape than
     * the texture (e.g. a circular shape attached to a square texture).
     */
    virtual void resetDebug() { }
    
    /**
     * Repositions the debug wireframe so that it agrees with the physics object.
     *
     * The debug wireframe is use to outline the fixtures attached to this object.
     * This is very useful when the fixtures have a very different shape than
     * the texture (e.g. a circular shape attached to a square texture).
     */
     virtual void updateDebug();

    
private:
    /// Track garbage collection status
    /** Whether the object should be removed from the world on next pass */
    bool _remove;
    /** Whether the object has changed shape and needs a new fixture */
    bool _dirty;
    

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new physics object at the origin.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead (in this case, in
     * one of the subclasses).
     */
    Obstacle(void);
    
    /**
     * Deletes this physics object and all of its resources.
     *
     * We have to make the destructor public so that we can polymorphically
     * delete physics objects.
     *
     * A non-default destructor is necessary since we must release all
     * claims on scene graph nodes.
     */
    virtual ~Obstacle();
    
    /**
     * Initializes a new physics object at the origin.
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() { return init(Vec2::ZERO); }
    
    /**
     * Initializes a new physics object at the given point
     *
     * @param  vec  Initial position in world coordinates
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2& vec);

    
#pragma mark -
#pragma mark BodyDef Methods
    /**
     * Returns the body type for Box2D physics
     *
     * If you want to lock a body in place (e.g. a platform) set this value to 
     * STATIC. KINEMATIC allows the object to move (and some limited collisions), 
     * but ignores external forces (e.g. gravity). DYNAMIC makes this is a 
     * full-blown physics object.
     *
     * @return the body type for Box2D physics
     */
    virtual b2BodyType getBodyType() const { return _bodyinfo.type; }
    
    /**
     * Sets the body type for Box2D physics
     *
     * If you want to lock a body in place (e.g. a platform) set this value to
     * STATIC. KINEMATIC allows the object to move (and some limited collisions),
     * but ignores external forces (e.g. gravity). DYNAMIC makes this is a
     * full-blown physics object.
     *
     * @param value the body type for Box2D physics
     */
    virtual void setBodyType(b2BodyType value) { _bodyinfo.type = value; }
    
    /**
     * Returns the current position for this physics body
     *
     * This method converts from a Box2D vector type to a CUGL vector type. This
     * cuts down on the confusion between vector types.  It also means that 
     * changes to the returned vector will have no effect on this obstacle.
     *
     * @return the current position for this physics body
     */
    virtual Vec2 getPosition() const { return Vec2(_bodyinfo.position.x,_bodyinfo.position.y); }
    
    /**
     * Sets the current position for this physics body
     *
     * This method converts from a CUGL vector type to a Box2D vector type.
     * This cuts down on the confusion between vector types.
     *
     * @param value  the current position for this physics body
     */
    virtual void setPosition(const Vec2& value) { setPosition(value.x,value.y); }
    
    /**
     * Sets the current position for this physics body
     *
     * @param x  the current x-coordinate for this physics body
     * @param y  the current y-coordinate for this physics body
     */
    virtual void setPosition(float x, float y) { _bodyinfo.position.Set(x,y); }
    
    /**
     * Returns the x-coordinate for this physics body
     *
     * @return the x-coordinate for this physics body
     */
    virtual float getX() const { return _bodyinfo.position.x; }
    
    /**
     * Sets the x-coordinate for this physics body
     *
     * @param value  the x-coordinate for this physics body
     */
    virtual void setX(float value) { _bodyinfo.position.x = value; }
    
    /**
     * Returns the y-coordinate for this physics body
     *
     * @return the y-coordinate for this physics body
     */
    virtual float getY() const { return _bodyinfo.position.y; }
    
    /**
     * Sets the y-coordinate for this physics body
     *
     * @param value  the y-coordinate for this physics body
     */
    virtual void setY(float value) { _bodyinfo.position.y = value; }
    
    /**
     * Returns the angle of rotation for this body (about the center).
     *
     * The value returned is in radians
     *
     * @return the angle of rotation for this body
     */
    virtual float getAngle() const { return _bodyinfo.angle; }
    
    /**
     * Sets the angle of rotation for this body (about the center).
     *
     * @param value  the angle of rotation for this body (in radians)
     */
    virtual void setAngle(float value) { _bodyinfo.angle = value; }
    
    /**
     * Returns the linear velocity for this physics body
     *
     * This method converts from a Box2D vector type to a CUGL vector type. This
     * cuts down on the confusion between vector types.  It also means that 
     * changes to the returned vector will have no effect on this object.
     *
     * @return the linear velocity for this physics body
     */
    virtual Vec2 getLinearVelocity() const {
        return Vec2(_bodyinfo.linearVelocity.x,_bodyinfo.linearVelocity.y);
    }
    
    /**
     * Sets the linear velocity for this physics body
     *
     * This method converts from a CUGL vector type to a Box2D vector type. This
     * cuts down on the confusion between vector types.
     *
     * @param value  the linear velocity for this physics body
     */
    virtual void setLinearVelocity(const Vec2& value) { setLinearVelocity(value.x,value.y); }
    
    /**
     * Sets the linear velocity for this physics body
     *
     * @param x  the x-coordinate of the linear velocity
     * @param y  the y-coordinate of the linear velocity
     */
    virtual void setLinearVelocity(float x, float y) { _bodyinfo.linearVelocity.Set(x,y); }
    
    /**
     * Returns the x-velocity for this physics body
     *
     * @return the x-velocity for this physics body
     */
    virtual float getVX() const { return _bodyinfo.linearVelocity.x; }
    
    /**
     * Sets the x-velocity for this physics body
     *
     * @param value  the x-velocity for this physics body
     */
    virtual void setVX(float value) { _bodyinfo.linearVelocity.x = value; }
    
    /**
     * Returns the y-velocity for this physics body
     *
     * @return the y-velocity for this physics body
     */
    virtual float getVY() const { return _bodyinfo.linearVelocity.y; }
    
    /**
     * Sets the y-velocity for this physics body
     *
     * @param value  the y-velocity for this physics body
     */
    virtual void setVY(float value) { _bodyinfo.linearVelocity.y = value; }
    
    /**
     * Returns the angular velocity for this physics body
     *
     * The rate of change is measured in radians per step
     *
     * @return the angular velocity for this physics body
     */
    virtual float getAngularVelocity() const { return _bodyinfo.angularVelocity; }
    
    /**
     * Sets the angular velocity for this physics body
     *
     * @param value the angular velocity for this physics body (in radians)
     */
    virtual void setAngularVelocity(float value) { _bodyinfo.angularVelocity = value; }
    
    /**
     * Returns true if the body is active
     *
     * An inactive body not participate in collision or dynamics. This state is 
     * similar to sleeping except the body will not be woken by other bodies and 
     * the body's fixtures will not be placed in the broad-phase. This means the 
     * body will not participate in collisions, ray casts, etc.
     *
     * @return true if the body is active
     */
    virtual bool isActive() const { return _bodyinfo.active; }
    
    /**
     * Sets whether the body is active
     *
     * An inactive body not participate in collision or dynamics. This state is 
     * similar to sleeping except the body will not be woken by other bodies and 
     * the body's fixtures will not be placed in the broad-phase. This means the 
     * body will not participate in collisions, ray casts, etc.
     *
     * @param value  whether the body is active
     */
    virtual void setActive(bool value) { _bodyinfo.active = value; }
    
    /**
     * Returns true if the body is awake
     *
     * An sleeping body is one that has come to rest and the physics engine has 
     * decided to stop simulating it to save CPU cycles. If a body is awake and 
     * collides with a sleeping body, then the sleeping body wakes up. Bodies 
     * will also wake up if a joint or contact attached to them is destroyed.  
     * You can also wake a body manually.
     *
     * @return true if the body is awake
     */
    virtual bool isAwake() const { return _bodyinfo.awake; }
    
    /**
     * Sets whether the body is awake
     *
     * An sleeping body is one that has come to rest and the physics engine has
     * decided to stop simulating it to save CPU cycles. If a body is awake and
     * collides with a sleeping body, then the sleeping body wakes up. Bodies
     * will also wake up if a joint or contact attached to them is destroyed.
     * You can also wake a body manually.
     *
     * @param value  whether the body is awake
     */
    virtual void setAwake(bool value) { _bodyinfo.awake = value; }
    
    /**
     * Returns false if this body should never fall asleep
     *
     * An sleeping body is one that has come to rest and the physics engine has
     * decided to stop simulating it to save CPU cycles. If a body is awake and
     * collides with a sleeping body, then the sleeping body wakes up. Bodies
     * will also wake up if a joint or contact attached to them is destroyed.
     * You can also wake a body manually.
     *
     * @return false if this body should never fall asleep
     */
    virtual bool isSleepingAllowed() const { return _bodyinfo.allowSleep; }
    
    /**
     * Sets whether the body should ever fall asleep
     *
     * An sleeping body is one that has come to rest and the physics engine has
     * decided to stop simulating it to save CPU cycles. If a body is awake and
     * collides with a sleeping body, then the sleeping body wakes up. Bodies
     * will also wake up if a joint or contact attached to them is destroyed.
     * You can also wake a body manually.
     *
     * @param value  whether the body should ever fall asleep
     */
    virtual void setSleepingAllowed(bool value) { _bodyinfo.allowSleep = value; }
    
    /**
     * Returns true if this body is a bullet
     *
     * By default, Box2D uses continuous collision detection (CCD) to prevent 
     * dynamic bodies from tunneling through static bodies. Normally CCD is not 
     * used between dynamic bodies. This is done to keep performance reasonable. 
     * In some game scenarios you need dynamic bodies to use CCD. For example, 
     * you may want to shoot a high speed bullet at a stack of dynamic bricks. 
     * Without CCD, the bullet might tunnel through the bricks.
     *
     * Fast moving objects in Box2D can be labeled as bullets. Bullets will 
     * perform CCD with both static and dynamic bodies. You should decide what 
     * bodies should be bullets based on your game design.
     *
     * @return true if this body is a bullet
     */
    virtual bool isBullet() const { return _bodyinfo.bullet; }
    
    /**
     * Sets whether this body is a bullet
     *
     * By default, Box2D uses continuous collision detection (CCD) to prevent
     * dynamic bodies from tunneling through static bodies. Normally CCD is not
     * used between dynamic bodies. This is done to keep performance reasonable.
     * In some game scenarios you need dynamic bodies to use CCD. For example,
     * you may want to shoot a high speed bullet at a stack of dynamic bricks.
     * Without CCD, the bullet might tunnel through the bricks.
     *
     * Fast moving objects in Box2D can be labeled as bullets. Bullets will
     * perform CCD with both static and dynamic bodies. You should decide what
     * bodies should be bullets based on your game design.
     *
     * @param value  whether this body is a bullet
     */
    virtual void setBullet(bool value) { _bodyinfo.bullet = value; }
    
    /**
     * Returns true if this body be prevented from rotating
     *
     * This is very useful for characters that should remain upright.
     *
     * @return true if this body be prevented from rotating
     */
    virtual bool isFixedRotation() const { return _bodyinfo.fixedRotation; }
    
    /**
     * Sets whether this body be prevented from rotating
     *
     * This is very useful for characters that should remain upright.
     *
     * @param value  whether this body be prevented from rotating
     */
    virtual void setFixedRotation(bool value) { _bodyinfo.fixedRotation = value; }
    
    /**
     * Returns the gravity scale to apply to this body
     *
     * This allows isolated objects to float.  Be careful with this, since
     * increased gravity can decrease stability.
     *
     * @return the gravity scale to apply to this body
     */
    virtual float getGravityScale() const { return _bodyinfo.gravityScale; }
    
    /**
     * Sets the gravity scale to apply to this body
     *
     * This allows isolated objects to float.  Be careful with this, since
     * increased gravity can decrease stability.
     *
     * @param value  the gravity scale to apply to this body
     */
    virtual void setGravityScale(float value) { _bodyinfo.gravityScale = value; }
    
    /**
     * Returns the linear damping for this body.
     *
     * Linear damping is use to reduce the linear velocity. Damping is different 
     * than friction because friction only occurs with contact. Damping is not a 
     * replacement for friction and the two effects should be used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no 
     * damping, and infinity meaning full damping. Normally you will use a 
     * damping value between 0 and 0.1. Most people avoid linear damping because 
     * it makes bodies look floaty.
     *
     * @return the linear damping for this body.
     */
    virtual float getLinearDamping() const { return _bodyinfo.linearDamping; }
    
    /**
     * Sets the linear damping for this body.
     *
     * Linear damping is use to reduce the linear velocity. Damping is different
     * than friction because friction only occurs with contact. Damping is not a
     * replacement for friction and the two effects should be used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no
     * damping, and infinity meaning full damping. Normally you will use a
     * damping value between 0 and 0.1. Most people avoid linear damping because
     * it makes bodies look floaty.
     *
     * @param value  the linear damping for this body.
     */
    virtual void setLinearDamping(float value) { _bodyinfo.linearDamping = value; }
    
    /**
     * Returns the angular damping for this body.
     *
     * Angular damping is use to reduce the angular velocity. Damping is 
     * different than friction because friction only occurs with contact. 
     * Damping is not a replacement for friction and the two effects should be 
     * used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no 
     * damping, and infinity meaning full damping. Normally you will use a 
     * damping value between 0 and 0.1.
     *
     * @return the angular damping for this body.
     */
    virtual float getAngularDamping() const { return _bodyinfo.angularDamping; }
    
    /**
     * Sets the angular damping for this body.
     *
     * Angular damping is use to reduce the angular velocity. Damping is
     * different than friction because friction only occurs with contact.
     * Damping is not a replacement for friction and the two effects should be
     * used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no
     * damping, and infinity meaning full damping. Normally you will use a
     * damping value between 0 and 0.1.
     *
     * @param value  the angular damping for this body.
     */
    virtual void setAngularDamping(float value) { _bodyinfo.angularDamping = value; }
    
    /**
     * Copies the state from the given body to the body def.
     *
     * This is important if you want to save the state of the body before
     * removing it from the world.
     */
    void setBodyState(const b2Body& body);

    
#pragma mark -
#pragma mark FixtureDef Methods
    /**
     * Returns the density of this body
     *
     * The density is typically measured in usually in kg/m^2. The density can 
     * be zero or positive. You should generally use similar densities for all 
     * your fixtures. This will improve stacking stability.
     *
     * @return the density of this body
     */
    float getDensity() const { return _fixture.density; }
    
    /**
     * Sets the density of this body
     *
     * The density is typically measured in usually in kg/m^2. The density can 
     * be zero or positive. You should generally use similar densities for all 
     * your fixtures. This will improve stacking stability.
     *
     * @param value  the density of this body
     */
    virtual void setDensity(float value) { _fixture.density = value; }
    
    /**
     * Returns the friction coefficient of this body
     *
     * The friction parameter is usually set between 0 and 1, but can be any 
     * non-negative value. A friction value of 0 turns off friction and a value 
     * of 1 makes the friction strong. When the friction force is computed 
     * between two shapes, Box2D must combine the friction parameters of the 
     * two parent fixtures. This is done with the geometric mean.
     *
     * @return the friction coefficient of this body
     */
    float getFriction() const { return _fixture.friction; }
    
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
    virtual void setFriction(float value) { _fixture.friction = value; }
    
    /**
     * Returns the restitution of this body
     *
     * Restitution is used to make objects bounce. The restitution value is 
     * usually set to be between 0 and 1. Consider dropping a ball on a table. 
     * A value of zero means the ball won't bounce. This is called an inelastic 
     * collision. A value of one means the ball's velocity will be exactly 
     * reflected. This is called a perfectly elastic collision.
     *
     * @return the restitution of this body
     */
    float getRestitution() const { return _fixture.restitution; }
    
    /**
     * Sets the restitution of this body
     *
     * Restitution is used to make objects bounce. The restitution value is
     * usually set to be between 0 and 1. Consider dropping a ball on a table.
     * A value of zero means the ball won't bounce. This is called an inelastic
     * collision. A value of one means the ball's velocity will be exactly
     * reflected. This is called a perfectly elastic collision.
     *
     * @param value  the restitution of this body
     */
    virtual void setRestitution(float value) { _fixture.restitution = value; }
    
    /**
     * Returns true if this object is a sensor.
     *
     * Sometimes game logic needs to know when two entities overlap yet there 
     * should be no collision response. This is done by using sensors. A sensor 
     * is an entity that detects collision but does not produce a response.
     *
     * @return true if this object is a sensor.
     */
    bool isSensor() const { return _fixture.isSensor; }
    
    /**
     * Sets whether this object is a sensor.
     *
     * Sometimes game logic needs to know when two entities overlap yet there
     * should be no collision response. This is done by using sensors. A sensor
     * is an entity that detects collision but does not produce a response.
     *
     * @param value  whether this object is a sensor.
     */
    virtual void setSensor(bool value) { _fixture.isSensor = value; }
    
    /**
     * Returns the filter data for this object (or null if there is none)
     *
     * Collision filtering allows you to prevent collision between fixtures. For 
     * example, say you make a character that rides a bicycle. You want the 
     * bicycle to collide with the terrain and the character to collide with 
     * the terrain, but you don't want the character to collide with the bicycle 
     * (because they must overlap). Box2D supports such collision filtering 
     * using categories and groups.
     *
     * @return the filter data for this object (or null if there is none)
     */
    b2Filter getFilterData() const { return _fixture.filter; }
    
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
    virtual void setFilterData(b2Filter value) { _fixture.filter = value; }
    
    
#pragma mark -
#pragma mark MassData Methods
    /**
     * Returns the center of mass of this body
     *
     * This method converts from a Box2D vector type to a CUGL vector type. This
     * cuts down on the confusion between vector types.  It also means that
     * changes to the returned vector will have no effect on this object.
     *
     * @return the center of mass for this physics body
     */
    virtual Vec2 getCentroid() const { return Vec2(_massdata.center.x,_massdata.center.y); }
    
    /**
     * Sets the center of mass for this physics body
     *
     * This method converts from a CUGL vector type to a Box2D vector type. This
     * cuts down on the confusion between vector types.
     *
     * @param value  the center of mass for this physics body
     */
    virtual void setCentroid(const Vec2& value) { setCentroid(value.x,value.y); }
    
    /**
     * Sets the center of mass for this physics body
     *
     * @param x  the x-coordinate of the center of mass for this physics body
     * @param y  the y-coordinate of the center of mass for this physics body
     */
    virtual void setCentroid(float x, float y);
    
    /**
     * Returns the rotational inertia of this body
     *
     * For static bodies, the mass and rotational inertia are set to zero. When
     * a body has fixed rotation, its rotational inertia is zero.
     *
     * @return the rotational inertia of this body
     */
    virtual float getInertia() const { return _massdata.I; }
    
    /**
     * Sets the rotational inertia of this body
     *
     * For static bodies, the mass and rotational inertia are set to zero. When
     * a body has fixed rotation, its rotational inertia is zero.
     *
     * @param value  the rotational inertia of this body
     */
    virtual void setInertia(float value);
    
    /**
     * Returns the mass of this body
     *
     * The value is usually in kilograms.
     *
     * @return the mass of this body
     */
    virtual float getMass() const { return _massdata.mass; }
    
    /**
     * Sets the mass of this body
     *
     * The value is usually in kilograms.
     *
     * @param value  the mass of this body
     */
    virtual void setMass(float value);
    
    /**
     * Resets this body to use the mass computed from the its shape and density
     */
    virtual void resetMass() { _masseffect = false; }
    
    
#pragma mark -
#pragma mark Garbage Collection
    /**
     * Returns true if our object has been flagged for garbage collection
     *
     * A garbage collected object will be removed from the physics world at
     * the next time step.
     *
     * @return true if our object has been flagged for garbage collection
     */
    bool isRemoved() const { return _remove; }
    
    /**
     * Sets whether our object has been flagged for garbage collection
     *
     * A garbage collected object will be removed from the physics world at
     * the next time step.
     *
     * @param value  whether our object has been flagged for garbage collection
     */
    void markRemoved(bool value) { _remove = value; }
    
    /**
     * Returns true if the shape information must be updated.
     *
     * Attributes tied to the geometry (and not just forces/position) must wait for
     * collisions to complete before they are reset.  Shapes (and their properties)
     * are reset in the update method.
     *
     * @return true if the shape information must be updated.
     */
    bool isDirty() const { return _dirty; }
    
    /**
     * Sets whether the shape information must be updated.
     *
     * Attributes tied to the geometry (and not just forces/position) must wait for
     * collisions to complete before they are reset.  Shapes (and their properties)
     * are reset in the update method.
     *
     * @param value  whether the shape information must be updated.
     */
    void markDirty(bool value) { _dirty = value; }
    
#pragma mark -
#pragma mark Physics Methods
    /**
     * Returns a (weak) reference to Box2D body for this obstacle.
     *
     * You use this body to add joints and apply forces. As a weak reference, 
     * this physics obstacle does not transfer ownership of this body.  In 
     * addition, the value may be a nullptr.
     *
     * @return a (weak) reference to Box2D body for this obstacle.
     */
    virtual b2Body* getBody() { return nullptr; }
    
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
    virtual bool activatePhysics(b2World& world) { return false; }
    
    /**
     * Destroys the physics Body(s) of this object if applicable.
     *
     * This removes the body from the Box2D world.
     *
     * @param world Box2D world that stores body
     */
    virtual void deactivatePhysics(b2World& world) {}

    
#pragma mark -
#pragma mark Update Methods
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
     * @param delta Timing values from parent loop
     */
    virtual void update(float delta) {
        if (_scene) { updateDebug(); }
        if (_listener) { _listener(this); }
    }
    
    /**
     * Returns the active listener to this object.
     *
     * Listeners are called after every physics update, to notify them of any
     * changes in this object state.  For performance reasons, a physics 
     * obstacle can have only one listener.  If you need multiple objects 
     * listening to a single physics obstacle, the listener should handle the
     * dispatch to other objects.
     *
     * @return the active listener to this object.
     */
    const std::function<void(Obstacle* obstacle)>& getListener() const {
        return _listener;
    }

    /**
     * Sets the active listener to this object.
     *
     * Listeners are called after every physics update, to notify them of any
     * changes in this object state.  For performance reasons, a physics
     * obstacle can have only one listener.  If you need multiple objects
     * listening to a single physics obstacle, the listener should handle the
     * dispatch to other objects.
     *
     * @param listener  the active listener to this object.
     */
    void setListener(const std::function<void(Obstacle* obstacle)>& listener) {
        _listener = listener;
    }

#pragma mark -
#pragma mark Debugging Methods
    /**
     * Returns the physics object tag.
     * 
     * A tag is a string attached to an object, in order to identify it in 
     * debugging.
     *
     * @return the physics object tag.
     */
    std::string getName() const { return _tag; }
    
    /**
     * Sets the physics object tag.
     *
     * A tag is a string attached to an object, in order to identify it in 
     * debugging.
     *
     * @param  value    the physics object tag
     */
    void setName(std::string value) { _tag = value; }

    /**
     * Returns a string representation of this physics object.
     *
     * This method converts the physics object into a string for debugging. By
     * default it shows the tag and position.  Other physics objects may want
     * to override this method for more detailed information.
     *
     * @return a string representation of this physics object
     */
    std::string toString() const;
    
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
    friend std::ostream&   operator<<(std::ostream& os, const Obstacle& obj);

    
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
    virtual void setDebugColor(Color4 color);
    
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
     * either use the method {@link update(float)} for subclasses or
     * {@link setListener} for decoupled classes.
     *
     * @return the scene graph node for the debug wireframe
     */
    Node* getDebugScene() const { return _scene.get(); }
    
    /**
     * Returns the scene graph node for the debug wireframe
     *
     * The returned node draws a wireframe of the physics body. The wireframe
     * consists of the physics fixtures adjusted by the drawing scale.  The
     * debug node is positioned in the coordinate space of the parent scene.
     *
     * The wireframe will be drawn using physics coordinates, which is possibly
     * much smaller than your drawing coordinates (e.g. 1 Box2D unit = 1 pixel).
     * If you want the wireframes to be larger, you should scale the parent
     * parent coordinate space to match the rest of the application.
     *
     * This scene graph node is intended for debugging purposes only.  If
     * you want a physics body to update a proper texture image, you should
     * either use the method {@link update(float)} for subclasses or
     * {@link setListener} for decoupled classes.
     *
     * @return the scene graph node for the debug wireframe
     */
    WireNode* getDebugNode() const { return _debug.get(); }
    
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
     * either use the method {@link update(float)} for subclasses or
     * {@link setListener} for decoupled classes.
     *
     * @param node  he parent scene graph node for the debug wireframe
     */
    virtual void setDebugScene(const std::shared_ptr<Node>& node);
    
    /**
     * Returns true if the obstacle has a wireframe for debugging.
     *
     * This method will return false if there is no active parent scene
     * for the wireframe.
     *
     * @return true if the obstacle has a wireframe for debugging.
     */
    bool hasDebug() { return _scene != nullptr; }
};

}

#endif /* __CU_OBSTACLE_H__ */
