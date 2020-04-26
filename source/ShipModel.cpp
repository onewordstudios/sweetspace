#include "ShipModel.h"

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "PlayerDonutModel.h"

bool ShipModel::init(unsigned int numPlayers, unsigned int numBreaches, unsigned int numDoors,
					 unsigned int playerID, float shipSize, int initHealth,
					 unsigned int numButtons) {
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

	// Instantiate button models
	for (unsigned int i = 0; i < numButtons; i++) {
		buttons.push_back(std::make_shared<ButtonModel>());
	}

	// Instantiate health
	health = (float)initHealth;

	// Initialize size
	this->shipSize = shipSize;

	challenge = false;
	challengeProg = 0;

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
	breaches.at(id)->decHealth(1);
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

bool ShipModel::createAllTask(int data) {
	setRollDir(data);
	challenge = true;
	endTime = timer - globals::ROLL_CHALLENGE_LENGTH;
	challengeProg = 0;
	return true;
}

bool ShipModel::failAllTask() {
	for (int i = 0; i < donuts.size(); i++) {
		float angle = (float)(rand() % (int)(getSize()));
		donuts.at(i)->setAngle(angle);
	}
	return true;
}

bool ShipModel::createButton(float angle1, int id1, float angle2, int id2) {
	buttons.at(id1)->init(angle1, buttons.at(id2), id2);
	buttons.at(id2)->init(angle2, buttons.at(id1), id1);
	return true;
}

bool ShipModel::flagButton(int id, int player, int flag) {
	if (flag == 0) {
		buttons.at(id)->removePlayer(player);
	} else {
		buttons.at(id)->addPlayer(player);
	}
	return true;
}

void ShipModel::resolveButton(int id) {
	auto btn = buttons.at(id);
	if (btn == nullptr || btn->getAngle() == -1 || btn->isResolved()) {
		return;
	}
	btn->getPair()->setResolved(true);
	btn->setResolved(true);
}

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
