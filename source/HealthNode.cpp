#include "HealthNode.h"

#include <cugl/2d/CUAnimationNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius of the ship. */
constexpr unsigned int RADIUS = 550;

const Vec2 pos0 = Vec2(RADIUS * sin(0), -205 * cos(0));

const Vec2 pos1 = Vec2(147 + RADIUS * sin(0), -142 * cos(0));

const Vec2 pos2 = Vec2(202 + RADIUS * sin(0), cos(0));

const Vec2 pos3 = Vec2(145 + RADIUS * sin(0), (RADIUS - 410) * cos(0));

const Vec2 pos4 = Vec2(RADIUS * sin(0), 202 * cos(0));

const Vec2 pos5 = Vec2(-145 + RADIUS * sin(0), (RADIUS - 410) * cos(0));

const Vec2 pos6 = Vec2(-202 + RADIUS * sin(0), cos(0));

const Vec2 pos7 = Vec2(-147 + RADIUS * sin(0), -142 * cos(0));

void HealthNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	switch (section) {
		case 0:
			setPosition(pos0);
			break;
		case 1:
			setPosition(pos1);
			break;
		case 2:
			setPosition(pos2);
			break;
		case 3:
			setPosition(pos3);
			break;
		case 4:
			setPosition(pos4);
			break;
		case 5:
			setPosition(pos5);
			break;
		case 6:
			setPosition(pos6);
			break;
		case 7:
			setPosition(pos7);
			break;
	}

	setAngle((float)(M_PI * 45 * section) / 180.0f);
	ship->getHealth() > 11 ? setFrame(11) : setFrame(ship->getHealth());

	AnimationNode::draw(batch, transform, tint);
}
