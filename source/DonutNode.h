#ifndef SWEETSPACE_DONUTNODE_H
#define SWEETSPACE_DONUTNODE_H

#include <cugl/2d/CUNode.h>

#include "DonutModel.h"

class DonutNode : public cugl::Node {
   public:
	enum FaceState {
		/** When donut is still or rolling */
		Idle,
		/** When donut collides with mismatched breach */
		Dizzy,
		/** When donut is fixing own breach or collides with door */
		Working
	};

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
	std::shared_ptr<DonutModel> donutModel;
	/** Reference to the player donut model */
	std::shared_ptr<DonutModel> playerDonutModel;
	/** Size of the ship. Needed for visibility determination */
	float shipSize;
	/** Whether the breach is being shown right now */
	bool isShown;
	/** State of current facial animation */
	FaceState faceState;

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
	DonutNode() : cugl::Node() {}

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
	void initChildren(const std::shared_ptr<cugl::Texture> &bodyTexture,
					  const std::shared_ptr<cugl::Texture> &faceIdleTexture,
					  const std::shared_ptr<cugl::Texture> &faceDizzyTexture,
					  const std::shared_ptr<cugl::Texture> &faceWorkTexture) {
		idleFace = faceIdleTexture;
		dizzyFace = faceDizzyTexture;
		workFace = faceWorkTexture;

		rotationNode = cugl::Node::alloc();
		bodyNode = cugl::PolygonNode::allocWithTexture(bodyTexture);
		faceNode = cugl::AnimationNode::alloc(idleFace, ANIMATION_IDLE_H, ANIMATION_IDLE_W,
											  ANIMATION_IDLE_FRAMES);
		bodyNode->setAnchor(cugl::Vec2::ANCHOR_CENTER);
		bodyNode->setPosition(0, 0);
		rotationNode->addChild(bodyNode);

		faceNode->setAnchor(cugl::Vec2::ANCHOR_CENTER);
		faceNode->setPosition(0, 0);
		rotationNode->addChild(faceNode);

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
	 * Sets player's donut model to given parameter
	 * @param model
	 */
	void setDonutModel(std::shared_ptr<DonutModel> model) { playerDonutModel = model; }

	/**
	 * Store size of ship level
	 * @param f
	 */
	void setShipSize(float f) { shipSize = f; }

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
