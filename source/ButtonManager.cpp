#include "ButtonManager.h"

#include "InputController.h"

void ButtonManager::registerButton(const std::shared_ptr<cugl::Button>& button) {
	buttons.push_back(button);
}

void ButtonManager::process() {
	cugl::Vec2 position = InputController::getInstance()->getCurrTapLoc();

	if (position == cugl::Vec2::ZERO) {
		if (wasDown) {
			wasDown = false;
			for (auto& downBtn : downBtns) {
				downBtn->setDown(false);
			}
			downBtns.clear();
		}
		return;
	}

	if (!wasDown) {
		wasDown = true;
		for (auto& button : buttons) {
			if (button->containsScreen(position)) {
				if (!button->isDown()) {
					button->setDown(true);
					downBtns.push_back(button);
				}
			} else {
				if (button->isDown()) {
					button->setDown(false);
				}
			}
		}
	} else {
		for (auto& downBtn : downBtns) {
			if (downBtn->containsScreen(position)) {
				if (!downBtn->isDown()) {
					downBtn->setDown(true);
				}
			} else {
				if (downBtn->isDown()) {
					downBtn->setDown(false);
				}
			}
		}
	}
}

bool ButtonManager::tappedButton(const std::shared_ptr<cugl::Button>& button,
								 std::tuple<cugl::Vec2, cugl::Vec2> tapData) {
	if (button->containsScreen(std::get<0>(tapData)) &&
		button->containsScreen(std::get<1>(tapData))) {
		// We only need to play sound effects for one button at a time, so start and end
		// the event
		SoundEffectController::getInstance()->startEvent(SoundEffectController::CLICK, 0);
		SoundEffectController::getInstance()->endEvent(SoundEffectController::CLICK, 0);
		return true;
	}
	return false;
}

void ButtonManager::clear() {
	wasDown = false;
	buttons.clear();
	downBtns.clear();
}
