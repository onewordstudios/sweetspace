#ifndef SWEETSPACE_EXTERNALDONUTNODE_H
#define SWEETSPACE_EXTERNALDONUTNODE_H

#include <cugl/2d/CUPolygonNode.h>

#include "DonutModel.h"
#include "DonutNode.h"

class ExternalDonutNode : public DonutNode {
   protected:
	bool isActive() override;
	void prePosition() override;
	void postPosition() override;

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

	bool init(const std::shared_ptr<DonutModel>& externalDonutModel, std::shared_ptr<DonutModel> player,
			  float shipSize, const std::shared_ptr<cugl::Texture> &bodyTexture,
			  const std::shared_ptr<cugl::Texture> &faceIdleTexture,
			  const std::shared_ptr<cugl::Texture> &faceDizzyTexture,
			  const std::shared_ptr<cugl::Texture> &faceWorkTexture);

	static std::shared_ptr<ExternalDonutNode> alloc(
		std::shared_ptr<DonutModel> externalDonutModel, std::shared_ptr<DonutModel> player,
		float shipSize, const std::shared_ptr<cugl::Texture> &bodyTexture,
		const std::shared_ptr<cugl::Texture> &faceIdleTexture,
		const std::shared_ptr<cugl::Texture> &faceDizzyTexture,
		const std::shared_ptr<cugl::Texture> &faceWorkTexture) {
		std::shared_ptr<ExternalDonutNode> node = std::make_shared<ExternalDonutNode>();
		return node->init(externalDonutModel, player, shipSize, bodyTexture, faceIdleTexture,
						  faceDizzyTexture, faceWorkTexture)
				   ? node
				   : nullptr;
	}

#pragma mark -
};

#endif // SWEETSPACE_EXTERNALDONUTNODE_H
