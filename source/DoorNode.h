#ifndef SWEETSPACE_DOORNODE_H
#define SWEETSPACE_DOORNODE_H

#include <cugl/2d/CUAnimationNode.h>

#include "CustomNode.h"
#include "DonutModel.h"
#include "DoorModel.h"
#include "ShipModel.h"

class DoorNode : public CustomNode {
#pragma mark Values
   protected:
	/** Reference to the DoorModel of this node */
	std::shared_ptr<DoorModel> doorModel;
	/** Reference to the AnimationNode child of this node */
	std::shared_ptr<cugl::AnimationNode> animationNode;

	/** The height of the door. */
	unsigned int height;
	/** The max frame this door can have. */
	unsigned int frameCap;

#pragma region State Methods
	bool isActive() override { return doorModel->getIsActive(); }
	void prePosition() override;
	void postPosition() override;
#pragma endregion

   public:
#pragma mark -
#pragma mark Constructor
	/**
	 * Creates an empty door node with degenerate texture.
	 *
	 * You must initialize this PolygonNode before use.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
	 * the heap, use one of the static constructors instead.
	 */
	DoorNode() : height(0), frameCap(0) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~DoorNode() { dispose(); }

	/**
	 * Properly initialize this door node. Do NOT use the constructors in the parent class. They
	 * will not initialize everything.
	 *
	 * @param door		Pointer to the door model
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param texture   The texture image to use
	 * @param rows      The number of rows in the filmstrip
	 * @param cols      The number of columns in the filmstrip
	 */
	virtual bool init(const std::shared_ptr<DoorModel> &door, std::shared_ptr<DonutModel> player,
					  float shipSize, const std::shared_ptr<cugl::AssetManager> &assets);

	/**
	 * Returns a newly allocated filmstrip node from the given texture.
	 *
	 * @param door		Pointer to the door model
	 * @param player	Pointer to the player's donut model
	 * @param shipSize	Size of the ship (in degrees)
	 * @param texture   The texture image to use
	 * @param rows      The number of rows in the filmstrip
	 * @param cols      The number of columns in the filmstrip
	 *
	 * @return a newly allocated door node.
	 */
	static std::shared_ptr<DoorNode> alloc(const std::shared_ptr<DoorModel> &door,
										   std::shared_ptr<DonutModel> player, float shipSize,
										   const std::shared_ptr<cugl::AssetManager> &assets) {
		std::shared_ptr<DoorNode> node = std::make_shared<DoorNode>();
		return node->init(door, std::move(player), shipSize, assets) ? node : nullptr;
	}

#pragma mark -

	std::shared_ptr<DoorModel> getModel() { return doorModel; }

	std::shared_ptr<cugl::AnimationNode> getAnimationNode() { return animationNode; }
};

#endif // SWEETSPACE_DOORNODE_H
