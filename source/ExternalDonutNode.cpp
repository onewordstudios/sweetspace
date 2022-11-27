#include "ExternalDonutNode.h"

#include <cugl/2d/CUNode.h>

#include <utility>

#include "GameGraphRoot.h"
#include "Globals.h"

using namespace cugl;

/** Offset of donut sprites from the radius of the ship */
constexpr int DONUT_OFFSET = 195;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS_OFFSET = 30;

bool ExternalDonutNode::init(const std::shared_ptr<DonutModel> &externalDonutModel,
							 std::shared_ptr<DonutModel> player, float shipSize,
							 const std::shared_ptr<cugl::AssetManager> &assets,
							 const std::string &color) {
	CustomNode::init(std::move(player), shipSize, externalDonutModel->getAngle(), globals::RADIUS);

	auto faceIdle = assets->get<Texture>("donut_face_idle");
	auto faceDizzy = assets->get<Texture>("donut_face_dizzy");
	auto faceWork = assets->get<Texture>("donut_face_work");
	auto bodyTexture = assets->get<Texture>("donut_" + color);

	DonutNode::init(bodyTexture, faceIdle, faceDizzy, faceWork, externalDonutModel);

	const Vec2 donutPos =
		Vec2(sin(externalDonutModel->getAngle() * (globals::RADIUS + DONUT_OFFSET)),
			 -cos(externalDonutModel->getAngle()) * (globals::RADIUS + DONUT_OFFSET));
	setPosition(donutPos);

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
		const float vel = referencedDonutModel->getVelocity();
		const float angle = rotationNode->getAngle() - vel * globals::PI_180 * globals::SPIN_RATIO;
		rotationNode->setAngle(angle);
		animateJumping();
	}
}
