#include "TutorialNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include "Globals.h"

using namespace cugl;

/** Position offset for indicators over breaches */
constexpr float BREACH_OFFSET_Y = 150;
/** Position offset for indicators over buttons */
constexpr float BUTTON_OFFSET_Y = 100;
/** Position offset for indicators over buttons */
constexpr float BUTTON_LABEL_OFFSET_Y = 250;

void TutorialNode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch, const Mat4& transform,
						Color4 tint) {
	float posX = 0;
	float posY = 0;
	float angle = 0;
	if (breachNode != nullptr) {
		posX = breachNode->getPositionX();
		posY = breachNode->getPositionY() + BREACH_OFFSET_Y;
		angle = breachNode->getAngle();
	} else if (doorNode != nullptr) {
		posX = doorNode->getPositionX();
		posY = doorNode->getPositionY();
		angle = doorNode->getAngle();
	} else if (buttonNode != nullptr && !isLabel) {
		posX = buttonNode->getPositionX();
		posY = buttonNode->getPositionY() + BUTTON_OFFSET_Y;
		angle = buttonNode->getAngle();
	} else if (buttonNode != nullptr && isLabel) {
		posX = buttonNode->getPositionX();
		posY = buttonNode->getPositionY() + BUTTON_LABEL_OFFSET_Y;
		angle = buttonNode->getAngle();
	}

	setPosition(posX, posY);
	setAngle(angle);

	AnimationNode::draw(batch, transform, tint);
}
