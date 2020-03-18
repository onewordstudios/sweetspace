#include "DonutNode.h"

#include <cugl/2d/CUPolygonNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS = 550;

/** Pi over 180 for converting between degrees and radians */
constexpr float PI_180 = (float)(M_PI / 180);

void DonutNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					 Color4 tint) {
	float vel = donutModel->getVelocity();

	Vec2 donutPos = Vec2(DIAMETER + RADIUS * sin(donutModel->getAngle()),
						 DIAMETER / 2.0f - RADIUS * cos(donutModel->getAngle()));
	if (donutModel->getAngle() < 0) {
		donutPos = Vec2(0, 0);
	}
	double radiusRatio = RADIUS / (getWidth() / 2.0);
	float angle = getAngle() - vel * PI_180 * radiusRatio;

	setPosition(donutPos);
	setAngle(angle);
	PolygonNode::draw(batch, transform, tint);
}
