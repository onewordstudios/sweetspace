//
//  CUDecoratorNode.cpp
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
#include <cugl/ai/behavior/CUDecoratorNode.h>
#include <cugl/util/CUDebug.h>

using namespace cugl::ai;
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
void DecoratorNode::query(float dt) {
    if (getState() == BehaviorNode::State::RUNNING) {
        _children[0]->setState(BehaviorNode::State::RUNNING);
        _activeChild = 0;
    }
}

/**
 * Updates this node and any nodes under it.
 *
 * This method runs the update function, which selects a child (if not
 * a leaf) or an action (if a leaf) to execture.  The method
 * {@link updatePriority} may be run as part of this function, especially
 * if it is a composite node supporting preemption.
 *
 * Calling update on a composite node whose children all have zero priority
 * will have unpredictable effects.
 *
 * @param dt    The elapsed time since the last frame.
 *
 * @return the state of this node after updating.
 */
BehaviorNode::State DecoratorNode::update(float dt) {
    if (_activeChild == 0) {
        setState(_children[0]->update(dt));
    }
	return getState();
}
