//
//  CUTimerNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a decorator behavior node with a timed
//  delay.  The delay may either be foreground (the node is selected an running,
//  but is not doing anything) or background (the node cannot be selected until
//  some time has passed).
//
//  BehaviorNode objects are managed by BehaviorManager, and should never
//  be allocated directly.  Instead, you create a behavior node definition
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
#include <cugl/ai/behavior/CUTimerNode.h>
#include <cugl/util/CUDebug.h>
#include <sstream>

using namespace cugl::ai;

#pragma mark Constructors
/**
 * Creates an uninitialized TimerNode.
 *
 * You should never call this constructor directly. Instead, you should
 * allocate a node with the {@link BehaviorManager} instance.
 */
TimerNode::TimerNode() : DecoratorNode(),
_background(false),
_delaying(false),
_delay(0.0f),
_timer(0.0f) {
    _classname = "TimerNode";
}

/**
 * Disposes all of the resources used by this node, including any descendants.
 *
 * A disposed node can be safely reinitialized. Any children owned
 * by this node will be released. They will be deleted if no other object
 * owns them.  This method should only be called by {@link BehaviorManager}.
 */
void TimerNode::dispose() {
	DecoratorNode::dispose();
    _background = false;
    _delaying = false;
	_delay = 0.0f;
	_timer = 0.0f;
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
std::string TimerNode::toString(bool verbose) const {
	std::stringstream ss;
	ss << (verbose ? "cugl::TimerNode(name:" : "(name:") << _name;
	ss << ", priority:" << _priority;
	ss << ", child:" << (_children[0] ? _children[0]->getName() : "None");
	ss << ", delay type:" << (_background ? "background" : "foreground");
	ss << ", delay time:" << _delay;
	ss << ")";
	return ss.str();
}

/**
 * Sets the state of this node.
 *
 * If this node has no parent, then this is the state of the behavior tree.
 *
 * @param state The state of this node.
 */

void TimerNode::setState(BehaviorNode::State state) {
	CUAssertLog(state != BehaviorNode::State::RUNNING || getPriority() != 0.0f,
				"A running node cannot have priority 0.");
	if (_state == state) { return; }
	if (_state != BehaviorNode::State::PAUSED &&
		state == BehaviorNode::State::RUNNING && !_background) {
		_delaying = true;
	}
	_state = state;
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
void TimerNode::reset() {
	_delaying = false;
	_timer = 0.0f;
	BehaviorNode::reset();
}

/**
 * Stops this node from running.
 *
 * This method also stops any running nodes under this one if they exist.
 */
void TimerNode::preempt() {
    if (_background) {
        _delaying = true;
        setPriority(0.0f);
    }
    DecoratorNode::preempt();
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
void TimerNode::query(float dt) {
	if (_delaying && _background) {
		setPriority(0.0f);
        _timer += dt;
        if (_timer >= _delay) {
            _delaying = false;
            _timer = 0.0f;
        }
	} else {
		_children[0]->query(dt);
		setPriority(_children[0]->getPriority());
	}
}

/**
 * Updates this node and any nodes under it.
 *
 * Runs an update function, meant to be used on each tick, for the
 * behavior node (and nodes below it in the tree).
 *
 * Update priority may be run as part of this function, based on whether a
 * composite node uses preemption.
 *
 * @param dt	The elapsed time since the last frame.
 *
 * @return the state of this node after updating.
 */
BehaviorNode::State TimerNode::update(float dt) {
	if (_delaying && !_background) {
		_timer += dt;
		if (_timer >= _delay) {
			_delaying = false;
			_timer = 0.0f;
		}
	}
	if (getState() == BehaviorNode::State::RUNNING && !_delaying) {
		_children[0]->setState(BehaviorNode::State::RUNNING);
		setState(_children[0]->update(dt));
	}
	return getState();
}
