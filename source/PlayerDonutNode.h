#ifndef SWEETSPACE_PLAYERDONUTNODE_H
#define SWEETSPACE_PLAYERDONUTNODE_H

#include <cugl/2d/CUPolygonNode.h>

#include "DonutModel.h"
#include "DonutNode.h"

class PlayerDonutNode : public DonutNode {
#pragma mark Values
   protected:
	/** Initial position of player donut node */
	cugl::Vec2 initPos;
	/** The height of the game screen */
	float screenHeight;

	bool isActive() override { return false; }

   public:
#pragma mark -
#pragma mark Constructor
	/**
	 * Creates an node.
	 */
	PlayerDonutNode() : screenHeight(0) {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~PlayerDonutNode() { dispose(); }

	bool init(const std::shared_ptr<DonutModel> &player, float screenHeight,
			  const std::shared_ptr<cugl::AssetManager> &assets, const std::string &color,
			  const cugl::Vec2 &position);

	static std::shared_ptr<PlayerDonutNode> alloc(const std::shared_ptr<DonutModel> &player,
												  float screenHeight,
												  const std::shared_ptr<cugl::AssetManager> &assets,
												  const std::string &color,
												  const cugl::Vec2 &position) {
		std::shared_ptr<PlayerDonutNode> node = std::make_shared<PlayerDonutNode>();
		return node->init(player, screenHeight, assets, color, position) ? node : nullptr;
	}

#pragma mark -
	void setInitPos(cugl::Vec2 vec) { initPos = vec; }

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_PLAYERDONUTNODE_H
