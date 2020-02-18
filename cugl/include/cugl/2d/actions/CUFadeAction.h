//
//  CUFadeAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the fading actions.  This includes fading
//  in and out.  Because they are absolute notions, these actions have no
//  addition state to them (as is the case with other actions)
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
#ifndef __CU_FADE_ACTION_H__
#define __CU_FADE_ACTION_H__

#include "CUAction.h"

namespace cugl {
    
/**
 * This action represents a fade-out towards total transparency.
 *
 * When applied to a node, this action will adjust the alpha value of the node 
 * color until it is eventually 0.  Unless the node is set for its children
 * to inherit is color, this will have no affect on the children of the node.
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
class FadeOut : public Action {
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized fade-out action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    FadeOut() {}
    
    /**
     * Deletes this action instance, disposing all resources
     */
    ~FadeOut() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() {}
    
    /**
     * Initializes an instantaneous fade-out towards transparency
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 0.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(0.0f);
    }

    /**
     * Initializes an fade-out towards transparency
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 0.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     * The animation will take place over the given number of seconds.
     *
     * @param time  The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(float time);

#pragma mark Static Constructors
    /**
     * Returns a newly allocated, instantaneous fade-out towards transparency
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 0.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     *
     * @return a newly allocated, instantaneous fade-out towards transparency
     */
    static std::shared_ptr<FadeOut> alloc() {
        std::shared_ptr<FadeOut> result = std::make_shared<FadeOut>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated fade-out towards transparency
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 0.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     * The animation will take place over the given number of seconds.
     *
     * @param time  The animation duration
     *
     * @return a newly allocated fade-out towards transparency
     */
    static std::shared_ptr<FadeOut> alloc(float time) {
        std::shared_ptr<FadeOut> result = std::make_shared<FadeOut>();
        return (result->init(time) ? result : nullptr);
    }

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
 * This action represents a fade-in towards total opacity.
 *
 * When applied to a node, this action will adjust the alpha value of the node
 * color until it is eventually 1.  Unless the node is set for its children
 * to inherit is color, this will have no affect on the children of the node.
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
class FadeIn : public Action {
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized fade-in action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    FadeIn() {}
    
    /**
     * Deletes this action instance, disposing all resources
     */
    ~FadeIn() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() {}
    
    /**
     * Initializes an instantaneous fade-in towards opacity
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 1.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(0.0f);
    }
    
    /**
     * Initializes an fade-in towards opacity
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 1.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     * The animation will take place over the given number of seconds.
     *
     * @param time  The animation duration
     *
     * @return true if initialization was successful.
     */
    bool init(float time);

#pragma mark Static Constructors
    /**
     * Returns a newly allocated, instantaneous fade-in towards opacity
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 1.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     *
     * @return a newly allocated, instantaneous fade-in towards opacity
     */
    static std::shared_ptr<FadeIn> alloc() {
        std::shared_ptr<FadeIn> result = std::make_shared<FadeIn>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated fade-in towards opacity
     *
     * When applied to a node, this action will adjust the alpha value of the node
     * color until it is eventually 1.  Unless the node is set for its children
     * to inherit is color, this will have no affect on the children of the node.
     * The animation will take place over the given number of seconds.
     *
     * @param time  The animation duration
     *
     * @return a newly allocated fade-in towards opacity
     */
    static std::shared_ptr<FadeOut> alloc(float time) {
        std::shared_ptr<FadeOut> result = std::make_shared<FadeOut>();
        return (result->init(time) ? result : nullptr);
    }
    
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

#endif /* CUFadeAction_h */
