#include "CustomNode.h"

using namespace cugl;

/** Position to hide stuff offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

bool CustomNode::init(std::shared_ptr<DonutModel> player, float shipSize, float angle,
					  float radius) {
	playerDonutModel = player;
	this->shipSize = shipSize;
	this->angle = angle;
	this->radius = radius;
	Node::init();
	return false;
}

void CustomNode::dispose() {
	playerDonutModel = nullptr;
	Node::dispose();
}

void CustomNode::draw(const shared_ptr<cugl::SpriteBatch>& batch, const cugl::Mat4& transform,
					  cugl::Color4 tint) {
	if (isActive()) {
		// Model is currently active

		prePosition();

		float onScreenAngle = getOnScreenAngle(angle);
		if (isComingIntoView(onScreenAngle)) {
			// Entering visible range
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
			setAngle(relativeAngle);
			setPosition(getPositionVec(relativeAngle, radius));
			isShown = true;
		} else if (isGoingOutOfView(onScreenAngle)) {
			// Leaving visible range
			Vec2 hidden = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(hidden);
			isShown = false;
		}

		postPosition();

	} else {
		// Model is currently inactive
		if (isShown) {
			Vec2 hidden = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(hidden);
			isShown = false;
		}
	}

	Node::draw(batch, transform, tint);
}
