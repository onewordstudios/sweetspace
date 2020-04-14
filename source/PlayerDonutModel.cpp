#include "PlayerDonutModel.h"

using namespace cugl;

void PlayerDonutModel::update(float timestep) {
	// Adjust the active forces.
	velocity = RANGE_CLAMP(velocity, -DONUT_MAX_TURN, DONUT_MAX_TURN);

	// Adjust the angle by the change in angle
	angle += velocity;
	// INVARIANT: -360 < ang < 720
	if (angle > shipSize) {
		angle -= shipSize;
	} else if (angle < 0) {
		angle += shipSize;
	}

	velocity *= friction;

	// Restore Friction expoentially per frame if under default friction
	if (friction < DEFAULT_DONUT_FRICTION_FACTOR) {
		friction =
			RANGE_CLAMP(friction * FRICTION_RESTORATION, 0.0f, DEFAULT_DONUT_FRICTION_FACTOR);
	}

	if (abs(velocity) < DONUT_STOP_THRESHOLD) {
		velocity = 0;
	}

	updateJump(timestep);
}
