//
//  CUScaleAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the scaling actions.  Scaling can specified
//  as either the final magnification or multiplicative factor.
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
#ifndef __CU_SCALE_ACTION_H__
#define __CU_SCALE_ACTION_H__

#include "CUAction.h"

namespace cugl {
    
#pragma mark -
#pragma mark ScaleBy

/**
 * This action represents a scale by a given factor.
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
class ScaleBy : public Action {
protected:
    /** The scaling factor */
    Vec2 _delta;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized scaling action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ScaleBy() { _delta = Vec2::ONE; }

    /**
     * Deletes this action instance, disposing all resources
     */
    ~ScaleBy() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() { _delta = Vec2::ONE; }
    
    /**
     * Initializes a degenerate scaling action.
     *
     * The scale amount is set to (1.0, 1.0), meaning no adjustment takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2(1.0, 1.0), 0.0f);
    }
    
    /**
     * Initializes an instaneous scaling animation by the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor. The animation will be instantaneous.
     *
     * @param factor    The amount to scale the target node
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& factor) {
        return init(factor,0.0f);
    }
    
    /**
     * Initializes a scaling animation by the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor. The animation will take place over 
     * the given number of seconds.
     *
     * @param factor    The amount to scale the target node
     * @param time      The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& factor, float time);

#pragma mark Static Constructors
    /**
     * Returns a newly allocated. degenerate scaling action.
     *
     * The scale amount is set to (1.0, 1.0), meaning no adjustment takes place.
     *
     * @return a newly allocated. degenerate scaling action.
     */
    static std::shared_ptr<ScaleBy> alloc() {
        std::shared_ptr<ScaleBy> result = std::make_shared<ScaleBy>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated, instaneous scaling animation by the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor. The animation will be instantaneous.
     *
     * @param factor    The amount to scale the target node
     *
     * @return a newly allocated, instaneous scaling animation by the given factor
     */
    static std::shared_ptr<ScaleBy> alloc(const Vec2& factor) {
        std::shared_ptr<ScaleBy> result = std::make_shared<ScaleBy>();
        return (result->init(factor) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated scaling animation by the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor. The animation will take place over
     * the given number of seconds.
     *
     * @param factor    The amount to scale the target node
     * @param time      The animation duration
     *
     * @return a newly allocated scaling animation by the given factor
     */
    static std::shared_ptr<ScaleBy> alloc(const Vec2& factor, float time) {
        std::shared_ptr<ScaleBy> result = std::make_shared<ScaleBy>();
        return (result->init(factor,time) ? result : nullptr);
    }

#pragma mark Attributes
    /**
     * Returns the scaling factor for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the scaling factor for this action.
     */
    const Vec2& getFactor() const { return _delta; }
    
    /**
     * Sets the scaling factor for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param factor the scaling factor for this action.
     */
    void setFactor(const Vec2& factor) { _delta = factor; }

    
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

    
#pragma mark -
/**
 * This action represents a scale towards a fixed magnification
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
class ScaleTo : public Action {
protected:
    /** The target scaling factor at the end of the animation */
    Vec2 _scale;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized scaling action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ScaleTo() { _scale = Vec2::ONE; }

    /**
     * Deletes this action instance, disposing all resources
     */
    ~ScaleTo() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() { _scale = Vec2::ONE; }
    
    /**
     * Initializes a scaling action returning the node to normal size
     *
     * The animation will be instantaneous.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2(1.0, 1.0), 0.0f);
    }
    
    /**
     * Initializes a scaling action towards the given scale amount
     *
     * The animation will be instantaneous.
     *
     * @param scale The target scaling amount     
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& scale) {
        return init(scale,0.0f);
    }

    /**
     * Initializes a scaling action towards the given scale amount
     *
     * The animation will take place over the given number of seconds.
     *
     * @param scale The target scaling amount
     * @param time  The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2& scale, float time);

#pragma mark Static Constructors
    /**
     * Returns a newly allocated scaling action returning the node to normal size
     *
     * The animation will be instantaneous.
     *
     * @return a newly allocated scaling action returning the node to normal size
     */
    static std::shared_ptr<ScaleTo> alloc() {
        std::shared_ptr<ScaleTo> result = std::make_shared<ScaleTo>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated scaling action towards the given scale amount
     *
     * The animation will be instantaneous.
     *
     * @param scale The target scaling amount
     *
     * @return a newly allocated scaling action towards the given scale amount
     */
    static std::shared_ptr<ScaleTo> alloc(const Vec2& scale) {
        std::shared_ptr<ScaleTo> result = std::make_shared<ScaleTo>();
        return (result->init(scale) ? result : nullptr);
    }

    /**
     * Returns a newly allocated scaling action towards the given scale amount
     *
     * The animation will take place over the given number of seconds.
     *
     * @param scale The target scaling amount
     * @param time  The animation duration
     *
     * @return a newly allocated scaling action towards the given scale amount
     */
    static std::shared_ptr<ScaleTo> alloc(const Vec2& scale, float time) {
        std::shared_ptr<ScaleTo> result = std::make_shared<ScaleTo>();
        return (result->init(scale, time) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the movement delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the movement delta for this action.
     */
    const Vec2& getScale() const { return _scale; }
    
    /**
     * Sets the movement delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param scale	The scale amount for this action.
     */
    void setScale(const Vec2& scale) { _scale = scale; }
    
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
#endif /* __CU_SCALE_ACTION_H__ */
