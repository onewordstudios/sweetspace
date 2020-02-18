//
//  CULeafNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a leaf behavior node.  A leaf node has
//  no children.  Instead, it only has an associated action.  Any running
//  leaf node will execute its action on update.
//
//  BehaviorNode objects are managed by BehaviorManager, and should never
//  be allocated directly.  Instead, you create a behavior node definition
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
#include <cugl/ai/behavior/CULeafNode.h>
#include <cugl/util/CUDebug.h>
#include <sstream>

using namespace cugl::ai;

#pragma mark Constructors
/**
 * Creates an uninitialized leaf node.
 *
 * You must initialize this node before use.
 */
LeafNode::LeafNode() : BehaviorNode(),
_action(nullptr) {
    _classname = "LeafNode";
}

/**
 * Disposes all of the resources used by this node.
 *
 * A disposed LeafNode can be safely reinitialized.
 *
 * It is unsafe to call this on a LeafNode that is still currently
 * inside of a running behavior tree.
 */
void LeafNode::dispose() {
	BehaviorNode::dispose();
	_action = nullptr;
    _prioritizer = nullptr;
}

#pragma mark -
#pragma mark Attributes
/**
 * Returns a string representation of this node for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose	Whether to include class information.
 *
 * @return a string representation of this node for debugging purposes.
 */
std::string LeafNode::toString(bool verbose) const {
	std::stringstream ss;
    if (verbose) {
        ss << "cugl::ai::" << _classname;
    }
	ss << "(name:" << _name;
	ss << ", priority:" << _priority;
	ss << ", action:" << (_action ? _action->getName() : "None");
	ss << ")";
	return ss.str();
}

#pragma mark -
#pragma mark Behavior Selection
/**
 * Resets this node and all nodes below it to an uninitialized state.
 *
 * This method also resets any class values to those set at the start of
 * the tree. This method allows the node to be started again, as if it had
 * not been run before.
 */
void LeafNode::reset() {
	setPriority(0.0f);
	if (getState() == BehaviorNode::State::FINISHED) {
		_action->reset();
	}
	setState(BehaviorNode::State::INACTIVE);
}

/**
 * Pauses this running node and all running nodes below it in the tree.
 *
 * A paused node can be resumed later. This method has no effect on values
 * stored within nodes, and values (such as priority or timer delay) will
 * not be updated while nodes are paused.
 */
void LeafNode::pause() {
	CUAssertLog(getState() == BehaviorNode::State::RUNNING,
                "Cannot pause a non-running node.");
    _action->pause();
	setState(BehaviorNode::State::PAUSED);
}

/**
 * Resumes a paused node and all paused nodes below it in the tree.
 *
 * Values such as priority or timer delay will not have been updated while
 * the node was paused.
 */
void LeafNode::resume() {
	CUAssertLog(getState() == BehaviorNode::State::PAUSED,
                "Cannot resume an unpaused node.");
    setState(BehaviorNode::State::RUNNING);
	_action->resume();
}

/**
 * Stops this node from running.
 *
 * This method also stops any running nodes under this one if they exist.
 */
void LeafNode::preempt() {
    _action->terminate();
    setState(BehaviorNode::State::INACTIVE);
}

/**
 * Updates the priority value(s) for this node.
 *
 * This method recursively determines the priority of this node and all
 * of its children. The priority may be determined by a user-provided
 * priority function or by the default priority function of the class.
 *
 * When this method is complete, it will chose a child node to run, but
 * will not run it. Unlike {@link update}, this method is guaranteed to
 * run every time step in {@link BehaviorManager}, provided that the root
 * node is running.
 *
 * @param dt    The elapsed time since the last frame.
 */
void LeafNode::query(float dt) {
    _priority = _prioritizer();
}

/**
 * Updates this node and any active children.
 *
 * This method runs the update function, which executes to active child
 * (if not a leaf) or the associated action (if a leaf).  This method is
 * not guaranteed to execute every time step; only if the node is the
 * root of the tree or is selected as part of the active path.
 *
 * If a node is not a leaf node and it has no active children, then the
 * method will return INACTIVE.
 *
 * @param dt    The elapsed time since the last frame.
 *
 * @return the state of this node after updating.
 */
BehaviorNode::State LeafNode::update(float dt) {
	if (getState() != BehaviorNode::State::RUNNING) {
		return getState();
	}

	if (_action->getState() == BehaviorAction::State::INACTIVE) {
		_action->start();
	}
	switch(_action->update(dt)) {
		case BehaviorAction::State::RUNNING:
			setState(BehaviorNode::State::RUNNING);
			break;
		case BehaviorAction::State::FINISHED:
			setState(BehaviorNode::State::FINISHED);
			break;
		default:
			break;
	}
	return getState();
}
