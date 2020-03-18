#include "ExternalDonutModel.h"

using namespace cugl;

constexpr unsigned int NETWORK_TICK = 12; // Originally defined in MagicInternetBox.cpp

bool ExternalDonutModel::init(const cugl::Vec2& pos) {
	bool ret = DonutModel::init(pos);
	// Initialize with finished interpolation
	networkMove.framesSinceUpdate = NETWORK_TICK;
	return ret;
}

void ExternalDonutModel::update(float timestep) {
	networkMove.framesSinceUpdate++;
	if (networkMove.framesSinceUpdate < NETWORK_TICK) {
		// Interpolate position
		float percent = (float)networkMove.framesSinceUpdate / NETWORK_TICK;
		networkMove.oldAngle += velocity;
		networkMove.angle += velocity;
		float newAngle = networkMove.angle * percent + networkMove.oldAngle * (1.0f - percent);
		angle = newAngle;
	} else {
		angle += velocity;
	}

	updateJump(timestep);
}