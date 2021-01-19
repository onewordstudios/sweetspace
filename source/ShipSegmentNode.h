#ifndef SHIP_SEGMENT_NODE_H
#define SHIP_SEGMENT_NODE_H

#include <cugl/cugl.h>

class ShipSegmentNode : public cugl::PolygonNode {
   public:
	/** The scale of the ship segments. */
	static constexpr float SEG_SCALE = 0.33f;

	/** Construct a degenerate segment node */
	ShipSegmentNode() = default;
	/** Destruct this segment node */
	~ShipSegmentNode();

	ShipSegmentNode(ShipSegmentNode const&) = delete;
	void operator=(ShipSegmentNode const&) = delete;

	/**
	 * Initialize this ship segment node with proper textures and assets
	 *
	 * @param assets Asset manager to pull assets from
	 * @param segment Current segment ID
	 *
	 * @return True iff initialization was successful
	 */
	bool init(const std::shared_ptr<cugl::AssetManager>& assets, unsigned int segment);

	/**
	 * Allocate a new ship segment node
	 */
	static std::shared_ptr<cugl::PolygonNode> alloc(
		const std::shared_ptr<cugl::AssetManager>& assets, unsigned int segmentID) {
		std::shared_ptr<ShipSegmentNode> node = std::make_shared<ShipSegmentNode>();
		return node->init(assets, segmentID) ? node : nullptr;
	}
};

#endif // SHIP_SEGMENT_NODE_H
