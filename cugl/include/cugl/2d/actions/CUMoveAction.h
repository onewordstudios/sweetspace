//
//  CUMoveAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the movement actions.  Movement can specified
//  as either the end target or the movement amount.
//
//  These classes use our standard shared-pointer architecture.
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
//  Author: Sophie Huang and Walker White
//  Version: 3/12/17
//
#ifndef __CU_MOVE_ACTION_H__
#define __CU_MOVE_ACTION_H__

#include "CUAction.h"

namespace cugl {

#pragma mark -
#pragma mark MoveBy

/**
 * This action represents a movement by a given vector amount.
 *
 * An action contains only the definition of the animation. This can include
 * information about the transform to use or the duration of the animation.
 * However, it does not contain any attribute of the target. Hence, an action
 * can be reapplied to different targets.
 *
 * By itself, an action does nothing.  It only specifies an action that may
 * take place. To use an action, it must be passed to the ActionManager.  The
 * manager will create an action instance and animate that instance.  While an
 * action may be reused many times, an action instance corresponds to a single
 * animation.
 */
class MoveBy : public Action {
protected:
    /** Difference between the destination and initial position */
    Vec2 _delta;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized movement action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    MoveBy() { _delta = Vec2::ZERO; }

    /**
     * Deletes this action instance, disposing all resources
     */
    ~MoveBy() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() { _delta = Vec2::ZERO; }
    
