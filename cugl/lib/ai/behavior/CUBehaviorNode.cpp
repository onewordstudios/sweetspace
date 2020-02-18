//
//  CUBehaviorNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a behavior node as part of a behavior tree.
//  The behavior tree node chooses an action by setting a priority for each node
//  and then traverses down the tree to select an action.
//
//  BehaviorNode objects are managed by BehaviorManager, and should never
//  be allocated directly.  Instead, you create a behavior node definition
//  and pass it to a factor method in BehaviorManager.
//
//  EXPERIMENTAL: This module is experimental.  The API may change significantly
//  in future CUGL releases.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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
//  Version: 5/22/2018
//
#include <cugl/ai/behavior/CUBehaviorNode.h>
#include <cugl/util/CUDebug.h>
#include <sstream>

using namespace cugl::ai;

#pragma mark Behavior Node Defintion
/**
 * Creates an uninitialized behavior node definition.
 *
 * To create a definition for an node, access the attributes directly.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use the static constructor instead.
 */
BehaviorNodeDef::BehaviorNodeDef() :
name(""),
type(Type::LEAF_NODE),
prioritizer(nullptr),
background(false),
preemptive(false),
uniform(false),
delay(0),
action(nullptr) {
}

/**
 * Returns the (first) node with the given name.
 *
 * This method performs a recursive search down the tree specified by this
 * BehaviorNodeDef. If there is more than one node definition of the given
 * name, it returns the first one that is found in an unspecified search
 * order. As a result, names should be unique for best results.
 *
 * This method returns nullptr if no node is found.
 *
 * @param name    An identifier to find the node.
 *
 * @return the (first) node with the given name.
 */
std::shared_ptr<BehaviorNodeDef> BehaviorNodeDef::getNodeByName(const std::string& name) {
	if (this->name == name) {
		return shared_from_this();
	}
	for (auto it = children.begin(); it != children.end(); ++it) {
		std::shared_ptr<BehaviorNodeDef> result = (*it)->getNodeByName(name);
		if (result) {
			return result;
		}
	}
	return nullptr;
}

#pragma mark -
#pragma mark Behavior Node
/**
 * Creates an uninitialized behavior tree node.
 *
 * You should never call this constructor directly. Instead, you should
 * allocate a node with the {@link BehaviorManager} instance.
 */
BehaviorNode::BehaviorNode() : 
_name(""),
_parent(nullptr),
_priority(1),
_prioritizer(nullptr),
_state(State::INACTIVE),
_childOffset(-2),
_activeChild(-1) {
    _classname = "BehaviorNode";
}

/**
 * Initializes a behavior tree node with the given name.
 *
 * You should never call this method directly. Instead, you should
 * initialize a node with the {@link BehaviorManager} instance.
 *
 * @param name      The name of the behavior node
 *
 * @return true if initialization was successful.
 */
bool BehaviorNode::init(const std::string& name) {
	_name = name;
	_childOffset = -1;
	return true;
}

/**
 * Disposes all of the resources used by this node, including any descendants.
 *
 * A disposed node can be safely reinitialized. Any children owned
 * by this node will be released. They will be deleted if no other object
 * owns them.  This method should only be called by {@link BehaviorManager}.
 */
void BehaviorNode::dispose() {
	_name = "";
	_state = BehaviorNode::State::INACTIVE;
	_priority = 0.0f;
	_prioritizer = nullptr;
	for (auto it = _children.begin(); it != _children.end(); ++it) {
		(*it)->_parent = nullptr;
		(*it)->_childOffset = -1;
	}
	_children.clear();
	removeFromParent();
	_parent = nullptr;
	_childOffset = -2;
	_activeChild = -1;
}

#pragma mark Attributes
/**
 * Sets the state of this node.
 *
 * If this node has no parent, then this is the state of the behavior tree.
 *
 * @param state The state of this node.
 */
void BehaviorNode::setState(State state) {
    CUAssertLog(state != BehaviorNode::State::RUNNING || getPriority() != 0.0f,
                "A running node cannot have priority 0.");
    _state = state;
}

/**
 * Returns a string representation of this node for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose    Whether to include class information.
 *
 * @return a string representation of this node for debugging purposes.
 */
std::string BehaviorNode::toString(bool verbose) const {
    std::stringstream ss;
    if (verbose) {
        ss << "cugl::ai::" << _classname;
    }
    ss << "(name:" << _name;
    ss << ", priority:" << _priority;
    ss << ", children[";
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        ss << (*it)->getName() << ",";
    }
    ss << "])";
    return ss.str();
}

#pragma mark Tree Access
/**
 * Returns the list of (weak) references to the node's children.
 *
 * The purpose of this collection is to allow access to the subtree
 * of a behavior tree.  It does not grant ownership, as ownership is
 * confined to {@link BehaviorManager}.
 *
 * @return the list of (weak) references the node's children.
 */
std::vector<const BehaviorNode*> BehaviorNode::getChildren() const {
    std::vector<const BehaviorNode*> result;
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        result.push_back(it->get());
    }
    return result;
}

