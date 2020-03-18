#include "PlayerDonutModel.h"

using namespace cugl;

void PlayerDonutModel::update(float timestep) {
	// Adjust the active forces.
	velocity = RANGE_CLAMP(velocity, -DONUT_MAX_TURN, DONUT_MAX_TURN);

	// Adjust the angle by the change in angle
	angle += velocity;
	// INVARIANT: -360 < ang < 720
	if (angle > FULL_CIRCLE) {
		angle -= FULL_CIRCLE;
	} else if (angle < 0) {
		angle += FULL_CIRCLE;
	}

	velocity *= DONUT_FRICTION_FACTOR;
	if (abs(velocity) < DONUT_STOP_THRESHOLD) {
		velocity = 0;
	}

	updateJump(timestep);
}