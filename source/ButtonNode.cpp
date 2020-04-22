#include "ButtonNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"

using namespace cugl;

/** The radius used for placement of the doors. */
constexpr float BUTTON_POS = 600;

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

		if (buttonModel->jumpedOn()) {
			baseNode->setTexture(btnBaseDown);
			bodyNode->setTexture(btnDown);
			bodyNode->setPositionY(-BUTTON_OFFSET);
		} else if (isShown) {
			bodyNode->setPositionY(0);
		}
	} else {
		// Button is currently inactive
		buttonPos = Vec2(OFF_SCREEN_POS, OFF_SCREEN_POS);
		setPosition(buttonPos);
		isShown = false;
		baseNode->setTexture(btnBaseUp);
		bodyNode->setTexture(btnUp);
	}
	Node::draw(batch, transform, tint);
}
