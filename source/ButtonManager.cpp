#include "ButtonManager.h"

void ButtonManager::registerButton(std::shared_ptr<cugl::Button> button) {
	buttons.push_back(button);
	down.push_back(false);
}

void ButtonManager::process(const cugl::Vec2& position) {
	for (unsigned int i = 0; i < buttons.size(); i++) {
		if (buttons[i]->containsScreen(position)) {
			if (!down[i]) {
				down[i] = true;
				buttons[i]->setDown(true);
			}
		} else {
			if (down[i]) {
				down[i] = false;
				buttons[i]->setDown(false);
			}
		}
	}
}