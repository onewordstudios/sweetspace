//
//  CURandomNode.h
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
#ifndef __CU_RANDOM_NODE_H__
#define __CU_RANDOM_NODE_H__
#include <cugl/ai/behavior/CUCompositeNode.h>
#include <random>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {

/**
 * This class provides a random composite node for a behavior tree.
 *
 * A random node is a composite node that is designed to run a randomly
 * selected nodes out of its children, based on either a uniform probability or
 * a weighted probability. A random node using a weighted probability will base
 * the weights of the probability of selecting each child on the priority of
 * that child.
 *
 * If a random node is not given a priority function, it will set its priority
 * as the average of the priorities of its children.
 *
 * A random node's state is directly based upon the child node currently
 * running or the child node that has finished running. Only one child node
 * will finish running as part of the RandomNode.
 */
class RandomNode : public CompositeNode {
#pragma mark Values
protected:
	/** Whether this node should choose a child uniformly at random. */
	bool _uniform;
    
    /** A reference to the behavior tree manager's random generator */
    std::minstd_rand* _generator;

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

public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates an uninitialized random node.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
    RandomNode();

	/**
	 * Deletes this node, disposing all resources.
	 */
	~RandomNode() { dispose(); }

	/**
	 * Initializes a random node with the given name and generator.
     *
     * The generator is provided by {@link BehaviorManager}. You should never
     * call this method directly. Instead, you should initialize a node with
     * the {@link BehaviorManager} instance.
	 *
	 * @param name			The name of the random node
	 * @param generator		The random number generator
	 *
	 * @return true if initialization was successful.
	 */
    bool init(const std::string& name, std::minstd_rand* generator);

    /**
     * Disposes all of the resources used by this node, including any descendants.
     *
     * A disposed node can be safely reinitialized. Any children owned
     * by this node will be released. They will be deleted if no other object
     * owns them.  This method should only be called by {@link BehaviorManager}.
     */
    void dispose() override;

#pragma mark Attributes
	/**
	 * Returns true if this node chooses uniformly at random.
	 *
	 * If true, then this node chooses its child uniformly at random. Otherwise,
	 * this node uses a weighted probability among its children based on each
	 * child's priority value. 
	 *
	 * @return true if this node chooses uniformly at random.
	 */
	bool isUniform() const { return _uniform; }

    /**
     * Returns true if this node chooses uniformly at random.
     *
     * If true, then this node chooses its child uniformly at random. Otherwise,
     * this node uses a weighted probability among its children based on each
     * child's priority value.
     *
     * @return true if this node chooses uniformly at random.
     */
    void setUniform(bool uniform) { _uniform = uniform; }

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
    std::string toString(bool verbose = false) const override;
    
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
#endif /* __CU_RANDOM_NODE_H__ */
