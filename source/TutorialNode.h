#ifndef SWEETSPACE_TUTORIALNODE_H
#define SWEETSPACE_TUTORIALNODE_H

#include <cugl/2d/CUAnimationNode.h>

#include "BreachNode.h"
#include "ButtonNode.h"
#include "DoorNode.h"

class TutorialNode : public cugl::AnimationNode {
#pragma mark Values
   protected:
	std::shared_ptr<BreachNode> breachNode;
	std::shared_ptr<DoorNode> doorNode;
	std::shared_ptr<ButtonNode> buttonNode;

   public:
#pragma mark -
#pragma mark Constructor
	/**
	 * Creates an empty polygon with the degenerate texture.
	 *
	 * You must initialize this PolygonNode before use.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
	 * the heap, use one of the static constructors instead.
	 */
	TutorialNode() : cugl::AnimationNode() {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~TutorialNode() { dispose(); }

	/**
	 * Returns a newly allocated filmstrip node from the given texture.
	 *
	 * This constructor assumes that the filmstrip is rectangular, and that
	 * there are no unused frames.
	 *
	 * The size of the node is equal to the size of a single frame in the
	 * filmstrip. To resize the node, scale it up or down.  Do NOT change the
	 * polygon, as that will interfere with the animation.
	 *
	 * @param texture   The texture image to use
	 * @param rows      The number of rows in the filmstrip
	 * @param cols      The number of columns in the filmstrip
	 *
	 * @return a newly allocated filmstrip node from the given texture.
	 */
	static std::shared_ptr<TutorialNode> alloc(const std::shared_ptr<cugl::Texture> &texture) {
		std::shared_ptr<TutorialNode> node = std::make_shared<TutorialNode>();
		return (node->initWithTexture(texture) ? node : nullptr);
	}

#pragma mark -

	void setBreachNode(std::shared_ptr<BreachNode> node) { breachNode = node; }
	void setDoorNode(std::shared_ptr<DoorNode> node) { doorNode = node; }
	void setButtonNode(std::shared_ptr<ButtonNode> node) { buttonNode = node; }
	int getPlayer() { return breachNode->getModel()->getPlayer(); }

	void draw(const std::shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_TUTORIALNODE_H
