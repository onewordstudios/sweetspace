#include "ButtonNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"
#include "Tween.h"

using namespace cugl;

/** The radius used for placement of the doors. */
constexpr float BUTTON_POS = 600;

/** Position to place DoorNode offscreen. */
constexpr float OFF_SCREEN_POS = 1500;

/** Amount that button body sinks when pressed on */
constexpr int DEPRESSION_AMOUNT = -100;

/** Length of animation */
constexpr int MAX_FRAMES = 32;

/** Frame marking beginning of animation */
constexpr int BEGIN_FRAME = 27;

void ButtonNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
					  Color4 tint) {
	Vec2 buttonPos;
	bool justJumpedOn = buttonModel->isJumpedOn() && !prevJumpedOn;
	if (buttonModel->getIsActive() || currentFrame != 0 || justJumpedOn) {
		// Button is currently active
		float onScreenAngle = buttonModel->getAngle() - playerDonutModel->getAngle();
		onScreenAngle = onScreenAngle >= 0 ? onScreenAngle : shipSize + onScreenAngle;
		onScreenAngle = onScreenAngle > shipSize / 2 ? onScreenAngle - shipSize : onScreenAngle;
		onScreenAngle *= globals::PI_180;
		if (!(buttonModel->getAngle() == -1 && (currentFrame != 0 || justJumpedOn))) {
			if (!isShown && onScreenAngle < globals::SEG_CUTOFF_ANGLE &&
				onScreenAngle > -globals::SEG_CUTOFF_ANGLE) {
				// Coming into visible range
				float relativeAngle = onScreenAngle - getParent()->getParent()->getAngle();
				buttonPos = Vec2(BUTTON_POS * sin(relativeAngle), -BUTTON_POS * cos(relativeAngle));
				setPosition(buttonPos);
				isShown = true;
				setAngle(relativeAngle);
				std::string s = std::to_string(buttonModel->getPair()->getSection());
				label->setText(s);
			} else if (isShown && (onScreenAngle >= globals::SEG_CUTOFF_ANGLE ||
								   onScreenAngle <= -globals::SEG_CUTOFF_ANGLE)) {
				// Leaving visible range
				buttonPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
				setPosition(buttonPos);
				isShown = false;
			}
		}

		if (justJumpedOn) {
			// Begin depression animation
			currentFrame = 1;
			baseNode->setTexture(btnBaseDown);
			bodyNode->setTexture(btnDown);
		} else if (!buttonModel->isJumpedOn()) {
			baseNode->setTexture(btnBaseUp);
			bodyNode->setTexture(btnUp);
			if (bodyNode->getPositionY() != 0 && currentFrame == 0) {
				// No player on button, reverse depression
				currentFrame = MAX_FRAMES - 1;
			}
		}
		if (currentFrame != 0) {
			// In the middle of animating
			if (currentFrame >= BEGIN_FRAME) {
				bodyNode->setPositionY(Tween::linear(
					0, DEPRESSION_AMOUNT, currentFrame - BEGIN_FRAME, MAX_FRAMES - BEGIN_FRAME));
			}
			if (!buttonModel->isJumpedOn()) {
				// No player on button, reverse depression
				currentFrame -= 1;
			} else {
				// Player is still on button, continue
				currentFrame += 1;
				if (currentFrame == MAX_FRAMES) {
					// End of animation
					currentFrame = 0;
				}
			}
		}
	} else {
		// Button is currently inactive
		buttonPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(buttonPos);
		isShown = false;
		resetAnimation();
	}
	prevJumpedOn = buttonModel->isJumpedOn();
	Node::draw(batch, transform, tint);
}
