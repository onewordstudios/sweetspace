#include "GLaDOS.h"

#include <array>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark GM Constants
/** The maximum number of events on ship at any one time. This will probably need to scale with
 * the number of players*/
const unsigned int MAX_EVENTS = 3;
/** The maximum number of events on ship at any one time. This will probably need to scale with
 * the number of players*/
const unsigned int MAX_DOORS = 1;
/** Spawn rate of breaches = 1/SPAWN_RATE for EVERY UPDATE FRAME. 100 is a very fast rate already.
 */
const unsigned int SPAWN_RATE = 100;
constexpr float MIN_ANGLE_DIFF = 29.0f;
/** Array recording which breaches are free or not. */
array<bool, MAX_EVENTS> breachFree;

/** Array recording which doors are free or not. */
array<bool, MAX_DOORS> doorFree;

#pragma mark -
#pragma mark GM
/**
 * Creates a new GM controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
GLaDOS::GLaDOS() : active(false), numEvents(0), playerID(0) {}

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
bool GLaDOS::init(std::shared_ptr<ShipModel> ship, std::shared_ptr<MagicInternetBox> mib,
				  float lvlSize) {
	bool success = true;
	this->ship = ship;
	this->mib = mib;
	this->playerID = mib->getPlayerID();
	size = lvlSize;
	for (int i = 0; i < MAX_EVENTS; i++) {
		breachFree.at(i) = true;
	}
	for (int i = 0; i < MAX_DOORS; i++) {
		doorFree.at(i) = true;
	}
	// Set random seed based on time
	srand(time(NULL));
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
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (ship->getBreaches().at(i) == nullptr) {
			continue;
		}
		if (ship->getBreaches().at(i)->getHealth() == 0) {
			ship->getBreaches().at(i)->setAngle(-1);
			breachFree.at(i) = true;
		}
	}

	// Remove doors that have been resolved and opened. Also raise doors that are resolved.
	for (int i = 0; i < MAX_DOORS; i++) {
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

	// Check if this is the host for generating breaches and doors
	if (playerID != 0) {
		return;
	}
	// Simple logic for adding a breach when under max and randomly, replace with actual logic
	// later
	if (rand() % SPAWN_RATE > 1) return;
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (breachFree.at(i)) {
			float angle = rand() % (int)size;
			bool goodAngle = true;
			for (int j = 0; j < ship->getDonuts().size(); j++) {
				float diff = (float)DonutModel::HALF_CIRCLE -
							 abs(abs(ship->getDonuts().at(j)->getAngle() - angle) -
								 (float)DonutModel::HALF_CIRCLE);
				if (diff < MIN_ANGLE_DIFF) {
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
				float diff = (float)DonutModel::HALF_CIRCLE -
							 abs(abs(breachAngle - angle) - (float)DonutModel::HALF_CIRCLE);
				if (breachAngle != -1 && diff < MIN_ANGLE_DIFF) {
					goodAngle = false;
					break;
				}
			}

			if (!goodAngle) {
				continue;
			}
			breachFree.at(i) = false;
			int p = rand() % ship->getDonuts().size();
			ship->getBreaches().at(i)->reset(angle, p);
			mib->createBreach(angle, p, i);
			break;
		}
	}
	for (int i = 0; i < MAX_DOORS; i++) {
		if (doorFree.at(i)) {
			float angle = rand() % (int)size;
			bool goodAngle = true;
			for (int j = 0; j < ship->getDonuts().size(); j++) {
				float diff = (float)DonutModel::HALF_CIRCLE -
							 abs(abs(ship->getDonuts().at(j)->getAngle() - angle) -
								 (float)DonutModel::HALF_CIRCLE);
				if (diff < MIN_ANGLE_DIFF) {
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
}

/**
 * Clears all events
 */
void GLaDOS::clear() {
	for (int i = 0; i < MAX_EVENTS; i++) {
		ship->getBreaches().at(i) = nullptr;
		breachFree.at(i) = true;
	}
	numEvents = 0;
}
