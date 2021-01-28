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
	ExternalDonutNode() = default;

	/**
	 * Releases all resources allocated with this node.
	 *
	 * This will release, but not necessarily delete the associated texture.
	 * However, the polygon and drawing commands will be deleted and no
	 * longer safe to use.
	 */
	~ExternalDonutNode() { dispose(); }

	bool init(const std::shared_ptr<DonutModel> &externalDonutModel,
			  std::shared_ptr<DonutModel> player, float shipSize,
			  const std::shared_ptr<cugl::AssetManager> &assets, const std::string &color);

	static std::shared_ptr<ExternalDonutNode> alloc(
		const std::shared_ptr<DonutModel> &externalDonutModel, std::shared_ptr<DonutModel> player,
		float shipSize, const std::shared_ptr<cugl::AssetManager> &assets,
		const std::string &color) {
		std::shared_ptr<ExternalDonutNode> node = std::make_shared<ExternalDonutNode>();
		return node->init(externalDonutModel, std::move(player), shipSize, assets, color) ? node
																						  : nullptr;
	}

#pragma mark -
};

#endif // SWEETSPACE_EXTERNALDONUTNODE_H
