//
//  CUBehaviorManager.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a behavior tree manager. The behavior tree
//  manager controls the creation and execution of behavior trees. It is akin
//  to a world object in Box2d.
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
#include <cugl/ai/behavior/CUBehaviorManager.h>
#include <cugl/ai/behavior/CUPriorityNode.h>
#include <cugl/ai/behavior/CUSelectorNode.h>
#include <cugl/ai/behavior/CURandomNode.h>
#include <cugl/ai/behavior/CUInverterNode.h>
#include <cugl/ai/behavior/CUTimerNode.h>
#include <cugl/ai/behavior/CULeafNode.h>
#include <cugl/util/CUDebug.h>
#include <chrono>

using namespace cugl::ai;

#pragma mark Constructors
/**
 * Initializes a behavior tree manager (with no trees).
 *
 * This initializer creates a random generator whose seed is the current
 * clock value.
 *
 * @return true if initialization was successful.
 */
bool BehaviorManager::init() {
    return init((Uint32)std::chrono::system_clock::now().time_since_epoch().count());
}

/**
 * Initializes a behavior tree manager (with no trees).
 *
 * This initializer creates a random generator from the given seed.
 *
 * @param seed  The random generator seed
 *
 * @return true if initialization was successful.
 */
bool BehaviorManager::init(Uint32 seed) {
    _random.seed(seed);
    return true;
}

/**
 * Disposes all of the resources used by this manager.
 *
 * This will delete all trees owned by the manager.  Unfinished actions
 * will not complete their execution.
 */
void BehaviorManager::dispose() {
    _trees.clear();
}

#pragma mark -
#pragma mark Tree Management
/**
 * Returns whether this manager contains a tree with the given name.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.
 *
 * @param name    An identifier to find the tree.
 *
 * @return whether this manager contains a tree with the given name.
 */
bool BehaviorManager::containsTree(const std::string& name) const {
	auto tree = _trees.find(name);
	return (tree != _trees.end());
}

/**
 * Returns a (weak) reference to the behavior tree with the given name.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.
 *
 * By returning a weak reference, this manager does not pass ownership of
 * the tree.
 *
 * @param name    An identifier to find the tree.
 *
 * @return a (weak) reference to the tree with the given name.
 */
const BehaviorNode* BehaviorManager::getTree(const std::string& name) const {
    if (_trees.find(name) != _trees.end()) {
        return _trees.at(name).get();
    }
    return nullptr;
}

/**
 * Adds the behavior tree described by the provided definition.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.  In this method, the BehaviorManager
 * uses the name of the root node of the behavior tree for the name of
 * the whole tree.
 *
 * This method ecursively creates a behavior tree from the template provided
 * by the {@link BehaviorNodeDef}, and adds it to the BehaviorManager.
 * This method returns false if the BehaviorNodeDef provided does not allow
 * the creation of a valid {@link BehaviorNode}, or if the name provided
 * is already in the manager.  Otherwise it returns true.
 *
 * @param name      The name to assign this tree.
 * @param treedef   The definition for the root of the behavior tree.
 *
 * @return whether the behaivor tree was successfully created and added.
 */
bool BehaviorManager::addTree(const std::shared_ptr<BehaviorNodeDef>& treedef) {
    return addTree(treedef->name,treedef);
}

/**
 * Adds the behavior tree described by the provided definition.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.  However, the name used to access
 * a tree in the manager does not need to be the same name as in the
 * tree node.  This allows the same tree (for navigation purposes) to
 * be used multiple times in the manager.
 *
 * This method ecursively creates a behavior tree from the template provided
 * by the {@link BehaviorNodeDef}, and adds it to the BehaviorManager.
 * This method returns false if the BehaviorNodeDef provided does not allow
 * the creation of a valid {@link BehaviorNode}, or if the name provided
 * is already in the manager.  Otherwise it returns true.
 *
 * @param name      The name to assign this tree.
 * @param treedef   The definition for the root of the behavior tree.
 *
 * @return whether the behaivor tree was successfully created and added.
 */
bool BehaviorManager::addTree(const std::string& name, const std::shared_ptr<BehaviorNodeDef>& treedef) {
    CUAssertLog(_trees.find(name) == _trees.end(),
                "Name '%s' is already in use.",name.c_str());
    std::shared_ptr<BehaviorNode> tree = createTree(treedef);
    if (tree == nullptr) {
        return false;
    }
    _trees[name] = tree;
    return true;
}

