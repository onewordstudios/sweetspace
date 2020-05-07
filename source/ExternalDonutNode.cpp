#include "ExternalDonutNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS_OFFSET = 30;

/** Position to place node offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

bool ExternalDonutNode::isActive() { return donutModel->getIsActive(); }

void ExternalDonutNode::prePosition() {
	const float jump = 1.0f - donutModel->getJumpOffset();
	radius = jump * (globals::RADIUS + RADIUS_OFFSET);
}

void ExternalDonutNode::postPosition() {
	if (isShown) {
		float vel = donutModel->getVelocity();
		float angle = rotationNode->getAngle() - vel * globals::PI_180 * globals::SPIN_RATIO;
		rotationNode->setAngle(angle);
		animateJumping();
	}
}