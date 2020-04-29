#ifndef SWEETSPACE_DOORNODE_H
#define SWEETSPACE_DOORNODE_H

#include <cugl/2d/CUAnimationNode.h>

#include "CustomNode.h"
#include "DonutModel.h"
#include "DoorModel.h"

class DoorNode : public CustomNode {
#pragma mark Values
   protected:
	/** Reference to the DoorModel of this node */
	std::shared_ptr<DoorModel> doorModel;
	/** Reference to the AnimationNode child of this node */
	std::shared_ptr<cugl::AnimationNode> animationNode;

	/** The height of the door. */
	int height;
	/** The max frame this door can have. */
	int frameCap;

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
	DoorNode() : CustomNode() {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~DoorNode() { dispose(); }

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
	static std::shared_ptr<DoorNode> alloc(const std::shared_ptr<cugl::Texture> &texture, int rows,
										   int cols, int size) {
		std::shared_ptr<DoorNode> node = std::make_shared<DoorNode>();
		if (node->init()) {
			node->animationNode = cugl::AnimationNode::alloc(texture, rows, cols, size);
			node->animationNode->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_CENTER);
			node->animationNode->setPosition(0, 0);
			node->addChild(node->animationNode);
			return node;
		} else {
			return nullptr;
		}
	}

#pragma mark -

	void setModel(std::shared_ptr<DoorModel> model) { doorModel = model; }

	std::shared_ptr<DoorModel> getModel() { return doorModel; }

	std::shared_ptr<cugl::AnimationNode> getAnimationNode() { return animationNode; }

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_DOORNODE_H
