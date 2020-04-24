#include "PlayerDonutNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

void PlayerDonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
						   Color4 tint) {
	double radiusRatio = (double)globals::RADIUS / (bodyNode->getWidth() / 2);

	float angle =
		(float)(getAngle() - playerDonutModel->getVelocity() * globals::PI_180 * radiusRatio);
	setAnchor(Vec2::ANCHOR_CENTER);
	setAngle(angle);
	// Draw Jump Offset
	float donutNewY = initPos.y + playerDonutModel->getJumpOffset() * screenHeight;
	setPositionY(donutNewY);

	Node::draw(batch, transform, tint);
}
