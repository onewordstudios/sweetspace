#include "GMController.h"

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
/** Default Max Health of a Breach*/
constexpr unsigned int HEALTH_DEFAULT = 3;
constexpr unsigned int FULL_CIRCLE = 360;
constexpr float MIN_ANGLE_DIFF = 0.5f;
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
GMController::GMController() : active(false), numEvents(0), playerId(0) {}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void GMController::dispose() {
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
bool GMController::init(std::vector<std::shared_ptr<DonutModel>> d,
						std::vector<std::shared_ptr<BreachModel>> b,
						std::vector<std::shared_ptr<DoorModel>> dr,
						std::shared_ptr<MagicInternetBox>& mib, int playerId) {
	bool success = true;
	donuts = d;
	breaches = b;
	doors = dr;
	this->mib = mib;
	this->playerId = playerId;
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
void GMController::update(float dt) {
	// Removing breaches that have 0 health left
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (breaches.at(i) == nullptr) {
			continue;
		}
		if (breaches.at(i)->getHealth() == 0) {
			breaches.at(i)->setAngle(-1);
			breachFree.at(i) = true;
		}
	}

	// Remove doors that have been resolved and opened. Also raise doors that are resolved.
	for (int i = 0; i < MAX_DOORS; i++) {
		if (doors.at(i) == nullptr) {
			continue;
		}
		if (doors.at(i)->resolvedAndRaised()) {
			doors.at(i)->setAngle(-1);
			doorFree.at(i) = true;
		} else if (doors.at(i)->resolved()) {
			doors.at(i)->raiseDoor();
		}
	}

	// Check if this is the host for generating breaches and doors
	if (playerId == 0) {
		// Simple logic for adding a breach when under max and randomly, replace with actual logic
		// later
		if (rand() % SPAWN_RATE > 1) return;
		for (int i = 0; i < MAX_EVENTS; i++) {
			if (breachFree.at(i)) {
				float angle = (rand() % FULL_CIRCLE) * (float)M_PI / HALF_CIRCLE;
				breaches.at(i)->setAngle(angle);
				breaches.at(i)->setHealth(HEALTH_DEFAULT);
				breachFree.at(i) = false;
				int p = rand() % donuts.size();
				breaches.at(i)->setPlayer(p);
				mib->createBreach(angle, p, i);
				break;
			}
		}
		for (int i = 0; i < MAX_DOORS; i++) {
			if (doorFree.at(i)) {
				float angle = (rand() % FULL_CIRCLE) * (float)M_PI / HALF_CIRCLE;
				bool goodAngle = true;
				for (int j = 0; j < donuts.size(); j++) {
					float diff =
						(float)M_PI - abs(abs(donuts.at(j)->getAngle() - angle) - (float)M_PI);
					if (diff < MIN_ANGLE_DIFF) {
						goodAngle = false;
						break;
					}
				}
				if (!goodAngle) {
					continue;
				}
				doors.at(i)->setAngle(angle);
				doors.at(i)->clear();
				doorFree.at(i) = false;
				mib->createDualTask(angle, -1, -1, i);
				break;
			}
		}
	}
}

/**
 * Clears all events
 */
void GMController::clear() {
	for (int i = 0; i < MAX_EVENTS; i++) {
		breaches.at(i) = nullptr;
		breachFree.at(i) = true;
	}
	numEvents = 0;
}
