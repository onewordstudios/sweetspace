﻿#include "DoorNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius used for placement of the doors. */
constexpr float DOOR_RADIUS = 650;

/** Position to place DoorNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** The frame of the animation strip to freeze on when one player is on the door */
constexpr int ONE_PLAYER_FRAME = 16;
/** The frame of the animation strip to freeze on when two players are on the door */
constexpr int TWO_PLAYER_FRAME = 31;

/** The height of the door. */
int height = 0;

/** The max frame this door can have. */
int frameCap = 0;

void DoorNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					Color4 tint) {
	Vec2 doorPos;
	if (doorModel->getAngle() >= 0) {
		// Door is currently active
		float absDiff =
			(shipSize / 2 -
			 abs(abs(playerDonutModel->getAngle() - doorModel->getAngle()) - shipSize / 2)) *
			globals::PI_180;
		float onScreenAngle =
			absDiff *
			(doorModel->getAngle() >= fmod(playerDonutModel->getAngle() + shipSize / 2, shipSize) ||
					 doorModel->getAngle() < playerDonutModel->getAngle()
				 ? -1
				 : 1);
		if (!isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
			onScreenAngle > -globals::SEG_CUTOFF_ANGLE) {
			// Door is coming into visible range
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
			doorPos = Vec2(DOOR_RADIUS * sin(relativeAngle), -DOOR_RADIUS * cos(relativeAngle));
			setPosition(doorPos);
			isShown = true;
			CULog("Door coming into view at %f", onScreenAngle / globals::PI_180);
		} else if (isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
							   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE)) {
			// Door is leaving visible range
			doorPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(doorPos);
			isShown = false;
			CULog("Door going out at %f", onScreenAngle / globals::PI_180);
		}

		frameCap = doorModel->getPlayersOn() < 2 ? doorModel->getPlayersOn() * ONE_PLAYER_FRAME
												 : TWO_PLAYER_FRAME;
		if (getFrame() < frameCap) {
			setFrame((int)getFrame() + 1);
		} else if (getFrame() > frameCap) {
			setFrame((int)getFrame() - 1);
		}

		int diff = height - doorModel->getHeight();
		height = doorModel->getHeight();
		if (diff != 0) {
			shiftPolygon(0, (float)diff);
		}

		setAngle(doorModel->getAngle() / shipSize * globals::PI * 2);
	} else {
		// Door is currently inactive
		doorPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(doorPos);
		isShown = false;
	}
	AnimationNode::draw(batch, transform, tint);
}
