#include "ExternalDonutModel.h"

using namespace cugl;

constexpr unsigned int NETWORK_TICK = 12; // Originally defined in MagicInternetBox.cpp

bool ExternalDonutModel::init(const cugl::Vec2& pos) {
	bool ret = DonutModel::init(pos);
	// Initialize with finished interpolation
	networkMove.framesSinceUpdate = NETWORK_TICK;
	return ret;
}

void ExternalDonutModel::setAngle(float value) {
	float newAngle = HALF_CIRCLE * value / (float)M_PI;
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
		if (networkMove.oldAngle > FULL_CIRCLE) {
			networkMove.oldAngle -= FULL_CIRCLE;
		} else if (networkMove.oldAngle < 0) {
			networkMove.oldAngle += FULL_CIRCLE;
		}
		if (networkMove.angle > FULL_CIRCLE) {
			networkMove.angle -= FULL_CIRCLE;
		} else if (networkMove.angle < 0) {
			networkMove.angle += FULL_CIRCLE;
		}

		float newAngle = networkMove.angle * percent + networkMove.oldAngle * (1.0f - percent);
		if (networkMove.oldAngle > 0.8f * FULL_CIRCLE && networkMove.angle < 0.2f * FULL_CIRCLE) {
			newAngle = (networkMove.angle - FULL_CIRCLE) * percent +
					   networkMove.oldAngle * (1.0f - percent);

		} else if (networkMove.angle > 0.8f * FULL_CIRCLE &&
				   networkMove.oldAngle < 0.2f * FULL_CIRCLE) {
			newAngle = (networkMove.angle - FULL_CIRCLE) * percent +
					   networkMove.oldAngle * (1.0f - percent);
		}
		if (newAngle < 0) {
			newAngle += FULL_CIRCLE;
		}

		angle = newAngle;
	} else {
		angle += velocity;
	}

	updateJump(timestep);
}