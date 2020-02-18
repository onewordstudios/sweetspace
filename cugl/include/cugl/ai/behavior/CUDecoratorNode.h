//
//  CUDecoratorNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a decorator behavior node. A decorator
//  node has exactly one child, and is used to "change" the behavior of an
//  existing node.
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
#ifndef __CU_DECORATOR_NODE_H__
#define __CU_DECORATOR_NODE_H__

#include <cugl/ai/behavior/CUBehaviorNode.h>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {

/**
 * A class providing a decorator node for a behavior tree.
 *
 * A decorator node is a node that has exactly one child. The decorator node
 * may alter the execution status of its child or use an altered version of its
 * child's priority. The exact method of performing these modifications is
 * defined within the different subclasses of this node. The status of the
 * decorator node is related to the status of its child.
 *
 * A composite node can be given a priority function to call when updating
 * its own priority. If a function is not provided, the composite node
 * will set its priority using a default algorithm, which is specified by
 * its subclasses.  Hence the priority function is a way of overriding the
 * behavior of this node.
 */
class DecoratorNode : public BehaviorNode {
public:
#pragma mark Constructors
	/**
	 * Creates an uninitialized decorator node.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
    DecoratorNode() : BehaviorNode() { _classname = "DecoratorNode"; };

	/**
	 * Deletes this node, disposing all resources.
	 */
	~DecoratorNode() { dispose(); }

#pragma mark Tree Management
	/**
	 * Returns a (weak) pointer to this node's child.
	 *
     * The purpose of this pointer is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
	 *
	 * @return a (weak) pointer to this node's child.
	 */
	const BehaviorNode* getChild() const {
		return _children[0].get();
	}

    /**
     * Returns a (weak) pointer to this node's child.
     *
     * The purpose of this pointer is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
     *
     * This version of the method performs a dynamic typecast to the correct type.
    *
     * @return a (weak) pointer to this node's child.
     */
	template <typename T>
	const T* getChild() const {
		return dynamic_cast<const T*>(getChild());
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
    virtual void query(float dt) override;
    
    /**
     * Updates this node and any nodes under it.
     *
     * This method runs the update function, which selects a child (if not
     * a leaf) or an action (if a leaf) to execture.  The method
     * {@link setPriority} may be run as part of this function, especially
     * if it is a composite node supporting preemption.
     *
     * Calling update on a composite node whose children all have zero priority
     * will have unpredictable effects.
     *
     * @param dt    The elapsed time since the last frame.
     *
     * @return the state of this node after updating.
     */
    virtual BehaviorNode::State update(float dt) override;
};

    }
}
#endif /* __CU_DECORATOR_NODE_H__ */
