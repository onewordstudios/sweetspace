//
//  CUCompositeNode.cpp
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
#include <cugl/ai/behavior/CUCompositeNode.h>
#include <cugl/util/CUDebug.h>
#include <algorithm>
#include <vector>

using namespace cugl::ai;

#pragma mark Constructors
/**
 * Creates an uninitialized composite node.
 *
 * You should never call this constructor directly. Instead, you should
 * allocate a node with the {@link BehaviorManager} instance.
 */
CompositeNode::CompositeNode() : BehaviorNode(),
_preemptive(false) {
    _classname = "CompositeNode";
}

/**
 * Disposes all of the resources used by this node, including any descendants.
 *
 * A disposed node can be safely reinitialized. Any children owned
 * by this node will be released. They will be deleted if no other object
 * owns them.  This method should only be called by {@link BehaviorManager}.
 */
void CompositeNode::dispose() {
	BehaviorNode::dispose();
    _prioritizer = nullptr;
	_preemptive = false;
}

#pragma mark -
#pragma mark Behavior Tree Interface
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
const BehaviorNode* CompositeNode::getChildByPriorityIndex(Uint32 index) const {
	CUAssertLog(index < _children.size(), "Priority index %d out of bounds",index);
	std::vector<std::shared_ptr<BehaviorNode>> ordered_children = _children;
	std::sort(ordered_children.begin(), ordered_children.end(), BehaviorNode::compareSiblings);
	return ordered_children[index].get();
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
const BehaviorNode* CompositeNode::getActiveChild() const {
    if (_activeChild != -1) {
        return nullptr;
    }
	return _children[_activeChild].get();
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
void CompositeNode::query(float dt) {
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->query(dt);
    }
    if ((_activeChild == -1) || _preemptive) {
        int candidate = selectChild();
        if (candidate != -1) {
            if (_preemptive && _activeChild != -1 && _children[_activeChild] != _children[candidate] ) {
                _children[_activeChild]->preempt();
            }
            _activeChild = candidate;
        }
    }
    if (_prioritizer) {
        setPriority(_prioritizer());
    } else if (_activeChild != -1 ) {
        setPriority(_children[_activeChild]->getPriority());
    }
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
BehaviorNode::State CompositeNode::update(float dt) {
    if (getState() == BehaviorNode::State::RUNNING) {
        _children[_activeChild]->setState(BehaviorNode::State::RUNNING);
        setState(_children[_activeChild]->update(dt));
    }
    
    return getState();
}
