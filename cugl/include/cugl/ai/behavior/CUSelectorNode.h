//
//  CUSelectorNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a priority composite behavior node. It
//  selects a single node which is the first of nonzero priority.
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
#ifndef __CU_SELECTOR_NODE_H__
#define __CU_SELECTOR_NODE_H__
#include <cugl/ai/behavior/CUCompositeNode.h>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {

/**
 * A class providing a selector composite node for a behavior tree.
 *
 * A selector node is a composite node that is designed to select and
 * run the first child with a non-zero priority and run it. If the selector
 * node is allowed to preempt, a child that is running may be overridden by an
 * earlier child with a non-zero priority during the update function.
 *
 * If the selector node is not assigned a priority function, its priority will
 * be assigned as the priority of the running child if this node is currently
 * running, or as the priority of first of its children with a non-zero
 * priority.
 *
 * A selector node's state is directly based upon the child node currently
 * running or the child node that has finished running. Only one child node
 * will finish running as part of the SelectorNode.
 */
class SelectorNode : public CompositeNode {
#pragma mark Constructors
public:
	/**
	 * Creates an uninitialized selector node.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
    SelectorNode() : CompositeNode() { _classname = "SelectorNode"; };

	/**
	 * Deletes this node, disposing all resources.
	 */
	~SelectorNode() { dispose(); }

#pragma mark -
#pragma mark Internal Helpers
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
#endif /* __CU_SELECTOR_NODE_H__ */
