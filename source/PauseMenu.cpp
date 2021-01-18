#include "PauseMenu.h"

#include "Globals.h"
#include "Tween.h"

constexpr size_t OPEN_SPEED = 30;

PauseMenu::PauseMenu(const std::shared_ptr<cugl::AssetManager>& assets)
	: currFrame(0), menuOpen(false), musicPaused(false), sfxPaused(false) {
	init(assets);
}

PauseMenu::~PauseMenu() { dispose(); }

bool PauseMenu::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	Node::init();
	setAnchor({1.f / 2, 1.f / 2});
	setPosition(0, 0);
	setVisible(true);

	auto screen = assets->get<Node>("pausemenu");
	menu = assets->get<Node>("pausemenu_menu");
	pauseBtn = std::dynamic_pointer_cast<cugl::Button>(assets->get<Node>("pausemenu_pauseBtn"));

	cugl::Size dimen = cugl::Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width;
	setContentSize(dimen);
	screen->setContentSize(dimen);

	addChild(screen);
	screen->setPosition(0, 0);
	menu->setVisible(false);

	doLayout();

	btns.registerButton(pauseBtn);

	menuOpen = false;

	return true;
}

void PauseMenu::dispose() {}

bool PauseMenu::manageButtons(const std::tuple<cugl::Vec2, cugl::Vec2>& tapData) {
	if (ButtonManager::tappedButton(pauseBtn, tapData)) {
		menuOpen = !menuOpen;
		currFrame = 0;
	}

	// AudioChannels::get()->resumeMusic();
	// AudioChannels::get()->pauseMusic();

	// AudioChannels::get()->resumeAllEffects();
	// AudioChannels::get()->pauseAllEffects();
	// cugl::AudioChannels::get()->pauseAllEffects();

	return false;
}

void PauseMenu::update() {
	btns.process();

	if (currFrame <= OPEN_SPEED) {
		if (menuOpen) {
			if (currFrame == 0) {
				menu->setVisible(true);
			}
			menu->setAnchor({Tween::easeOut(0.f, 1.f, currFrame, OPEN_SPEED), 0.f});
		} else {
			menu->setAnchor({Tween::easeIn(1.f, 0.f, currFrame, OPEN_SPEED), 0.f});
			if (currFrame == OPEN_SPEED) {
				menu->setVisible(false);
			}
		}
		doLayout();
	}

	currFrame++;
}
