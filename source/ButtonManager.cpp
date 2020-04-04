#include "ButtonManager.h"

void ButtonManager::registerButton(std::shared_ptr<cugl::Button> button) {
	buttons.push_back(button);
}

void ButtonManager::process(const cugl::Vec2& position) {
	if (position == cugl::Vec2::ZERO) {
		if (wasDown) {
			wasDown = false;
			for (unsigned int i = 0; i < downBtns.size(); i++) {
				downBtns[i]->setDown(false);
			}
			downBtns.clear();
		}
		return;
	}

	if (!wasDown) {
		wasDown = true;
		for (unsigned int i = 0; i < buttons.size(); i++) {
			if (buttons[i]->containsScreen(position)) {
				if (!buttons[i]->isDown()) {
					buttons[i]->setDown(true);
					downBtns.push_back(buttons[i]);
				}
			} else {
				if (buttons[i]->isDown()) {
					buttons[i]->setDown(false);
				}
			}
		}
	} else {
		for (unsigned int i = 0; i < downBtns.size(); i++) {
			if (downBtns[i]->containsScreen(position)) {
				if (!downBtns[i]->isDown()) {
					downBtns[i]->setDown(true);
				}
			} else {
				if (downBtns[i]->isDown()) {
					downBtns[i]->setDown(false);
				}
			}
		}
	}
}
