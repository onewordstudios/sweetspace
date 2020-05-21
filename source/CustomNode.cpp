#include "CustomNode.h"

using namespace cugl;

/** Position to hide stuff offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

// NOLINTNEXTLINE We'd hope C++ std containers don't throw exceptions on default construction
std::unordered_set<CustomNode*> CustomNode::allActiveNodes;

#pragma region Lifecycle

bool CustomNode::init(std::shared_ptr<DonutModel> player, float shipSize, float angle,
					  float radius) {
	playerDonutModel = player;
	this->shipSize = shipSize;
	this->angle = angle;
	this->radius = radius;
	isDirty = true;
	isShown = true;
	wasActive = false;
	Node::init();
	return false;
}

void CustomNode::dispose() {
	playerDonutModel = nullptr;
	Node::dispose();
}

#pragma endregion

#pragma region Positioning

float CustomNode::getOnScreenAngle(float modelAngle) {
	float onScreenAngle = modelAngle - playerDonutModel->getAngle();
	onScreenAngle = onScreenAngle >= 0 ? onScreenAngle : shipSize + onScreenAngle;
	onScreenAngle = onScreenAngle > shipSize / 2 ? onScreenAngle - shipSize : onScreenAngle;
	onScreenAngle *= globals::PI_180;
	return onScreenAngle;
}

bool CustomNode::isComingIntoView(float onScreenAngle) {
	return (!isShown || isDirty) && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
		   onScreenAngle > -globals::SEG_CUTOFF_ANGLE;
}

bool CustomNode::isGoingOutOfView(float onScreenAngle) {
	return isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
					   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE);
}

cugl::Vec2 CustomNode::getPositionVec(float relAngle, float radius) {
	return cugl::Vec2(radius * sin(relAngle), -radius * cos(relAngle));
}

#pragma endregion

void CustomNode::draw(const shared_ptr<cugl::SpriteBatch>& batch, const cugl::Mat4& transform,
					  cugl::Color4 tint) {
	if (isActive()) {
		// Model is currently active

		if (!wasActive) {
			becomeActive();
			wasActive = true;
			isDirty = true;
		}

		prePosition();

		float onScreenAngle = getOnScreenAngle(angle);
		if (isComingIntoView(onScreenAngle)) {
			// Entering visible range
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
			setAngle(relativeAngle);
			setPosition(getPositionVec(relativeAngle, radius));
			isShown = true;
			isDirty = false;
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
		if (wasActive) {
			becomeInactive();
			wasActive = false;
		}
	}

	Node::draw(batch, transform, tint);
}

void CustomNode::recomputeAll() {
	for (auto node : allActiveNodes) {
		node->isDirty = true;
	}
}
