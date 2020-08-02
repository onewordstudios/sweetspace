#ifndef __BUTTON_MANAGER_H__
#define __BUTTON_MANAGER_H__
#include <cugl/cugl.h>

#include <unordered_map>

class AnimationManager {
   public:
	/**
	 * Tweens that can be used.
	 */
	enum class TweenType { Linear, EaseIn, EaseOut, EaseInOut };

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
		TweenType ease;
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
	void queue(std::shared_ptr<cugl::Node> node, AnimationProperty property, TweenType ease,
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
	void registerNode(std::string name, const std::shared_ptr<cugl::AssetManager>& assets);

	/**
	 * Step the animation forward by one frame.
	 *
	 * @return True iff an animation is in progress
	 */
	bool animate();

	/**
	 * Animate the x position of a node.
	 *
	 * @param node The scene graph key of the node
	 * @param ease The easing function to use
	 * @param destination The target position after the animation
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void animateX(std::string node, TweenType ease, float destination, unsigned int duration,
				  unsigned int delay = 0);

	/**
	 * Animate the y position of a node.
	 *
	 * @param node The scene graph key of the node
	 * @param ease The easing function to use
	 * @param destination The target position after the animation
	 * @param duration The number of frames to run the animation for
	 * @param delay The number of frames to wait before starting the animation
	 */
	void animateY(std::string node, TweenType ease, float destination, unsigned int duration,
				  unsigned int delay = 0);

	void fadeIn(std::string node, unsigned int duration, unsigned int delay = 0);

	void fadeOut(std::string node, unsigned int duration, unsigned int delay = 0);
};
#endif /* __BUTTON_MANAGER_H__ */
