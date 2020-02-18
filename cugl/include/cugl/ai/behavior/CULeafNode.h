//
//  CULeafNode.h
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
#ifndef __CU_LEAF_NODE_H__
#define __CU_LEAF_NODE_H__
#include <cugl/ai/behavior/CUBehaviorNode.h>
#include <cugl/ai/behavior/CUBehaviorAction.h>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {
/**
 * A class providing a leaf behavior node for a behavior tree.
 *
 * A leaf node within a behavior tree is a node that performs an action.
 * Each leaf node has a user defined priority function which it will call
 * each update tick to set its priority. This priority is used to select
 * one of the leaf nodes for execution. When a leaf node is selected, it
 * has an associate action which it begins running.
 */
class LeafNode : public BehaviorNode {
#pragma mark Values
protected:
	/** The action used when this node is run. */
	std::shared_ptr<BehaviorAction> _action;

public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates an uninitialized leaf node.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
    LeafNode();

	/**
	 * Deletes this node, disposing all resources.
	 */
	~LeafNode() { dispose(); }
    
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
     * Returns a (weak) pointer to the action used by this leaf node.
     *
     * This method returns a weak reference since it does not transfer
     * ownership of the action.
     *
     * @return a (weak) pointer to the action used by this leaf node.
     */
    const BehaviorAction* getAction() const { return _action.get(); }

    /**
     * Sets the action to be used by this leaf node.
     *
     * @param action    The action to be used by this leaf node.
     */
    void setAction(const std::shared_ptr<BehaviorAction>& action) {
        _action = action;
    }

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
     * Pauses this running node and all running nodes below it in the tree.
     *
     * A paused node can be resumed later. This method has no effect on values
     * stored within nodes, and values (such as priority or timer delay) will
     * not be updated while nodes are paused.
     */
	void pause() override;

    /**
     * Resumes a paused node and all paused nodes below it in the tree.
     *
     * Values such as priority or timer delay will not have been updated while
     * the node was paused.
     */
	void resume() override;
    
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
    void query(float dt) override;
    
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
#endif /* __CU_LEAF_NODE_H__ */
