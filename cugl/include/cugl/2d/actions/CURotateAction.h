//
//  CURotateAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the rotation actions.  Rotation can specified
//  as either the end angle or the rotation amount.
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
#ifndef __CU_ROTATE_ACTION_H__
#define __CU_ROTATE_ACTION_H__

#include "CUAction.h"

namespace cugl {
    
#pragma mark -
#pragma mark RotateBy

/**
 * This action represents a rotation by a given angle amount.
 *
 * The angle is measured in radians , counter-clockwise from the x-axis.
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
class RotateBy : public Action {
protected:
    /** Difference between the final and initial angle in radians */
    float _delta;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized rotation action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    RotateBy() : _delta(0) {}

    /**
     * Deletes this action instance, disposing all resources
     */
    ~RotateBy() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() { _delta = 0; }
    
    /**
     * Initializes a degenerate rotation action.
     *
     * The rotation amount is set to 0.0, meaning no rotation takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(0.0f, 0.0f);
    }
    
    /**
     * Initializes a rotation action of the given angle.
     *
     * When animated, this action will rotate its target by the given delta.
     * The angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will be instantaneous.
     *
     * @param delta The amount to rotate the target node
     *
     * @return true if initialization was successful.
     */
    bool init(float delta) {
        return init(delta, 0.0f);
    }
    
    /**
     * Initializes a rotation action of the given angle.
     *
     * When animated, this action will rotate its target by the given delta.
     * The angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will take place over the given number of seconds.
     *
     * @param delta The amount to rotate the target node
     * @param time  The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(float delta, float time);
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated, degenerate rotation action.
     *
     * The rotation amount is set to 0.0, meaning no rotation takes place.
     *
     * @return a newly allocated, degenerate rotation action.
     */
    static std::shared_ptr<RotateBy> alloc() {
        std::shared_ptr<RotateBy> result = std::make_shared<RotateBy>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated rotation action of the given angle.
     *
     * When animated, this action will rotate its target by the given delta.
     * The angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will be instantaneous.
     *
     * @param delta The amount to rotate the target node
     *
     * @return a newly allocated rotation action of the given angle.
     */
    static std::shared_ptr<RotateBy> alloc(float delta) {
        std::shared_ptr<RotateBy> result = std::make_shared<RotateBy>();
        return (result->init(delta) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated rotation action of the given angle.
     *
     * When animated, this action will rotate its target by the given delta.
     * The angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will take place over the given number of seconds.
     *
     * @param delta The amount to rotate the target node
     * @param time  The animation duration
     *
     * @return a newly allocated rotation action of the given angle.
     */
    static std::shared_ptr<RotateBy> alloc(float delta, float time) {
        std::shared_ptr<RotateBy> result = std::make_shared<RotateBy>();
        return (result->init(delta, time) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the rotation delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the rotation delta for this action.
     */
    float getDelta() const { return _delta; }
    
    /**
     * Sets the rotation delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param delta the rotation delta for this action.
     */
    void setDelta(float delta) { _delta = delta; }
    
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
#pragma mark RotateTo
/**
 * This action represents a rotation to a specific angle.
 *
 * The angle is measured in radians , counter-clockwise from the x-axis.
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
class RotateTo : public Action {
protected:
    /** The target angle for this action */
    float _angle;
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized rotation action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    RotateTo() : _angle(0) {}
    
    /**
     * Deletes this action instance, disposing all resources
     */
    ~RotateTo() { dispose(); }

    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() { _angle = 0; }
    
    /**
     * Initializes a rotation action towards the x-axis
     *
     * The target angle is set to 0.0.  Because of how rotatations are 
     * interpolated, this guarantees that the rotation will be clockwise.
     * The animation will be instantaneous.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(0.0f, 0.0f);
    }
    
    /**
     * Initializes a rotation action towards the given angle
     *
     * This angle is measured in radians, counter-clockwise from the x-axis.  
     * The animation will be counter-clockwise if the target angle is larger
     * than the current one.  Otherwise it will be clockwise. The animation 
     * will be instantaneous.
     *
     * @param angle The target rotation angle
     *
     * @return true if initialization was successful.
     */
    bool init(float angle) {
        return init(angle, 0.0f);
    }
    
    /**
     * Initializes a rotation action towards the given angle
     *
     * This angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will be counter-clockwise if the target angle is larger
     * than the current one.  Otherwise it will be clockwise. The animation
     * will take place over the given number of seconds.
     *
     * @param angle The target rotation angle
     * @param time  The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(float angle, float time);
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated rotation action towards the x-axis
     *
     * The target angle is set to 0.0.  Because of how rotatations are
     * interpolated, this guarantees that the rotation will be clockwise.
     * The animation will be instantaneous.
     *
     * @return a newly allocated rotation action towards the x-axis
     */
    static std::shared_ptr<RotateTo> alloc() {
        std::shared_ptr<RotateTo> result = std::make_shared<RotateTo>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated rotation action towards the given angle
     *
     * This angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will be counter-clockwise if the target angle is larger
     * than the current one.  Otherwise it will be clockwise. The animation
     * will be instantaneous.
     *
     * @param angle The target rotation angle
     *
     * @return a newly allocated rotation action towards the given angle
     */
    static std::shared_ptr<RotateTo> alloc(float angle) {
        std::shared_ptr<RotateTo> result = std::make_shared<RotateTo>();
        return (result->init(angle) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated rotation action towards the given angle
     *
     * This angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will be counter-clockwise if the target angle is larger
     * than the current one.  Otherwise it will be clockwise. The animation
     * will take place over the given number of seconds.
     *
     * @param angle The target rotation angle
     * @param time  The animation duration
     *
     * @return a newly allocated rotation action towards the given angle
     */
    static std::shared_ptr<RotateTo> alloc(float angle, float time) {
        std::shared_ptr<RotateTo> result = std::make_shared<RotateTo>();
        return (result->init(angle,time) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the rotation target angle for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the rotation target angle for this action.
     */
    float getAngle() const { return _angle; }
    
    /**
     * Sets the rotation target angle for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param angle	The rotation target angle for this action.
     */
    void setAngle(float angle) { _angle = angle; }
    
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
#endif /* __CU_ROTATE_ACTION_H__ */
