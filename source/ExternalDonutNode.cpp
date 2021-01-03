#include "ExternalDonutNode.h"

#include <cugl/2d/CUNode.h>

#include <utility>

#include "GameGraphRoot.h"
#include "Globals.h"

using namespace cugl;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS_OFFSET = 30;

bool ExternalDonutNode::init(const std::shared_ptr<DonutModel>& externalDonutModel,
							 std::shared_ptr<DonutModel> player, float shipSize,
							 const std::shared_ptr<cugl::Texture> &bodyTexture,
							 const std::shared_ptr<cugl::Texture> &faceIdleTexture,
							 const std::shared_ptr<cugl::Texture> &faceDizzyTexture,
							 const std::shared_ptr<cugl::Texture> &faceWorkTexture) {
	CustomNode::init(std::move(player), shipSize, externalDonutModel->getAngle(), globals::RADIUS);
	DonutNode::init(bodyTexture, faceIdleTexture, faceDizzyTexture, faceWorkTexture,
					externalDonutModel);
	return true;
}

bool ExternalDonutNode::isActive() { return referencedDonutModel->getIsActive(); }

void ExternalDonutNode::prePosition() {
	const float jump = 1.0f - referencedDonutModel->getJumpOffset();
	radius = jump * (globals::RADIUS + RADIUS_OFFSET);
	angle = referencedDonutModel->getAngle();
	isDirty = true;
}

void ExternalDonutNode::postPosition() {
	if (isShown) {
		float vel = referencedDonutModel->getVelocity();
		float angle = rotationNode->getAngle() - vel * globals::PI_180 * globals::SPIN_RATIO;
		rotationNode->setAngle(angle);
		animateJumping();
	}
}
