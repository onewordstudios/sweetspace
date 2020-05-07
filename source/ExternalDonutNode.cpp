#include "ExternalDonutNode.h"

#include <cugl/2d/CUNode.h>

#include "GameGraphRoot.h"
#include "Globals.h"

using namespace cugl;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS_OFFSET = 30;

bool ExternalDonutNode::init(std::shared_ptr<DonutModel> externalDonutModel,
							 std::shared_ptr<DonutModel> player, float shipSize,
							 const std::shared_ptr<cugl::Texture>& bodyTexture) {
	CustomNode::init(player, shipSize, externalDonutModel->getAngle(), globals::RADIUS);
	DonutNode::init(bodyTexture, externalDonutModel);
	return true;
}

bool ExternalDonutNode::isActive() { return donutModel->getIsActive(); }

void ExternalDonutNode::prePosition() {
	const float jump = 1.0f - donutModel->getJumpOffset();
	radius = jump * (globals::RADIUS + RADIUS_OFFSET);
	angle = donutModel->getAngle();
}

void ExternalDonutNode::postPosition() {
	if (isShown) {
		float vel = donutModel->getVelocity();
		float angle = rotationNode->getAngle() - vel * globals::PI_180 * globals::SPIN_RATIO;
		rotationNode->setAngle(angle);
		animateJumping();
	}
}