    /**
     * Initializes a degenerate movement action.
     *
     * The movement amount is set to (0.0, 0.0), meaning no movement takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2(0.0,0.0), 0.0f);
    }
    
    /**
     * Initializes an instaneous movement animation over the given vector.
     *
     * When animated, this action will move its target by the given delta. The
     * animation will be instantaneous.
     *
     * @param delta The amount to move the target node
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& delta) {
        return init(delta,0.0f);
    }
    
    /**
     * Initializes a movement animation over the given vector.
     *
     * When animated, this action will move its target by the given delta. The
     * animation will take place over the given number of seconds.
     *
     * @param delta The amount to move the target node
     * @param time  The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& delta, float time);
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated movement animation over the given vector.
     *
     * When animated, this action will move its target by the given delta. The
     * animation will be instantaneous.
     *
     * @return a newly allocated movement animation over the given vector.
     */
    static std::shared_ptr<MoveBy> alloc() {
        std::shared_ptr<MoveBy> result = std::make_shared<MoveBy>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated, instaneous movement animation over the given vector.
     *
     * When animated, this action will move its target by the given delta. The
     * animation will be instantaneous.
     *
     * @param delta The amount to move the target node
     *
     * @return a newly allocated, instaneous movement animation over the given vector.
     */
    static std::shared_ptr<MoveBy> alloc(const Vec2& delta) {
        std::shared_ptr<MoveBy> result = std::make_shared<MoveBy>();
        return (result->init(delta) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated, instaneous movement animation over the given vector.
     *
     * When animated, this action will move its target by the given delta. The
     * animation will take place over the given number of seconds.
     *
     * @param delta The amount to move the target node
     * @param time  The animation duration
     *
     * @return a newly allocated, instaneous movement animation over the given vector.
     */
    static std::shared_ptr<MoveBy> alloc(const Vec2& delta, float time) {
        std::shared_ptr<MoveBy> result = std::make_shared<MoveBy>();
        return (result->init(delta,time) ? result : nullptr);
    }

#pragma mark Atributes
    /**
     * Returns the movement delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the movement delta for this action.
     */
    const Vec2& getDelta() const { return _delta; }

    /**
     * Sets the movement delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param delta the movement delta for this action.
     */
    void setDelta(const Vec2& delta) { _delta = delta; }
    
#pragma mark Animation Methods
    /**
     * Returns a newly allocated copy of this Action.
     *
     * @return a newly allocated copy of this Action.
     */
    virtual std::shared_ptr<Action> clone() override;
    
    /**
     * Executes an action on the given target node.
     *
     * The important state of the target is stored in the given state parameter.
     * The semantics of this state is action-dependent.
     *
     * @param target    The node to act on
     * @param state     The relevant node state
     * @param dt        The elapsed time to animate.
     */
    virtual void update(const std::shared_ptr<Node>& target, Uint64* state, float dt) override;

#pragma mark Debugging Methods
    /**
     * Returns a string representation of the action for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this action for debuggging purposes.
     */
    virtual std::string toString(bool verbose = false) const override;
};


#pragma mark -
#pragma mark MoveTo

/**
 * This action represents a movement to a given position
 *
 * An action contains only the definition of the transformation; it does not
 * contain any attribute of the target. Hence, an action it can be reapplied
 * to different targets.
 *
 * By itself, an action does nothing.  It only specifies an action that may
 * take place. To use an action, it must be passed to the ActionManager.  The
 * manager will create an action instance and animate that instance.  While an
 * action may be reused many times, an action instance corresponds to a single
 * animation.
 */
class MoveTo : public Action {
protected:
    /** The target destination for this action */
    Vec2 _target;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized movement action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    MoveTo() { _target = Vec2::ZERO; }

    /**
    * Deletes this action instance, disposing all resources
    */
    ~MoveTo() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() { _target = Vec2::ZERO; }
    
    /**
     * Initializes an instantaneous movement towards the origin.
     *
     * The target position is set to (0.0, 0.0), meaning that this action will
     * move a node towards the origin. The animation will be instantaneous.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2(0.0, 0.0), 0.0f);
    }
    
    /**
     * Initializes an instantaneous movement towards towards the given position.
     *
     * The animation will be instantaneous.
     *
     * @param target    The target position
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& target) {
        return init(target,0.0f);
    }
    
    /**
     * Initializes a movement animation towards towards the given position.
     *
     * The animation will take place over the given number of seconds.
     *
     * @param target    The target position
     * @param time      The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& target, float time);

#pragma mark Static Constructors
    /**
     * Returns a newly allocated, instantaneous motion towards the origin.
     *
     * The target position is set to (0.0, 0.0), meaning that this action will
     * move a node towards the origin. The animation will be instantaneous.
     *
     * @return a newly allocated, instantaneous motion towards the origin.
     */
    static std::shared_ptr<MoveTo> alloc() {
        std::shared_ptr<MoveTo> result = std::make_shared<MoveTo>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated, instantaneous motion towards the given position.
     *
     * The animation will be instantaneous.
     *
     * @param target    The target position
     *
     * @return a newly allocated, instantaneous motion towards the given position.
     */
    static std::shared_ptr<MoveTo> alloc(const Vec2& target) {
        std::shared_ptr<MoveTo> result = std::make_shared<MoveTo>();
        return (result->init(target) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated motion towards the given position.
     *
     * The animation will take place over the given number of seconds.
     *
     * @param target    The target position
     * @param time      The animation duration
     *
     * @return a newly allocated motion towards the given position.
     */
    static std::shared_ptr<MoveTo> alloc(const Vec2& target, float time) {
        std::shared_ptr<MoveTo> result = std::make_shared<MoveTo>();
        return (result->init(target, time) ? result : nullptr);
    }

#pragma mark Attributes
    /**
     * Returns the movement target for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the movement target for this action.
     */
    const Vec2& getTarget() const { return _target; }
    
    /**
     * Sets the movement target for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param target    the movement target for this action.
     */
    void setTarget(const Vec2& target) { _target = target; }
    
#pragma mark Animation Methods
    /**
     * Returns a newly allocated copy of this Action.
     *
     * @return a newly allocated copy of this Action.
     */
    virtual std::shared_ptr<Action> clone() override;
    
    /**
     * Prepares a target for action
     *
     * The important state of the target is stored in the given state parameter.
     * The semantics of this state is action-dependent.
     *
     * @param target    The node to act on
     * @param state     The relevant node state
     */
    virtual void load(const std::shared_ptr<Node>& target, Uint64* state) override;
    
    /**
     * Executes an action on the given target node.
     *
     * The important state of the target is stored in the given state parameter.
     * The semantics of this state is action-dependent.
     *
     * @param target    The node to act on
     * @param state     The relevant node state
     * @param dt        The elapsed time to animate.
     */
    virtual void update(const std::shared_ptr<Node>& target, Uint64* state, float dt) override;
    
#pragma mark Debugging Methods
    /**
     * Returns a string representation of the action for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this action for debuggging purposes.
     */
    virtual std::string toString(bool verbose = false) const override;
};

}
#endif /* __CU_MOVE_ACTION_H__ */
