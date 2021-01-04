#ifndef SWEETSPACE_BREACHNODE_H
#define SWEETSPACE_BREACHNODE_H

#include "BreachModel.h"
#include "CustomNode.h"
#include "DonutModel.h"
#include "SparkleNode.h"
#include "cugl/2d/CUAnimationNode.h"
#include "cugl/2d/CUPolygonNode.h"

class BreachNode : public CustomNode {
#pragma mark Values
   protected:
	/** Reference to the model of this node. */
	std::shared_ptr<BreachModel> breachModel;
	/** Reference to the shape node of this breach */
	std::shared_ptr<cugl::AnimationNode> shapeNode;
	/** Reference to the pattern node of this breach */
	std::shared_ptr<cugl::AnimationNode> patternNode;
	/** Reference to special resolve animation node */
	std::shared_ptr<SparkleNode> sparkleNodeBig;
	/** Reference to special resolve animation node */
	std::shared_ptr<SparkleNode> sparkleNodeSmall;
	/** Whether the breach is playing idle animation */
	bool isAnimatingShrink;
	/** Health of the breach model from previous frame */
	int prevHealth;
	/** Current frame for idle animation */
	int currentFrameIdle;

	/** Helper function to calculate frame */
	unsigned int getFrameFromHealth(int health) {
		unsigned int currentHealth =
			health > BreachModel::HEALTH_DEFAULT ? BreachModel::HEALTH_DEFAULT : health;
		return (BreachModel::HEALTH_DEFAULT - currentHealth) *
			   (shapeNode->getSize() / BreachModel::HEALTH_DEFAULT);
	}

#pragma region State Methods
	bool isActive() override;
	void prePosition() override;
	void postPosition() override;
	void becomeInactive() override;
#pragma endregion

   public:
#pragma mark -
	static constexpr int BREACH_H = 6;
	static constexpr int BREACH_W = 8;
	static constexpr int BREACH_SIZE = 45;
#pragma mark Constructor
	/**
	 * Creates an empty Breach with the degenerate texture.
	 *
	 * You must initialize this BreachNode before use.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
	 * the heap, use one of the static constructors instead.
	 */
	BreachNode() : isAnimatingShrink(false), prevHealth(0), currentFrameIdle(0) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the breach and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~BreachNode() { dispose(); }

	/**
	 * Properly initialize this breach node. Do NOT use the constructors in the parent class. They
	 * will not initialize everything.
	 *
	 * @param breach	Pointer to the breach model
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param filmstrip	The texture image to use
	 * @param pattern	The texture of the inside pattern
	 * @param color		The color of the player's breach
	 */
	virtual bool init(const std::shared_ptr<BreachModel> &breach,
					  std::shared_ptr<DonutModel> player, float shipSize,
					  const std::shared_ptr<cugl::Texture> &filmstrip,
					  const std::shared_ptr<cugl::Texture> &pattern, cugl::Color4 color,
					  std::shared_ptr<SparkleNode> sparkle,
					  std::shared_ptr<SparkleNode> sparkleSmall);

	/**
	 * Returns a newly allocated BreachNode at the world origin.
	 *
	 * @param breach	Pointer to the breach model
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param filmstrip	The texture image to use
	 * @param pattern	The texture of the inside pattern
	 * @param color		The color of the player's breach
	 *
	 * @return a newly allocated node at the world origin.
	 */
	static std::shared_ptr<BreachNode> alloc(const std::shared_ptr<BreachModel> &breach,
											 std::shared_ptr<DonutModel> player, float shipSize,
											 const std::shared_ptr<cugl::Texture> &filmstrip,
											 const std::shared_ptr<cugl::Texture> &pattern,
											 cugl::Color4 color,
											 std::shared_ptr<SparkleNode> sparkleBig,
											 std::shared_ptr<SparkleNode> sparkleSmall) {
		std::shared_ptr<BreachNode> result = std::make_shared<BreachNode>();
		return (result->init(breach, std::move(player), shipSize, filmstrip, pattern, color,
							 std::move(sparkleBig), std::move(sparkleSmall))
					? result
					: nullptr);
	}

#pragma mark -
#pragma mark Getters & Setters

	bool getIsAnimatingShrink() const { return isAnimatingShrink; }

	std::shared_ptr<cugl::AnimationNode> getShapeNode() { return shapeNode; }

	std::shared_ptr<cugl::AnimationNode> getPatternNode() { return patternNode; }

	std::shared_ptr<BreachModel> getModel() { return breachModel; }

	bool getIsShown() { return isShown; }

#pragma mark -
	/**
	 * Reset flags for node animation.
	 */
	void resetAnimation() {
		isAnimatingShrink = false;
		prevHealth = BreachModel::HEALTH_DEFAULT;
		shapeNode->setFrame(0);
		patternNode->setFrame(0);
		currentFrameIdle = 0;
	}

	/**
	 * Update appearance when breach is reused.
	 * @param pattern
	 * @param color
	 */
	void resetAppearance(const std::shared_ptr<cugl::Texture> &pattern, cugl::Color4 color) {
		shapeNode->setColor(color);
		patternNode->setTexture(pattern);
		patternNode->setColor(color);
		resetAnimation();
	}
#pragma mark Drawing

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_BREACHNODE_H
