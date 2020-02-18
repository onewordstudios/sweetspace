//
//  CUPriorityNode.cpp
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
#include <cugl/ai/behavior/CUPriorityNode.h>
#include <algorithm>
#include <sstream>

using namespace cugl::ai;

/**
 * Returns a (possibly new) active child for this node.
 *
 * This method is subclass dependent, and uses the rules of that subclass
 * to select a child.  If no child is selected, this method returns -1.
 *
 * @return a (possibly new) active child for this node.
 */
int PriorityNode::selectChild() const {
	auto child = std::min_element(_children.begin(), _children.end(),
                                  BehaviorNode::compareSiblings);
    return (*child)->getParentalOffset();
}
