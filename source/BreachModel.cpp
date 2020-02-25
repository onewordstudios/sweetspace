#include "BreachModel.h"
/**
 * Initializes a new breach with the given angle
 *
 * This is an initializer.  It, combined with the constructor, produces the static
 * constructor create().  The initializer and normal constructor are private while
 * the static constructor is not.
 *
 * @param  a  The angle at which the breach exists
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool BreachModel::init(float a) {
	angle = a;
	return true;
}

/**
 * Disposes all resources and assets of this breach.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a breach may not be used until it is initialized again.
 */
void BreachModel::dispose() { sprite = nullptr; }
