//
//  CUBoxObstacle.h
//  Cornell University Game Library (CUGL)
//
//  This class implements a rectangular physics object, and is the primary type
//  of physics object to use.  Hence the name, Box2D.
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
#ifndef __CU_BOX_OBSTACLE_H__
#define __CU_BOX_OBSTACLE_H__

#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include "CUSimpleObstacle.h"

namespace cugl {

#pragma mark -
#pragma mark Box Obstacle
/**
 * Box-shaped model to support collisions.
 *
 * Given the name Box2D, this is your primary model class.  Most of the time,
 * unless it is a player controlled avatar, you do not even need to subclass
 * BoxObject.  Look through some of our samples and see how many times we use 
 * this class.
 *
 * Unless otherwise specified, the center of mass is as the center.
 */
class BoxObstacle : public SimpleObstacle {
protected:
    /** Shape information for this box */
    b2PolygonShape _shape;
    /** A cache value for the fixture (for resizing) */
    b2Fixture* _geometry;
    /** The width and height of the box */
    Size _dimension;
    
    
#pragma mark -
#pragma mark Scene Graph Methods
    /**
     * Resets the polygon vertices in the shape to match the dimension.
     *
     * This is an internal method and it does not mark the physics object as 
     * dirty.
     *
     * @param  size The new dimension (width and height)
     */
    void resize(const Size& size);
    
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
     * Creates a new box object at the origin.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    BoxObstacle(void) : SimpleObstacle(), _geometry(nullptr) { }
    
    /**
     * Deletes this physics object and all of its resources.
     *
     * We have to make the destructor public so that we can polymorphically
     * delete physics objects.
     *
     * A non-default destructor is necessary since we must release all
     * claims on scene graph nodes.
     */
    virtual ~BoxObstacle() {
        CUAssertLog(_geometry == nullptr, "You must deactive physics before deleting an object");
    }

    /**
     * Initializes a new box object at the origin with no size.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() override { return init(Vec2::ZERO,Size::ZERO); }
    
    /**
     * Initializes a new box object at the given point with no size.
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
    virtual bool init(const Vec2& pos) override { return init(pos,Size::ZERO); }
    
    /**
     * Initializes a new box object of the given dimensions.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos  Initial position in world coordinates
     * @param  size The box size (width and height)
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2& pos, const Size& size);

    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated box object at the origin with no size.
     *
     * @return a newly allocated box object at the given point with no size.
     */
    static std::shared_ptr<BoxObstacle> alloc() {
        std::shared_ptr<BoxObstacle> result = std::make_shared<BoxObstacle>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated box object at the given point with no size.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos  Initial position in world coordinates
     *
     * @return a newly allocated box object at the given point with no size.
     */
    static std::shared_ptr<BoxObstacle> alloc(const Vec2& pos) {
        std::shared_ptr<BoxObstacle> result = std::make_shared<BoxObstacle>();
        return (result->init(pos) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated box object of the given dimensions.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos  Initial position in world coordinates
     * @param  size The box size (width and height)
     *
     * @return a newly allocated box object of the given dimensions.
     */
    static std::shared_ptr<BoxObstacle> alloc(const Vec2& pos, const Size& size) {
        std::shared_ptr<BoxObstacle> result = std::make_shared<BoxObstacle>();
        return (result->init(pos,size) ? result : nullptr);
    }
    
    
#pragma mark -
#pragma mark Dimensions
    /**
     * Returns the dimensions of this box
     *
     * @return the dimensions of this box
     */
    const Size& getDimension() const { return _dimension; }
    
    /**
     * Sets the dimensions of this box
     *
     * @param value  the dimensions of this box
     */
    void setDimension(const Size& value)         { resize(value); markDirty(true); }
    
    /**
     * Sets the dimensions of this box
     *
     * @param width   The width of this box
     * @param height  The height of this box
     */
    void setDimension(float width, float height) { setDimension(Size(width,height)); }

    /**
     * Returns the box width
     *
     * @return the box width
     */
    float getWidth() const { return _dimension.width; }
    
    /**
     * Sets the box width
     *
     * @param value  the box width
     */
    void setWidth(float value) { setDimension(value,_dimension.height); }
    
    /**
     * Returns the box height
     *
     * @return the box height
     */
    float getHeight() const { return _dimension.height; }
    
    /**
     * Sets the box height
     *
     * @param value  the box height
     */
    void setHeight(float value) { setDimension(_dimension.width,value); }
    
    
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
#endif /* __CU_BOX_OBSTACLE_H__ */
