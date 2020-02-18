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
#ifndef __CU_TIMER_NODE_H__
#define __CU_TIMER_NODE_H__
#include <cugl/ai/behavior/CUDecoratorNode.h>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {

/**
 * A class decorating a behavior tree node with a timed delay.
 *
 * There are two ways to delay a node.  One is to chose the child, but
 * not update the child until after a delay period.  This is a "foreground"
 * delay.  The node is chosen, preventing other nodes from being chosen
 * (if the parent is not preemptive), but it is delaying its completion.
 *
 * The other type of delay is to delay when this node can be selected again
 * once it has completed successfully.  This is a "background" delay. It will
 * set the priority to 0, and reassign this once the delay has passed.
 */
class TimerNode : public DecoratorNode {
#pragma mark Values
protected:
	/** Whether this node supports an sleepy delay. */
	bool _background;

	/** The amount of time to delay execution of the child in seconds. */
	float _delay;

	/** Whether this node is currently delaying (active or passive) */
	bool _delaying;

	/** The amount of time delayed so far (active or passive) */
	float _timer;

public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates an uninitialized TimerNode.
     *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
	TimerNode();

	/**
	 * Deletes this node, disposing all resources.
	 */
	~TimerNode() { dispose(); }

    /**
     * Disposes all of the resources used by this node, including any descendants.
     *
     * A disposed node can be safely reinitialized. Any children owned
     * by this node will be released. They will be deleted if no other object
     * owns them.  This method should only be called by {@link BehaviorManager}.
     */
	void dispose() override;

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
	std::string toString(bool verbose = false) const override;

	/**
	 * Returns true if this node implements a background delay.
     *
     * There are two ways to delay a node.  One is to chose the child, but
     * not update the child until after a delay period.  This is a "foreground"
     * delay.  The node is chosen, preventing other nodes from being chosen
     * (if the parent is not preemptive), but it is delaying its completion.
     *
     * The other type of delay is to delay when this node can be selected again
     * once it has completed successfully.  This is a "background" delay. It will
     * set the priority to 0, and reassign this once the delay has passed.
     *
	 * @return true if this node implements a background delay.
	 */
	bool isBackground() const { return _background; }

    /**
     * Sets whether this node implements a background delay.
     *
     * There are two ways to delay a node.  One is to chose the child, but
     * not update the child until after a delay period.  This is a "foreground"
     * delay.  The node is chosen, preventing other nodes from being chosen
     * (if the parent is not preemptive), but it is delaying its completion.
     *
     * The other type of delay is to delay when this node can be selected again
     * once it has completed successfully.  This is a "background" delay. It will
     * set the priority to 0, and reassign this once the delay has passed.
     *
     * @param background    Whether this node implements a background delay.
     */
    void setBackground(bool background) { _background = background; }
    
	/**
	 * Returns the delay time in seconds.
     *
     * There are two ways to delay a node.  One is to chose the child, but
     * not update the child until after a delay period.  This is a "foreground"
     * delay.  The node is chosen, preventing other nodes from being chosen
     * (if the parent is not preemptive), but it is delaying its completion.
     *
     * The other type of delay is to delay when this node can be selected again
     * once it has completed successfully.  This is a "background" delay. It will
     * set the priority to 0, and reassign this once the delay has passed.
     *
	 * @return the delay time in seconds.
	 */
	float getDelay() const { return _delay; }

    /**
     * Sets the delay time in seconds.
     *
     * There are two ways to delay a node.  One is to chose the child, but
     * not update the child until after a delay period.  This is a "foreground"
     * delay.  The node is chosen, preventing other nodes from being chosen
     * (if the parent is not preemptive), but it is delaying its completion.
     *
     * The other type of delay is to delay when this node can be selected again
     * once it has completed successfully.  This is a "background" delay. It will
     * set the priority to 0, and reassign this once the delay has passed.
     *
     * @param delay the delay time in seconds.
     */
    void setDelay(float delay) { _delay = delay; }
    
	/**
	 * Returns the amount of time delayed so far (in seconds).
     *
     * This value is reset to zero once the delay is complete.
	 *
	 * @return the amount of time delayed so far (in seconds).
	 */
	float getCurrentDelay() const { return _timer; }

    /**
     * Sets the state of this node.
     *
     * If this node has no parent, then this is the state of the behavior tree.
     *
     * @param state The state of this node.
     */
    void setState(BehaviorNode::State state) override;

#pragma mark Behavior Management
    /**
     * Resets this node and all nodes below it to an uninitialized state.
     *
     * This method also resets any class values to those set at the start of
     * the tree. This method allows the node to be started again, as if it had
     * not been run before.
     */
	void reset() override;

    /**
     * Stops this node from running.
     *
     * This method also stops any running nodes under this one if they exist.
     */
    void preempt() override;

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
    virtual void query(float dt) override;

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
    BehaviorNode::State update(float dt) override;
};

    }
}
#endif /* __CU_TIMER_NODE_H__ */
