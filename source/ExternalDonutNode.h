#ifndef SWEETSPACE_EXTERNALDONUTNODE_H
#define SWEETSPACE_EXTERNALDONUTNODE_H

#include <cugl/2d/CUPolygonNode.h>

#include "DonutModel.h"
#include "DonutNode.h"

class ExternalDonutNode : public DonutNode {
   public:
#pragma mark -
#pragma mark Constructor
	/**
	 * Creates an node.
	 */
	ExternalDonutNode() : DonutNode() {}

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~ExternalDonutNode() { dispose(); }

	/**
	 * Returns an ExternalDonutNode
	 *
	 * @param bodyTexture   A shared pointer to the body Texture object.
	 *
	 * @return a textured polygon from a Texture object.
	 */
	static std::shared_ptr<ExternalDonutNode> allocWithTextures(
		const std::shared_ptr<cugl::Texture> &bodyTexture) {
		std::shared_ptr<ExternalDonutNode> node = std::make_shared<ExternalDonutNode>();
		if (node->init()) {
			node->initChildren(bodyTexture);
			return node;
		} else {
			return nullptr;
		}
	}

#pragma mark -
	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;
};

#endif // SWEETSPACE_EXTERNALDONUTNODE_H
