//
//  CUBehaviorAction.cpp
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
//  EXPERIMENTAL: This module is experimental.  The API may change significantly
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
#include <cugl/ai/behavior/CUBehaviorAction.h>

using namespace cugl::ai;

#pragma mark Constructors
/**
 * Creates an uninitialized action.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
BehaviorAction::BehaviorAction() :
_name(""),
_state(BehaviorAction::State::INACTIVE),
_start(nullptr),
_update(nullptr),
_terminate(nullptr) {};

/**
 * Disposes all of the resources used by this action.
 *
 * A disposed action can be safely reinitialized.  This method may only
 * be called by {@link BehaviorManager}
 */
void BehaviorAction::dispose() {
	_name = "";
	_state = BehaviorAction::State::INACTIVE;
	_start = nullptr;
	_update = nullptr;
	_terminate = nullptr;
}

/**
 * Initializes an action, using the definition template.
 *
 * @param actiondef The definition constructing this action
 *
 * @return true if initialization was successful.
 */
bool BehaviorAction::init(const std::shared_ptr<BehaviorActionDef>& actiondef) {
	CUAssertLog(actiondef, "Action definition is missing.");
	_name = actiondef->name;
	_start  = actiondef->start;
	_update = actiondef->update;
	_terminate = actiondef->terminate;
    setState(BehaviorAction::State::INACTIVE);
	return true;
}

#pragma mark -
#pragma mark Action Management
/**
 * Begins running the action.
 *
 * This method will call the _start function, if one was provided.
 */
void BehaviorAction::start() {
	CUAssertLog(getState() == BehaviorAction::State::INACTIVE, "Attempt to restart an live action");
	setState(BehaviorAction::State::RUNNING);
	if (_start) {
		_start();
	}
}

/**
 * Terminates an currently running action.
 *
 * This method provides a way to get back to a stable state while in the
 * middle of running an action.  This method will be called (indirectly)
 * by the {@link BehaviorManager}, but only if the action is currently
 * running.
 */
void BehaviorAction::terminate() {
	CUAssertLog(getState() == BehaviorAction::State::RUNNING,
		"Attempt to terminate a non-running action");
	if (_terminate) {
		_terminate();
	}
	setState(BehaviorAction::State::INACTIVE);
}

/**
 * Pauses the currently running action.
 *
 * Actions will not be updated while paused. This method will be called
 * (indirectly) by the {@link BehaviorManager}, but only if the action is
 * currently running.
 */
void BehaviorAction::pause() {
	CUAssertLog(getState() == BehaviorAction::State::RUNNING,
		"Attempt to pause a non-running action");
	setState(BehaviorAction::State::PAUSED);
}

/**
 * Resumes the currently paused action.
 *
 * This method will be called (indirectly) by the {@link BehaviorManager},
 * but only if the action is currently paused.
 */
void BehaviorAction::resume() {
	CUAssertLog(getState() == BehaviorAction::State::PAUSED,
		"Attempt to resume an unpaused action.");
	setState(BehaviorAction::State::RUNNING);
}

/**
 * Resets the currently finished action.
 *
 * An action can be safely rerun after resetting. This method will be
 * called (indirectly) by the {@link BehaviorManager}, but only if the
 * action is successfuly finished.
 */
void BehaviorAction::reset() {
	CUAssertLog(getState() == BehaviorAction::State::FINISHED,
		"Attempt to reset an unfinished action.");
	setState(BehaviorAction::State::INACTIVE);
}

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
BehaviorAction::State BehaviorAction::update(float dt) {
	if (getState() == BehaviorAction::State::RUNNING && _update) {
		setState(_update(dt) ? BehaviorAction::State::FINISHED
                             : BehaviorAction::State::RUNNING);
	}
	return getState();
}
