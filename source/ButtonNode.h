#ifndef SWEETSPACE_BUTTONNODE_H
#define SWEETSPACE_BUTTONNODE_H

#include "ButtonModel.h"
#include "CustomNode.h"
#include "DonutModel.h"
#include "SparkleNode.h"

class ButtonNode : public CustomNode {
#pragma mark Values
   protected:
	shared_ptr<ButtonModel> buttonModel;

	/** Texture for activated button base */
	std::shared_ptr<cugl::Texture> btnBaseDown;
	/** Texture for unactivated button base */
	std::shared_ptr<cugl::Texture> btnBaseUp;
	/** Texture for activated button body */
	std::shared_ptr<cugl::Texture> btnDown;
	/** Texture for unactivated button base */
	std::shared_ptr<cugl::Texture> btnUp;
	/** Reference to button body node */
	std::shared_ptr<cugl::PolygonNode> bodyNode;
	/** Reference to button base node */
	std::shared_ptr<cugl::PolygonNode> baseNode;
	/** Reference to button label */
	std::shared_ptr<cugl::Label> label;
	/** Reference to special resolve animation node */
	std::shared_ptr<SparkleNode> sparkleNode;

#pragma region State Methods
	bool isActive() override;
	void prePosition() override;
	void postPosition() override;
	void becomeInactive() override;
#pragma endregion

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
	ButtonNode() = default;

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	virtual ~ButtonNode() { dispose(); }

	/**
	 * Properly initialize this button node. Do NOT use the constructors in the parent class. They
	 * will not initialize everything.
	 *
	 * @param btn		Pointer to the button model
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param texture   The texture image to use
	 * @param rows      The number of rows in the filmstrip
	 * @param cols      The number of columns in the filmstrip
	 */
	virtual bool init(std::shared_ptr<ButtonModel> btn, std::shared_ptr<DonutModel> player,
					  float shipSize, const std::shared_ptr<cugl::AssetManager> &assets,
					  std::shared_ptr<SparkleNode> sparkleNode);

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
	static std::shared_ptr<ButtonNode> alloc(std::shared_ptr<ButtonModel> btn,
											 std::shared_ptr<DonutModel> player, float shipSize,
											 const std::shared_ptr<cugl::AssetManager> &assets,
											 std::shared_ptr<SparkleNode> sparkleNode) {
		const std::shared_ptr<ButtonNode> node = std::make_shared<ButtonNode>();
		return (
			node->init(std::move(btn), std::move(player), shipSize, assets, std::move(sparkleNode))
				? node
				: nullptr);
	}

	/**
	 * Resets animation.
	 */
	void resetAnimation() {
		baseNode->setTexture(btnBaseUp);
		bodyNode->setTexture(btnUp);
		bodyNode->setPositionY(0);
	}

#pragma mark -

	std::shared_ptr<ButtonModel> getModel() { return buttonModel; }

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_BUTTONNODE_H
