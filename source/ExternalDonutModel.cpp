#include "ExternalDonutModel.h"

using namespace cugl;

constexpr unsigned int NETWORK_TICK = 12; // Originally defined in MagicInternetBox.cpp
constexpr float BEG_DONUT = 0.2f;
constexpr float END_DONUT = 1.0f - BEG_DONUT;

bool ExternalDonutModel::init(const cugl::Vec2& pos, float lvlSize) {
	bool ret = DonutModel::init(pos, lvlSize);
	// Initialize with finished interpolation
	networkMove.framesSinceUpdate = NETWORK_TICK;
	return ret;
}

void ExternalDonutModel::setAngle(float value) {
	float newAngle = value;
	networkMove.framesSinceUpdate = 0;
	networkMove.oldAngle = angle;
	networkMove.angle = newAngle;
}

void ExternalDonutModel::update(float timestep) {
	networkMove.framesSinceUpdate++;
	if (networkMove.framesSinceUpdate < NETWORK_TICK) {
		// Interpolate position
		float percent = (float)networkMove.framesSinceUpdate / NETWORK_TICK;
		networkMove.oldAngle += velocity;
		networkMove.angle += velocity;

		// Clamp angles again
		if (networkMove.oldAngle > maxAngle) {
			networkMove.oldAngle -= maxAngle;
		} else if (networkMove.oldAngle < 0) {
			networkMove.oldAngle += maxAngle;
		}
		if (networkMove.angle > maxAngle) {
			networkMove.angle -= maxAngle;
		} else if (networkMove.angle < 0) {
			networkMove.angle += maxAngle;
		}

		float newAngle = networkMove.angle * percent + networkMove.oldAngle * (1.0f - percent);
		if (networkMove.oldAngle > END_DONUT * maxAngle &&
			networkMove.angle < BEG_DONUT * maxAngle) {
			newAngle =
				networkMove.angle * percent + (networkMove.oldAngle - maxAngle) * (1.0f - percent);

		} else if (networkMove.angle > END_DONUT * maxAngle &&
				   networkMove.oldAngle < BEG_DONUT * maxAngle) {
			newAngle =
				(networkMove.angle - maxAngle) * percent + networkMove.oldAngle * (1.0f - percent);
		}
		if (newAngle < 0) {
			newAngle += maxAngle;
		}

		angle = newAngle;
	} else {
		angle += velocity;
	}

	updateJump(timestep);
}
