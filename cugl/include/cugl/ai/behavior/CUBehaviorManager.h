//
//  CUBehaviorManager.h
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
#ifndef __CU_BEHAVIOR_MANAGER_H__
#define __CU_BEHAVIOR_MANAGER_H__
#include <cugl/ai/behavior/CUBehaviorNode.h>
#include <cugl/ai/behavior/CUBehaviorAction.h>
#include <unordered_map>
#include <functional>
#include <random>
#include <string>

namespace cugl {
    namespace ai {
/**
 * A class providing a centralized manager for behavior trees.
 *
 * An instance of this class owns, runs, and updates all active behavior trees.
 * You should always use a BehaviorManager to create behavior trees, and you
 * should never use a behavior tree not owned by a BehaviorManager.
 *
 * A behavior manager also has a single, centralized random number generator
 * used for all tree processing.  This generator can be given a seed to ensure
 * deterministic behaviors (for testing or networking).
 *
 * To create a behavior tree, the manager uses a {@link BehaviorNodeDef} for
 * the root node, and constructs the behavior tree defined by that definition.
 *
 * Each update frame, the behavior manager updates all running behavior trees
 * until they are finished. The behavior manager can pause, reset or restart
 * any behavior tree it owns.
 */
class BehaviorManager {
#pragma mark Values
protected:
	/** A map of the trees currently being run by the manager. */
	std::unordered_map<std::string, std::shared_ptr<BehaviorNode>> _trees;

    /** The centralized random number generator */
    std::minstd_rand _random;
    
#pragma mark -
#pragma mark Constructors
public:
	/**
	 * Creates an uninitialized behavior manager.
	 *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use the static constructor instead.
	 */
	BehaviorManager() {};

	/**
	 * Deletes this manager, disposing all resources.
     *
     * This will delete all trees owned by the manager.
	 */
	~BehaviorManager() { dispose(); }

	/**
	 * Disposes all of the resources used by this manager.
	 *
	 * This will delete all trees owned by the manager.  Unfinished actions
     * will not complete their execution.
	 */
	void dispose();

	/**
	 * Initializes a behavior tree manager (with no trees).
	 *
     * This initializer creates a random generator whose seed is the current
     * clock value.
     *
	 * @return true if initialization was successful.
	 */
	bool init();
    
    /**
     * Initializes a behavior tree manager (with no trees).
     *
     * This initializer creates a random generator from the given seed.
     *
     * @param seed  The random generator seed
     *
     * @return true if initialization was successful.
     */
    bool init(Uint32 seed);

	/**
	 * Returns a newly allocated behavior tree manager (with no trees).
     *
     * This allocator creates a random generator whose seed is the current
     * clock value.
	 *
	 * @return a newly allocated behavior tree manager (with no trees).
	 */
	static std::shared_ptr<BehaviorManager> alloc() {
		std::shared_ptr<BehaviorManager> result = std::make_shared<BehaviorManager>();
		return (result->init() ? result : nullptr);
	}

    /**
     * Returns a newly allocated behavior tree manager (with no trees).
     *
     * This initializer creates a random generator from the given seed.
     *
     * @param seed  The random generator seed
     *
     * @return a newly allocated behavior tree manager (with no trees).
     */
    static std::shared_ptr<BehaviorManager> alloc(Uint32 seed) {
        std::shared_ptr<BehaviorManager> result = std::make_shared<BehaviorManager>();
        return (result->init(seed) ? result : nullptr);
    }

#pragma mark -
#pragma mark Tree Management
	/**
	 * Returns whether this manager contains a tree with the given name.
	 *
     * All trees must be stored with a unique names in the BehaviorManager.
     * No two trees may have the same name.
     *
	 * @param name	An identifier to find the tree.
	 * 
	 * @return whether this manager contains a tree with the given name.
	 */
	bool containsTree(const std::string& name) const;
    
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
    bool containsTree(const char* name) const {
        return containsTree(std::string(name));
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
	 * @param name	An identifier to find the tree.
	 *
	 * @return a (weak) reference to the tree with the given name.
	 */
	const BehaviorNode* getTree(const std::string& name) const;
    
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
    const BehaviorNode* getTree(const char* name) const {
        return getTree(std::string(name));
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
     * @param treedef   The definition for the root of the behavior tree.
     *
     * @return whether the behaivor tree was successfully created and added.
     */
    bool addTree(const std::shared_ptr<BehaviorNodeDef>& treedef);

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
    bool addTree(const std::string& name, const std::shared_ptr<BehaviorNodeDef>& treedef);

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
    bool addTree(const char* name, const std::shared_ptr<BehaviorNodeDef>& treedef) {
        return addTree(std::string(name),treedef);
    }
    
	/**
	 * Returns the state of the tree with the given name.
	 *
     * All trees must be stored with a unique names in the BehaviorManager.
     * No two trees may have the same name.  However, the name used to access
     * a tree in the manager does not need to be the same name as in the
     * tree node.  This allows the same tree (for navigation purposes) to
     * be used multiple times in the manager.
	 *
	 * @param name	An identifier to find the tree.
	 *
	 * @return the state of the tree with the given name.
	 */
	BehaviorNode::State getTreeState(const std::string& name) const {
		return getTree(name)->getState();
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
	 * @param name	An identifier to find the tree.
	 */
	void startTree(const std::string& name);

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
	 * @param name	An identifier to find the tree.
	 */
	void pauseTree(const std::string& name);

	/**
	 * Resumes running the paused tree with the given name.
	 *
     * All trees must be stored with a unique names in the BehaviorManager.
     * No two trees may have the same name.  However, the name used to access
     * a tree in the manager does not need to be the same name as in the
     * tree node.  This allows the same tree (for navigation purposes) to
     * be used multiple times in the manager.
	 *
	 * @param name	An identifier to find the tree.
	 */
	void resumeTree(const std::string& name);

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
	 * @param name	An identifier to find the tree.
	 */
	void removeTree(const std::string& name);

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
	 * @param name	An identifier to find the tree.
	 */
	void resetTree(const std::string& name);

	/**
     * Updates all associated behavior trees.
     *
     * This function should be called in the main game loop to process the
     * behaviors for each animation frame.
	 *
	 * @param dt	The elapsed time since the last frame.
	 */
	void update(float dt);

private:
	/**
	 * Creates the behavior tree created from the provided definition.
	 *
	 * This method recursively creates a behavior tree from the template
     * provided by the {@link BehaviorNodeDef} of the root. This method will
     * fail if the behavior node definition does not define a valid behavior
     * tree.
	 *
	 * @param treedef	The definition for the root of the behavior tree.
	 *
	 * @return The behavior tree created from provided definition.
	 */
	std::shared_ptr<BehaviorNode> createTree(const std::shared_ptr<BehaviorNodeDef>& treedef);
};
    }
}
#endif /* __CU_BEHAVIOR_MANAGER_H__ */
