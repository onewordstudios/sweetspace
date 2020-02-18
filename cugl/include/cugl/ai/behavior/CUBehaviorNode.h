//
//  CUBehaviorNode.h
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
#ifndef __CU_BEHAVIOR_NODE_H__
#define __CU_BEHAVIOR_NODE_H__
#include <cugl/ai/behavior/CUBehaviorAction.h>
#include <functional>
#include <string>
#include <vector>

namespace cugl {
    namespace ai {

#pragma mark Behavior Node Defintion
/**
 * A reusable definition for {@link BehaviorNode}.
 *
 * This definition format allows us to have a single node definition that is
 * used across mutliple instances.  The motivation is the same as the difference
 * between a Body and BodyDef in Box2d.  This node definition can be used for
 * {@link BehaviorNode} or any of its subclasses.
 */
class BehaviorNodeDef : public std::enable_shared_from_this<BehaviorNodeDef> {
public:
	/**
	 * An enum used to describe the type of the {@link BehaviorNode}.
	 *
	 * When creating an instance of a behavior tree node from a BehaviorNodeDef,
     * this enum is used to determine the type of {@link BehaviorNode} created.
     *
     * Behavior tree nodes are either composite, decorator, or leaf nodes.  A
     * leaf node has no children, a decorator has only one, and a composite has
     * one or more. Only leaf nodes have actions attached.
	 */
	enum class Type : int {
		/**
		 * A composite node to select the child with the highest priority.
		 */
		PRIORITY_NODE,
        /**
         * A composite node to select the child first in a list.
         */
		SELECTOR_NODE,
        /**
         * A composite node to select the child randomly.
         *
         * The selection is either uniform or from a weighted probability based
         * on priority values.
         */
		RANDOM_NODE,
		/**
		 * A decorator to invert a child's priority value.
         *
         * As priorities are measure 0 to 1, the inverted priority it 1-priority.
		 * This node does not use the priority function provided by the user.
		 */
		INVERTER_NODE,
		/**
		 * A decorator to delay the execution of a child node.
         *
         * Based on the value of _timeDelay, this will delay the initial
         * execution of its child, and also ensure that the child is not run
         * again after a subsequent delay.
		 */
		TIMER_NODE,
		/**
         * A leaf node in charge of  running an action.
         *
         * This is the base  node used for conditional execution (through the
         * priority function). A leaf node must have an action associated with
         * it, and cannot have any children.
		 */
		LEAF_NODE
	};
    
	/** The descriptive, identifying name of the node. */
	std::string name;

	/** The type of behavior tree node this definition describes. */
	Type type;

	/**
	 * The priority function for this behavior tree node.
	 *
	 * This function is used to assign a priority to a particular node. This
	 * function must return a value between 0 and 1.
	 *
     * This option is currently ignored by any decorator node, but is used
     * by all other nodes.
	 */
	std::function<float()> prioritizer;
    
    /**
     * Whether this node should be run in the "background".
     *
     * A background node performs some limited update even when the method
     * {@link BehaviorNode#query} is called (which happens every step).  Otherwise, 
     * the node only updates when {@link BehaviorNode#update} is called.
     *
     * Currently this option is only used by {@link TimerNode}, which uses
     * it to implement a background delay.
     */
    bool background;

	/**
	 * Whether a node should choose a child to run on each update.
     *
     * A preemptive composite node can interrupt an old child node's execution
     * if a different child is chosen. If false, a new child is never rechosen
     * until the current active child finishes.  This value does not effect
     * whether this node can be preempted by its parent.
	 *
	 * This option is only used if this node is a composite node
	 * ({@link PriorityNode}, {@link SelectorNode}, {@link RandomNode}).
	 */
	bool preemptive;

	/**
	 * Whether a random node should use a uniform probability.
     *
     * When true, the composite node chooses among its children uniformly
     * at random. Otherwise, it uses a weighted probability computed from the
     * priority of each child.
	 *
	 * This option is currently only used if this node is a {@link RandomNode}.
	 */
	bool uniform;

