#ifndef SWEETSPACE_DONUTNODE_H
#define SWEETSPACE_DONUTNODE_H

#include "CustomNode.h"
#include "DonutModel.h"

class DonutNode : public CustomNode {
#pragma mark Values
   protected:
	/** Reference to the donut model this node represents */
	std::shared_ptr<DonutModel> referencedDonutModel;

	/** Reference to child node which is responsible for rotation */
	std::shared_ptr<cugl::Node> rotationNode;
	/** Reference to node of donut body */
	std::shared_ptr<cugl::PolygonNode> bodyNode;
	/** Reference to node of donut idle face, is active by default */
	std::shared_ptr<cugl::AnimationNode> faceNodeIdle;
	/** Reference to node of donut idle face */
	std::shared_ptr<cugl::AnimationNode> faceNodeDizzy;
	/** Reference to node of donut idle face */
	std::shared_ptr<cugl::AnimationNode> faceNodeWorking;
	/** Counter for controlling speed of facial animation */
	int animationCounter;
	/** Last face state of the model */
	DonutModel::FaceState lastFaceState;

   public:
	/** The scale of the donut textures. */
	static constexpr float DONUT_SCALE = 0.4f;

	/** Spritesheet dimensions for idle face animation */
	static constexpr int ANIMATION_IDLE_W = 4;
	static constexpr int ANIMATION_IDLE_H = 3;
	static constexpr int ANIMATION_IDLE_FRAMES = 10;

	/** Spritesheet dimensions for non-idle face animation */
	static constexpr int ANIMATION_NOTIDLE_W = 5;
	static constexpr int ANIMATION_NOTIDLE_H = 4;
	static constexpr int ANIMATION_NOTIDLE_FRAMES = 20;

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
	DonutNode() : animationCounter(0), lastFaceState(DonutModel::FaceState::Idle) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	virtual ~DonutNode() { dispose(); }

	/**
	 * Init child nodes of donut node
	 */
	bool init(const std::shared_ptr<cugl::Texture> &bodyTexture,
			  const std::shared_ptr<cugl::Texture> &faceIdleTexture,
			  const std::shared_ptr<cugl::Texture> &faceDizzyTexture,
			  const std::shared_ptr<cugl::Texture> &faceWorkTexture,
			  std::shared_ptr<DonutModel> donut);
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