#pragma mark -
#pragma mark Behavior Management
/**
 * Starts running the tree with the given name.
 *
 * Adding a tree with {@link addTree} is not enough for the manager
 * to execute it.  This method must be called as well.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.  However, the name used to access
 * a tree in the manager does not need to be the same name as in the
 * tree node.  This allows the same tree (for navigation purposes) to
 * be used multiple times in the manager.
 *
 * @param name    An identifier to find the tree.
 */
void BehaviorManager::startTree(const std::string& name) {
	CUAssertLog(_trees.find(name) != _trees.end(),
				"Behavior tree '%s' does not exist.",name.c_str());
	_trees.at(name)->start();
}

/**
 * Pauses the running tree with the given name.
 *
 * A paused tree will be ignored by the {@link update} method.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.  However, the name used to access
 * a tree in the manager does not need to be the same name as in the
 * tree node.  This allows the same tree (for navigation purposes) to
 * be used multiple times in the manager.
 *
 * @param name    An identifier to find the tree.
 */
void BehaviorManager::pauseTree(const std::string& name) {
    CUAssertLog(_trees.find(name) != _trees.end(),
                "Behavior tree '%s' does not exist.",name.c_str());
	if (_trees.at(name)->getState() == BehaviorNode::State::RUNNING) {
		_trees.at(name)->pause();
	}
}

/**
 * Resumes running the paused tree with the given name.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.  However, the name used to access
 * a tree in the manager does not need to be the same name as in the
 * tree node.  This allows the same tree (for navigation purposes) to
 * be used multiple times in the manager.
 *
 * @param name    An identifier to find the tree.
 */
void BehaviorManager::resumeTree(const std::string& name) {
    CUAssertLog(_trees.find(name) != _trees.end(),
                "Behavior tree '%s' does not exist.",name.c_str());
	if (_trees.at(name)->getState() == BehaviorNode::State::PAUSED) {
		_trees.at(name)->resume();
	}
}

/**
 * Removes the tree with the given name.
 *
 * This method only succeeds the tree is not currently running. Otherwise
 * it will cause an error.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.  However, the name used to access
 * a tree in the manager does not need to be the same name as in the
 * tree node.  This allows the same tree (for navigation purposes) to
 * be used multiple times in the manager.
 *
 * @param name    An identifier to find the tree.
 */
void BehaviorManager::removeTree(const std::string& name) {
    CUAssertLog(_trees.find(name) != _trees.end(),
                "Behavior tree '%s' does not exist.",name.c_str());
	if (_trees.at(name)->getState() != BehaviorNode::State::RUNNING) {
		_trees.erase(name);
	}
}

/**
 * Resets the tree with the given name.
 *
 * This method is used to reset a tree back to its initial state once it
 * has been finished. However, it does not restart the tree.  The
 * {@link startTree} method must be called separately.
 *
 * All trees must be stored with a unique names in the BehaviorManager.
 * No two trees may have the same name.  However, the name used to access
 * a tree in the manager does not need to be the same name as in the
 * tree node.  This allows the same tree (for navigation purposes) to
 * be used multiple times in the manager.
 *
 * @param name    An identifier to find the tree.
 */
void BehaviorManager::resetTree(const std::string& name) {
    CUAssertLog(_trees.find(name) != _trees.end(),
                "Behavior tree '%s' does not exist.",name.c_str());
	if (_trees.at(name)->getState() == BehaviorNode::State::FINISHED) {
		_trees.at(name)->reset();
	}
}

/**
 * Updates all associated behavior trees.
 *
 * This function should be called in the main game loop to process the
 * behaviors for each animation frame.
 *
 * @param dt    The elapsed time since the last frame.
 */
void BehaviorManager::update(float dt) {
	for(auto it = _trees.begin(); it != _trees.end(); ++it) {
		if (it->second->getState() == BehaviorNode::State::RUNNING) {
            it->second->query(dt);
			it->second->update(dt);
		}
	}
}


