#include "PlayerDonutNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

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
