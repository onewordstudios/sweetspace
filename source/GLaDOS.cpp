#include "GLaDOS.h"

#include <vector>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark GM Variables
/** The maximum number of events on ship at any one time. This will probably need to scale with
 * the number of players*/
unsigned int maxEvents;
/** The maximum number of events on ship at any one time. This will probably need to scale with
 * the number of players*/
unsigned int maxDoors;
/** The maximum number of buttons on ship at any one time. This will probably need to scale with
 * the number of players*/
unsigned int maxButtons;
/** Spawn rate of breaches = 1/spawnRate for EVERY UPDATE FRAME. 100 is a very fast rate already.
 */
unsigned int spawnRate;
float minAngleDiff;
/** Array recording which breaches are free or not. */
vector<bool> breachFree;

/** Array recording which doors are free or not. */
vector<bool> doorFree;

/** Array recording which doors are free or not. */
vector<bool> buttonFree;

#pragma mark -
#pragma mark GM
/**
 * Creates a new GM controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
GLaDOS::GLaDOS() : active(false), numEvents(0), playerID(0), mib(nullptr) {}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void GLaDOS::dispose() {
	if (active) {
		active = false;
	}
}

/**
 * Initializes the GM
 *
 * This method works like a proper constructor, initializing the GM
 * controller and allocating memory.
 *
 * @return true if the controller was initialized successfully
 */
bool GLaDOS::init(std::shared_ptr<ShipModel> ship, std::shared_ptr<LevelModel> level) {
	bool success = true;
	this->ship = ship;
	this->mib = MagicInternetBox::getInstance();
	this->playerID = mib->getPlayerID();
	maxEvents = level->getMaxBreaches();
	maxDoors = level->getMaxDoors();
	maxButtons = 2; // add to level
	minAngleDiff = level->getMinAngleDiff();
	spawnRate = level->getSpawnRate();
	breachFree.resize(maxEvents);
	doorFree.resize(maxDoors);
	buttonFree.resize(maxButtons);

	for (int i = 0; i < maxEvents; i++) {
		breachFree.at(i) = true;
	}
	for (int i = 0; i < maxDoors; i++) {
		doorFree.at(i) = true;
	}

	for (int i = 0; i < maxButtons; i++) {
		buttonFree.at(i) = true;
	}
	challengeInProg = false;
	// Set random seed based on time
	srand((unsigned int)time(NULL));
	active = success;
	return success;
}

/**
 * Processes the GM.
 *
 * This method is used to run the GM for generating and managing current ship events
 */
