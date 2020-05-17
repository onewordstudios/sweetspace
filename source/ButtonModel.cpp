#include "ButtonModel.h"

#include "Globals.h"

constexpr float MAX_BUTTON_HEIGHT = 0.1f;

bool ButtonModel::init(const float a, std::shared_ptr<ButtonModel> pair, int pairID) {
	clear();
	angle = a;
	pairButton = pair;
	this->pairID = pairID;
	isActive = true;
	return true;
};

int ButtonModel::getSection() {
	float mod = fmod(getAngle(), (float)globals::SEG_DEG);
	int section = (int)(mod < ((float)globals::SEG_DEG / 2) ? floorf(getAngle() / globals::SEG_DEG)
															: ceilf(getAngle() / globals::SEG_DEG));
	return section;
}

void ButtonModel::setPlayerHeight(float h) {
	height = h > MAX_BUTTON_HEIGHT ? 1.0f : h / MAX_BUTTON_HEIGHT;
}

void ButtonModel::clear() {
	playersOn.reset();
	height = 0;
	resolved = false;
	angle = -1;
	isActive = false;
}
