#include "HealthNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius of the ship. */
constexpr unsigned int RADIUS = 550;

/** Angle between each section of the ship. */
constexpr float ANGLE = 45;

/** Maximum health of the ship. */
constexpr unsigned int MAX_HEALTH = 11;

/** An offset in the x direction for health bar display. */
constexpr float X_OFFSET_1 = 147;
/** An offset in the x direction for health bar display. */
constexpr float X_OFFSET_2 = 202;
/** An offset in the x direction for health bar display. */
constexpr float X_OFFSET_3 = 145;

/** An offset in the y direction for health bar display. */
constexpr float Y_OFFSET_1 = -205;
/** An offset in the y direction for health bar display. */
constexpr float Y_OFFSET_2 = -142;
/** An offset in the y direction for health bar display. */
constexpr float Y_OFFSET_3 = (RADIUS - 410);
/** An offset in the y direction for health bar display. */
constexpr float Y_OFFSET_4 = 202;

void HealthNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	switch (section) {
		case 0:
			setPosition(static_cast<float>(RADIUS * sin(0)),
						static_cast<float>(Y_OFFSET_1 * cos(0)));
			break;
		case 1:
			setPosition(static_cast<float>(X_OFFSET_1 + RADIUS * sin(0)),
						static_cast<float>(Y_OFFSET_2 * cos(0)));
			break;
		case 2:
			setPosition(static_cast<float>(X_OFFSET_2 + RADIUS * sin(0)),
						static_cast<float>(cos(0)));
			break;
		case 3:
			setPosition(static_cast<float>(X_OFFSET_3 + RADIUS * sin(0)),
						static_cast<float>(Y_OFFSET_3 * cos(0)));
			break;
		case 4:
			setPosition(static_cast<float>(RADIUS * sin(0)),
						static_cast<float>(Y_OFFSET_4 * cos(0)));
			break;
		case 5: // NOLINT
			setPosition(static_cast<float>(-X_OFFSET_3 + RADIUS * sin(0)),
						static_cast<float>(Y_OFFSET_3 * cos(0)));
			break;
		case 6: // NOLINT
			setPosition(static_cast<float>(-X_OFFSET_2 + RADIUS * sin(0)),
						static_cast<float>(cos(0)));
			break;
		default:
			setPosition(static_cast<float>(-X_OFFSET_1 + RADIUS * sin(0)),
						static_cast<float>(Y_OFFSET_2 * cos(0)));
			break;
	}

	setAngle(ANGLE * static_cast<float>(section) * globals::PI_180);
	ship->getHealth() > MAX_HEALTH ? setFrame(MAX_HEALTH) : setFrame(static_cast<int>(ship->getHealth()));

	AnimationNode::draw(batch, transform, tint);
}
