#include "ExternalDonutModel.h"

#include "Globals.h"

using namespace cugl;

constexpr float BEG_DONUT = 0.2f;
constexpr float END_DONUT = 1.0f - BEG_DONUT;

bool ExternalDonutModel::init(const cugl::Vec2& pos) {
	bool ret = DonutModel::init(pos);
	// Initialize with finished interpolation
	networkMove.framesSinceUpdate = globals::NETWORK_TICK;
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
	if (networkMove.framesSinceUpdate < globals::NETWORK_TICK) {
		// Interpolate position
		float percent = (float)networkMove.framesSinceUpdate / globals::NETWORK_TICK;
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
		if (networkMove.oldAngle > END_DONUT * FULL_CIRCLE &&
			networkMove.angle < BEG_DONUT * FULL_CIRCLE) {
			newAngle = networkMove.angle * percent +
					   (networkMove.oldAngle - FULL_CIRCLE) * (1.0f - percent);

		} else if (networkMove.angle > END_DONUT * FULL_CIRCLE &&
				   networkMove.oldAngle < BEG_DONUT * FULL_CIRCLE) {
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
