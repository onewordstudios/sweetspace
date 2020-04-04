#include "DonutNode.h"

#include <cugl/2d/CUPolygonNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS_OFFSET = 30;

/** Position to place node offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

void DonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					 Color4 tint) {
	const float jump = 1.0f - donutModel->getJumpOffset();
	float vel = donutModel->getVelocity();
	float radiusRatio = (globals::RADIUS + RADIUS_OFFSET) / (getWidth() / 2);
	Vec2 donutPos;
	if (donutModel->getAngle() >= 0) {
		// Donut is currently active
		float onScreenAngle = donutModel->getAngle() - playerDonutModel->getAngle();
		onScreenAngle = onScreenAngle < 0 ? shipSize + onScreenAngle : onScreenAngle;
		onScreenAngle = onScreenAngle > shipSize / 2 ? onScreenAngle - shipSize : onScreenAngle;
		onScreenAngle *= globals::PI_180;
		if (!isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
			onScreenAngle > -globals::SEG_CUTOFF_ANGLE) {
			// Donut is coming into visible range
			isShown = true;
		} else if (isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
							   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE)) {
			// Donut is leaving visible range
			donutPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(donutPos);
			isShown = false;
		}
		if (isShown) {
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
			donutPos = Vec2(jump * (globals::RADIUS + RADIUS_OFFSET) * sin(relativeAngle),
							-jump * (globals::RADIUS + RADIUS_OFFSET) * cos(relativeAngle));
			setPosition(donutPos);

			float angle = getAngle() - vel * globals::PI_180 * radiusRatio;
			setAngle(angle);
		}
	} else {
		// Donut is currently inactive
		donutPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(donutPos);
		isShown = false;
	}
	PolygonNode::draw(batch, transform, tint);
}