#pragma mark -
#pragma mark Builders
/**
 * Creates the behavior tree created from the provided definition.
 *
 * This method recursively creates a behavior tree from the template
 * provided by the {@link BehaviorNodeDef} of the root. This method will
 * fail if the behavior node definition does not define a valid behavior
 * tree.
 *
 * @param treedef    The definition for the root of the behavior tree.
 *
 * @return The behavior tree created from provided definition.
 */
std::shared_ptr<BehaviorNode> BehaviorManager::createTree(const std::shared_ptr<BehaviorNodeDef>& treedef) {
    std::shared_ptr<BehaviorNode> result = nullptr;
    std::shared_ptr<BehaviorNode> child;
    std::shared_ptr<BehaviorAction> action;
    switch (treedef->type) {
        case BehaviorNodeDef::Type::INVERTER_NODE:
            CUAssertLog(treedef->children.size() == 1,
                        "Incorrect number of children [%lu] for decorator node",
                        treedef->children.size());
            result = std::make_shared<InverterNode>();
            result->init(treedef->name);
            child = createTree(treedef->children[0]);
            result->addChild(child);
            break;
        case BehaviorNodeDef::Type::TIMER_NODE:
            CUAssertLog(treedef->children.size() == 1,
                        "Incorrect number of children [%lu] for decorator node",
                        treedef->children.size());
            result = std::make_shared<TimerNode>();
            result->init(treedef->name);
            dynamic_cast<TimerNode*>(result.get())->setPrioritizer(treedef->prioritizer);
            dynamic_cast<TimerNode*>(result.get())->setBackground(treedef->background);
            dynamic_cast<TimerNode*>(result.get())->setDelay(treedef->delay);
            child = createTree(treedef->children[0]);
            result->addChild(child);
            break;
        case BehaviorNodeDef::Type::PRIORITY_NODE:
            CUAssertLog(treedef->children.size() > 0,
                        "Incorrect number of children [%lu] for composite node",
                        treedef->children.size());
            result = std::make_shared<PriorityNode>();
            result->init(treedef->name);
            dynamic_cast<PriorityNode*>(result.get())->setPrioritizer(treedef->prioritizer);
            dynamic_cast<PriorityNode*>(result.get())->setPreemptive(treedef->preemptive);
            for(auto it = treedef->children.begin(); it != treedef->children.end(); ++it) {
                child = createTree(*it);
                result->addChild(child);
            }
            break;
        case BehaviorNodeDef::Type::SELECTOR_NODE:
            CUAssertLog(treedef->children.size() > 0,
                        "Incorrect number of children [%lu] for composite node",
                        treedef->children.size());
            result = std::make_shared<SelectorNode>();
            result->init(treedef->name);
            dynamic_cast<SelectorNode*>(result.get())->setPrioritizer(treedef->prioritizer);
            dynamic_cast<SelectorNode*>(result.get())->setPreemptive(treedef->preemptive);
            for(auto it = treedef->children.begin(); it != treedef->children.end(); ++it) {
                child = createTree(*it);
                result->addChild(child);
            }
            break;
        case BehaviorNodeDef::Type::RANDOM_NODE:
            CUAssertLog(treedef->children.size() > 0,
                        "Incorrect number of children [%lu] for composite node",
                        treedef->children.size());
            result = std::make_shared<RandomNode>();
            dynamic_cast<RandomNode*>(result.get())->init(treedef->name,&_random);
            dynamic_cast<RandomNode*>(result.get())->setPrioritizer(treedef->prioritizer);
            dynamic_cast<RandomNode*>(result.get())->setPreemptive(treedef->preemptive);
            dynamic_cast<RandomNode*>(result.get())->setUniform(treedef->uniform);
            for(auto it = treedef->children.begin(); it != treedef->children.end(); ++it) {
                child = createTree(*it);
                result->addChild(child);
            }
            break;
        case BehaviorNodeDef::Type::LEAF_NODE:
            CUAssertLog(treedef->children.size() == 0,
                        "Incorrect number of children [%lu] for leaf node",
                        treedef->children.size());
            result = std::make_shared<LeafNode>();
            result->init(treedef->name);
            dynamic_cast<LeafNode*>(result.get())->setPrioritizer(treedef->prioritizer);
            action = std::make_shared<BehaviorAction>();
            if (action->init(treedef->action)) {
                dynamic_cast<LeafNode*>(result.get())->setAction(action);
            }
            break;
        default:
            ;  // pass
    }
    return result;
}
