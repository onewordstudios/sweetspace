//
//  CUWheelObstacle.h
//  Cornell Extensions to Cocos2D
//
//  This class implements a circular Physics object. We do not use it in any of
//  our samples,  but it is included for your education.  Note that the shape
//  must be circular, not elliptical.  If you want to make an ellipse, you will
//  need to use the PolygonObstacle class.
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
#ifndef __CU_WHEEL_OBSTACLE_H__
#define __CU_WHEEL_OBSTACLE_H__

#include <Box2D/Collision/Shapes/b2CircleShape.h>
#include "CUSimpleObstacle.h"


namespace cugl {

#pragma mark -
#pragma mark Wheel Obstacle

/**
 * Circle-shaped model to support collisions.
 *
 * Note that the shape must be circular, not elliptical. If you want to make an 
 * ellipse, you will need to use the PolygonObstacle class.
 *
 * Unless otherwise specified, the center of mass is as the center.
 */
class WheelObstacle : public SimpleObstacle {
protected:
    /** Shape information for this box */
    b2CircleShape _shape;
    /** A cache value for the fixture (for resizing) */
    b2Fixture* _geometry;

    
#pragma mark -
#pragma mark Scene Graph Methods
    
    /**
     * Creates the outline of the physics fixtures in the debug node
     *
     * The debug node is use to outline the fixtures attached to this object.
     * This is very useful when the fixtures have a very different shape than
     * the texture (e.g. a circular shape attached to a square texture).
     */
    virtual void resetDebug() override;

#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new wheel object at the origin.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    WheelObstacle(void) : SimpleObstacle(), _geometry(nullptr) { }
    
    /**
     * Deletes this physics object and all of its resources.
     *
     * We have to make the destructor public so that we can polymorphically
     * delete physics objects.
     *
     * A non-default destructor is necessary since we must release all
     * claims on scene graph nodes.
     */
    virtual ~WheelObstacle() {
        CUAssertLog(_geometry == nullptr, "You must deactive physics before deleting an object");
    }

    /**
     * Initializes a new wheel object at the origin with no size.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() override { return init(Vec2::ZERO,0.0); }
    
    /**
     * Initializes a new wheel object at the given point with no size.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos  Initial position in world coordinates
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2& pos) override { return init(pos,0.0); }
    
    /**
     * Initializes a new wheel object of the given dimensions.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos      Initial position in world coordinates
     * @param  radius   The wheel radius
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2& pos, float radius);
    
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a new wheel object at the origin with no radius.
     *
     * @return a new wheel object at the origin with no radius.
     */
    static std::shared_ptr<WheelObstacle> alloc() {
        std::shared_ptr<WheelObstacle> result = std::make_shared<WheelObstacle>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a new wheel object at the given point with no radius.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos  Initial position in world coordinates
     *
     * @return a new wheel object at the given point with no radius.
     */
    static std::shared_ptr<WheelObstacle> alloc(const Vec2& pos) {
        std::shared_ptr<WheelObstacle> result = std::make_shared<WheelObstacle>();
        return (result->init(pos) ? result : nullptr);
    }
    
    /**
     * Returns a new wheel object of the given radius.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos      Initial position in world coordinates
     * @param  radius   The wheel radius
     *
     * @return a new wheel object of the given radius.
     */
    static std::shared_ptr<WheelObstacle> alloc(const Vec2& pos, float radius) {
        std::shared_ptr<WheelObstacle> result = std::make_shared<WheelObstacle>();
        return (result->init(pos,radius) ? result : nullptr);
    }

    
#pragma mark -
#pragma mark Dimensions
    /**
     * Returns the radius of this circle
     *
     * @return the radius of this circle
     */
    float getRadius() const { return _shape.m_radius; }
    
    /**
     * Sets the radius of this circle
     *
     * @param value  the radius of this circle
     */
    void setRadius(float value) { _shape.m_radius = value; markDirty(true); }
    
    
#pragma mark -
#pragma mark Physics Methods
    /**
     * Create new fixtures for this body, defining the shape
     *
     * This is the primary method to override for custom physics objects
     */
    virtual void createFixtures() override;
    
    /**
     * Release the fixtures for this body, reseting the shape
     *
     * This is the primary method to override for custom physics objects
     */
    virtual void releaseFixtures() override;

};

}
#endif /* __CU_WHEEL_OBSTACLE_H__ */
