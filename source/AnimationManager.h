#ifndef __ANIMATION_MANAGER_H__
#define __ANIMATION_MANAGER_H__
#include <cugl/cugl.h>

#include <list>
#include <unordered_map>

#include "Tween.h"

/**
 * Helper class to run animations on scene graph nodes. Supports fading nodes in an out, as well as
 * changing their x and y positions. Register a node by name with {@code registerNode}, and then
 * call one of the animate or fade methods to queue up an animation. Call {@code step} each frame to
 * run the animation. Also supports resetting all nodes to their initial positions.
 */
class AnimationManager {
   private:
	/**
	 * Properties that can be animated.
	 */
	enum AnimationProperty { Visibility, PositionX, PositionY };

	/**
	 * Information for a currently in progress animation.
	 */
	struct AnimationTween {
		std::shared_ptr<cugl::Node> node;
		AnimationProperty property;
		Tween::TweenType ease;
		float startVal;
		float endVal;
		unsigned int startFrame;
		unsigned int endFrame;
	};

	/**
	 * All properties of a node we can change.
	 */
	struct AnimationData {
		std::shared_ptr<cugl::Node> node;
		bool visible;
		float posX;
		float posY;
	};

	/** Map of nodes from scene graph name to node pointer */
	std::unordered_map<std::string, std::shared_ptr<cugl::Node>> nodes;

	/** Each node's initial properties from the scene graph; used to reset */
	std::list<AnimationData> initialData;

	/** List of currently in progress animations */
	std::list<AnimationTween> inProgress;

	/** The current frame of animation */
	unsigned int currentFrame;

	/**
	 * Queue a new animation.
	 *
	 * @param node The node to animate
	 * @param property The property to animate
	 * @param ease The easing function to use
	 * @param startVal The starting value of the animated property
	 * @param endVal The ending value of the animated property
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void queue(std::shared_ptr<cugl::Node> node, AnimationProperty property, Tween::TweenType ease,
			   float startVal, float endVal, unsigned int duration, unsigned int delay);

   public:
	/**
	 * Initialize a new, empty Animation Manager.
	 */
	AnimationManager();

	/**
	 * Resets all animations performed by this class, and releases all held pointers.
	 */
	void reset();

	/**
	 * Add a new node for this class to animate.
	 *
	 * @param name The scene graph key of the node
	 * @param assets The asset manager containing the scene graph to get this node from
	 */
	void registerNode(const std::string& name, const std::shared_ptr<cugl::AssetManager>& assets);

	/**
	 * Step the animation forward by one frame.
	 * Clears the input controller queue if an animation has just completed.
	 *
	 * @return True iff an animation is in progress
	 */
	bool step();

	/**
	 * Animate the x position of a node.
	 *
	 * @param node The scene graph key of the node
	 * @param ease The easing function to use
	 * @param destination The target position after the animation
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void animateX(const std::string& node, Tween::TweenType ease, float destination,
				  unsigned int duration, unsigned int delay = 0);

	/**
	 * Animate the y position of a node.
	 *
	 * @param node The scene graph key of the node
	 * @param ease The easing function to use
	 * @param destination The target position after the animation
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void animateY(const std::string& node, Tween::TweenType ease, float destination,
				  unsigned int duration, unsigned int delay = 0);

	/**
	 * Fade in a node linearly
	 *
	 * @param node The scene graph key of the node
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void fadeIn(const std::string& node, unsigned int duration, unsigned int delay = 0);

	/**
	 * Fade out a node linearly
	 *
	 * @param node The scene graph key of the node
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void fadeOut(const std::string& node, unsigned int duration, unsigned int delay = 0);

	/**
	 * Fade out a node linearly
	 *
	 * @param node The scene graph key of the node
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void fadeOut(std::shared_ptr<cugl::Node> node, unsigned int duration, unsigned int delay = 0);
};
#endif /* __ANIMATION_MANAGER_H__ */
