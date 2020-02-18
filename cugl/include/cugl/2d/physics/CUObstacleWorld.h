//
//  CUObstacleWorld.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a wrapper to Box2d that for use with CUGL obstacle
//  heirarchy.  Obstacles provide a simple and direct way to create physics
//  objects that does not require the multi-step approach of Box2D.  It also
//  supports shared pointers for simply memory management.
//
//  However, this class is not as flexible as Box2D.  Therefore, it may be
//  necessary to access Box2D directly at times.
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
//  Version: 11/1/16
#ifndef __CU_PHYSICS_WORLD_H__
#define __CU_PHYSICS_WORLD_H__

#include <vector>
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <cugl/math/cu_math.h>
class b2World;

namespace cugl {

// Forward declaration of the Obstacle class
class Obstacle;

/** Default amount of time for a physics engine step. */
#define DEFAULT_WORLD_STEP  1/60.0f
/** Default number of velocity iterations for the constrain solvers */
#define DEFAULT_WORLD_VELOC 6
/** Default number of position iterations for the constrain solvers */
#define DEFAULT_WORLD_POSIT 2


#pragma mark -
#pragma mark World Controller
/**
 * A CUGL wrapper for a Box2d world.
 *
 * This module provides a wrapper to Box2d that for use with CUGL obstacle
 * heirarchy.  Obstacles provide a simple and direct way to create physics
 * objects that does not require the multi-step approach of Box2D.  It also
 * supports shared pointers for simply memory management.
 *
 * In addition, this class provides a modern callback approach supporting 
 * closures assigned to attributes.  This allows you to modify the callback 
 * functions while the program is running.
 */
class ObstacleWorld : public b2ContactListener, b2DestructionListener, b2ContactFilter {
protected:
    /** Reference to the Box2D world */
    b2World* _world;
    /** Whether to lock the physic timestep to a constant amount */
    bool _lockstep;
    /** The amount of time for a single engine step */
    float _stepssize;
    /** The number of velocity iterations for the constrain solvers */
    int _itvelocity;
    /** The number of position iterations for the constrain solvers */
    int _itposition;
    /** The current gravitational value of the world */
    Vec2 _gravity;
    
    /** The list of objects in this world */
    std::vector<std::shared_ptr<Obstacle>> _objects;
    
    /** The boundary of the world */
    Rect _bounds;
    
    /** Whether or not to activate the collision listener */
    bool _collide;
    /** Whether or not to activate the filter listener */
    bool _filters;
    /** Whether or not to activate the destruction listener */
    bool _destroy;
    
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new degenerate ObstacleWorld on the stack.
     *
     * The scene has no backing Box2d world and must be initialized.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ObstacleWorld();
    
    /**
     * Deletes this world, disposing all resources
     */
    ~ObstacleWorld() { dispose(); }

    /**
     * Disposes all of the resources used by this world.
     *
     * A disposed ObstacleWorld can be safely reinitialized. Any obstacles owned
     * by this world will be deactivates.  They will be deleted if no other 
     * object owns them.
     */
    void dispose();

    /**
     * Initializes a new physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A few attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * This constructor will use the default gravitational value.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     *
     * @return  true if the controller is initialized properly, false otherwise.
     */
    bool init(const Rect& bounds);
    
