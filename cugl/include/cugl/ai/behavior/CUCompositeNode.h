//
//  CUCompositeNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a composite behavior node. It is used
//  to select from one or more children, according to priority.
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
#ifndef __CU_COMPOSITE_NODE_H__
#define __CU_COMPOSITE_NODE_H__

#include <cugl/ai/behavior/CUBehaviorNode.h>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {
/**
 * A class representing a composite node for a behavior tree.
 *
 * A composite node is a node that has one or more children. When a composite
 * node starts, it chooses a child to run in some order specified by its
 * subclasses. The composite node can be set to preempt its currently running
 * child and choose a new child to run. If it is not set to preempt, the child
 * will continue running until it has either finished running, or the composite
 * node itself is interrupted by its parent. If a child successfully finishes
 * running, the composite node will indicate this in the return status of
 * {@link update}.
 *
 * A composite node can be given a priority function to call when updating
 * its own priority. If a function is not provided, the composite node
 * will set its priority using a default algorithm, which is specified by
 * its subclasses.  Hence the priority function is a way of overriding the
 * behavior of this node.
 */
class CompositeNode : public BehaviorNode {
#pragma mark Values
protected:
	/** Whether to allow preemption among this node's children. */
	bool _preemptive;

#pragma mark Internal Helpers
    /**
     * Returns a (possibly new) active child for this node.
     *
     * This method is subclass dependent, and uses the rules of that subclass
     * to select a child.  If no child is selected, this method returns -1.
     *
     * @return a (possibly new) active child for this node.
     */
    virtual int selectChild() const = 0;

public:
#pragma mark Constructors
	/**
	 * Creates an uninitialized composite node.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
    CompositeNode();

	/**
	 * Deletes this node, disposing all resources.
	 */
	~CompositeNode() { dispose(); }

    /**
     * Disposes all of the resources used by this node, including any descendants.
     *
     * A disposed node can be safely reinitialized. Any children owned
     * by this node will be released. They will be deleted if no other object
     * owns them.  This method should only be called by {@link BehaviorManager}.
     */
	virtual void dispose() override;
    
#pragma mark Attributes
    /**
     * Returns true this node allows preemption among its children.
     *
     * If preemption is allowed, this node may choose a new child to run during
     * an update, possibly interrupting an old child node. Otherwise, the
     * composite node cannot interrupt its running child to select another
     * child to run.
     *
     * @return true this node allows preemption among its children.
     */
    bool isPreemptive() const { return _preemptive; }
    
    /**
     * Sets whether this node allows preemption among its children.
     *
     * If preemption is allowed, this node may choose a new child to run during
     * an update, possibly interrupting an old child node. Otherwise, the
     * composite node cannot interrupt its running child to select another
     * child to run.
     *
     * @param preemptive    Whether this node allows preemption among its children.
     */
    void setPreemptive(bool preemptive) { _preemptive = preemptive; }

#pragma mark Tree Management
	/**
	 * Returns a (weak) reference to the child with the given priority index.
	 *
	 * A child with a specific priority index i is the child with the ith
	 * highest priority. Ties are broken by the position of the child  in its
	 * parent's list.
	 *
     * The purpose of this pointer is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
     *
	 * @param index The child's priority index.
	 *
	 * @return a (weak) reference the child with the given priority index.
	 */
	const BehaviorNode* getChildByPriorityIndex(Uint32 index) const;

    /**
     * Returns a (weak) reference to the child with the given priority index.
     *
     * A child with a specific priority index i is the child with the ith
     * highest priority. Ties are broken by the position of the child  in its
     * parent's list.
     *
     * The purpose of this pointer is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
     *
     * This version of the method performs a dynamic typecast to the correct type.
     *
     * @param index The child's priority index.
     *
     * @return a (weak) reference the child with the given priority index.
     */
	template <typename T>
	const T* getChildByPriorityIndex(Uint32 index) const {
		return dynamic_cast<const T*>(getChildByPriorityIndex(index));
	}

	/**
	 * Returns a (weak) reference to the child currently running.
	 *
     * The purpose of this pointer is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
	 *
	 * @return a (weak) reference to the currently active child.
	 */
	const BehaviorNode* getActiveChild() const;

    /**
     * Returns a (weak) reference to the child currently running.
     *
     * The purpose of this pointer is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
     *
     * This version of the method performs a dynamic typecast to the correct type.
     *
     * @return a (weak) reference to the currently active child.
     */
	template <typename T>
	const T* getActiveChild() const {
		return dynamic_cast<const T*>(getActiveChild());
	}
    
#pragma mark Behavior Management
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
#endif /* __CU_COMPOSITE_NODE_H__ */
