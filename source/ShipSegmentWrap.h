#ifndef SHIP_SEGMENT_WRAP_H
#define SHIP_SEGMENT_WRAP_H

#include <cugl/cugl.h>

class ShipSegmentWrap : public cugl::Node {
   private:
	/** Tag of the left most ship segment */
	unsigned int leftMostSeg;
	/** Tag of the right most ship segment */
	unsigned int rightMostSeg;

   public:
	ShipSegmentWrap();
	virtual ~ShipSegmentWrap();

	ShipSegmentWrap(const ShipSegmentWrap&) = delete;
	void operator=(const ShipSegmentWrap&) = delete;

	bool init(const std::shared_ptr<cugl::AssetManager>& assets);

	static std::shared_ptr<ShipSegmentWrap> alloc(
		const std::shared_ptr<cugl::AssetManager>& assets) {
		const std::shared_ptr<ShipSegmentWrap> node = std::make_shared<ShipSegmentWrap>();
		return node->init(assets) ? node : nullptr;
	}

	void updateSegments(float nearSpaceAngle, float shipSize, float playerAngle);
};

#endif // SHIP_SEGMENT_WRAP_H
