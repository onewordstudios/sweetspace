#include "ButtonNode.h"

#include <cugl/2d/CUNode.h>

#include <utility>

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

/** Scale of small sparkle effect */
constexpr float SPARKLE_SCALE_SMALL = 0.5;

bool ButtonNode::init(std::shared_ptr<ButtonModel> btn, std::shared_ptr<DonutModel> player,
					  float shipSize, const std::shared_ptr<cugl::AssetManager>& assets,
					  std::shared_ptr<SparkleNode> sparkle) {
	CustomNode::init(std::move(player), shipSize, -1, BUTTON_RADIUS);
	// Initialize angle to -1 to force the button to correctly process the label on first frame

	buttonModel = std::move(btn);

	btnBaseDown = assets->get<Texture>("challenge_btn_base_down");
	btnBaseUp = assets->get<Texture>("challenge_btn_base_up");
	btnDown = assets->get<Texture>("challenge_btn_down");
	btnUp = assets->get<Texture>("challenge_btn_up");

	setScale(BUTTON_SCALE);
	setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);

	baseNode = PolygonNode::allocWithTexture(btnBaseUp);
	bodyNode = PolygonNode::allocWithTexture(btnUp);

	baseNode->setAnchor(Vec2::ANCHOR_CENTER);
	baseNode->setPosition(0, 0);

	bodyNode->setAnchor(Vec2::ANCHOR_CENTER);
	bodyNode->setPosition(0, 0);

	label = Label::alloc("0000", assets->get<Font>("mont_black_italic_big"));

	sparkleNode = std::move(sparkle);
	sparkleNode->setScale(SPARKLE_SCALE_SMALL);

	addChild(bodyNode);
	addChild(baseNode);
	addChild(label);

	label->setScale(BUTTON_LABEL_SCALE);
	label->setHorizontalAlignment(Label::HAlign::CENTER);
	label->setForeground(Color4::WHITE);
	label->setAnchor(Vec2::ANCHOR_CENTER);
	label->setPosition(0, static_cast<float>(btnBaseUp->getHeight()) * BUTTON_LABEL_Y);

	isDirty = true;

	return true;
}

bool ButtonNode::isActive() { return buttonModel->getIsActive(); }

void ButtonNode::prePosition() {
	if (angle != buttonModel->getAngle()) {
		isDirty = true;
		label->setText(std::to_string(buttonModel->getPair()->getSection()));
		angle = buttonModel->getAngle();
	}
}

void ButtonNode::postPosition() {
	bodyNode->setPositionY(DEPRESSION_AMOUNT * buttonModel->getHeight());
	if (buttonModel->isJumpedOn()) {
		baseNode->setTexture(btnBaseDown);
		bodyNode->setTexture(btnDown);
	} else {
		baseNode->setTexture(btnBaseUp);
		bodyNode->setTexture(btnUp);
	}
}

void ButtonNode::becomeInactive() {
	sparkleNode->setRadius(radius);
	sparkleNode->setAngle(getAngle());
	sparkleNode->setOnShipAngle(angle);
	sparkleNode->beginAnimation();
}

void ButtonNode::draw(const shared_ptr<cugl::SpriteBatch>& batch, const cugl::Mat4& transform,
					  cugl::Color4 tint) {
	CustomNode::draw(batch, transform, tint);
	if (!isActive()) {
		resetAnimation();
	}
}
