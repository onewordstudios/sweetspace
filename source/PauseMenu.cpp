#include "PauseMenu.h"

#include "AudioController.h"
#include "Globals.h"
#include "MagicInternetBox.h"
#include "Tween.h"

constexpr size_t OPEN_SPEED = 30;
constexpr float NEEDLE_SPEED = 0.3f;
constexpr float NEEDLE_CUTOFF = 0.01f;

PauseMenu::PauseMenu(const std::shared_ptr<cugl::AssetManager>& assets)
	: currFrame(0), menuOpen(false) {
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
	needle = assets->get<Node>("pausemenu_menu_dial_hand");

	pauseBtn = std::dynamic_pointer_cast<cugl::Button>(assets->get<Node>("pausemenu_pauseBtn"));
	leaveBtn =
		std::dynamic_pointer_cast<cugl::Button>(assets->get<Node>("pausemenu_menu_leaveBtn"));
	musicMuteBtn =
		std::dynamic_pointer_cast<cugl::Button>(assets->get<Node>("pausemenu_menu_musicBtn"));
	sfxMuteBtn =
		std::dynamic_pointer_cast<cugl::Button>(assets->get<Node>("pausemenu_menu_soundBtn"));

	cugl::Size dimen = cugl::Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width;
	setContentSize(dimen);
	screen->setContentSize(dimen);

	addChild(screen);
	screen->setPosition(0, 0);
	menu->setVisible(false);

	doLayout();

	btns.registerButton(pauseBtn);
	btns.registerButton(leaveBtn);

	auto& audio = AudioController::getInstance();
	musicMuteBtn->setDown(!audio.isMusicActive());
	sfxMuteBtn->setDown(!audio.isSfxActive());

	menuOpen = false;

	return true;
}

void PauseMenu::dispose() {
	menu = nullptr;
	needle = nullptr;
	pauseBtn = nullptr;
	leaveBtn = nullptr;
	musicMuteBtn = nullptr;
	sfxMuteBtn = nullptr;
	Node::dispose();
	btns.clear();
}

bool PauseMenu::manageButtons(const std::tuple<cugl::Vec2, cugl::Vec2>& tapData) {
	if (ButtonManager::tappedButton(pauseBtn, tapData)) {
		menuOpen = !menuOpen;
		currFrame = 0;
		return false;
	}

	if (ButtonManager::tappedButton(musicMuteBtn, tapData)) {
		auto& audio = AudioController::getInstance();
		audio.toggleMusic();
		musicMuteBtn->setDown(!audio.isMusicActive());
	} else if (ButtonManager::tappedButton(sfxMuteBtn, tapData)) {
		auto& audio = AudioController::getInstance();
		audio.toggleSfx();
		sfxMuteBtn->setDown(!audio.isSfxActive());
	}

	return menuOpen && ButtonManager::tappedButton(leaveBtn, tapData);
}

void PauseMenu::updateNeedle() {
	float needlePer = static_cast<float>(MagicInternetBox::getInstance().getNumPlayers() - 1) /
					  static_cast<float>(globals::MAX_PLAYERS);
	float needleTarget = -needlePer * globals::TWO_PI * globals::NEEDLE_OFFSET;
	float currNeedle = needle->getAngle();
	if (needleTarget != needle->getAngle()) {
		float diff = needleTarget - currNeedle;
		float newNeedle = diff * NEEDLE_SPEED + currNeedle;
		if (diff < NEEDLE_CUTOFF) {
			newNeedle = needleTarget;
		}
		needle->setAngle(newNeedle);
	}
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

	updateNeedle();

	currFrame++;
}
