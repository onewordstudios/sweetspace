#include "DonutNode.h"

#include <cugl/2d/CUPolygonNode.h>

using namespace cugl;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS = 580;

/** Pi over 180 for converting between degrees and radians */
constexpr float PI_180 = (float)(M_PI / 180);

void DonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					 Color4 tint) {
	const float jump = 1.0f - donutModel->getJumpOffset();

	float vel = donutModel->getVelocity();
	Vec2 donutPos = Vec2(jump * RADIUS * sin(donutModel->getAngle() * PI_180),
						 -jump * RADIUS * cos(donutModel->getAngle() * PI_180));
	if (donutModel->getAngle() < 0) {
		donutPos = Vec2(0, 0);
	}
	float radiusRatio = RADIUS / (getWidth() / 2);
	float angle = getAngle() - vel * PI_180 * radiusRatio;

	setPosition(donutPos);
	setAngle(angle);
	PolygonNode::draw(batch, transform, tint);
}
