//
//  CUComplexObstacle.h
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
#ifndef __CC_COMPLEX_OBSTACLE_H__
#define __CC_COMPLEX_OBSTACLE_H__

#include <Box2D/Dynamics/Joints/b2Joint.h>
#include "CUObstacle.h"


namespace cugl {

#pragma mark -
#pragma mark Complex Obstacle


/**
 * Composite model class to support collisions.
 *
 * ComplexObstacle instances are built of many bodies, and are assumed to be
 * connected by joints (though this is not actually a requirement). This is the 
 * class to use for chains, ropes, levers, and so on. This class does not 
 * provide Shape information, and cannot be instantiated directly. There are 
 * no default complex objects.  You will need to create your own subclasses to 
 * use this class.
 *
 * ComplexObstacle is a hierarchical class.  It groups children as Obstacles, 
 * not not bodies.  So you could have a ComplexObstacle made up of other
 * ComplexObstacles. However, it is not the same as a scene graph.  Children 
 * have absolute, nott relative, position data.  Indeed, this class illustrates 
 * the need for decoupling the physics representation from the scene graph.
 *
 * Transformations to an object of this class are restricted to the root body.
 * They do not automatically effect the children (like a scene graph).  If you
 * want changes to the root body to effect the children, you should connect 
 * them with joints and allow Box2D to handle this.
 *
 * Many of the method comments in this class are taken from the Box2d manual by
 * Erin Catto (2011).
 */
class ComplexObstacle : public Obstacle {
protected:
    /** A root body for this box 2d. */
    b2Body* _body;
    /** A complex physics object has multiple bodies */
    std::vector<std::shared_ptr<Obstacle>> _bodies;
    /** Potential joints for connecting the multiple bodies */
    std::vector<b2Joint*>  _joints;
    
#pragma mark -
#pragma mark Scene Graph Internals
    /**
     * Creates the outline of the physics fixtures in the debug wireframe
     *
     * The debug wireframe is use to outline the fixtures attached to this object.
     * This is very useful when the fixtures have a very different shape than
     * the texture (e.g. a circular shape attached to a square texture).
     */
    virtual void resetDebug() override;
    
    /**
     * Repositions the debug wireframe so that it agrees with the physics object.
     *
     * The debug wireframe is use to outline the fixtures attached to this object.
     * This is very useful when the fixtures have a very different shape than
     * the texture (e.g. a circular shape attached to a square texture).
     */
    virtual void updateDebug() override;

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new complex physics object at the origin.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead (in this case, in
     * one of the subclasses).
     */
    ComplexObstacle() : Obstacle(), _body(nullptr) { }
    
    /**
     * Deletes this physics object and all of its resources.
     *
     * We have to make the destructor public so that we can polymorphically
     * delete physics objects.
     *
     * The purpose of this destructor is to warn us if we delete an object
     * pre-maturely.
     */
    virtual ~ComplexObstacle();
 
    
#pragma mark -
#pragma mark BodyDef Methods
    /**
     * Returns the body type for Box2D physics
     *
     * If you want to lock a body in place (e.g. a platform) set this value to STATIC.
     * KINEMATIC allows the object to move (and some limited collisions), but ignores
     * external forces (e.g. gravity). DYNAMIC makes this is a full-blown physics object.
     *
     * This method returns the body type for the root object of this composite structure.
     *
     * @return the body type for Box2D physics
     */
    virtual b2BodyType getBodyType() const override {
        return (_body != nullptr ? _body->GetType() : _bodyinfo.type);
    }
    
    /**
     * Sets the body type for Box2D physics
     *
     * If you want to lock a body in place (e.g. a platform) set this value to STATIC.
     * KINEMATIC allows the object to move (and some limited collisions), but ignores
     * external forces (e.g. gravity). DYNAMIC makes this is a full-blown physics object.
     *
     * This sets the body type for the root body only.  If you want to set the type
     * of other objects in this class, iterate over the children.
     *
     * @param value the body type for Box2D physics
     */
    virtual void setBodyType(b2BodyType value) override {
        if (_body != nullptr) {
            _body->SetType(value);
        } else {
            _bodyinfo.type = value;
        }
    }
    
