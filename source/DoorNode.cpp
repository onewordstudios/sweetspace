#include "DoorNode.h"

#include <cugl/2d/CUAnimationNode.h>

using namespace cugl;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius used for placement of the doors. */
constexpr float DOOR_RADIUS = 650;

/** Position to place DoorNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** The height of the door. */
int height = 0;

void DoorNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					Color4 tint) {
	Vec2 doorPos = Vec2(DOOR_RADIUS * sin(doorModel->getAngle()),
						- (DOOR_RADIUS)*cos(doorModel->getAngle()));
	if (doorModel->getAngle() < 0) {
		doorPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
	}
	setPosition(doorPos);
	setAngle(doorModel->getAngle());
	doorModel->getPlayersOn() < 2 ? setFrame(doorModel->getPlayersOn()) : setFrame(2);
	float diff = height - doorModel->getHeight();
	height = doorModel->getHeight();
	if (diff != 0) {
		shiftPolygon(0, diff);
	}
	AnimationNode::draw(batch, transform, tint);
}
