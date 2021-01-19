#include "PlayerDonutNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

bool PlayerDonutNode::init(const std::shared_ptr<DonutModel>& player, float screenHeight,
						   const std::shared_ptr<cugl::AssetManager>& assets,
						   const std::string& color, const Vec2& position) {
	this->screenHeight = screenHeight;

	setAnchor(Vec2::ANCHOR_CENTER);
	setPosition(position);
	initPos = position;

	std::shared_ptr<Texture> faceIdle = assets->get<Texture>("donut_face_idle");
	std::shared_ptr<Texture> faceDizzy = assets->get<Texture>("donut_face_dizzy");
	std::shared_ptr<Texture> faceWork = assets->get<Texture>("donut_face_work");
	std::shared_ptr<Texture> bodyTexture = assets->get<Texture>("donut_" + color);

	DonutNode::init(bodyTexture, faceIdle, faceDizzy, faceWork, player);
	CustomNode::init(player, 0, 0, 0);
	return true;
}

void PlayerDonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
						   Color4 tint) {
	auto angle = (rotationNode->getAngle() -
				  playerDonutModel->getVelocity() * globals::PI_180 * globals::SPIN_RATIO);
	setAnchor(Vec2::ANCHOR_CENTER);
	rotationNode->setAngle(angle);
	// Draw Jump Offset
	float donutNewY = initPos.y + playerDonutModel->getJumpOffset() * screenHeight;
	setPositionY(donutNewY);
	animateJumping();
	animateFacialExpression();
	// NOLINTNEXTLINE Intentional skip of CustomNode
	Node::draw(batch, transform, tint);
}