void GLaDOS::update(float dt) {
	// Removing breaches that have 0 health left
	for (int i = 0; i < maxEvents; i++) {
		if (ship->getBreaches().at(i) == nullptr) {
			continue;
		}
		// check if health is zero or the assigned player is inactive
		if (ship->getBreaches().at(i)->getHealth() == 0 ||
			!ship->getDonuts().at(ship->getBreaches().at(i)->getPlayer())->getIsActive()) {
			ship->getBreaches().at(i)->setHealth(0);
			ship->getBreaches().at(i)->setAngle(-1);
			breachFree.at(i) = true;
		}
	}

	// Remove doors that have been resolved and opened. Also raise doors that are resolved.
	for (int i = 0; i < maxDoors; i++) {
		if (ship->getDoors().at(i) == nullptr) {
			continue;
		}
		if (ship->getDoors().at(i)->resolvedAndRaised()) {
			ship->getDoors().at(i)->setAngle(-1);
			doorFree.at(i) = true;
		} else if (ship->getDoors().at(i)->resolved()) {
			ship->getDoors().at(i)->raiseDoor();
		}
	}

	for (int i = 0; i < maxButtons; i++) {
		if (ship->getButtons().at(i) == nullptr) {
			continue;
		}
		if (ship->getButtons().at(i)->isResolved()) {
			ship->getButtons().at(i)->setAngle(-1);
			ship->getButtons().at(i)->getPair()->setAngle(-1);
			buttonFree.at(ship->getButtons().at(i)->getPairID()) = true;
			buttonFree.at(i) = true;
			mib->flagButton(i, playerID, 0);
			ship->getButtons().at(i)->setJumpedOn(false);
			ship->getButtons().at(i)->getPair()->setJumpedOn(false);


		}
	}

	// Check if this is the host for generating breaches and doors
	if (playerID != 0) {
		return;
	}
	// Simple logic for adding a breach when under max and randomly, replace with actual logic
	// later
	if (rand() % spawnRate > 1) return;
	for (int i = 0; i < maxEvents; i++) {
		if (breachFree.at(i)) {
			float angle = (float)(rand() % (int)(ship->getSize()));
			bool goodAngle = true;
			for (int j = 0; j < ship->getDonuts().size(); j++) {
				float diff =
					ship->getSize() / 2 -
					abs(abs(ship->getDonuts().at(j)->getAngle() - angle) - ship->getSize() / 2);
				if (diff < minAngleDiff) {
					goodAngle = false;
					break;
				}
			}

			// Make sure it's not too close to other breaches
			for (unsigned int k = 0; k < ship->getBreaches().size(); k++) {
				if (k == i) {
					continue;
				}
				float breachAngle = ship->getBreaches()[k]->getAngle();
				float diff =
					ship->getSize() / 2 - abs(abs(breachAngle - angle) - ship->getSize() / 2);
				if (breachAngle != -1 && diff < minAngleDiff) {
					goodAngle = false;
					break;
				}
			}

			if (!goodAngle ||
				!ship->getDonuts().at(ship->getBreaches().at(i)->getPlayer())->getIsActive()) {
				continue;
			}
			breachFree.at(i) = false;
			int p = (int)(rand() % ship->getDonuts().size());
			ship->getBreaches().at(i)->reset(angle, p);
			ship->getBreaches().at(i)->setTimeCreated(ship->timer);
			mib->createBreach(angle, p, i);
			break;
		}
	}
	for (int i = 0; i < maxDoors; i++) {
		if (doorFree.at(i)) {
			float angle = (float)(rand() % (int)(ship->getSize()));
			bool goodAngle = true;
			for (int j = 0; j < ship->getDonuts().size(); j++) {
				float diff =
					ship->getSize() / 2 -
					abs(abs(ship->getDonuts().at(j)->getAngle() - angle) - ship->getSize() / 2);
				if (diff < minAngleDiff) {
					goodAngle = false;
					break;
				}
			}
			if (!goodAngle) {
				continue;
			}
			ship->getDoors().at(i)->setAngle(angle);
			ship->getDoors().at(i)->clear();
			doorFree.at(i) = false;
			mib->createDualTask(angle, -1, -1, i);
			break;
		}
	}

	for (int i = 0; i < maxButtons; i++) {
		if (buttonFree.at(i)) {
			float angle = (float)(rand() % (int)(ship->getSize()));
			bool goodAngle = true;
			for (int j = 0; j < ship->getDonuts().size(); j++) {
				float diff =
					ship->getSize() / 2 -
					abs(abs(ship->getDonuts().at(j)->getAngle() - angle) - ship->getSize() / 2);
				if (diff < minAngleDiff) {
					goodAngle = false;
					break;
				}
			}
			if (!goodAngle) {
				continue;
			}
			float pairAngle;
			int pairID;
			float minButtonDiff = 100;
			for (int j = 0; j < maxButtons; j++) {
				if (i != j && buttonFree.at(j)) {
					ship->getButtons().at(i)->setPair(ship->getButtons().at(j), j);
					ship->getButtons().at(j)->setPair(ship->getButtons().at(i), i);
					pairAngle = (float)(rand() % (int)(ship->getSize()));
					while(abs(pairAngle - angle) < minButtonDiff) {
						pairAngle = (float)(rand() % (int)(ship->getSize()));
					}
					ship->getButtons().at(j)->setAngle(pairAngle);
					ship->getButtons().at(j)->clear();
					buttonFree.at(j) = false;
					pairAngle = ship->getButtons().at(j)->getAngle();
					break;
				}
			}
			if (ship->getButtons().at(i)->getPair() == nullptr) {
				continue;
			} else {
				ship->getButtons().at(i)->setAngle(angle);
				ship->getButtons().at(i)->clear();
				buttonFree.at(i) = false;
				pairID = ship->getButtons().at(i)->getPairID();
				mib->createButtonTask(angle, i, pairAngle, pairID);
				break;
			}
		}
	}

	if (rand() % 15 == 1 && !ship->getChallenge()) {
		((int)(rand() % 2 == 0)) ? ship->setRollDir(0) : ship->setRollDir(1);
		int p = (int)(rand() % ship->getDonuts().size());
		if (p != playerID && ship->getDonuts().at(p)->getIsActive()) {
			mib->createAllTask(p, ship->getRollDir());
		} else {
			ship->setChallengeProg(0);
			ship->setEndTime((ship->timer) - 6);
			ship->setChallenge(true);
		}
	}
}

/**
 * Clears all events
 */
void GLaDOS::clear() {
	for (int i = 0; i < maxEvents; i++) {
		ship->getBreaches().at(i) = nullptr;
		breachFree.at(i) = true;
	}
	numEvents = 0;
}
