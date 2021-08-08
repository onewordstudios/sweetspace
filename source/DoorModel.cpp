#include "DoorModel.h"

/** The max height of the door*/
constexpr unsigned int MAX_HEIGHT = 1600;
/** The max height of the door*/
constexpr unsigned int HALF_OPEN = 400;
/** The speed of the door raising */
constexpr unsigned int SPEED = 20;

bool DoorModel::init(float a) {
	this->angle = a;
	isActive = true;
	return true;
}

/**
 * Disposes all resources and assets of this door.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a door may not be used until it is initialized again.
 */
void DoorModel::dispose() {}

void DoorModel::reset() {
	playersOn.reset();
	height = 0;
	isActive = false;
}

void DoorModel::removePlayer(uint8_t id) {
	if (!resolved()) {
		playersOn.reset(id);
	}
}

void DoorModel::update(float timestep) { // NOLINT assuming 60fps
	// Assuming 60fps is probably a bad idea down the line but for now is what all the other code
	// does too

	if (!getIsActive() || !resolved()) {
		return;
	}

	if (height < MAX_HEIGHT) {
		height += SPEED;
	}
	if (resolvedAndRaised()) {
		reset();
	}
}

bool DoorModel::halfOpen() const { return height >= HALF_OPEN; }

bool DoorModel::resolvedAndRaised() const { return resolved() && height >= MAX_HEIGHT; }