    /**
     * Returns the current position for this physics body
     *
     * This method does NOT return a reference to the position vector. Changes to this
     * vector will not affect the body.  However, it returns the same vector each time
     * its is called, and so cannot be used as an allocator.
     *
     * This method returns the position for the root object of this composite structure.
     *
     * @return the current position for this physics body
     */
    virtual Vec2 getPosition() const override {
        if (_body != nullptr) {
            return Vec2(_body->GetPosition().x,_body->GetPosition().y);
        } else {
            return Vec2(_bodyinfo.position.x,_bodyinfo.position.y);
        }
    }
    
    /**
     * Sets the current position for this physics body
     *
     * This method converts from a Cocos2D vector type to a Box2D vector type. This
     * cuts down on the confusion between vector types.
     *
     * @param value  the current position for this physics body
     */
    virtual void setPosition(const Vec2& value) override { setPosition(value.x,value.y); }
    
    /**
     * Sets the current position for this physics body
     *
     * This method sets the position for the root body of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  x    the x-coordinate of the linear velocity
     * @param  y    the y-coordinate of the linear velocity
     */
    virtual void setPosition(float x, float y) override {
        if (_body != nullptr) {
            _body->SetTransform(b2Vec2(x,y),_body->GetAngle());
        } else {
            _bodyinfo.position.Set(x,y);
        }
    }
    
    /**
     * Returns the x-coordinate for this physics body
     *
     * This method uses the position for the root object of this composite structure.
     *
     * @return the x-coordinate for this physics body
     */
    virtual float getX() const override {
        return (_body != nullptr ? _body->GetPosition().x : _bodyinfo.position.x);
    }
    
    /**
     * Sets the x-coordinate for this physics body
     *
     * This method sets the position for the root body of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  value  the x-coordinate for this physics body
     */
    virtual void setX(float value) override {
        if (_body != nullptr) {
            _body->SetTransform(b2Vec2(value,_body->GetPosition().y),_body->GetAngle());
        } else {
            _bodyinfo.position.x = value;
        }
    }
    
    /**
     * Returns the y-coordinate for this physics body
     *
     * This method uses the position for the root object of this composite structure.
     *
     * @return the y-coordinate for this physics body
     */
    virtual float getY() const override {
        return (_body != nullptr ? _body->GetPosition().y : _bodyinfo.position.y);
    }
    
    /**
     * Sets the y-coordinate for this physics body
     *
     * This method sets the position for the root body of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  value  the y-coordinate for this physics body
     */
    virtual void setY(float value) override {
        if (_body != nullptr) {
            _body->SetTransform(b2Vec2(_body->GetPosition().x,value),_body->GetAngle());
        } else {
            _bodyinfo.position.y = value;
        }
    }
    
    /**
     * Returns the angle of rotation for this body (about the center).
     *
     * The value is determined by the angle of the root object of this
     * composite structure.  The value returned is in radians.
     *
     * @return the angle of rotation for this body
     */
    virtual float getAngle() const override {
        return (_body != nullptr ? _body->GetAngle() : _bodyinfo.angle);
    }
    
    /**
     * Sets the angle of rotation for this body (about the center).
     *
     * This method sets the angle for the root body of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  value  the angle of rotation for this body (in radians)
     */
    virtual void setAngle(float value) override {
        if (_body != nullptr) {
            _body->SetTransform(_body->GetPosition(),value);
        } else {
            _bodyinfo.angle = value;
        }
    }
    
    /**
     * Returns the linear velocity for this physics body
     *
     * This method does NOT return a reference to the velocity vector. Changes to this
     * vector will not affect the body.  However, it returns the same vector each time
     * its is called, and so cannot be used as an allocator.
     *
     * This method returns the velocity for the root of this composite structure.
     *
     * @return the linear velocity for this physics body
     */
    virtual Vec2 getLinearVelocity() const override {
        if (_body != nullptr) {
            return Vec2(_body->GetLinearVelocity().x,_body->GetLinearVelocity().y);
        } else {
            return Vec2(_bodyinfo.linearVelocity.x,_bodyinfo.linearVelocity.y);
        }
    }
    
