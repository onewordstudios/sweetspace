#include "ExternalDonutNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS_OFFSET = 30;

/** Position to place node offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

void ExternalDonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
							 Color4 tint) {
	if (!donutModel->getIsActive()) return;
	const float jump = 1.0f - donutModel->getJumpOffset();
	float vel = donutModel->getVelocity();
	float radiusRatio = (globals::RADIUS + RADIUS_OFFSET) / (bodyNode->getWidth() / 2);
	Vec2 donutPos;
	if (donutModel->getIsActive()) {
		// Donut is currently active
		float onScreenAngle = getOnScreenAngle(donutModel->getAngle());
		if (isComingIntoView(onScreenAngle)) {
			// Donut is coming into visible range
			isShown = true;
		} else if (isGoingOutOfView(onScreenAngle)) {
			// Donut is leaving visible range
			donutPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(donutPos);
			isShown = false;
		}
		if (isShown) {
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
            donutPos = getPositionVec(relativeAngle, jump * (globals::RADIUS + RADIUS_OFFSET));
			setPosition(donutPos);

			float angle = rotationNode->getAngle() - vel * globals::PI_180 * radiusRatio;
			rotationNode->setAngle(angle);
			animateJumping();
		}
	} else {
		// Donut is currently inactive
		donutPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(donutPos);
		isShown = false;
	}
	Node::draw(batch, transform, tint);
}
