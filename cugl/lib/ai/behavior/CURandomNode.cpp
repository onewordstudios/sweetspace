//
//  CURandomNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a random composite behavior node.  The
//  random behavior may be uniform or weighted.
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
//  Author: Apurv Sethi and Andrew Matsumoto  (with Walker White)
//  Version: 5/21/2018
//
#include <cugl/ai/behavior/CURandomNode.h>
#include <sstream>

using namespace cugl::ai;

#pragma mark Constructors
/**
 * Creates an uninitialized random node.
 *
 * You must initialize this RandomNode before use.
 */
RandomNode::RandomNode() : CompositeNode(),
_uniform(true),
_generator(nullptr) {
    _classname = "RandomNode";
}

/**
 * Disposes all of the resources used by this node.
 *
 * A disposed RandomNode can be safely reinitialized. Any children owned
 * by this node will be released. They will be deleted if no other object
 * owns them.
 */
void RandomNode::dispose() {
	CompositeNode::dispose();
	_uniform = true;
}

/**
 * Initializes a random node with the given name, children, and priority
 * function. Utilizes uniformly at random to choose child nodes.
 *
 * @param name            The name of the random node.
 * @param priority        The priority function of the random node.
 * @param children         The children of the random node.
 * @param preempt        Whether child nodes can be preempted.
 *
 * @return true if initialization was successful.
 */
bool RandomNode::init(const std::string& name, std::minstd_rand* generator) {
    _generator = generator;
    return _generator && CompositeNode::init(name);
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
std::string RandomNode::toString(bool verbose) const {
	std::stringstream ss;
	ss << (verbose ? "cugl::RandomNode(name:" : "(name:");
	ss << ", priority:" << _priority;
	ss << ", random type:" << (_uniform ? "uniform" : "weighted");
	ss << ", children:[";
	for (auto it = _children.begin(); it != _children.end(); ++it) {
		ss << (*it)->getName();
	}
	ss << "])";
	return ss.str();
}

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
int RandomNode::selectChild() const {
	if (!_uniform) {
		float priority_sum = 0.0f;
		for (auto it = _children.begin(); it != _children.end(); ++it) {
			priority_sum += (*it)->getPriority();
		}
		if (priority_sum > 0.0f) {
			float r = static_cast<float>((*_generator)()) / static_cast<float>(_generator->max() / priority_sum);
			for (auto it = _children.begin(); it != _children.end(); ++it) {
				if (r < (*it)->getPriority()) {
                    return (*it)->getParentalOffset();
				}
				r -= (*it)->getPriority();
			}
		}
	}

    return _children[(*_generator)() % _children.size()]->getParentalOffset();
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
void RandomNode::query(float dt) {
    CompositeNode::query(dt);
    float priority_sum = 0.0f;
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        priority_sum += (*it)->getPriority();
    }
    setPriority(priority_sum/_children.size());
    
}
