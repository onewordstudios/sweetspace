//
//  CUBehaviorAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for an action, which is a user-defined result
//  chosen by a behavior tree. Instead of requiring the user to subclass an
//  interface, have elected to use functions pointers to allow the user more
//  flexibilty.
//
//  BehaviorAction objects are managed by BehaviorManager, and should never
//  be allocated directly.  Instead, you create a behavior action definition
//  and pass it to a factor method in BehaviorManager.
//
//  EXPERIMENTAL: This module is experimental. The API may change significantly
//  in future CUGL releases.
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
//  Author: Apurv Sethi and Andrew Matsumoto (with Walker White)
//  Version: 5/21/2018
//
#ifndef __CU_BEHAVIOR_ACTION_H__
#define __CU_BEHAVIOR_ACTION_H__
#include <cugl/util/CUDebug.h>
#include <functional>
#include <memory>
#include <string>

namespace cugl {
    namespace ai {

#pragma mark -
#pragma mark Behavior Action Definition
/**
 * A reusable definition for {@link BehaviorAction}.
 *
 * This definition format allows us to have a single action definition that is
 * used across mutliple instances.  The motivation is the same as the difference
 * between a Body and BodyDef in Box2d.
 */
class BehaviorActionDef {
public:
	/** The descriptive, identifying name of the action. */
	std::string name;

	/**
	 * The initialization function to begin running an action.
     *
     * This function should set up the inital action state for the update
     * function.
	 */
    std::function<void()> start;

	/**
	 * The update function processing the action over time.
	 *
	 * This returns true if the action is finished and false otherwise.  If the
     * function returns true, it is assumed that any necessary clean-up has
     * already happened and no further function calls are necessary.
	 */
	std::function<bool(float dt)> update;

	/**
     * The terminate function to manage interruptions.
     *
     * This function is to handle any interruptions that occur before the action
     * has completed. This function should ensure that the actor returns
	 * to a stable state when the action is interrupted.
	 */
	std::function<void()> terminate;

#pragma mark Methods
    /**
	 * Creates an uninitialized behavior action definition.
	 *
     * To create a definition for an action, access the attributes directly.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use the static constructor instead.
     */
    BehaviorActionDef() : name(""), start(nullptr), update(nullptr), terminate(nullptr) {}
    
    /**
     * Returns a newly allocated (uninitialized) behavior action definition.
     *
     * To create a definition for an action, access the attributes directly.
     *
     * @return a newly allocated (uninitialized) behavior action definition.
     */
    static std::shared_ptr<BehaviorActionDef> alloc() {
        return std::make_shared<BehaviorActionDef>();
    }
};

#pragma mark -
#pragma mark Behavior Action
/**
 * A class representing an action contained in a leaf node of a behavior tree.
 *
 * An BehaviorAction refers to the action chosen to execute by a behavior
 * tree. Each action is provided to a leaf node of a behavior tree, and is
 * updated during each tick while that leaf node is running.
 *
 * You should never create object of this class directly.  It is managed by
 * {@link BehaviorManager}.  Instead, you should provide a {@link BehaviorActionDef}
 * to the {@link BehaviorNodeDef} specifying a leaf node, which is then passed
 * to the behavior manager.
 */
class BehaviorAction {
#pragma mark Values
public:
	/**
     * An enumeration indicating the current state of the action.
     *
     * Actions are long running, across multiple animation frames.  Therefore,
     * we need to track them in the same way that we would track an audio
     * asset.
     */
	enum class State : unsigned int {
		/** The action is neither currently running mor has finished running. */
		INACTIVE = 0,
		/** The action is running. */
		RUNNING = 1,
		/** The action is paused (but would be running otherwise). */
		PAUSED = 2,
		/** The action is finished. */
		FINISHED = 3
	};

protected:
	/** The descriptive, identifying name of the action. */
	std::string _name;

	/** The state of the action. */
	BehaviorAction::State _state;

	/**
	 * The initialization function to begin running an action.
	 *
	 * This method is optional to provide.
	 */
	std::function<void()> _start;

	/**
	 * The update function processing the action over time.
	 *
	 * This return true if the action is finished and false otherwise.
	 */
	std::function<bool(float dt)> _update;

	/**
	 * The terminate function to interrupt an action over time.
	 *
	 * This return true if the action is finished and false otherwise.
	 *
	 * This method is optional to provide.
	 */
	std::function<void()> _terminate;

public:
#pragma mark Constructors
	/**
	 * Creates an uninitialized action.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate an action with the {@link BehaviorManager} instance.
	 */
	BehaviorAction();

	/**
	 * Deletes this action, disposing all resources.
	 */
	~BehaviorAction() { dispose(); }

    /**
     * Initializes an action, using the definition template.
     *
     * @param actiondef The definition constructing this action
     *
     * @return true if initialization was successful.
     */
    bool init(const std::shared_ptr<BehaviorActionDef>& actiondef);
    
	/**
	 * Disposes all of the resources used by this action.
	 *
	 * A disposed action can be safely reinitialized.  This method should only
     * be called by {@link BehaviorManager}.
	 */
	void dispose();

#pragma mark Attributes
	/**
	 * Returns a string that is used to identify the action.
	 *
	 * This name is used to identify actions in a behavior tree. It is used
     * by the {@link BehaviorManager} to access this action.
	 *
	 * @return a string that is used to identify the action.
	 */
	const std::string& getName() const { return _name; }

    /**
     * Returns the current state of the action.
     *
     * @return the current state of the action.
     */
    BehaviorAction::State getState() const { return _state; }

    /**
     * Sets the state of this action.
     *
     * @param state The state of this action.
     */
    void setState(BehaviorAction::State state) { _state = state; }

#pragma mark Action Management
	/**
	 * Begins running the action.
	 *
	 * This method will call the _start function, if one was provided.
	 */
	void start();

	/**
	 * Terminates an currently running action.
	 * 
	 * This method provides a way to get back to a stable state while in the
	 * middle of running an action.  This method will be called (indirectly)
     * by the {@link BehaviorManager}, but only if the action is currently
     * running.
	 */
	void terminate();

	/**
	 * Pauses the currently running action.
     *
     * Actions will not be updated while paused. This method will be called
     * (indirectly) by the {@link BehaviorManager}, but only if the action is
     * currently running.
	 */
	void pause();
	
	/**
	 * Resumes the currently paused action. 
	 *
     * This method will be called (indirectly) by the {@link BehaviorManager},
     * but only if the action is currently paused.
	 */
	void resume();

	/**
	 * Resets the currently finished action.
	 *
	 * An action can be safely rerun after resetting. This method will be
     * called (indirectly) by the {@link BehaviorManager}, but only if the
     * action is successfuly finished.
	 */
	void reset();
    
    /**
     * Updates the action.
     *
     * The update function is called each animation frame to further process
     * this action. It will be called by the {@link BehaviorManager}, but
     * only if the action is currently running.
     *
     * @param dt    The elapsed time since the last frame.
     *
     * @return the state of the action after updating.
     */
    BehaviorAction::State update(float dt);

};
    }
}
#endif /* __CU_BEHAVIOR_ACTION_H__ */
