#include "GMController.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark GM Factors

/** The maximum number of events on ship at any one time. This will probably need to scale with the
 * number of players*/
constexpr unsigned int MAX_EVENTS = 3;

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
	// Simple logic for adding breaches when under max, replace with actual logic later
	if (rand() % 1000 > 1) return;
	if (numEvents < MAX_EVENTS) {
		breaches.at(numEvents)->setAngle((rand() % 360) * (float)M_PI / 180.0f);
		numEvents++;
	}
}

/**
 * Clears all events
 */
void GMController::clear() { numEvents = 0; }
