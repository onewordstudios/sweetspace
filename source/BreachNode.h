#ifndef SWEETSPACE_BREACHNODE_H
#define SWEETSPACE_BREACHNODE_H

#include <cugl/2d/CUNode.h>

#include "BreachModel.h"
#include "DonutModel.h"
#include "cugl/2d/CUAnimationNode.h"
#include "cugl/2d/CUPolygonNode.h"

class BreachNode : public cugl::Node {
#pragma mark Values
   protected:
	/** Reference to the model of this node. */
	std::shared_ptr<BreachModel> breachModel;
	/** Reference to the player donut model */
	std::shared_ptr<DonutModel> playerDonutModel;
	/** Reference to the shape node of this breach */
	std::shared_ptr<cugl::AnimationNode> shapeNode;
	/** Reference to the pattern node of this breach */
	std::shared_ptr<cugl::PolygonNode> patternNode;
	/** Size of the ship. Needed for visibility determination */
	float shipSize;
	/** Whether the breach is being shown right now */
	bool isShown;
	/** Whether the breach is playing idle animation */
	bool isAnimatingShrink;
	/** Health of the breach model from previous frame */
	int prevHealth;
	/** Current frame for idle animation */
	int currentFrameIdle;

	/** Helper function to calculate frame */
	int getFrameFromHealth(int health) {
		int currentHealth =
			health > BreachModel::HEALTH_DEFAULT ? BreachModel::HEALTH_DEFAULT : health;
		return (BreachModel::HEALTH_DEFAULT - currentHealth) *
			   (shapeNode->getSize() / BreachModel::HEALTH_DEFAULT);
	}

   public:
#pragma mark -
	static constexpr int BREACH_H = 9;
	static constexpr int BREACH_W = 10;
#pragma mark Constructor
	/**
	 * Creates an empty Breach with the degenerate texture.
	 *
	 * You must initialize this BreachNode before use.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
	 * the heap, use one of the static constructors instead.
	 */
	BreachNode() : cugl::Node() {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the breach and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~BreachNode() { dispose(); }

	/**
	 * Returns a newly allocated BreachNode at the world origin.
	 *
	 * The node has both position and size (0,0).
	 *
	 * @return a newly allocated node at the world origin.
	 */
	static std::shared_ptr<BreachNode> alloc() {
		std::shared_ptr<BreachNode> result = std::make_shared<BreachNode>();
		return (result->init() ? result : nullptr);
	}

#pragma mark -
#pragma mark Getters & Setters

	void setModel(std::shared_ptr<BreachModel> model) { breachModel = model; }

	void setDonutModel(std::shared_ptr<DonutModel> model) { playerDonutModel = model; }

	void setShipSize(float f) { shipSize = f; }

	void setPrevHealth(int i) { prevHealth = i; }

	void setShapeNode(std::shared_ptr<cugl::AnimationNode> n) { shapeNode = n; }

	void setPatternNode(std::shared_ptr<cugl::PolygonNode> n) { patternNode = n; }

	bool getIsAnimatingShrink() { return isAnimatingShrink; }

	std::shared_ptr<cugl::AnimationNode> getShapeNode() { return shapeNode; }

	std::shared_ptr<cugl::PolygonNode> getPatternNode() { return patternNode; }

	std::shared_ptr<BreachModel> getModel() { return breachModel; }

#pragma mark -
	/**
	 * Reset flags for node animation.
	 */
	void resetAnimation() {
		isAnimatingShrink = false;
		prevHealth = BreachModel::HEALTH_DEFAULT;
		shapeNode->setFrame(0);
		currentFrameIdle = 0;
	}
#pragma mark Drawing

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_BREACHNODE_H