    /**
     * Initializes a new physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A few attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     * @param  gravity  The gravitational force on this Box2d world
     *
     * @return  true if the controller is initialized properly, false otherwise.
     */
    bool init(const Rect& bounds, const Vec2& gravity);

    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A few attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * This constructor will use the default gravitational value.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     *
     * @return a newly allocated physics world
     */
    static std::shared_ptr<ObstacleWorld> alloc(const Rect& bounds) {
        std::shared_ptr<ObstacleWorld> result = std::make_shared<ObstacleWorld>();
        return (result->init(bounds) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A few attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     * @param  gravity  The gravitational force on this Box2d world
     *
     * @return a newly allocated physics world
     */
    static std::shared_ptr<ObstacleWorld> alloc(const Rect& bounds, const Vec2& gravity) {
        std::shared_ptr<ObstacleWorld> result = std::make_shared<ObstacleWorld>();
        return (result->init(bounds,gravity) ? result : nullptr);
    }

    
#pragma mark -
#pragma mark Physics Handling
    /**
     * Returns a (weak) reference to the Box2d world.
     *
     * This accessor is for any world methods that are not encapsulated by this
     * constroller.  We have largely limited the controller to functionality that
     * requires b2WorldCallbacks, as those classes are antiquated in the face of
     * modern closures.
     *
     * As a weak reference, this physics world does not transfer ownership of
     * this object.  In addition, the value may be a nullptr.
     *
     * @return a reference to the Box2d world.
     */
    b2World* getWorld() { return _world; }
    
    /**
     * Returns true if the physics is locked to a constant timestep.
     *
     * If this is false, the physics timestep will vary with the graphics framerate.
     *
     * @return true if the physics is locked to a constant timestep.
     */
    bool isLockStep() const { return _lockstep; }
    
    /**
     * Sets whether the physics is locked to a constant timestep.
     *
     * If this is false, the physics timestep will vary with the graphics framerate.
     * Any change will take effect at the time of the next call to update.
     *
     * @param  flag whether the physics is locked to a constant timestep.
     */
    void setLockStep(bool flag) { _lockstep = flag; }
    
    /** 
     * Returns the amount of time for a single engine step.
     *
     * This attribute is only relevant if isLockStep() is true.
     *
     * @return the amount of time for a single engine step.
     */
    float getStepsize() const { return _stepssize; }
    
    /**
     * Sets the amount of time for a single engine step.
     *
     * This attribute is only relevant if isLockStep() is true. Any change will take 
     * effect at the time of the next call to update.
     *
     * @param  step the amount of time for a single engine step.
     */
    void setStepsize(float step) { _stepssize = step; }

    /** 
     * Returns number of velocity iterations for the constrain solvers 
     *
     * @return number of velocity iterations for the constrain solvers
     */
    int getVelocityIterations() const { return _itvelocity; }

    /**
     * Sets number of velocity iterations for the constrain solvers
     *
     * Any change will take effect at the time of the next call to update.
     *
     * @param  velocity number of velocity iterations for the constrain solvers
     */
    void setVelocityIterations(int velocity) { _itvelocity = velocity; }

    /**
     * Returns number of position iterations for the constrain solvers
     *
     * @return number of position iterations for the constrain solvers
     */
    int getPositionIterations() const { return _itposition; }
    
    /**
     * Sets number of position iterations for the constrain solvers
     *
     * Any change will take effect at the time of the next call to update.
     *
     * @param  position number of position iterations for the constrain solvers
     */
    void setPositionIterations(int position) { _itposition = position; }
    
    /**
     * Returns the global gravity vector.
     *
     * @return the global gravity vector.
     */
    const Vec2& getGravity() const { return _gravity; }

    /**
     * Sets the global gravity vector.
     *
     * Any change will take effect at the time of the next call to update.
     *
     * @param  gravity  the global gravity vector.
     */
    void setGravity(const Vec2& gravity);
    
    /**
     * Executes a single step of the physics engine.
     *
     * This method contains the specific update code for this mini-game. It does
     * not handle collisions, as those are managed by the parent class WorldController.
     * This method is called after input is read, but before collisions are resolved.
     * The very last thing that it should do is apply forces to the appropriate objects.
     *
     * Once the update phase is over, but before we draw, we are ready to handle
     * physics.  The primary method is the step() method in world.  This implementation
     * works for all applications and should not need to be overwritten.
     *
     * @param dt Number of seconds since last animation frame
     */
    void update(float dt);
    
    /**
     * Returns the bounds for the world controller.
     *
     * @return the bounds for the world controller.
     */
    const Rect& getBounds() const { return _bounds; }
    
    /**
     * Returns true if the object is in bounds.
     *
     * This assertion is useful for debugging the physics.
     *
     * @param obj The object to check.
     *
     * @return true if the object is in bounds.
     */
    bool inBounds(Obstacle* obj);
    
    
#pragma mark -
#pragma mark Object Management
    /**
     * Returns a read-only reference to the list of active obstacles.
     *
     * @return a read-only reference to the list of active obstacles.
     */
    const std::vector<std::shared_ptr<Obstacle>>& getObstacles() { return _objects; }

    /**
     * Immediately adds the obstacle to the physics world
     *
     * Adding an obstacle activates the underlying physics.  It will now have
     * a body.  In the case of a {@link ComplexObstacle}, joints will be added
     * between the obstacles.  The physics world will include the obstacle in
     * its next call to update.
     *
     * The obstacle will be retained by this world, preventing it from being 
     * garbage collected.
     *
     * param obj The obstacle to add
     */
    void addObstacle(const std::shared_ptr<Obstacle>& obj);
    
    /**
     * Immediately removes an obstacle from the physics world
     *
     * The obstacle will be released immediately. The physics will be deactivated
     * and it will be removed from the Box2D world. This method of removing 
     * objects is very heavy weight, and should only be used for single object 
     * removal.  If you want to remove multiple objects, then you should mark 
     * them for removal and call garbageCollect.
     *
     * Removing an obstacle does not automatically delete the obstacle itself.
     * However, this world releases ownership, which may lead to it being
     * garbage collected.
     *
     * param obj The obstacle to remove
     */
    void removeObstacle(Obstacle* obj);
    
    /**
     * Remove all objects marked for removal.
     *
     * The obstacles will be released immediately. The physics will be deactivated
     * and they will be removed from the Box2D world.
     *
     * Removing an obstacle does not automatically delete the obstacle itself.
     * However, this world releases ownership, which may lead to it being
     * garbage collected.
     *
     * This method is the efficient, preferred way to remove objects.
     */
    void garbageCollect();

    /**
     * Remove all objects, emptying this physics world.
     *
     * This method is different from {@link dispose()} in that the world can
     * still receive new objects.
     */
    void clear();

    
#pragma mark -
#pragma mark Collision Callback Functions
    /**
     * Activates the collision callbacks.
     *
     * If flag is false, then the collision callbacks (even if defined) will be ignored.
     * Otherwise, the callbacks will be executed (on collision) if they are defined.
     *
     * @param  flag whether to activate the collision callbacks.
     */
    void activateCollisionCallbacks(bool flag);
    
    /**
     * Returns true if the collision callbacks are active
     *
     * If this value is false, then the collision callbacks (even if defined) will be ignored.
     * Otherwise, the callbacks will be executed (on collision) if they are defined.
     *
     * @return true if the collision callbacks are active
     */
    bool enabledCollisionCallbacks() const { return _collide; }
    
    /**
     * Called when two fixtures begin to touch
     *
     * This attribute is a dynamically assignable callback and may be changed at
     * any given time.
     * 
     * @param  contact  the contact information
     */
    std::function<void(b2Contact* contact)> onBeginContact;
    
    /**
     * Called when two fixtures cease to touch
     *
     * This attribute is a dynamically assignable callback and may be changed at
     * any given time.
     *
     * @param  contact  the contact information
     */
    std::function<void(b2Contact* contact)> onEndContact;
    
    /**
     * Called after a contact is updated. 
     *
     * This callback allows you to inspect a contact before it goes to the solver. 
     * If you are careful, you can modify the contact manifold (e.g. disable contact).
     *
     * A copy of the old manifold is provided so that you can detect changes.
     *
     * Note: this is called only for awake bodies.
     * Note: this is called even when the number of contact points is zero.
     * Note: this is not called for sensors.
     * Note: if you set the number of contact points to zero, you will not get an
     * EndContact callback. However, you may get a BeginContact callback the 
     * next step.
     *
     * This attribute is a dynamically assignable callback and may be changed at
     * any given time.
     *
     * @param  contact      the contact information
     * @param  oldManifold  the contact manifold last iteration
     */
    std::function<void(b2Contact* contact, const b2Manifold* oldManifold)> beforeSolve;
    
    /**
     * Called after the solver is finished.
     *
     * This callback lets you inspect a contact after the solver is finished. 
     * This is useful for inspecting impulses.
     *
     * Note: the contact manifold does not include time of impact impulses, which 
     * can be arbitrarily large if the sub-step is small. Hence the impulse is 
     * provided explicitly in a separate data structure.
     * Note: this is only called for contacts that are touching, solid, and awake.
     *
     * This attribute is a dynamically assignable callback and may be changed at
     * any given time.
     *
     * @param  contact  the contact information
     * @param  impulse  the impulse produced by the solver
     */
    std::function<void(b2Contact* contact, const b2ContactImpulse* impulse)> afterSolve;
    
    /**
     * Called when two fixtures begin to touch
     *
     * This method is the static callback required by the Box2d API.  It should
     * not be altered.
     *
     * @param  contact  the contact information
     */
    void BeginContact(b2Contact* contact) override {
        if (onBeginContact != nullptr) {
            onBeginContact(contact);
        }
    }
    
    /**
     * Called when two fixtures cease to touch
     *
     * This method is the static callback required by the Box2d API.  It should
     * not be altered.
     *
     * @param  contact  the contact information
     */
    void EndContact(b2Contact* contact) override {
        if (onEndContact != nullptr) {
            onEndContact(contact);
        }
    }
    
    /**
     * Called after a contact is updated.
     *
     * This callback allows you to inspect a contact before it goes to the solver. 
     * If you are careful, you can modify the contact manifold (e.g. disable contact).
     *
     * A copy of the old manifold is provided so that you can detect changes.
     *
     * Note: this is called only for awake bodies.
     * Note: this is called even when the number of contact points is zero.
     * Note: this is not called for sensors.
     * Note: if you set the number of contact points to zero, you will not get an
     * EndContact callback. However, you may get a BeginContact callback the 
     * next step.
     *
     * This method is the static callback required by the Box2d API.  It should
     * not be altered.
     *
     * @param  contact      the contact information
     * @param  oldManifold  the contact manifold last iteration
     */
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override {
        if (beforeSolve != nullptr) {
            beforeSolve(contact,oldManifold);
        }
    }
    
    /** 
     * Called after the solver is finished.
     * 
     * This callback lets you inspect a contact after the solver is finished. 
     * This is useful for inspecting impulses.
     *
     * Note: the contact manifold does not include time of impact impulses, 
     * which can be arbitrarily large if the sub-step is small. Hence the 
     * impulse is provided explicitly in a separate data structure.
     * Note: this is only called for contacts that are touching, solid, and awake.
     *
     * This method is the static callback required by the Box2d API.  It should
     * not be altered.
     *
     * @param  contact  the contact information
     * @param  impulse  the impulse produced by the solver
     */
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override {
        if (afterSolve != nullptr) {
            afterSolve(contact,impulse);
        }
    }

    
#pragma mark -
#pragma mark Filter Callback Functions
    /**
     * Activates the collision filter callbacks.
     *
     * If flag is false, then the collision filter callbacks (even if defined) will be 
     * ignored. Otherwise, the callbacks will be executed (to test a collision) if they
     * are defined.
     *
     * @param  flag whether to activate the collision callbacks.
     */
    void activateFilterCallbacks(bool flag);
    
    /**
     * Returns true if the collision filter callbacks are active
     *
     * If this value is false, then the collision filter callbacks (even if defined) will
     * be ignored. Otherwise, the callbacks will be executed (to test a collision) if they
     * are defined.
     *
     * @return true if the collision filter callbacks are active
     */
    bool enabledFilterCallbacks() const { return _filters; }
    
    /**
     * Return true if contact calculations should be performed between these two shapes.
     *
     * For performance reasons this is only called when the AABBs begin to overlap.
     *
     * @param  fixtureA the first colliding shape
     * @param  fixtureB the second colliding shape
     *
     * @return true if contact calculations should be performed between these two shapes.
     */
    std::function<bool(b2Fixture* fixtureA, b2Fixture* fixtureB)> shouldCollide;
    
    /**
     * Return true if contact calculations should be performed between these two shapes.
     *
     * For performance reasons this is only called when the AABBs begin to overlap.
     *
     * @param  fixtureA the first colliding shape
     * @param  fixtureB the second colliding shape
     *
     * @return true if contact calculations should be performed between these two shapes.
     */
    bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override {
        if (shouldCollide != nullptr) {
            return shouldCollide(fixtureA,fixtureB);
        }
        return false;
    }

    
#pragma mark -
#pragma mark Destruction Callback Functions
    /**
     * Activates the destruction callbacks.
     *
     * If flag is false, then the destruction callbacks (even if defined) will be ignored.
     * Otherwise, the callbacks will be executed (on body destruction) if they are defined.
     *
     * @param  flag whether to activate the collision callbacks.
     */
    void activateDestructionCallbacks(bool flag);
    
    /**
     * Returns true if the destruction callbacks are active
     *
     * If this value is false, then the destruction callbacks (even if defined) will be 
     * ignored. Otherwise, the callbacks will be executed (on body destruction) if they 
     * are defined.
     *
     * @return true if the destruction callbacks are active
     */
    bool enabledDestructionCallbacks() const { return _destroy; }
    
    /**
     * Called when a fixture is about to be destroyed.
     *
     * This function is only called when the destruction is the result of the
     * destruction of its parent body.
     *
     * @param  fixture  the fixture to be destroyed
     */
    std::function<void(b2Fixture* fixture)> destroyFixture;

    /**
     * Called when a joint is about to be destroyed.
     *
     * This function is only called when the destruction is the result of the
     * destruction of one of its attached bodies.
     *
     * @param  joint    the joint to be destroyed
     */
    std::function<void(b2Joint* joint)>     destroyJoint;

    /**
     * Called when a joint is about to be destroyed.
     *
     * This function is only called when the destruction is the result of the
     * destruction of one of its attached bodies.
     *
     * @param  joint    the joint to be destroyed
     */
    void SayGoodbye(b2Joint* joint) override {
        if (destroyJoint != nullptr) {
            destroyJoint(joint);
        }
    }
    
    /**
     * Called when a fixture is about to be destroyed.
     *
     * This function is only called when the destruction is the result of the
     * destruction of its parent body.
     *
     * @param  fixture  the fixture to be destroyed
     */
    void SayGoodbye(b2Fixture* fixture) override {
        if (destroyFixture != nullptr) {
            destroyFixture(fixture);
        }
    }


#pragma mark -
#pragma mark Query Functions
    /**
     * Query the world for all fixtures that potentially overlap the provided AABB.
     *
     * The AABB is specified by a Cocos2d rectangle.
     *
     * @param  callback A user implemented callback function.
     * @param  aabb     The axis-aligned bounding box
     */
    void queryAABB(std::function<bool(b2Fixture* fixture)> callback, const Rect& aabb) const;

    /**
     * Ray-cast the world for all fixtures in the path of the ray.
     *
     * The callback controls whether you get the closest point, any point, or n-points.
     * The ray-cast ignores shapes that contain the starting point.
     *
     * @param  callback a user implemented callback function.
     * @param  point1   The ray starting point
     * @param  point2   The ray ending point
     */
    void rayCast(std::function<float(b2Fixture* fixture, const Vec2& point,
                                     const Vec2& normal, float fraction)> callback,
                 const Vec2& point1, const Vec2& point2) const;
    
};

}
#endif /* __CU_PHYSICS_WORLD_H__ */
