#ifndef SWEETSPACE_SPARKLENODE_H
#define SWEETSPACE_SPARKLENODE_H

#include "CustomNode.h"
#include "cugl/2d/CUAnimationNode.h"

class SparkleNode : public CustomNode {
#pragma mark Values
   protected:
	/** Reference to the model of this node. */
	std::shared_ptr<cugl::AnimationNode> filmstrip;
	/** Internal counter for advancing animation frame */
	int animationCounter;
	/** Whether this sparkle is animating */
	bool isAnimating;

#pragma region State Methods
	bool isActive() override;
	void prePosition() override;
	void postPosition() override;
#pragma endregion

   public:
#pragma mark -
	static constexpr int FILMSTRIP_H = 3;
	static constexpr int FILMSTRIP_W = 5;
	static constexpr int FILMSTRIP_SIZE = 13;
#pragma mark Constructor
	/**
	 * Creates an empty Sparkle with the degenerate texture.
	 *
	 * You must initialize this SparkleNode before use.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
	 * the heap, use one of the static constructors instead.
	 */
	SparkleNode() : CustomNode(), animationCounter(0), isAnimating(false) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the breach and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~SparkleNode() { dispose(); }

	/**
	 * Properly initialize this sparkle node. Do NOT use the constructors in the parent class. They
	 * will not initialize everything.
	 *
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param filmstrip	The texture image to use
	 */
	virtual bool init(std::shared_ptr<DonutModel> player, float shipSize,
					  std::shared_ptr<cugl::Texture> filmstrip, cugl::Color4 color);

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
	static std::shared_ptr<SparkleNode> alloc(std::shared_ptr<DonutModel> player, float shipSize,
											  std::shared_ptr<cugl::Texture> filmstrip,
											  cugl::Color4 color) {
		std::shared_ptr<SparkleNode> result = std::make_shared<SparkleNode>();
		return (result->init(player, shipSize, filmstrip, color) ? result : nullptr);
	}

#pragma mark -
#pragma mark Lifecycle
	/**
	 * Reset flags for node animation.
	 */
	void resetAnimation() {
		isAnimating = false;
		animationCounter = 0;
	}

	/**
	 * Start the animation.
	 */
	void beginAnimation() {
		isAnimating = true;
		animationCounter = 0;
		filmstrip->setFrame(0);
	}

	/**
	 * Set the in-ship angle of this sparkle. Should only be called by owner game object
	 * @param a
	 */
	void setOnShipAngle(float a) { angle = a; }

	/**
	 * Tint the filmstrip
	 * @param color
	 */
	void setFilmstripColor(cugl::Color4 color) { filmstrip->setColor(color); }

	/**
	 * Sets the radius of this node. Should only be called by owner game object
	 * @param r
	 */
	void setRadius(float r) { radius = r; }
#pragma mark Drawing

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_SPARKLENODE_H
