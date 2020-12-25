#include "BreachModel.h"

/**
 * Initializes a new breach with the given angle and max health
 *
 * @param  a  The angle at which the breach exists
 * @param  health  The initial health of the breach
 * @param  p  the player id
 * @param  time  the time this was created
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool BreachModel::init(float a, uint8_t health, uint8_t p, float time) {
	angle = a;
	this->health = health;
	player = p;
	timeCreated = time;
	isActive = true;
	needSpriteUpdate = true;
	return true;
}

void BreachModel::dispose() {}