    /**
     * Sets the linear velocity for this physics body
     *
     * This method converts from a Cocos2D vector type to a Box2D vector type. This
     * cuts down on the confusion between vector types.
     *
     * @param value  the linear velocity for this physics body
     */
    virtual void setLinearVelocity(const Vec2& value) override { setLinearVelocity(value.x,value.y); }
    
    /**
     * Sets the linear velocity for this physics body
     *
     * This method sets the linear velocity for the root of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  x    the x-coordinate of the linear velocity
     * @param  y    the y-coordinate of the linear velocity
     */
    virtual void setLinearVelocity(float x, float y) override {
        if (_body != nullptr) {
            _body->SetLinearVelocity(b2Vec2(x,y));
        } else {
            _bodyinfo.linearVelocity.Set(x,y);
        }
    }
    
    /**
     * Returns the x-velocity for this physics body
     *
     * This method uses the velocity for the root of this composite structure.
     *
     * @return the x-velocity for this physics body
     */
    virtual float getVX() const override {
        if (_body != nullptr) {
            return _body->GetLinearVelocity().x;
        } else {
            return _bodyinfo.linearVelocity.x;
        }
    }
    
    /**
     * Sets the x-velocity for this physics body
     *
     * This method sets the  velocity for the root body of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  value  the x-velocity for this physics body
     */
    virtual void setVX(float value) override {
        if (_body != nullptr) {
            _body->SetLinearVelocity(b2Vec2(value,_body->GetLinearVelocity().y));
        } else {
            _bodyinfo.linearVelocity.x = value;
        }
    }
    
    /**
     * Returns the y-velocity for this physics body
     *
     * @return the y-velocity for this physics body
     */
    virtual float getVY() const override {
        if (_body != nullptr) {
            return _body->GetLinearVelocity().y;
        } else {
            return _bodyinfo.linearVelocity.y;
        }
    }
    
    /**
     * Sets the y-velocity for this physics body
     *
     * This method sets the velocity for the root body of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  value  the y-velocity for this physics body
     */
    virtual void setVY(float value) override {
        if (_body != nullptr) {
            _body->SetLinearVelocity(b2Vec2(value,_body->GetLinearVelocity().y));
        } else {
            _bodyinfo.linearVelocity.x = value;
        }
    }
    
    /**
     * Returns the angular velocity for this physics body
     *
     * This method uses the velocity for the root object of this composite structure.
     * The rate of change is measured in radians per step
     *
     * @return the angular velocity for this physics body
     */
    virtual float getAngularVelocity() const override {
        return (_body != nullptr ? _body->GetAngularVelocity() : _bodyinfo.angularVelocity);
    }
    
    /**
     * Sets the angular velocity for this physics body
     *
     * This method sets the velocity for the root body of this composite structure
     * only.  If you want to set the value for any of the child obstacles, iterate
     * over the children. Alternatively attach the other children to the parent
     * with joints so that they move together.
     *
     * @param  value    the angular velocity for this physics body (in radians)
     */
    virtual void setAngularVelocity(float value) override {
        if (_body != nullptr) {
            _body->SetAngularVelocity(value);
        } else {
            _bodyinfo.angularVelocity = value;
        }
    }
    
    /**
     * Returns true if the body is active
     *
     * An inactive body not participate in collision or dynamics. This state is similar
     * to sleeping except the body will not be woken by other bodies and the body's
     * fixtures will not be placed in the broad-phase. This means the body will not
     * participate in collisions, ray casts, etc.
     *
     * This method only tests the activity of the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return true if the body is active
     */
    virtual bool isActive() const override {
        return (_body != nullptr ? _body->IsActive() : _bodyinfo.active);
    }
    
    /**
     * Sets whether the body is active
     *
     * An inactive body not participate in collision or dynamics. This state is similar
     * to sleeping except the body will not be woken by other bodies and the body's
     * fixtures will not be placed in the broad-phase. This means the body will not
     * participate in collisions, ray casts, etc.
     *
     * This method affects the root body of this composite structure only.  If you want
     * to set the value for any of the child obstacles, iterate over the children.
     *
     * @param value  whether the body is active
     */
    virtual void setActive(bool value) override {
        if (_body != nullptr) {
            _body->SetActive(value);
        } else {
            _bodyinfo.active = value;
        }
    }
    
