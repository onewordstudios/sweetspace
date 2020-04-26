﻿#include "ButtonModel.h"

#include "Globals.h"

bool ButtonModel::init(const float a, std::shared_ptr<ButtonModel> pair, int pairID) {
	clear();
	jumped = false;
	angle = a;
	pairButton = pair;
	this->pairID = pairID;
	return true;
};

int ButtonModel::getSection() {
	float mod = fmod(getAngle(), (float)globals::SEG_DEG);
	int section = (int)(mod < ((float)globals::SEG_DEG / 2) ? floorf(getAngle() / globals::SEG_DEG)
															: ceilf(getAngle() / globals::SEG_DEG));
	return section;
}

void ButtonModel::clear() {
	playersOn = 0;
	height = 0;
	resolved = false;
	jumped = false;
	angle = -1;
}
