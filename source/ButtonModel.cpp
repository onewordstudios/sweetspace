#include "ButtonModel.h"

#include <utility>

#include "Globals.h"

#pragma region Animation Constants
/** Number of frames to animate down */
constexpr unsigned int DOWN_ANIMATION_DURATION = 5;

/** Number of frames to animate up */
constexpr unsigned int UP_ANIMATION_DURATION = 10;

/** Number of frames for button to stay depressed */
constexpr unsigned int DOWN_DURATION = 45;

/** Number of frames to ignore repeat jump commands */
constexpr unsigned int I_FRAMES = 10;
#pragma endregion

bool ButtonModel::init(const float a, std::shared_ptr<ButtonModel> pair, uint8_t pairID) {
	clear();
	angle = a;
	pairButton = std::move(pair);
	this->pairID = pairID;
	isActive = true;
	return true;
};

int ButtonModel::getSection() const {
	float mod = fmod(getAngle(), static_cast<float>(globals::SEG_DEG));
	int section = static_cast<int>(mod < (static_cast<float>(globals::SEG_DEG) / 2)
									   ? floorf(getAngle() / globals::SEG_DEG)
									   : ceilf(getAngle() / globals::SEG_DEG));
	return section;
}

void ButtonModel::update() {
	if (!jumped) {
		return;
	}

	frame++;

	if (frame < DOWN_ANIMATION_DURATION) {
		height = static_cast<float>(frame) / DOWN_ANIMATION_DURATION;
	} else if (frame - DOWN_ANIMATION_DURATION < DOWN_DURATION) {
		height = 1.0f;
	} else if (frame - DOWN_ANIMATION_DURATION - DOWN_DURATION < UP_ANIMATION_DURATION) {
		height = static_cast<float>(frame - DOWN_ANIMATION_DURATION - DOWN_DURATION) /
				 UP_ANIMATION_DURATION;
		height = 1.0f - height;
	} else {
		height = 0.0f;
		jumped = false;
		frame = 0;
	}
}

bool ButtonModel::trigger() {
	if (jumped && frame < I_FRAMES) {
		return false;
	}
	frame = jumped ? DOWN_ANIMATION_DURATION : 0;
	jumped = true;
	return true;
}

void ButtonModel::clear() {
	jumped = false;
	height = 0;
	resolved = false;
	angle = -1;
	isActive = false;
	frame = 0;
}
