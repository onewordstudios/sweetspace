#include "BreachModel.h"

/**
 * Initializes a new breach with the given angle and max health
 *
 * This is an initializer.  It, combined with the constructor, produces the static
 * constructor create().  The initializer and normal constructor are private while
 * the static constructor is not.
 *
 * @param  a  The angle at which the breach exists
 * @param  health  The initial health of the breach
 * @param  p  the player id
 * @param  time  the time this was created
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool BreachModel::init(float a, int health, int p, float time) {
	angle = a;
	this->health = health;
	player = p;
	timeCreated = time;
	isActive = true;
	needSpriteUpdate = true;
	return true;
}

/**
 * Disposes all resources and assets of this breach.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a breach may not be used until it is initialized again.
 */
void BreachModel::dispose() {}
