#ifndef SWEETSPACE_UNOPENABLE_NODE_H
#define SWEETSPACE_UNOPENABLE_NODE_H

#include <cugl/2d/CUAnimationNode.h>

#include "CustomNode.h"
#include "DonutModel.h"
#include "ShipModel.h"
#include "Unopenable.h"

class UnopenableNode : public CustomNode {
#pragma mark Values
   protected:
	/** Reference to the UnopenableModel of this node */
	std::shared_ptr<Unopenable> unopModel;

#pragma region State Methods
	bool isActive() override { return unopModel->getIsActive(); }
	void prePosition() override;
#pragma endregion

   public:
#pragma mark -
#pragma mark Constructor
	/**
	 * Creates an empty Unopenable node with degenerate texture.
	 *
	 * You must initialize this PolygonNode before use.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
	 * the heap, use one of the static constructors instead.
	 */
	UnopenableNode() {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~UnopenableNode() { dispose(); }

	/**
	 * Properly initialize this Unopenable node. Do NOT use the constructors in the parent class.
	 * They will not initialize everything.
	 *
	 * @param Unopenable		Pointer to the Unopenable model
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param texture   The texture image to use
	 * @param rows      The number of rows in the filmstrip
	 * @param cols      The number of columns in the filmstrip
	 */
	virtual bool init(const std::shared_ptr<Unopenable> &unop, std::shared_ptr<DonutModel> player,
					  float shipSize, const std::shared_ptr<cugl::Texture> &texture);

	/**
	 * Returns a newly allocated filmstrip node from the given texture.
	 *
	 * @param Unopenable		Pointer to the Unopenable model
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param texture   The texture image to use
	 *
	 * @return a newly allocated Unopenable node.
	 */
	static std::shared_ptr<UnopenableNode> alloc(const std::shared_ptr<Unopenable> &unop,
												 std::shared_ptr<DonutModel> player, float shipSize,
												 const std::shared_ptr<cugl::Texture> &texture) {
		std::shared_ptr<UnopenableNode> node = std::make_shared<UnopenableNode>();
		return node->init(unop, std::move(player), shipSize, texture) ? node : nullptr;
	}

#pragma mark -

	std::shared_ptr<Unopenable> getModel() { return unopModel; }
};

#endif // SWEETSPACE_UnopenableNODE_H
