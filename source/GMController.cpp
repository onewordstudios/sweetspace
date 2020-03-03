#include "GMController.h"

#include <array>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark GM Constants
/** The maximum number of events on ship at any one time. This will probably need to scale with
 * the number of players*/
const unsigned int MAX_EVENTS = 3;
/** Spawn rate of breaches = 1/SPAWN_RATE for EVERY UPDATE FRAME. 100 is a very fast rate already.
 */
const unsigned int SPAWN_RATE = 100;
/** Default Max Health of a Breach*/
constexpr unsigned int HEALTH_DEFAULT = 3;
constexpr float HALF_CIRCLE = 180.0f;
constexpr unsigned int FULL_CIRCLE = 360;

/** Array recording which breaches are free or not. */
array<bool, MAX_EVENTS> breachFree;

#pragma mark -
#pragma mark GM
/**
 * Creates a new GM controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
GMController::GMController() : active(false), numEvents(0) {}

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
bool GMController::init(const std::vector<std::shared_ptr<BreachModel>> b) {
	bool success = true;
	breaches = b;
	for (int i = 0; i < MAX_EVENTS; i++) {
		breachFree.at(i) = true;
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
		if (!breachFree.at(i) && breaches.at(i)->getHealth() == 0) {
			breaches.at(i)->setAngle(-1);
			breachFree.at(i) = true;
		}
	}

	// Simple logic for adding a breach when under max and randomly, replace with actual logic later
	if (rand() % SPAWN_RATE > 1) return;
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (breachFree.at(i)) {
			breaches.at(i)->setAngle((rand() % FULL_CIRCLE) * (float)M_PI / HALF_CIRCLE);
			breaches.at(i)->setHealth(HEALTH_DEFAULT);
			breachFree.at(i) = false;
			break;
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
