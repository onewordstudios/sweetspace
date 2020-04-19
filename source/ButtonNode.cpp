#include "ButtonNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius used for placement of the doors. */
constexpr float BUTTON_POS = 660;

/** Position to place DoorNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** Amount to offset button */
constexpr float BUTTON_OFFSET = 10.0f;

void ButtonNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	Vec2 buttonPos;
	if (buttonModel->getAngle() >= 0) {
		// Button is currently active
		float onScreenAngle = buttonModel->getAngle() - playerDonutModel->getAngle();
		onScreenAngle = onScreenAngle >= 0 ? onScreenAngle : shipSize + onScreenAngle;
		onScreenAngle = onScreenAngle > shipSize / 2 ? onScreenAngle - shipSize : onScreenAngle;
		onScreenAngle *= globals::PI_180;
		float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
		if (!isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
			onScreenAngle > -globals::SEG_CUTOFF_ANGLE) {
			// Button is coming into visible range
			relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();

			buttonPos = Vec2(BUTTON_POS * sin(relativeAngle), -BUTTON_POS * cos(relativeAngle));

			setPosition(buttonPos);
			isShown = true;
			setAngle(relativeAngle);
			if (buttonType == 0) {
				std::string s = std::to_string(buttonModel->getPair()->getSection());
				CULog("Section Label %d", buttonModel->getPair()->getSection());
				label->setText(s);
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
				setPosition((BUTTON_POS + BUTTON_OFFSET) * sin(relativeAngle),
							(-BUTTON_POS - BUTTON_OFFSET) * cos(relativeAngle));
			}
		} else if (isShown) {
			setPosition(BUTTON_POS * sin(relativeAngle), -BUTTON_POS * cos(relativeAngle));
		}
	} else {
		// Button is currently inactive
		buttonPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(buttonPos);
		isShown = false;
		buttonType == 0 ? setTexture(getButtonBaseUp()) : setTexture(getButtonUp());
	}
	AnimationNode::draw(batch, transform, tint);
}
