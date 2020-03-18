#include "ShipModel.h"

/**
 * Initializes a new ship with the given donuts and breaches
 *
 * This is an initializer.  It, combined with the constructor, produces the static
 * constructor create().  The initializer and normal constructor are private while
 * the static constructor is not.
 *
 * @param  d  The list of donuts
 * @param  b  The list of breaches
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool ShipModel::init(std::vector<std::shared_ptr<DonutModel>> &d,
					 std::vector<std::shared_ptr<BreachModel>> &b) {
	donuts = d;
	breaches = b;
	return true;
}

bool ShipModel::createBreach(float angle, int player, int id) {
	breaches.at(id)->setAngle(angle);
	breaches.at(id)->setHealth(3);
	breaches.at(id)->setPlayer(player);
	return true;
}

bool ShipModel::createBreach(float angle, int health, int player, int id) {
	breaches.at(id)->setAngle(angle);
	breaches.at(id)->setHealth(health);
	breaches.at(id)->setPlayer(player);
	breaches.at(id)->setNeedSpriteUpdate(true);
	return true;
}

bool ShipModel::resolveBreach(int id) {
	breaches.at(id)->setHealth(0);
	return true;
}

/**
 * Disposes all resources and assets of this breach.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a breach may not be used until it is initialized again.
 */
void ShipModel::dispose() {}
