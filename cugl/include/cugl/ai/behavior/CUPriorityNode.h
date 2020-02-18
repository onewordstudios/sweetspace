//
//  CUPriorityNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a priority composite behavior node. It
//  selects a single node by highest priority.
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
#ifndef __CU_PRIORITY_NODE_H__
#define __CU_PRIORITY_NODE_H__
#include <cugl/ai/behavior/CUCompositeNode.h>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {

/**
 * A class providing a priority composite node for a behavior tree.
 *
 * A priority node is a composite node that chooses a child to run with the
 * highest priority value. If a priority node is allowed to preempt, a child
 * node that is running may be interrupted by another child node that has a
 * higher priority value during the update function.
 *
 * If the priority node is not assigned a priority function, its priority
 * will be assigned as the priority of the running child if this node is
 * currently running, or as the priority of the child with the highest priority
 * if this node is not currently running.
 *
 * A priority node's state is directly based upon the child node currently
 * running or the child node that has finished running. Only one child node
 * will finish running as part of the PriorityNode.
 */
class PriorityNode : public CompositeNode {
#pragma mark Constructors
public:
	/**
	 * Creates an uninitialized priority node.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
    PriorityNode() : CompositeNode() { _classname = "PriorityNode"; };

	/**
	 * Deletes this node, disposing all resources.
	 */
	~PriorityNode() { dispose(); }

#pragma mark -
#pragma mark Internal Helpers
protected:
    /**
     * Returns a (possibly new) active child for this node.
     *
     * This method is subclass dependent, and uses the rules of that subclass
     * to select a child.  If no child is selected, this method returns -1.
     *
     * @return a (possibly new) active child for this node.
     */
    virtual int selectChild() const override;
};

    }
}
#endif /* __CU_PRIORITY_NODE_H__ */
