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

   public:
#pragma mark -
#pragma mark Constructor
	/**
	 * Creates an node.
	 */
	PlayerDonutNode() : DonutNode() {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~PlayerDonutNode() { dispose(); }

	/**
	 * Returns an PlayerDonutNode
	 *
	 * @param bodyTexture   A shared pointer to the body Texture object.
	 *
	 * @return a textured polygon from a Texture object.
	 */
	static std::shared_ptr<PlayerDonutNode> allocWithTextures(
		const std::shared_ptr<cugl::Texture> &bodyTexture) {
		std::shared_ptr<PlayerDonutNode> node = std::make_shared<PlayerDonutNode>();
		if (node->init()) {
			node->initChildren(bodyTexture);
			return node;
		} else {
			return nullptr;
		}
	}

#pragma mark -
	void setInitPos(cugl::Vec2 vec) { initPos = vec; }

	void setScreenHeight(float h) { screenHeight = h; }

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_PLAYERDONUTNODE_H
