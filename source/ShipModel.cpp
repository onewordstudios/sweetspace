#include "ShipModel.h"

#include "ExternalDonutModel.h"
#include "PlayerDonutModel.h"

constexpr int INITIAL_HEALTH = 11;

bool ShipModel::init(unsigned int numPlayers, unsigned int numBreaches, unsigned int numDoors,
					 unsigned int playerID, float shipSize) {
	// Instantiate donut models and assign colors
	for (unsigned int i = 0; i < numPlayers; i++) {
		donuts.push_back(playerID == i ? PlayerDonutModel::alloc(shipSize)
									   : ExternalDonutModel::alloc(shipSize));
		// TODO modulo max number of colors once constants are factored out
		donuts[i]->setColorId((int)i);
	}

	// Instantiate breach models
	for (unsigned int i = 0; i < numBreaches; i++) {
		breaches.push_back(BreachModel::alloc());
	}

	// Instantiate door models
	for (unsigned int i = 0; i < numDoors; i++) {
		doors.push_back(DoorModel::alloc());
	}

	// Instantiate health
	health = INITIAL_HEALTH;

	// Initialize size
	this->shipSize = shipSize;

	return true;
}

bool ShipModel::createBreach(float angle, int player, int id) {
	breaches.at(id)->reset(angle, player);
	return true;
}

bool ShipModel::createBreach(float angle, int health, int player, int id) {
	breaches.at(id)->reset(angle, health, player);
	return true;
}

bool ShipModel::createDoor(float angle, int id) {
	doors.at(id)->clear();
	doors.at(id)->setAngle(angle);
	return true;
}

bool ShipModel::resolveBreach(int id) {
	breaches.at(id)->setHealth(0);
	return true;
}

bool ShipModel::flagDoor(int id, int player, int flag) {
	if (flag == 0) {
		doors.at(id)->removePlayer(player);
	} else {
		doors.at(id)->addPlayer(player);
	}
	return true;
}

bool ShipModel::closeDoor(int id) { return false; }

/**
 * Disposes all resources and assets of this breach.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a breach may not be used until it is initialized again.
 */
void ShipModel::dispose() {
	donuts.clear();
	doors.clear();
	breaches.clear();
}