/**
 * Returns a (weak) pointer to the child node at the given position.
 *
 * The purpose of this pointer is to allow access to the subtree
 * of a behavior tree.  It does not grant ownership, as ownership is
 * confined to {@link BehaviorManager}.
 *
 * @param pos   The child position
 *
 * @return a (weak) pointer to the child node at the given position.
 */
const BehaviorNode* BehaviorNode::getChild(Uint32 pos) const {
    CUAssertLog(pos < _children.size(), "Position %d is out of range",pos);
    return _children[pos].get();
}

/**
 * Returns the (first) node with the given name.
 *
 * This method performs a recursive search down the behavior tree. If
 * there is more than one node with the given name, it returns the first
 * one that is found in an unspecified search order. As a result, names
 * should be unique for best results.
 *
 * The purpose of this pointer is to allow access to the subtree
 * of a behavior tree.  It does not grant ownership, as ownership is
 * confined to {@link BehaviorManager}.
 *
 * @param name    An identifier to find the node.
 *
 * @return the (first) node with the given name.
 */
const BehaviorNode* BehaviorNode::getNodeByName(const std::string& name) const {
    if (_name == name) {
        return this;
    }
    for (auto it = _children.begin(); it != _children.end(); ++it) {
        const BehaviorNode* result = (*it)->getNodeByName(name);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

#pragma mark Behavior Selection
/**
 * Resets this node and all nodes below it to an uninitialized state.
 *
 * This method also resets any class values to those set at the start of
 * the tree. This method allows the node to be started again, as if it had
 * not been run before.
 */
void BehaviorNode::reset() {
	setState(BehaviorNode::State::INACTIVE);
	setPriority(0.0f);
	_activeChild = -1;
	for (auto it = _children.begin(); it != _children.end(); ++it) {
		(*it)->reset();
	}
}

/**
 * Pauses this running node and all running nodes below it in the tree.
 *
 * A paused node can be resumed later. This method has no effect on values
 * stored within nodes, and values (such as priority or timer delay) will
 * not be updated while nodes are paused.
 */
void BehaviorNode::pause() {
	CUAssertLog(getState() == BehaviorNode::State::RUNNING,
				"Cannot pause a non-running node.");
	if (_activeChild != -1) {
		_children[_activeChild]->pause();
	}
	setState(BehaviorNode::State::PAUSED);
}

/**
 * Resumes a paused node and all paused nodes below it in the tree.
 *
 * Values such as priority or timer delay will not have been updated while
 * the node was paused.
 */
void BehaviorNode::resume() {
	CUAssertLog(getState() == BehaviorNode::State::PAUSED,
				"Cannot resume an unpaused node.");
	setState(BehaviorNode::State::RUNNING);
	if (_activeChild != -1) {
		_children[_activeChild]->resume();
	}
}

/**
 * Stops this node from running.
 *
 * This method also stops any running nodes under this one if they exist.
 */
void BehaviorNode::preempt() {
    if (_activeChild != -1) {
        _children[_activeChild]->preempt();
        _activeChild = -1;
    }
    setState(BehaviorNode::State::INACTIVE);
}

/**
 * Initializes this node for execution.
 *
 * When called this node moves from an uninitialized state to one where
 * the {@link update()} function is safe to be called.
 */
void BehaviorNode::start() {
	query(0);
	setState(BehaviorNode::State::RUNNING);
	update(0.0f);
}

#pragma mark Internal Helpers
/**
 * Sets the priority of this node.
 *
 * @param priority The priority of this node.
 */
void BehaviorNode::setPriority(float priority) {
	CUAssertLog(priority >= 0.0f && priority <= 1.0f,
				"Priority %f is out of range",priority);
	_priority = priority;
}

/**
 * Removes the child at the given position from this node.
 *
 * @param pos   The position of the child node that will be removed.
 *
 * @return the child removed at the given position
 */
std::shared_ptr<BehaviorNode> BehaviorNode::removeChild(unsigned int pos) {
	CUAssertLog(pos < _children.size(), "Index out of bounds");
	std::shared_ptr<BehaviorNode> child = _children[pos];
	child->setParent(nullptr);
	child->_childOffset = -1;
	for (int ii = pos; ii < _children.size() - 1; ii++) {
		_children[ii] = _children[ii + 1];
		_children[ii]->_childOffset = ii;
	}
    _children.resize(_children.size() - 1);
    return child;
}

/**
 * Adds the child at the end of the child list of this node.
 *
 * @param child The child to add.
 */
void BehaviorNode::addChild(const std::shared_ptr<BehaviorNode> child) {
    child->setParent(this);
    child->_childOffset = (int)_children.size();
    _children.push_back(child);

}

/**
 * Returns true if sibling a has a larger priority than sibling b.
 *
 * This method is used by std::sort to sort the children. Ties are
 * broken from the offset of the children.
 *
 * @param a The first child
 * @param b The second child
 *
 * @return true if sibling a is has a larger priority than sibling b.
 */
bool BehaviorNode::compareSiblings(const std::shared_ptr<BehaviorNode>& a,
								   const std::shared_ptr<BehaviorNode>& b) {
	return a->_priority > b->_priority
		|| (a->_priority == b->_priority && a->_childOffset > b->_childOffset);
}