    /**
     * Returns true if the body is awake
     *
     * An sleeping body is one that has come to rest and the physics engine has decided
     * to stop simulating it to save CPU cycles. If a body is awake and collides with a
     * sleeping body, then the sleeping body wakes up. Bodies will also wake up if a
     * joint or contact attached to them is destroyed.  You can also wake a body manually.
     *
     * This method only tests the status of the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return true if the body is awake
     */
    virtual bool isAwake() const override {
        return (_body != nullptr ? _body->IsAwake() : _bodyinfo.awake);
    }
    
    /**
     * Sets whether the body is awake
     *
     * An sleeping body is one that has come to rest and the physics engine has decided
     * to stop simulating it to save CPU cycles. If a body is awake and collides with a
     * sleeping body, then the sleeping body wakes up. Bodies will also wake up if a
     * joint or contact attached to them is destroyed.  You can also wake a body manually.
     *
     * This method affects the root body of this composite structure only.  If you want
     * to set the value for any of the child obstacles, iterate over the children.
     *
     * @param value  whether the body is awake
     */
    virtual void setAwake(bool value) override {
        if (_body != nullptr) {
            _body->SetAwake(value);
        } else {
            _bodyinfo.awake = value;
        }
    }
    
    /**
     * Returns false if this body should never fall asleep
     *
     * An sleeping body is one that has come to rest and the physics engine has decided
     * to stop simulating it to save CPU cycles. If a body is awake and collides with a
     * sleeping body, then the sleeping body wakes up. Bodies will also wake up if a
     * joint or contact attached to them is destroyed.  You can also wake a body manually.
     *
     * This method only tests the status of the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return false if this body should never fall asleep
     */
    virtual bool isSleepingAllowed() const override {
        return (_body != nullptr ? _body->IsSleepingAllowed() : _bodyinfo.allowSleep);
    }
    
    /**
     * Sets whether the body should ever fall asleep
     *
     * An sleeping body is one that has come to rest and the physics engine has decided
     * to stop simulating it to save CPU cycles. If a body is awake and collides with a
     * sleeping body, then the sleeping body wakes up. Bodies will also wake up if a
     * joint or contact attached to them is destroyed.  You can also wake a body manually.
     *
     * This method affects the root body of this composite structure only.  If you want 
     * to set the value for any of the child obstacles, iterate over the children. 
     *
     * @param value  whether the body should ever fall asleep
     */
    virtual void setSleepingAllowed(bool value) override {
        if (_body != nullptr) {
            _body->SetSleepingAllowed(value);
        } else {
            _bodyinfo.allowSleep = value;
        }
    }

    
    /**
     * Returns true if this body is a bullet
     *
     * By default, Box2D uses continuous collision detection (CCD) to prevent dynamic
     * bodies from tunneling through static bodies. Normally CCD is not used between
     * dynamic bodies. This is done to keep performance reasonable. In some game
     * scenarios you need dynamic bodies to use CCD. For example, you may want to shoot
     * a high speed bullet at a stack of dynamic bricks. Without CCD, the bullet might
     * tunnel through the bricks.
     *
     * Fast moving objects in Box2D can be labeled as bullets. Bullets will perform CCD
     * with both static and dynamic bodies. You should decide what bodies should be
     * bullets based on your game design.
     *
     * This method only tests the status of the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return true if this body is a bullet
     */
    virtual bool isBullet() const override {
        return (_body != nullptr ? _body->IsBullet() : _bodyinfo.bullet);
    }
    
    /**
     * Sets whether this body is a bullet
     *
     * By default, Box2D uses continuous collision detection (CCD) to prevent dynamic
     * bodies from tunneling through static bodies. Normally CCD is not used between
     * dynamic bodies. This is done to keep performance reasonable. In some game
     * scenarios you need dynamic bodies to use CCD. For example, you may want to shoot
     * a high speed bullet at a stack of dynamic bricks. Without CCD, the bullet might
     * tunnel through the bricks.
     *
     * Fast moving objects in Box2D can be labeled as bullets. Bullets will perform CCD
     * with both static and dynamic bodies. You should decide what bodies should be
     * bullets based on your game design.
     *
     * This method affects the root body of this composite structure only.  If you want
     * to set the value for any of the child obstacles, iterate over the children.
     *
     * @param value  whether this body is a bullet
     */
    virtual void setBullet(bool value) override {
        if (_body != nullptr) {
            _body->SetBullet(value);
        } else {
            _bodyinfo.bullet = value;
        }
    }
    
