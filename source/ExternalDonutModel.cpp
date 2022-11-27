#include "ExternalDonutModel.h"

#include "Globals.h"

using namespace cugl;

constexpr float BEG_DONUT = 0.2f;
constexpr float END_DONUT = 1.0f - BEG_DONUT;

bool ExternalDonutModel::init(const cugl::Vec2& pos, float shipSize) {
	const bool ret = DonutModel::init(pos, shipSize);
	// Initialize with finished interpolation
	networkMove.framesSinceUpdate = globals::NETWORK_TICK;
	return ret;
}

void ExternalDonutModel::setAngle(float value) {
	const float newAngle = value;
	networkMove.framesSinceUpdate = 0;
	networkMove.oldAngle = angle;
	networkMove.angle = newAngle;
}

void ExternalDonutModel::update(float timestep) {
	networkMove.framesSinceUpdate++;
	if (networkMove.framesSinceUpdate < globals::NETWORK_TICK) {
		// Interpolate position
		const float percent =
			static_cast<float>(networkMove.framesSinceUpdate) / globals::NETWORK_TICK;
		networkMove.oldAngle += velocity;
		networkMove.angle += velocity;

		// Clamp angles again
		if (networkMove.oldAngle > shipSize) {
			networkMove.oldAngle -= shipSize;
		} else if (networkMove.oldAngle < 0) {
			networkMove.oldAngle += shipSize;
		}
		if (networkMove.angle > shipSize) {
			networkMove.angle -= shipSize;
		} else if (networkMove.angle < 0) {
			networkMove.angle += shipSize;
		}

		float newAngle = networkMove.angle * percent + networkMove.oldAngle * (1.0f - percent);
		if (networkMove.oldAngle > END_DONUT * shipSize &&
			networkMove.angle < BEG_DONUT * shipSize) {
			newAngle =
				networkMove.angle * percent + (networkMove.oldAngle - shipSize) * (1.0f - percent);

		} else if (networkMove.angle > END_DONUT * shipSize &&
				   networkMove.oldAngle < BEG_DONUT * shipSize) {
			newAngle =
				(networkMove.angle - shipSize) * percent + networkMove.oldAngle * (1.0f - percent);
		}
		if (newAngle < 0) {
			newAngle += shipSize;
		}

		angle = newAngle;
	} else {
		angle += velocity;
	}

	updateJump(timestep);
}