	/**
	 * The amount of time to delay execution of a child.
     *
     * There are two ways to delay a node.  One is to chose the child, but
     * not update the child until after a delay period.  This is an "active"
     * delay.  The node is chosen, preventing other nodes from being chosen
     * (if the parent is not preemptable), but it is delaying its completion.
     *
     * The other type of delay is to delay when this node can be selected again
     * once it has completed successfully.  This is a "sleepy" delay.  It will
     * set the priority to 0, and reassign this once the delay has passed.
	 *
	 * This option is currently only used if this node is a {@link TimerNode}.
	 */
	float delay;
    
    /**
     * The array of definitions for the children for this node.
     *
     * If this node is a leaf node, then this vector should be empty. If this
     * node is a decorator node, then this vector should have exactly one
     * element.
     */
    std::vector<std::shared_ptr<BehaviorNodeDef>> children;

	/**
	 * The action performed when this node is run.
	 *
	 * This value is only used when this node is a {@link LeafNode}.
	 */
	std::shared_ptr<BehaviorActionDef> action;

#pragma mark Methods
    /**
     * Creates an uninitialized behavior node definition.
     *
     * To create a definition for an node, access the attributes directly.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use the static constructor instead.
     */
    BehaviorNodeDef();

    /**
     * Returns a newly allocated (uninitialized) behavior node definition.
     *
     * To create a definition for an node, access the attributes directly.
     *
     * @return a newly allocated (uninitialized) behavior node definition.
     */
    static std::shared_ptr<BehaviorNodeDef> alloc() {
        return std::make_shared<BehaviorNodeDef>();
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
	 * @param name	An identifier to find the node.
	 *
	 * @return the (first) node with the given name.
	 */
	std::shared_ptr<BehaviorNodeDef> getNodeByName(const std::string& name);

    /**
     * Returns the (first) node definition with the given name.
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
    std::shared_ptr<BehaviorNodeDef> getNodeByName(const char* name) {
        return getNodeByName(std::string(name));
    }

};

    
#pragma mark -
#pragma mark Behavior Node
/**
 * An abstract class for a behavior tree node.
 *
 * This class is a base class for the individual nodes of the behavior tree.
 * Behavior tree nodes are either composite, decorator, or leaf nodes.  A leaf
 * node has no children, a decorator has only one, and a composite has
 * one or more. Only leaf nodes have actions attached.
 *
 * A behavior tree is a construction of behavior nodes. The top node without
 * a parent is the the root of the tree. The tree chooses the action to run
 * based on the priority value of each of the root's descendents. The tree
 * must use an update function to run on each tick, updating the state of each
 * node. The root node of a behavior tree returns the state of the selected
 * leaf node to run.
 *
 * This class has abstract methods for calculating the priority and updating,
 * which are implemented by the subclasses.
 *
 * Behavior trees should be managed by a {@link BehaviorManager}, which creates
 * each BehaviorNode from a {@link BehaviorNodeDef} and runs and updates the
 * behavior trees. While in the {@link BehaviorManager}, a behavior tree cannot
 * be modified by any outside methods and any references to the nodes of the
 * behavior tree will be constant.
 */
class BehaviorNode {
#pragma mark Values
public:
    /**
     * An enumeration indicating the current state of the tree node.
     *
     * Behaviors are long running, across multiple animation frames.  Therefore,
     * we need to track them in the same way that we would track an audio
     * asset.
     */
    enum class State : unsigned int {
		/** The node is neither running nor has already finished with an action. */
		INACTIVE = 0,
		/** The node is active and currently running. */
		RUNNING = 1,
		/** The node is active but currently paused. */
		PAUSED  = 2,
		/** The node is finished with an action. */
		FINISHED = 3
	};

protected:
	/** The descriptive, identifying name of the node. */
	std::string _name;

    /** The name of this class (for debugging polymorphism). */
    std::string _classname;
    
	/** A weaker pointer to the parent (or null if root). */
	BehaviorNode* _parent;

	/** The current state of this node. */
	BehaviorNode::State _state;

	/** The current priority, or relevance of this node. */
	float _priority;

    /** The current priority function for this behavior node. */
    std::function<float()> _prioritizer;

	/** The array of children for this composite node. */
	std::vector<std::shared_ptr<BehaviorNode>> _children;

	/** The index of the child running (-1 if no child is currently running). */
	int _activeChild;

	/** The (current) child offset of this node (-1 if root) */
	int _childOffset;

public:
#pragma mark Constructors
	/**
	 * Creates an uninitialized behavior tree node.
	 *
     * You should never call this constructor directly. Instead, you should
     * allocate a node with the {@link BehaviorManager} instance.
	 */
	BehaviorNode();

	/**
	 * Deletes this node, disposing all resources.
	 */
	~BehaviorNode() { dispose(); }

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
	bool init(const std::string& name);

	/**
	 * Disposes all of the resources used by this node, including any descendants.
	 *
	 * A disposed node can be safely reinitialized. Any children owned
	 * by this node will be released. They will be deleted if no other object
     * owns them.  This method should only be called by {@link BehaviorManager}.
	 */
	virtual void dispose();

#pragma mark Attributes
	/**
	 * Returns a string that is used to identify the node.
	 *
     * This name is used to identify nodes in a behavior tree. It is used
     * by the {@link BehaviorManager} to access this node.
	 *
	 * @return a string that is used to identify the node.
	 */
	const std::string& getName() const { return _name; }

	/**
	 * Returns the current priority of this node.
	 *
	 * This priority value is used to determine the relevance of a node in
	 * comparison to other nodes. This value is between 0 and 1. Higher
     * priority nodes are more likely to be selected.  It will be updated
     * each time {@link query} is called.
	 *
	 * @return a float that signifies the priority of this behavior tree node.
	 */
	float getPriority() const { return _priority; }

	/**
	 * Returns the state of this node.
	 *
	 * If this node has no parent, then this is the state of the behavior tree.
	 *
	 * @return the state of this node.
	 */
	BehaviorNode::State getState() const { return _state; }

	/**
	 * Sets the state of this node.
	 *
     * If this node has no parent, then this is the state of the behavior tree.
	 *
	 * @param state The state of this node.
	 */
	virtual void setState(BehaviorNode::State state);

    /**
     * Returns the priority function for this node.
     *
     * This function should return a value between 0 and 1 representing the
     * priority. When this function is defined, it overrides the rules that
     * this node uses for defining its priority in {@link query}.
     *
     * @return the priority function for this node.
     */
    std::function<float()> getPrioritizer() const { return _prioritizer; }
    
    /**
     * Sets the priority function for this node.
     *
     * This function should return a value between 0 and 1 representing the
     * priority. When this function is defined, it overrides the rules that
     * this node uses for defining its priority in {@link query}.
     *
     * @param func  The priority function for this node.
     */
    void setPrioritizer(const std::function<float()>& func) {
        _prioritizer = func;
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
    virtual std::string toString(bool verbose = false) const;
    
    /**
     * Cast from a BehaviorNode to a string.
     */
    operator std::string() const { return toString(); }

#pragma mark Tree Access
	/**
	 * Returns a (weak) pointer to the parent node.
	 *
	 * The purpose of this pointer is to climb back up the behavior tree.
	 * No child asserts ownership of its parent.
	 *
	 * @return a (weak) pointer to the parent node.
	 */
	const BehaviorNode* getParent() const { return _parent; }

	/**
	 * Sets the parent of this node.
	 *
	 * The purpose of this pointer is to climb back up the behavior tree.
	 * No child asserts ownership of its parent.
	 *
	 * @param parent The parent of this node.
	 */
	void setParent(BehaviorNode* parent) { _parent = parent; }

	/**
	 * Removes this node from its parent.
	 *
	 * If this node has no parent, nothing happens.
	 */
	void removeFromParent() { if (_parent) _parent->removeChild(_childOffset); }

	/**
     * Returns the offset of this behavior tree node within its parent node.
     *
     * If this node is a root node, it will return -1.
     *
     * @return The child offset of this behavior node.
     */
	int getParentalOffset() const { return _childOffset; }

    /**
     * Returns the number of children of this composite node.
     *
     * @return The number of children of this composite node.
     */
    size_t getChildCount() const { return _children.size(); }
    
    /**
     * Returns the list of (weak) references to the node's children.
     *
     * The purpose of this collection is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
     *
     * @return the list of (weak) references the node's children.
     */
    std::vector<const BehaviorNode*> getChildren() const;
    
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
    const BehaviorNode* getChild(Uint32 pos) const;

    /**
     * Returns a (weak) pointer to the child node at the given position.
     *
     * The purpose of this pointer is to allow access to the subtree
     * of a behavior tree.  It does not grant ownership, as ownership is
     * confined to {@link BehaviorManager}.
     *
     * This version of the method performs a dynamic typecast to the correct type.
     *
     * @param pos   The child position
     *
     * @return a (weak) pointer to the child node at the given position.
     */
    template <typename T>
    const T* getChild(Uint32 pos) const {
        return dynamic_cast<const T*>(getChild(pos));
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
    const BehaviorNode* getNodeByName(const std::string& name) const;
    
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
    const BehaviorNode* getNodeByName(const char* name) const {
        return getNodeByName(std::string(name));
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
     * This version of the method performs a dynamic typecast to the correct type.
     *
     * @param name    An identifier to find the node.
     *
     * @return the (first) node with the given name.
     */
    template <typename T>
    const T* getNodeByName(const std::string& name) const {
        return dynamic_cast<const T*>(getNodeByName(name));
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
     * This version of the method performs a dynamic typecast to the correct type.
     *
     * @param name    An identifier to find the node.
     *
     * @return the (first) node with the given name.
     */
    template <typename T>
    const T* getNodeByName(const char* name) const {
        return dynamic_cast<const T*>(getNodeByName(name));
    }
    
#pragma mark Behavior Management
	/**
	 * Resets this node and all nodes below it to an uninitialized state.
     *
     * This method also resets any class values to those set at the start of
     * the tree. This method allows the node to be started again, as if it had
     * not been run before.
	 */
	virtual void reset();

	/**
	 * Pauses this running node and all running nodes below it in the tree.
     *
	 * A paused node can be resumed later. This method has no effect on values
     * stored within nodes, and values (such as priority or timer delay) will
     * not be updated while nodes are paused.
	 */
	virtual void pause();

	/**
	 * Resumes a paused node and all paused nodes below it in the tree.
	 *
	 * Values such as priority or timer delay will not have been updated while
     * the node was paused.
	 */
	virtual void resume();

    /**
     * Stops this node from running.
     *
     * This method also stops any running nodes under this one if they exist.
     */
    virtual void preempt();

	/**
	 * Initializes this node for execution.
     *
     * When called this node moves from an uninitialized state to one where
     * the {@link update()} function is safe to be called.
	 */
	virtual void start();
    
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
    virtual void query(float dt) = 0;
    
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
    virtual BehaviorNode::State update(float dt) = 0;

	/**
	 * Sets the priority of this node.
	 *
	 * @param priority The priority of this node.
	 */
	void setPriority(float priority);

	/**
	 * Removes the child at the given position from this node.
	 *
	 * @param pos   The position of the child node that will be removed.
     *
     * @return the child removed at the given position
	 */
	std::shared_ptr<BehaviorNode> removeChild(Uint32 pos);

    /**
     * Adds the child at the end of the child list of this node.
     *
     * @param child The child to add.
     */
    void addChild(const std::shared_ptr<BehaviorNode> child);

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
	static bool compareSiblings(const std::shared_ptr<BehaviorNode>& a,
                                const std::shared_ptr<BehaviorNode>& b);
};
    }
}
#endif /* __CU_BEHAVIOR_NODE_H__ */
