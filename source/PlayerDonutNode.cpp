#include "PlayerDonutNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

bool PlayerDonutNode::init(std::shared_ptr<DonutModel> player, float screenHeight,
						   const std::shared_ptr<cugl::Texture>& bodyTexture,
						   const std::shared_ptr<cugl::Texture>& faceIdleTexture,
						   const std::shared_ptr<cugl::Texture>& faceDizzyTexture,
						   const std::shared_ptr<cugl::Texture>& faceWorkTexture,
						   const Vec2& position) {
	this->screenHeight = screenHeight;

	setAnchor(Vec2::ANCHOR_CENTER);
	setPosition(position);
	// donutNode->setShipSize(ship->getSize());
	initPos = position;

	DonutNode::init(bodyTexture, faceIdleTexture, faceDizzyTexture, faceWorkTexture, player);
	CustomNode::init(player, 0, 0, 0);
	return true;
}

void PlayerDonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
						   Color4 tint) {
	float angle = (float)(rotationNode->getAngle() -
						  playerDonutModel->getVelocity() * globals::PI_180 * globals::SPIN_RATIO);
	setAnchor(Vec2::ANCHOR_CENTER);
	rotationNode->setAngle(angle);
	// Draw Jump Offset
	float donutNewY = initPos.y + playerDonutModel->getJumpOffset() * screenHeight;
	setPositionY(donutNewY);
	animateJumping();
	Node::draw(batch, transform, tint);
}
