#include "WinScreen.h"

#include "Globals.h"

constexpr size_t TRAVEL_TIME = 30;
constexpr float WIDTH = static_cast<float>(globals::SCENE_WIDTH);

WinScreen::WinScreen() : prevLevel(0), currLevel(0), currFrame(0) {}

WinScreen::~WinScreen() { dispose(); }

bool WinScreen::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	Node::init();
	setAnchor({0, 0});
	setPosition(0, 0);
	setVisible(false);

	screen = assets->get<Node>("winscreen");
	addChild(screen);

	cugl::Size dimen = cugl::Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width;
	setContentSize(dimen);
	screen->setPosition(WIDTH / 2, dimen.height / 2);
	doLayout();

	return true;
}

void WinScreen::dispose() { Node::dispose(); }

void WinScreen::draw(const shared_ptr<cugl::SpriteBatch>& batch, const cugl::Mat4& transform,
					 cugl::Color4 tint) {
	Node::draw(batch, transform, tint);
}

void WinScreen::activate(uint8_t completedLevel) {
	if (_isVisible) {
		return;
	}
	setVisible(true);
	currLevel = completedLevel;
	currFrame = 0;
}

bool WinScreen::isActive() { return _isVisible; }

void WinScreen::update() {
	if (!_isVisible) {
		return;
	}

	if (currFrame < TRAVEL_TIME) {
		currFrame++;
	}
}
