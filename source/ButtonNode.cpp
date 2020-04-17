#include "ButtonNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius used for placement of the doors. */
constexpr float BUTTON_POS = 660;

/** Position to place DoorNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** The frame of the animation strip to freeze on when one player is on the door */
constexpr int ONE_PLAYER_FRAME = 16;
/** The frame of the animation strip to freeze on when two players are on the door */
constexpr int TWO_PLAYER_FRAME = 31;

void ButtonNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	Vec2 buttonPos;
	if (buttonModel->getAngle() >= 0) {
		// Button is currently active
		float onScreenAngle = buttonModel->getAngle() - playerDonutModel->getAngle();
		onScreenAngle = onScreenAngle >= 0 ? onScreenAngle : shipSize + onScreenAngle;
		onScreenAngle = onScreenAngle > shipSize / 2 ? onScreenAngle - shipSize : onScreenAngle;
		onScreenAngle *= globals::PI_180;
		if (!isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
			onScreenAngle > -globals::SEG_CUTOFF_ANGLE) {
			// Button is coming into visible range
			float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();

			buttonPos = Vec2(BUTTON_POS * sin(relativeAngle), -BUTTON_POS * cos(relativeAngle));

			setPosition(buttonPos);
			isShown = true;
			setAngle(relativeAngle);
			if(buttonType == 0) {
                label->setPosition(Vec2((BUTTON_POS - 15) * sin(relativeAngle), (-BUTTON_POS) * cos(relativeAngle)));
                label->setAngle(relativeAngle);
				std::string s = std::to_string(buttonModel->getPair()->getSection());
				CULog("Section Label %d", buttonModel->getPair()->getSection());
				label->setText(s);
				label->setVisible(true);
			}
		} else if (isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
							   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE)) {
			// Door is leaving visible range
			buttonPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
			setPosition(buttonPos);
			isShown = false;
		}

		if (buttonModel->jumpedOn()) {
			CULog("jump in scenegraph");
			if (buttonType == 0) {
				setTexture(getButtonBaseDown());
			} else {
				setTexture(getButtonDown());
				setPosition((BUTTON_POS + 10) * sin(getAngle()),
							(-BUTTON_POS - 10) * cos(getAngle()));
			}
		} else {
			setPosition(BUTTON_POS * sin(getAngle()), -BUTTON_POS * cos(getAngle()));
		}
	} else {
        // Button is currently inactive
        buttonPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
        setPosition(buttonPos);
        isShown = false;
        buttonType == 0 ? setTexture(getButtonBaseUp()) : setTexture(getButtonUp());
		if(buttonType == 0) {
            label->setVisible(false);
        }
	}
	AnimationNode::draw(batch, transform, tint);
}
