//
//  CUActionManager.cpp
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
#include <cugl/2d/actions/CUActionManager.h>

using namespace cugl;

/**
 * Disposes all of the resources used by this action manager.
 *
 * A disposed action manager can be safely reinitialized.Any children owned by this
 * action manager will be released.They will be deleted if no other object owns them.
 */
void ActionManager::dispose() {
    _keys.clear();
    for(auto it = _actions.begin(); it != _actions.end(); ++it) {
        delete it->second;
        it->second = nullptr;
    }
    _actions.clear();
}

/**
 * Deletes this action instance, disposing all resources
 */
ActionManager::ActionInstance::~ActionInstance() {
    interpolant = nullptr;
    action = nullptr;
    target = nullptr;
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
bool ActionManager::isActive(std::string key) const {
    auto action = _actions.find(key);
    return (action != _actions.end());
}

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
bool ActionManager::activate(std::string key,
                             const std::shared_ptr<Action>& action,
                             const std::shared_ptr<Node>& target,
                             std::function<float(float)>interpolation) {
    auto item = _actions.find(key);
    if (item != _actions.end()) {
        return false;
    }
    
    ActionInstance* instance = new ActionInstance();
    instance->action = action;
    instance->target = target;
    instance->interpolant = interpolation;
    action->load(target, &(instance->state));
    _actions.emplace(key,instance);
    _keys[target.get()].emplace(key);
    return true;
}

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
bool ActionManager::remove(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return false;
    }
    ActionInstance* instance = action->second;
    _keys[instance->target.get()].erase(key);
    delete instance;
    action->second = nullptr;
    _actions.erase(action);
    return true;
}

/**
 * Updates all non-paused animations by dt seconds
 *
 * Each animation is moved forward by dt second.  If this causes an animation
 * to reach its duration, the animation is removed and the key is once
 * again available.
 *
 * @param dt    The number of seconds to animate
 */
void ActionManager::update(float dt) {
    auto completed = std::vector<std::unordered_map<std::string,ActionInstance*>::iterator>();
    for(auto it = _actions.begin(); it != _actions.end(); ++it) {
        ActionInstance* instance = it->second;
        Action* action = instance->action.get();
        float current = 1.0;
        float future  = 1.0;
        if (action->getDuration() > 0) {
            current = (instance->elapsed) / action->getDuration();
            future  = (instance->elapsed+dt)/ action->getDuration();
        } else {
            current = 0.0f;
        }
        
        if (instance->interpolant) {
            current = instance->interpolant(current);
            future  = instance->interpolant(future);
        }
        
        action->update(instance->target, &(instance->state), future-current);
        instance->elapsed = instance->elapsed+dt;
        if (instance->elapsed >= action->getDuration()) {
            completed.push_back(it);
        }
    }
    
    for (auto it = completed.begin(); it != completed.end(); ++it) {
        delete (*it)->second;
        (*it)->second = nullptr;
        _actions.erase(*it);
    }
}


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
bool ActionManager::isPaused(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return false;
    }
    return action->second->paused;
}

/**
 * Pauses the animation for the given key.
 *
 * If there is no active animation for the given key, or if it is already
 * paused, this method does nothing.
 *
 * @param key       The identifying key
 */
void ActionManager::pause(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return;
    }
    action->second->paused = true;
}


/**
 * Unpauses the animation for the given key.
 *
 * If there is no active animation for the given key, or if it is not
 * currently paused, this method does nothing.
 *
 * @param key       The identifying key
 */
void ActionManager::unpause(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return;
    }
    action->second->paused = false;
}


#pragma mark -
#pragma mark Node Management
/**
 * Removes all animations for the given target.
 *
 * If the target has no associated animations, this method does nothing.
 *
 * @param target    The node to stop animating
 */
void ActionManager::clearAllActions(const std::shared_ptr<Node>& target) {
    auto set = _keys.find(target.get());
    if (set == _keys.end()) {
        return;
    }
    for(auto it = set->second.begin(); it != set->second.end(); ++it) {
        auto action = _actions.find(*it);
        if (action != _actions.end()) {
            delete action->second;
            action->second = nullptr;
            _actions.erase(action);
        }
    }
    _keys.erase(target.get());
}

/**
 * Pauses all animations for the given target.
 *
 * If the target has no associated animations, or if all of its animations
 * are already paused, this method does nothing.
 *
 * @param target    The node to pause animating
 */
void ActionManager::pauseAllActions(const std::shared_ptr<Node>& target) {
    auto set = _keys.find(target.get());
    if (set == _keys.end()) {
        return;
    }
    for(auto it = set->second.begin(); it != set->second.end(); ++it) {
        auto action = _actions.find(*it);
        if (action != _actions.end()) {
            action->second->paused = true;
        }
    }
}

/**
 * Unpauses all animations for the given target.
 *
 * If the target has no associated animations, or if none of its animations
 * are currenly paused, this method does nothing.
 *
 * @param target    The node to pause animating
 */
void ActionManager::unpauseAllActions(const std::shared_ptr<Node>& target) {
    auto set = _keys.find(target.get());
    if (set == _keys.end()) {
        return;
    }
    for(auto it = set->second.begin(); it != set->second.end(); ++it) {
        auto action = _actions.find(*it);
        if (action != _actions.end()) {
            action->second->paused = false;
        }
    }
}

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
std::vector<std::string> ActionManager::getAllActions(const std::shared_ptr<Node>& target) const {
    std::vector<std::string> result;
    auto set = _keys.find(target.get());
    if (set == _keys.end()) {
        return std::vector<std::string>();
    }
    for(auto it = set->second.begin(); it != set->second.end(); ++it) {
        result.push_back(*it);
    }
    return result;
}




