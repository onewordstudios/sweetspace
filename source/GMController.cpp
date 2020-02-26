#include "GMController.h"

#include <array>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark GM Constants
/** The maximum number of events on ship at any one time. This will probably need to scale with
 * the number of players*/
const int MAX_EVENTS = 3;
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
	// Removing breaches that have been flagged as resolved
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (breaches.at(i) == nullptr) {
			continue;
		}
		if (breaches.at(i)->getIsResolved() == true) {
			breaches.at(i)->setAngle(0);
			breachFree.at(i) = true;
			breaches.at(i)->setIsResolved(false);
			numEvents--;
			// CULog("Remove Breach");
		}
	}

	// Simple logic for adding a breach when under max and randomly, replace with actual logic later
	if (rand() % 100 > 1) return;
	if (numEvents < MAX_EVENTS) {
		for (int i = 0; i < MAX_EVENTS; i++) {
			if (breachFree.at(i) == true) {
				breaches.at(i)->setAngle((rand() % 360) * (float)M_PI / 180.0f);
				// breaches.at(i)->setIsResolved(false);
				breachFree.at(i) = false;
				numEvents++;
				// CULog("Add Breach");
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
