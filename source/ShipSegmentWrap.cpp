#include "ShipSegmentWrap.h"

#include "Globals.h"
#include "ShipSegmentNode.h"

ShipSegmentWrap::ShipSegmentWrap() : leftMostSeg(0), rightMostSeg(0) {}

ShipSegmentWrap::~ShipSegmentWrap() {
	removeAllChildren();
	// More
}

bool ShipSegmentWrap::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	cugl::Node::init();
	setAnchor({0, 0});
	setPosition(0, 0);
	setZOrder(-1);
	_zDirty = true;

	leftMostSeg = 0;
	rightMostSeg = globals::VISIBLE_SEGS - 1;
	for (unsigned int i = 0; i < globals::VISIBLE_SEGS; i++) {
		auto segment = ShipSegmentNode::alloc(assets, i);
		addChildWithTag(segment, i + 1);
	}

	doLayout();

	return true;
}

float wrapAngle(float f) { return globals::remainderPos(f, globals::TWO_PI); };

void ShipSegmentWrap::updateSegments(float nearSpaceAngle, float shipSize, float playerAngle) {
	for (unsigned int i = 0; i < globals::VISIBLE_SEGS; i++) {
		auto segment = getChildByTag<ShipSegmentNode>(i + 1);

		// If segments rotate too far left, move left-most segment to the right side
		if (i == rightMostSeg &&
			wrapAngle(nearSpaceAngle + segment->getAngle()) < globals::SEG_CUTOFF_ANGLE) {
			rightMostSeg = (i + 1) % globals::VISIBLE_SEGS;
			leftMostSeg = (i + 2) % globals::VISIBLE_SEGS;
			auto newRightSegment = getChildByTag(rightMostSeg + 1);
			newRightSegment->setAngle(wrapAngle(segment->getAngle() + globals::SEG_SIZE));
		} else if (i == leftMostSeg && wrapAngle(nearSpaceAngle + segment->getAngle()) >
										   globals::TWO_PI - globals::SEG_CUTOFF_ANGLE) {
			leftMostSeg = (i + globals::VISIBLE_SEGS - 1) % globals::VISIBLE_SEGS;
			rightMostSeg = (i + globals::VISIBLE_SEGS - 2) % globals::VISIBLE_SEGS;
			auto newLeftSegment = getChildByTag(leftMostSeg + 1);
			newLeftSegment->setAngle(wrapAngle(segment->getAngle() - globals::SEG_SIZE));
		}

		// Update text label of segment
		segment->updateLabel(nearSpaceAngle, shipSize, playerAngle);
	}
}
