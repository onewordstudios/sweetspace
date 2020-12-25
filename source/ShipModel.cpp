﻿#include "ShipModel.h"

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "MagicInternetBox.h"
#include "PlayerDonutModel.h"

/** Max number of attempts of generating a new teleportation angle */
constexpr int MAX_NEW_ANGLE_ATTEMPTS = 1000;

bool ShipModel::init(uint8_t numPlayers, uint8_t numBreaches, uint8_t numDoors, uint8_t playerID,
					 float shipSize, unsigned int initHealth, uint8_t numButtons) {
	timeless = false;
	// Instantiate donut models and assign colors
	for (uint8_t i = 0; i < numPlayers; i++) {
		donuts.push_back(playerID == i ? PlayerDonutModel::alloc(shipSize)
									   : ExternalDonutModel::alloc(shipSize));
		// TODO modulo max number of colors once constants are factored out
		donuts[i]->setColorId((int)i);
		if (!MagicInternetBox::getInstance()->isPlayerActive(i)) {
			donuts[i]->setIsActive(false);
		}
	}

	// Instantiate breach models
	for (uint8_t i = 0; i < numBreaches; i++) {
		breaches.push_back(std::make_shared<BreachModel>());
	}

	// Instantiate door models
	for (uint8_t i = 0; i < numDoors; i++) {
		doors.push_back(std::make_shared<DoorModel>());
	}

	// Instantiate button models
	for (uint8_t i = 0; i < numButtons; i++) {
		buttons.push_back(std::make_shared<ButtonModel>());
	}

	// Instantiate health
	health = (float)initHealth;
	this->initHealth = health;

	// Initialize size
	this->shipSize = shipSize;

	stabilizer.reset();

	return true;
}

bool ShipModel::createBreach(float angle, uint8_t player, uint8_t id) {
	breaches.at(id)->init(angle, player, timePassed());
	return true;
}

bool ShipModel::createBreach(float angle, unsigned int health, uint8_t player, uint8_t id) {
	breaches.at(id)->init(angle, health, player, timePassed());
	return true;
}

bool ShipModel::createDoor(float angle, uint8_t id) {
	doors.at(id)->init(angle);
	return true;
}

bool ShipModel::createUnopenable(float angle, uint8_t id) {
	unopenable.at(id)->init(angle);
	return true;
}

bool ShipModel::resolveBreach(uint8_t id) {
	breaches.at(id)->decHealth(1);
	return true;
}

bool ShipModel::flagDoor(uint8_t id, uint8_t player, uint8_t flag) {
	if (flag == 0) {
		doors.at(id)->removePlayer(player);
	} else {
		doors.at(id)->addPlayer(player);
	}
	return true;
}

bool ShipModel::createAllTask() {
	stabilizer.startChallenge(canonicalTimeElapsed);
	return true;
}

bool ShipModel::failAllTask() {
	for (int i = 0; i < donuts.size(); i++) {
		float newAngle = 0;
		bool goodAngle = false;
		int attempts = 0;
		while (!goodAngle && attempts < MAX_NEW_ANGLE_ATTEMPTS) {
			// Generate random angle
			newAngle = (float)(rand() % (int)(getSize()));
			goodAngle = true;
			attempts += 1;
			// Check against breaches
			for (unsigned int k = 0; k < breaches.size(); k++) {
				float breachAngle = breaches[k]->getAngle();
				float diff = getAngleDifference(breachAngle, newAngle);
				if (diff <= MIN_DISTANCE && breachAngle != -1) {
					goodAngle = false;
					break;
				}
			}
			// Check against doors
			if (goodAngle) {
				for (unsigned int k = 0; k < doors.size(); k++) {
					float doorAngle = doors[k]->getAngle();
					float diff = getAngleDifference(doorAngle, newAngle);
					if (diff <= MIN_DISTANCE && doorAngle != -1) {
						goodAngle = false;
						break;
					}
				}
			}
		}
		donuts.at(i)->setTeleportAngle(newAngle);
	}
	return true;
}

void ShipModel::setStabilizerStatus(StabilizerStatus s) { stabilizerStatus = s; }

bool ShipModel::createButton(float angle1, uint8_t id1, float angle2, uint8_t id2) {
	buttons.at(id1)->init(angle1, buttons.at(id2), id2);
	buttons.at(id2)->init(angle2, buttons.at(id1), id1);
	return true;
}

bool ShipModel::flagButton(uint8_t id) { return buttons[id]->trigger(); }

void ShipModel::resolveButton(uint8_t id) {
	auto btn = buttons.at(id);
	if (btn == nullptr || !btn->getIsActive() || btn->isResolved()) {
		return;
	}
	btn->getPair()->resolve();
	btn->resolve();
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
