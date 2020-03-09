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

bool ShipModel::createBreach() {
	breaches.push_back(BreachModel::alloc());
	return true;
}

bool ShipModel::createBreach(int id) {
	std::shared_ptr<BreachModel> breach = BreachModel::alloc();
	breach->setID(id);
	breaches.push_back(breach);
	return true;
}

bool ShipModel::resolveBreach(int id) {
	bool resolved = false;
	for (int i = 0; i < breaches.size(); i++) {
		if (breaches.at(i)->getID() == id) {
			breaches.at(i)->setHealth(0);
			resolved = true;
		}
	}
	return resolved;
}

/**
 * Disposes all resources and assets of this breach.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a breach may not be used until it is initialized again.
 */
void ShipModel::dispose() {}
