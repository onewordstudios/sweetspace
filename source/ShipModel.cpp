#include "ShipModel.h"

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "MagicInternetBox.h"
#include "PlayerDonutModel.h"

/** Max number of attempts of generating a new teleportation angle */
constexpr int MAX_NEW_ANGLE_ATTEMPTS = 1000;

ShipModel::ShipModel()
	: rand(static_cast<unsigned int>(
		  std::chrono::high_resolution_clock::now().time_since_epoch().count())),
	  donuts(0),
	  breaches(0),
	  doors(0),
	  initHealth(0),
	  health(0),
	  shipSize(0),
	  timeless(false),
	  totalTime(0),
	  levelNum(0),
	  timeLeftInTimer(0),
	  canonicalTimeElapsed(0),
	  stabilizerTutorial(false) {}

bool ShipModel::init(uint8_t numPlayers, uint8_t numBreaches, uint8_t numDoors, uint8_t playerID,
					 float shipSize, float initHealth, uint8_t numButtons) {
	timeless = false;
	// Instantiate donut models and assign colors
	for (uint8_t i = 0; i < numPlayers; i++) {
		donuts.push_back(playerID == i ? PlayerDonutModel::alloc(shipSize)
									   : ExternalDonutModel::alloc(shipSize));

		donuts[i]->setColorId(i);
		if (!MagicInternetBox::getInstance().isPlayerActive(i)) {
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
	health = initHealth;
	this->initHealth = health;

	// Initialize size
	this->shipSize = shipSize;

	stabilizer.reset();
	stabilizerTutorial = false;

	return true;
}

bool ShipModel::createBreach(float angle, uint8_t player, uint8_t id) {
	breaches.at(id)->init(angle, player, timePassed());
	return true;
}

bool ShipModel::createBreach(float angle, uint8_t health, uint8_t player, uint8_t id) {
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
	stabilizer.fail();

	const auto& donut = donuts.at(*MagicInternetBox::getInstance().getPlayerID());
	float newAngle = 0;
	bool goodAngle = false;
	int attempts = 0;
	while (!goodAngle && attempts < MAX_NEW_ANGLE_ATTEMPTS) {
		// Generate random angle
		newAngle = std::fmodf(static_cast<float>(rand()), getSize());
		goodAngle = true;
		attempts += 1;
		// Check against breaches
		for (auto& breach : breaches) {
			float breachAngle = breach->getAngle();
			float diff = getAngleDifference(breachAngle, newAngle);
			if (diff <= MIN_DISTANCE && breachAngle != -1) {
				goodAngle = false;
				break;
			}
		}
		if (!goodAngle) {
			continue;
		}
		// Check against doors
		for (auto& door : doors) {
			float doorAngle = door->getAngle();
			float diff = getAngleDifference(doorAngle, newAngle);
			if (diff <= MIN_DISTANCE && doorAngle != -1) {
				goodAngle = false;
				break;
			}
		}
	}
	CULog("Setting teleport angle %f", newAngle);
	donut->setTeleportAngle(newAngle);

	return true;
}

bool ShipModel::createButton(float angle1, uint8_t id1, float angle2, uint8_t id2) {
	buttons[id1]->init(angle1, buttons.at(id2), id2);
	buttons[id2]->init(angle2, buttons.at(id1), id1);
	return true;
}

bool ShipModel::flagButton(uint8_t id) { return buttons[id]->trigger(); }

void ShipModel::resolveButton(uint8_t id) {
	auto& btn = buttons.at(id);
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
