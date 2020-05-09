#ifndef SWEETSPACE_DONUTNODE_H
#define SWEETSPACE_DONUTNODE_H

#include "CustomNode.h"
#include "DonutModel.h"

class DonutNode : public CustomNode {
   public:

	/** Spritesheet dimensions for idle face animation */
	static constexpr int ANIMATION_IDLE_W = 4;
	static constexpr int ANIMATION_IDLE_H = 3;
	static constexpr int ANIMATION_IDLE_FRAMES = 10;

	/** Spritesheet dimensions for non-idle face animation */
	static constexpr int ANIMATION_NOTIDLE_W = 5;
	static constexpr int ANIMATION_NOTIDLE_H = 4;
	static constexpr int ANIMATION_NOTIDLE_FRAMES = 20;

#pragma mark Values
   protected:
	/** Reference to the donut model this node represents */
	std::shared_ptr<DonutModel> referencedDonutModel;

	/** Reference to child node which is responsible for rotation */
	std::shared_ptr<cugl::Node> rotationNode;
	/** Reference to node of donut body */
	std::shared_ptr<cugl::PolygonNode> bodyNode;
	/** Reference to node of donut face */
	std::shared_ptr<cugl::AnimationNode> faceNode;

	/** Reference to texture of idle face animation */
	std::shared_ptr<cugl::Texture> idleFace;
	/** Reference to texture of dizzy face animation */
	std::shared_ptr<cugl::Texture> dizzyFace;
	/** Reference to texture of working face animation */
	std::shared_ptr<cugl::Texture> workFace;

	/** The scale of the donut textures. */
	static constexpr float DONUT_SCALE = 0.4f;

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
	bool init(const std::shared_ptr<cugl::Texture> &bodyTexture, std::shared_ptr<DonutModel> donut);
#pragma mark -
#pragma mark Getters Setters

	/**
	 * Returns this node's DonutModel
	 * @return
	 */
	std::shared_ptr<DonutModel> getModel() { return referencedDonutModel; }
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