    /**
     * Returns true if this body be prevented from rotating
     *
     * This is very useful for characters that should remain upright. This method only
     * tests the status of the root object of this composite structure.  For fine-grain
     * control, you will need to loop over all elements in the composite structure.
     *
     * @return true if this body be prevented from rotating
     */
    virtual bool isFixedRotation() const override {
        return (_body != nullptr ? _body->IsFixedRotation() : _bodyinfo.fixedRotation);
    }
    
    /**
     * Sets whether this body be prevented from rotating
     *
     * This is very useful for characters that should remain upright. 
     *
     * This method affects the root body of this composite structure only.  If you want
     * to set the value for any of the child obstacles, iterate over the children.
     *
     * @param value  whether this body be prevented from rotating
     */
    virtual void setFixedRotation(bool value) override {
        if (_body != nullptr) {
            _body->SetFixedRotation(value);
        } else {
            _bodyinfo.fixedRotation = value;
        }
    }
    
    /**
     * Returns the gravity scale to apply to this body
     *
     * This allows isolated objects to float.  Be careful with this, since increased
     * gravity can decrease stability.
     *
     * This method only grabs the parameter for the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return the gravity scale to apply to this body
     */
    virtual float getGravityScale() const override {
        return (_body != nullptr ? _body->GetGravityScale() : _bodyinfo.gravityScale);
    }
    
    /**
     * Sets the gravity scale to apply to this body
     *
     * This allows isolated objects to float.  Be careful with this, since increased
     * gravity can decrease stability.
     *
     * This method affects the root body of this composite structure only.  If you want
     * to set the value for any of the child obstacles, iterate over the children.
     *
     * @param value  the gravity scale to apply to this body
     */
    virtual void setGravityScale(float value) override {
        if (_body != nullptr) {
            _body->SetGravityScale(value);
        } else {
            _bodyinfo.gravityScale = value;
        }
    }
    
    /**
     * Returns the linear damping for this body.
     *
     * Linear damping is use to reduce the linear velocity. Damping is different than
     * friction because friction only occurs with contact. Damping is not a replacement
     * for friction and the two effects should be used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no damping,
     * and infinity meaning full damping. Normally you will use a damping value between
     * 0 and 0.1. Most people avoid linear damping because it makes bodies look floaty.
     *
     * This method only grabs the parameter for the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return the linear damping for this body.
     */
    virtual float getLinearDamping() const override {
        return (_body != nullptr ? _body->GetLinearDamping() : _bodyinfo.linearDamping);
    }
    
    /**
     * Sets the linear damping for this body.
     *
     * Linear damping is use to reduce the linear velocity. Damping is different than
     * friction because friction only occurs with contact. Damping is not a replacement
     * for friction and the two effects should be used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no damping,
     * and infinity meaning full damping. Normally you will use a damping value between
     * 0 and 0.1. Most people avoid linear damping because it makes bodies look floaty.
     *
     * This method affects the root body of this composite structure only.  If you want
     * to set the value for any of the child obstacles, iterate over the children.
     *
     * @param value  the linear damping for this body.
     */
    virtual void setLinearDamping(float value) override {
        if (_body != nullptr) {
            _body->SetLinearDamping(value);
        } else {
            _bodyinfo.linearDamping = value;
        }
    }
    
    /**
     * Returns the angular damping for this body.
     *
     * Angular damping is use to reduce the angular velocity. Damping is different than
     * friction because friction only occurs with contact. Damping is not a replacement
     * for friction and the two effects should be used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no damping,
     * and infinity meaning full damping. Normally you will use a damping value between
     * 0 and 0.1.
     *
     * This method only grabs the parameter for the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return the angular damping for this body.
     */
    virtual float getAngularDamping() const override {
        return (_body != nullptr ? _body->GetAngularDamping() : _bodyinfo.angularDamping);
    }
    
