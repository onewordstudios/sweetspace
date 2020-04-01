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
	if (breachModel->getHealth() > 0) {
		Vec2 breachPos =
			Vec2(globals::RADIUS * sin(breachModel->getAngle() * globals::PI_180),
				 -(float)globals::RADIUS * cos(breachModel->getAngle() * globals::PI_180));
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
