#include "ShipSegmentNode.h"

#include "Globals.h"

/** Size of ship segment label */
constexpr int SEG_LABEL_SIZE = 100;

/** Y position of ship segment label */
constexpr int SEG_LABEL_Y = 1113;

/** Color of the text label */
static const cugl::Color4 SHIP_LABEL_COLOR{255, 248, 161}; // NOLINT

ShipSegmentNode::~ShipSegmentNode() { removeAllChildren(); }

bool ShipSegmentNode::init(const std::shared_ptr<cugl::AssetManager>& assets,
						   unsigned int segment) {
	auto seg0 = assets->get<cugl::Texture>("shipseg0");
	auto seg1 = assets->get<cugl::Texture>("shipseg1");
	auto segRed = assets->get<cugl::Texture>("shipsegred");

	if (!cugl::PolygonNode::initWithTexture(segment % 2 == 0 ? seg0 : seg1)) {
		return false;
	}
	setAnchor(cugl::Vec2::ANCHOR_TOP_CENTER);
	setScale(SEG_SCALE);
	setPosition(cugl::Vec2(0, 0));
	setAngle(globals::SEG_SIZE * (static_cast<float>(segment) - 2));

	segLabel = cugl::Label::alloc(cugl::Size(SEG_LABEL_SIZE, SEG_LABEL_SIZE),
								  assets->get<cugl::Font>("mont_black_italic_big"));
	segLabel->setAnchor(cugl::Vec2::ANCHOR_CENTER);
	segLabel->setHorizontalAlignment(cugl::Label::HAlign::CENTER);
	segLabel->setPosition(static_cast<float>(getTexture()->getWidth()) / 2, SEG_LABEL_Y);
	segLabel->setForeground(SHIP_LABEL_COLOR);
	addChild(segLabel);

	const std::shared_ptr<PolygonNode> segmentRed = cugl::PolygonNode::allocWithTexture(segRed);
	segmentRed->setColor(cugl::Color4::CLEAR);
	addChild(segmentRed);

	return true;
}

void ShipSegmentNode::updateLabel(float nearSpaceAngle, float shipSize, float playerAngle) {
	float relSegAngle = globals::remainderPos(getAngle() + nearSpaceAngle, globals::TWO_PI);
	relSegAngle = relSegAngle > globals::PI ? relSegAngle - globals::TWO_PI : relSegAngle;
	auto segAngle = (playerAngle * globals::PI_180 + relSegAngle +
					 ShipSegmentNode::SEG_SCALE * globals::PI_180);
	segAngle = fmod(segAngle, shipSize * globals::PI_180);
	segAngle = segAngle < 0 ? segAngle + shipSize * globals::PI_180 : segAngle;

	auto segNum = static_cast<unsigned int>(segAngle / globals::SEG_SIZE);
	const std::string segText = std::to_string(segNum);
	if (segLabel->getText() != segText) {
		segLabel->setText(segText);
	}
}