    /**
     * Sets the angular damping for this body.
     *
     * Angular damping is use to reduce the angular velocity. Damping is different than
     * friction because friction only occurs with contact. Damping is not a replacement
     * for friction and the two effects should be used together.
     *
     * Damping parameters should be between 0 and infinity, with 0 meaning no damping,
     * and infinity meaning full damping. Normally you will use a damping value between
     * 0 and 0.1.
     *
     * This method affects the root body of this composite structure only.  If you want
     * to set the value for any of the child obstacles, iterate over the children.
     *
     * @param value  the angular damping for this body.
     */
    virtual void setAngularDamping(float value) override {
        if (_body != nullptr) {
            _body->SetAngularDamping(value);
        } else {
            _bodyinfo.angularDamping = value;
        }
    }
    
    
#pragma mark -
#pragma mark FixtureDef Methods
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
    virtual void setDensity(float value) override;
    
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
    virtual void setFriction(float value) override;
    
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
    virtual void setRestitution(float value) override;
    
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
    virtual void setSensor(bool value) override;
    
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
    virtual void setFilterData(b2Filter value) override;
    
    
#pragma mark -
#pragma mark MassData Methods    
    /**
     * Returns the center of mass of this body
     *
     * This method does NOT return a reference to the centroid position. Changes to this
     * vector will not affect the body.  However, it returns the same vector each time
     * its is called, and so cannot be used as an allocator.
     *
     * This method returns the centroid for the root object of this composite structure.
     * While it would make more sense to use the true centroid, that is much more
     * computationally expensive, as that centroid is not rigid.
     *
     * @return the center of mass for this physics body
     */
    virtual Vec2 getCentroid() const override {
        if (_body != nullptr) {
            return Vec2(_body->GetLocalCenter().x,_body->GetLocalCenter().y);
        } else {
            return Vec2(_massdata.center.x,_massdata.center.y);
        }
    }
    
    /**
     * Sets the center of mass for this physics body
     *
     * This method converts from a Cocos2D vector type to a Box2D vector type. This
     * cuts down on the confusion between vector types.
     *
     * @param value  the center of mass for this physics body
     */
    virtual void setCentroid(const Vec2& value) override { setCentroid(value.x,value.y); }
    
    /**
     * Sets the center of mass for this physics body
     *
     * This method does not keep a reference to the parameter. This method sets the
     * centroid for the root object of this composite structure. While it would make
     * more sense to use the true centroid, that is much more computationally expensive,
     * as that centroid is not rigid.
     *
     * @param x the x-coordinate of the center of mass for this physics body
     * @param y the y-coordinate of the center of mass for this physics body
     */
    virtual void setCentroid(float x, float y) override {
        Obstacle::setCentroid(x, y);
        if (_body != nullptr) {
            _body->SetMassData(&_massdata); // Protected accessor?
        }
    }
    
    /**
     * Returns the rotational inertia of this body
     *
     * For static bodies, the mass and rotational inertia are set to zero. When
     * a body has fixed rotation, its rotational inertia is zero.
     *
     * This method only grabs the parameter for the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @return the rotational inertia of this body
     */
    virtual float getInertia() const override {
        return  (_body != nullptr ? _body->GetInertia() : _massdata.I);
    }
    
    /**
     * Sets the rotational inertia of this body
     *
     * For static bodies, the mass and rotational inertia are set to zero. When
     * a body has fixed rotation, its rotational inertia is zero.
     *
     * This method only modifies the parameter for the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements
     * in the composite structure.
     *
     * @param value  the rotational inertia of this body
     */
    virtual void setInertia(float value) override {
        Obstacle::setInertia(value);
        if (_body != nullptr) {
            _body->SetMassData(&_massdata); // Protected accessor?
        }
    }
    
    /**
     * Returns the mass of this body
     *
     * The value is usually in kilograms. This method only grabs the parameter for the
     * root object of this composite structure.  For fine-grain control, you will need
     * to loop over all elements in the composite structure.
     *
     * @return the mass of this body
     */
    virtual float getMass() const override {
        return  (_body != nullptr ? _body->GetMass() : _massdata.mass);
    }
    
