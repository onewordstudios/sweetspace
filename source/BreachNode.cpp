#include "BreachNode.h"

#include <cugl/2d/CUPolygonNode.h>

#include "GameGraphRoot.h"

using namespace cugl;

/** The radius of the ship. */
constexpr float RADIUS = 550;

/** Pi over 180 for converting between degrees and radians */
constexpr float PI_180 = (float)(M_PI / 180);

/** Position to place BreachNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** How much to scale down the size of the breach */
constexpr float BREACH_SCALE = 3.0f;

void BreachNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	if (breachModel->getHealth() > 0) {
		Vec2 breachPos = Vec2(RADIUS * sin(breachModel->getAngle() * PI_180),
							  -RADIUS * cos(breachModel->getAngle() * PI_180));
		if (breachModel->getAngle() < 0) {
			breachPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		}
		setScale(GameGraphRoot::BREACH_SCALE * (float)breachModel->getHealth() / BREACH_SCALE);
		setPosition(breachPos);
	} else {
		Vec2 breachPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(breachPos);
	}
	AnimationNode::draw(batch, transform, tint);
}
