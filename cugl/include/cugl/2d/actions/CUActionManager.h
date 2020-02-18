//
//  CUActionManager.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the manager to provide tweened animations.  Actions
//  themselves are not animations; they are only templates for an animation.
//  The action manager attaches an action to a scene graph node and performs
//  the (timed) animation.
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
#ifndef __CU_ACTION_MANAGER_H__
#define __CU_ACTION_MANAGER_H__

#include "CUAction.h"
#include <SDL/SDL.h>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace cugl {

/**
 * This class provides an action manager for instantiating animations.
 *
 * To create an animation, the manager attaches an action to a scene graph node
 * via a key.  This key allows the user to pause an animation or query when it
 * is complete. Each update frame, the manager moves the animation further along 
 * until it is complete.
 *
 * An action manager is not implemented as a singleton.  However, you typically
 * only need one manager per application.
 */
class ActionManager {
#pragma mark ActionInstance
    /**
     * This internal class represents and action being actively animated.
     *
     * The instance contains the state of an action assigned to a node including 
     * duration, elapsed time, the target, the interpolation function, and any 
     * internal state. This class is only meant to be used by ActionManager, not 
     * directly by the user.
     *
     * Because this is an internal class, it is used as a struct.  There are
     * no methods other than the constructor.
     */
    class ActionInstance {
    public:
        /** The node the action is performed on */
        std::shared_ptr<Node> target;
        
        /** The action template associated with this instance */
        std::shared_ptr<Action> action;
        
        /** The interpolation function on [0,1] to allow non-linear behavior */
        std::function<float(float)> interpolant;
        
        /** Any internal state needed by this action */
        Uint64 state;
        
        /* The desired completion time of the action */
        float duration;
        
        /** The execution time since initialization */
        float elapsed;
        
        /** Whether or not this instance is currently paused */
        bool  paused;
        
    public:
        /**
         * Creates a new degenerate ActionInstance on the stack.
         *
         * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
         * the heap, use one of the static constructors instead.
         */
        ActionInstance() : state(0), duration(0.0f), elapsed(0.0f), paused(false) {}
        
        /**
         * Deletes this action instance, disposing all resources
         */
        ~ActionInstance();
    };
    
#pragma mark Values
protected:
    /** A map that associates nodes with their (multiple) animations */
    std::unordered_map<Node*, std::unordered_set<std::string>> _keys;
    
    /** A map that associates keys with animations */
    std::unordered_map<std::string, ActionInstance*> _actions;
    

public:
#pragma mark Constructors
    /**
     * Creates a new degenerate ActionManager on the stack.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on 
     * the heap, use one of the static constructors instead.
     */
    ActionManager() {}
    
    /**
     * Deletes this action manager, disposing all resources
     */
    ~ActionManager() { dispose(); }
    
    /**
     * Disposes all of the resources used by this action manager.
     *
     * A disposed action manager can be safely reinitialized. Any animations
     * owned by this action manager will immediately stop and be released.
     */
    void dispose();
    
    /**
     * Initializes an action manager.
     *
     * @return true if initialization was successful.
     */
    bool init() { return true; }
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated action manager.
     *
     * @return a newly allocated action manager.
     */
    static std::shared_ptr<ActionManager> alloc() {
        std::shared_ptr<ActionManager> result = std::make_shared<ActionManager>();
        return (result->init() ? result : nullptr);
    }

#pragma mark -
#pragma mark Action Management
    /**
     * Returns true if the given key represents an active animation
     *
     * @param key       The identifying key
     *
     * @return true if the given key represents an active animation
     */
    bool isActive(std::string key) const;
    
    /**
     * Actives an animation with the given target and action
     *
     * This method will fail if the provided key is already in use.
     *
     * @param key       The identifying key
     * @param action    The action to animate with
     * @param target    The node to animate on
     *
     * @return true if the animation was successfully started
     */
    bool activate(std::string key,
                  const std::shared_ptr<Action>& action,
                  const std::shared_ptr<Node>& target) {
        return activate(key,action,target, nullptr);
    };


    /**
     * Actives an animation with the given target and action
     *
     * The easing function allows for effects like bouncing or elasticity in
     * the linear interpolation. If null, the animation will use the standard 
     * linear easing.
     *
     * This method will fail if the provided key is already in use.
     *
     * @param key       The identifying key
     * @param action    The action to animate with
     * @param target    The node to animate on
     * @param easing    The easing (interpolation) function
     *
     * @return true if the animation was successfully started
     */
    bool activate(std::string key,
                  const std::shared_ptr<Action>& action,
                  const std::shared_ptr<Node>& target,
                  std::function<float(float)> easing);
    
    /**
     * Removes the animation for the given key.
     *
     * This act will immediately stop the animation.  The animated node will
     * continue to have whatever state it had when the animation stopped.
     *
     * If there is no animation for the give key (e.g. the animation is complete)
     * this method will return false.
     *
     * @param key       The identifying key
     *
     * @return true if the animation was successfully removed
     */
    bool remove(std::string key);

    /**
     * Updates all non-paused animations by dt seconds
     *
     * Each animation is moved forward by dt second.  If this causes an animation
     * to reach its duration, the animation is removed and the key is once
     * again available.
     *
     * @param dt    The number of seconds to animate
     */
    void update(float dt);

#pragma mark -
#pragma mark Pausing
    /**
     * Returns true if the animation for the given key is paused
     *
     * This method will return false if there is no active animation with the
     * given key.
     *
     * @param key       The identifying key
     *
     * @return true if the animation for the given key is paused
     */
    bool isPaused(std::string key);

    /** 
     * Pauses the animation for the given key.
     *
     * If there is no active animation for the given key, or if it is already
     * paused, this method does nothing.
     *
     * @param key       The identifying key
     */
    void pause(std::string key);

    /**
     * Unpauses the animation for the given key.
     *
     * If there is no active animation for the given key, or if it is not
     * currently paused, this method does nothing.
     *
     * @param key       The identifying key
     */
    void unpause(std::string key);

#pragma mark -
#pragma mark Node Management
    /**
     * Removes all animations for the given target.
     *
     * If the target has no associated animations, this method does nothing.
     *
     * @param target    The node to stop animating
     */
    void clearAllActions(const std::shared_ptr<Node>& target);
    
    /**
     * Pauses all animations for the given target.
     *
     * If the target has no associated animations, or if all of its animations
     * are already paused, this method does nothing.
     *
     * @param target    The node to pause animating
     */
    void pauseAllActions(const std::shared_ptr<Node>& target);

    /**
     * Unpauses all animations for the given target.
     *
     * If the target has no associated animations, or if none of its animations
     * are currenly paused, this method does nothing.
     *
     * @param target    The node to pause animating
     */
    void unpauseAllActions(const std::shared_ptr<Node>& target);

    /**
     * Returns the keys for all active animations of the given target
     *
     * The returned vector is a copy of the keys.  Modifying it has no affect
     * on the underlying animation.
     *
     * @param target    The node to query animations
     *
     * @return the keys for all active animations of the given target
     */
    std::vector<std::string> getAllActions(const std::shared_ptr<Node>& target) const;

};

}
#endif /* __CU_ACTION_MANAGER_H__ */