    /**
     * Sets the mass of this body
     *
     * The value is usually in kilograms. This method only modifies the parameter for
     * the root object of this composite structure.  For fine-grain control, you will
     * need to loop over all elements in the composite structure.
     *
     * @param value  the mass of this body
     */
    virtual void setMass(float value) override {
        Obstacle::setMass(value);
        if (_body != nullptr) {
            _body->SetMassData(&_massdata); // Protected accessor?
        }
    }
    
    /**
     * Resets this body to use the mass computed from the its shape and density.
     *
     * This method only modifies the parameter for the root object of this composite
     * structure.  For fine-grain control, you will need to loop over all elements in
     * the composite structure.
     */
    virtual void resetMass() override {
        Obstacle::resetMass();
        if (_body != nullptr) {
            _body->ResetMassData();
        }
    }
    
#pragma mark -
#pragma mark Physics Methods
    /**
     * Returns the Box2D body for this object.
     *
     * This method only returrns the root body in this composite structure.  For more
     * fine-grain control, you should use the iterator methods.
     *
     * @return the Box2D body for this object.
     */
    virtual b2Body* getBody() override { return _body; }
    
    /**
     * Returns the collection of component physics objects.
     *
     * While the vector does not allow you to modify the list, it is possible to
     * modify the individual objects.
     *
     * @return the collection of component physics objects.
     */
    const std::vector<std::shared_ptr<Obstacle>>& getBodies() { return _bodies; }
    
    /**
     * Returns the collection of joints for this object (may be empty).
     *
     * While the iterable does not allow you to modify the list, it is possible to
     * modify the individual joints.
     *
     * @return the collection of joints for this object.
     */
    const std::vector<b2Joint*>& getJoints() { return _joints; }
    
    /**
     * Creates the physics Body(s) for this object, adding them to the world.
     *
     * This method invokes ActivatePhysics for the individual PhysicsObjects
     * in the list. It also calls the internal method createJoints() to 
     * link them all together. You should override that method, not this one, 
     * for specific physics objects.
     *
     * @param world Box2D world to store body
     *
     * @return true if object allocation succeeded
     */
    virtual bool activatePhysics(b2World& world) override;
    
    /**
     * Destroys the physics Body(s) of this object if applicable,
     * removing them from the world.
     * 
     * @param world Box2D world that stores body
     */
    virtual void deactivatePhysics(b2World& world) override;
    
    /**
     * Create new fixtures for this body, defining the shape
     *
     * This method is typically undefined for complex objects.  While they
     * need a root body, they rarely need a root shape.  However, we provide
     * this method for maximum flexibility.
     */
    virtual void createFixtures() {}
    
    /**
     * Release the fixtures for this body, reseting the shape
     *
     * This method is typically undefined for complex objects.  While they
     * need a root body, they rarely need a root shape.  However, we provide
     * this method for maximum flexibility.
     */
    virtual void releaseFixtures() {}
    
    /**
     * Creates the joints for this object.
     * 
     * This method is executed as part of activePhysics. This is the primary method to 
     * override for custom physics objects.
     *
     * @param world Box2D world to store joints
     *
     * @return true if object allocation succeeded
     */
    virtual bool createJoints(b2World& world) {  return false;  }
    
    /**
     * Updates the object's physics state (NOT GAME LOGIC).
     *
     * This method is called AFTER the collision resolution state. Therefore, it 
     * should not be used to process actions or any other gameplay information.  Its 
     * primary purpose is to adjust changes to the fixture, which have to take place 
     * after collision.
     *
     * @param delta Timing values from parent loop
     */
    virtual void update(float delta) override;

    
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
    virtual void setDebugColor(Color4 color) override;
    
    /**
     * Sets the color of the debug wireframe.
     *
     * The default color is white, which means that the objects will be shown
     * with a white wireframe.
     *
     * @param color     the color of the debug wireframe.
     * @param cascade   whether to cascade the color to the component objects
     */
    void setDebugColor(Color4 color, bool cascade);
    
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
    virtual void setDebugScene(const std::shared_ptr<Node>& node) override;
    
};

}
#endif /* __CU_COMPLEX_OBSTACLE_H__ */
