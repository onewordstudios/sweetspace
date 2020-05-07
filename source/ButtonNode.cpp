#include "ButtonNode.h"

#include <cugl/2d/CUNode.h>

#include "Globals.h"
#include "Tween.h"

using namespace cugl;

/** Scale of button label text */
constexpr float BUTTON_LABEL_SCALE = 1;

/** Scale of the button */
constexpr float BUTTON_SCALE = 0.3f;

/** Determines vertical positino of button label */
constexpr float BUTTON_LABEL_Y = -0.28f;

/** The radius used for placement of the buttons. */
constexpr float BUTTON_RADIUS = 600;

/** Amount that button body sinks when pressed on */
constexpr int DEPRESSION_AMOUNT = -100;

/** Length of animation */
constexpr int MAX_FRAMES = 32;

/** Frame marking beginning of animation */
constexpr int BEGIN_FRAME = 27;

bool ButtonNode::init(std::shared_ptr<ButtonModel> btn, std::shared_ptr<DonutModel> player,
					  float shipSize, std::shared_ptr<cugl::Texture> baseDown,
					  std::shared_ptr<cugl::Texture> baseUp, std::shared_ptr<cugl::Texture> btnDown,
					  std::shared_ptr<cugl::Texture> btnUp, std::shared_ptr<cugl::Font> labelFont) {
	CustomNode::init(player, shipSize, btn->getAngle(), BUTTON_RADIUS);

	buttonModel = btn;

	btnBaseDown = baseDown;
	btnBaseUp = baseUp;
	this->btnDown = btnDown;
	this->btnUp = btnUp;

	setScale(BUTTON_SCALE);
	setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);

	baseNode = PolygonNode::allocWithTexture(baseUp);
	bodyNode = PolygonNode::allocWithTexture(btnUp);

	baseNode->setAnchor(Vec2::ANCHOR_CENTER);
	baseNode->setPosition(0, 0);

	bodyNode->setAnchor(Vec2::ANCHOR_CENTER);
	bodyNode->setPosition(0, 0);

	label = Label::alloc("", labelFont);

	addChild(bodyNode);
	addChild(baseNode);
	addChild(label);

	label->setScale(BUTTON_LABEL_SCALE);
	label->setHorizontalAlignment(Label::HAlign::CENTER);
	label->setForeground(Color4::WHITE);
	label->setAnchor(Vec2::ANCHOR_CENTER);
	label->setPosition(0, (float)baseUp->getHeight() * BUTTON_LABEL_Y);

	return true;
}

bool ButtonNode::isActive() {
	bool justJumpedOn = buttonModel->isJumpedOn() && !prevJumpedOn;
	return buttonModel->getIsActive() || currentFrame != 0 || justJumpedOn;
}

void ButtonNode::prePosition() {
	label->setText(std::to_string(buttonModel->getPair()->getSection()));
	angle = buttonModel->getAngle();
}

void ButtonNode::postPosition() {
	bool justJumpedOn = buttonModel->isJumpedOn() && !prevJumpedOn;
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
			bodyNode->setPositionY(Tween::linear(0, DEPRESSION_AMOUNT, currentFrame - BEGIN_FRAME,
												 MAX_FRAMES - BEGIN_FRAME));
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
	prevJumpedOn = buttonModel->isJumpedOn();
}

void ButtonNode::draw(const shared_ptr<cugl::SpriteBatch>& batch, const cugl::Mat4& transform,
					  cugl::Color4 tint) {
	CustomNode::draw(batch, transform, tint);
	if (!isActive()) {
		resetAnimation();
	}
}
