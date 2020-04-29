#include "DoorNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius used for placement of the doors. */
constexpr float DOOR_RADIUS = 660;

/** Position to place DoorNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** The frame of the animation strip to freeze on when one player is on the door */
constexpr int ONE_PLAYER_FRAME = 16;
/** The frame of the animation strip to freeze on when two players are on the door */
constexpr int TWO_PLAYER_FRAME = 31;

void DoorNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					Color4 tint) {
	Vec2 doorPos;
	if (doorModel->getIsActive()) {
		// Door is currently active
		float onScreenAngle = getOnScreenAngle(doorModel->getAngle());
		if (isComingIntoView(onScreenAngle)) {
			// Door is coming into visible range
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
			doorPos = getPositionVec(relativeAngle, DOOR_RADIUS);
			setPosition(doorPos);
			isShown = true;
			setAngle(relativeAngle);
		} else if (isGoingOutOfView(onScreenAngle)) {
			// Door is leaving visible range
			doorPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(doorPos);
			isShown = false;
		}

		frameCap = doorModel->getPlayersOn() < 2 ? doorModel->getPlayersOn() * ONE_PLAYER_FRAME
												 : TWO_PLAYER_FRAME;
		if (animationNode->getFrame() < frameCap) {
			animationNode->setFrame((int)animationNode->getFrame() + 1);
		} else if (animationNode->getFrame() > frameCap) {
			animationNode->setFrame((int)animationNode->getFrame() - 1);
		}

		int diff = height - doorModel->getHeight();
		height = doorModel->getHeight();
		if (diff != 0) {
			animationNode->shiftPolygon(0, (float)diff);
		}
	} else {
		// Door is currently inactive
		doorPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(doorPos);
		isShown = false;
	}
	CustomNode::draw(batch, transform, tint);
}
