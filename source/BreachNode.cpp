#include "BreachNode.h"

#include <cugl/2d/CUPolygonNode.h>

#include "GameGraphRoot.h"
#include "Globals.h"

using namespace cugl;

/** Position to place BreachNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** How much to scale down the size of the breach */
constexpr float BREACH_SCALE = 3.0f;

void BreachNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	Vec2 breachPos;
	if (breachModel->getHealth() > 0) {
		// Breach is currently active
		float absDiff =
			(shipSize / 2 -
			 abs(abs(playerDonutModel->getAngle() - breachModel->getAngle()) - shipSize / 2)) *
			globals::PI_180;
		float onScreenAngle =
			absDiff * (breachModel->getAngle() >=
								   fmod(playerDonutModel->getAngle() + shipSize / 2, shipSize) ||
							   breachModel->getAngle() < playerDonutModel->getAngle()
						   ? -1
						   : 1);
		if (!isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
			onScreenAngle > -globals::SEG_CUTOFF_ANGLE) {
			// Breach is coming into visible range
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
			breachPos = Vec2(globals::RADIUS * sin(relativeAngle),
							 -(float)globals::RADIUS * cos(relativeAngle));
			setPosition(breachPos);
			isShown = true;
			CULog("Coming into view at %f", onScreenAngle / globals::PI_180);
		} else if (isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
							   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE)) {
			// Breach is leaving visible range
			breachPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(breachPos);
			isShown = false;
			CULog("Going out at %f", onScreenAngle / globals::PI_180);
		}
		setScale(GameGraphRoot::BREACH_SCALE * (float)breachModel->getHealth() / BREACH_SCALE);
	} else {
		// Breach is currently inactive
		breachPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(breachPos);
		isShown = false;
	}
	AnimationNode::draw(batch, transform, tint);
}
