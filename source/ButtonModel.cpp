#include "ButtonModel.h"

#include "Globals.h"

#pragma region Animation Constants
/** Number of frames to animate down */
constexpr int DOWN_ANIMATION_DURATION = 5;

/** Number of frames to animate up */
constexpr int UP_ANIMATION_DURATION = 30;

/** Number of frames for button to stay depressed */
constexpr int DOWN_DURATION = 120;

/** Number of frames to ignore repeat jump commands */
constexpr int I_FRAMES = 10;
#pragma endregion

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

void ButtonModel::update() {
	if (!jumped) {
		return;
	}
	frame++;

	if (frame < DOWN_ANIMATION_DURATION) {
		height = (float)frame / DOWN_ANIMATION_DURATION;
	} else if (frame - DOWN_ANIMATION_DURATION < DOWN_DURATION) {
		height = 1.0f;
	} else if (frame - DOWN_ANIMATION_DURATION - DOWN_DURATION < UP_ANIMATION_DURATION) {
		height = (float)(frame - DOWN_ANIMATION_DURATION - DOWN_DURATION) / UP_ANIMATION_DURATION;
		height = 1.0f - height;
	} else {
		height = 0.0f;
		jumped = false;
		frame = 0;
	}
}

void ButtonModel::trigger() {
	if (jumped && frame < I_FRAMES) {
		return;
	}
	jumped = true;
	frame = 0;
}

void ButtonModel::clear() {
	jumped = false;
	height = 0;
	resolved = false;
	angle = -1;
	isActive = false;
	frame = 0;
}
