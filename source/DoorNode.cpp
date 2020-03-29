#include "DoorNode.h"

#include <cugl/2d/CUAnimationNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius used for placement of the doors. */
constexpr float DOOR_RADIUS = 650;

/** Position to place DoorNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** Pi over 180 for converting between degrees and radians */
constexpr float PI_180 = (float)(M_PI / 180);

/** The height of the door. */
int height = 0;

/** The max frame this door can have. */
int frameCap = 0;

void DoorNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					Color4 tint) {
	Vec2 doorPos = Vec2(DOOR_RADIUS * sin(doorModel->getAngle() * PI_180),
						-(DOOR_RADIUS)*cos(doorModel->getAngle() * PI_180));
	if (doorModel->getAngle() < 0) {
		doorPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
	}
	setPosition(doorPos);
	setAngle(doorModel->getAngle() * PI_180);

	frameCap = doorModel->getPlayersOn() < 2 ? doorModel->getPlayersOn() * 16 : 31;
	if (getFrame() < frameCap) {
		setFrame(getFrame() + 1);
	} else if (getFrame() > frameCap) {
		setFrame(getFrame() - 1);
	}

	float diff = height - doorModel->getHeight();
	height = doorModel->getHeight();
	if (diff != 0) {
		shiftPolygon(0, diff);
	}
	AnimationNode::draw(batch, transform, tint);
}
