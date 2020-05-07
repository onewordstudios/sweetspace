#ifndef SWEETSPACE_DONUTNODE_H
#define SWEETSPACE_DONUTNODE_H

#include "CustomNode.h"
#include "DonutModel.h"

class DonutNode : public CustomNode {
   public:
	enum FaceState {
		/** When donut is still or rolling */
		Idle,
		/** When donut collides with mismatched breach */
		Dizzy,
		/** When donut is fixing own breach or collides with door */
		Working
	};
#pragma mark Values
   protected:
	/** Reference to the player donut model */
	std::shared_ptr<DonutModel> donutModel;

	/** Reference to child node which is responsible for rotation */
	std::shared_ptr<cugl::Node> rotationNode;
	/** Reference to node of donut body */
	std::shared_ptr<cugl::PolygonNode> bodyNode;

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
	DonutNode() : CustomNode() {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~DonutNode() { dispose(); }

	/**
	 * Init child nodes of donut node
	 */
	void initChildren(const std::shared_ptr<cugl::Texture> &bodyTexture) {
		rotationNode = cugl::Node::alloc();
		bodyNode = cugl::PolygonNode::allocWithTexture(bodyTexture);
		bodyNode->setAnchor(cugl::Vec2::ANCHOR_CENTER);
		bodyNode->setPosition(0, 0);
		rotationNode->addChild(bodyNode);
		addChild(rotationNode);
	}
#pragma mark -
#pragma mark Getters Setters
	/**
	 * Sets self's model to given parameter
	 * @param model
	 */
	void setModel(std::shared_ptr<DonutModel> model) { donutModel = model; }

	/**
	 * Returns this node's DonutModel
	 * @return
	 */
	std::shared_ptr<DonutModel> getModel() { return donutModel; }
#pragma mark -
#pragma mark Draw Cycle
	/**
	 * Handles jumping-related animation each frame. Does NOT re-position node
	 */
	void animateJumping();

	/**
	 * Handles facial expression animation each frame
	 */
	void animateFacialExpression();
};

#endif // SWEETSPACE_DONUTNODE_H
