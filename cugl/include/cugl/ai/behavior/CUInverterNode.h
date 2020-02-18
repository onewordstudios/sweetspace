//
//  CUInverterNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for an inverter decorator behavior node.
//  An inverter takes a node of priority p and gives it the opposite priority
//  1-p.  This is type of negation operator.
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
#ifndef __CU_INVERTER_NODE_H__
#define __CU_INVERTER_NODE_H__
#include <cugl/ai/behavior/CUDecoratorNode.h>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {

/**
 * A class providing an inverter decorator node for a behavior tree.
 *
 * An inverter node is a decorator node that sets its priority value equal to
 * the opposite of its child's priority. As the priority values for behavior
 * tree nodes are between 0 to 1, the priority of this node is 1 - the child's
 * priority value.
 *
 * An inverter node's state is directly based on its child's state. When an
 * inverter node starts, it immediately starts its child. When the child
 * finishes execution, the inverter node also finishes execution.
 */
class InverterNode : public DecoratorNode {
#pragma mark Constructors
public:
	/**
	 * Creates an uninitialized inverter node.
     *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
    InverterNode() : DecoratorNode() { _classname = "InverterNode"; };

	/**
	 * Deletes this node, disposing all resources.
	 */
	~InverterNode() { dispose(); }

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
};
    }
}
#endif /* __CU_INVERTER_NODE_H__ */
