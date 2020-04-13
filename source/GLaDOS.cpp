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
/** Array recording which breaches are free or not. */
vector<bool> breachFree;
/** Array recording which doors are free or not. */
vector<bool> doorFree;
/** List of building blocks for this level*/
map<std::string, std::shared_ptr<BuildingBlockModel>> blocks;
/** List of events for this level*/
vector<std::shared_ptr<EventModel>> events;
/** List of events that are ready to be executed*/
vector<std::shared_ptr<EventModel>> readyQueue;

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
	breachFree.resize(maxEvents);
	doorFree.resize(maxDoors);
	blocks = level->getBlocks();
	events = level->getEvents();

	for (int i = 0; i < maxEvents; i++) {
		breachFree.at(i) = true;
	}
	for (int i = 0; i < maxDoors; i++) {
		doorFree.at(i) = true;
	}
	challengeInProg = false;
	// Set random seed based on time
	srand((unsigned int)time(NULL));
	active = success;
	return success;
}

/**
 * Places an object in the game. Requires that enough resources are present.
 *
 * @param obj the object to place
 * @param zeroAngle the angle corresponding to the relative angle zero
 * @param ids a vector of relative ids, scrambled by the caller
 */
void GLaDOS::placeObject(BuildingBlockModel::Object obj, float zeroAngle, vector<int> ids) {
	int i = 0;
	int p = obj.player == -1 ? (int)(rand() % ship->getDonuts().size()) : ids.at(obj.player);
	switch (obj.type) {
		case BuildingBlockModel::Breach:
			i = distance(breachFree.begin(), find(breachFree.begin(), breachFree.end(), true));
			breachFree.at(i) = false;
			ship->getBreaches().at(i)->reset((float)obj.angle + zeroAngle, p);
			ship->getBreaches().at(i)->setTimeCreated(ship->timer);
			mib->createBreach((float)obj.angle + zeroAngle, p, i);
			break;
		case BuildingBlockModel::Door:
			i = distance(doorFree.begin(), find(doorFree.begin(), doorFree.end(), true));
			ship->getDoors().at(i)->setAngle((float)obj.angle + zeroAngle);
			ship->getDoors().at(i)->clear();
			doorFree.at(i) = false;
			mib->createDualTask((float)obj.angle + zeroAngle, -1, -1, i);
			break;
		case BuildingBlockModel::Button:
			break;
		case BuildingBlockModel::Roll:
			if (ship->getChallenge()) break;
			((int)(rand() % 2 == 0)) ? ship->setRollDir(0) : ship->setRollDir(1);
			if (p != playerID && ship->getDonuts().at(p)->getIsActive()) {
				mib->createAllTask(p, ship->getRollDir());
			} else {
				ship->setChallengeProg(0);
				ship->setEndTime((ship->timer) - globals::ROLL_CHALLENGE_LENGTH);
				ship->setChallenge(true);
			}
			break;
	}
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

	// Check if this is the host for generating breaches and doors
	if (playerID != 0) {
		return;
	}

	for (int i = 0; i < events.size(); i++) {
		std::shared_ptr<EventModel> event = events.at(i);
		int spawnRate = (int)(event->getProbability());
		CULog("%d rate and time %f", spawnRate, ship->timePassed());
		if (event->isActive((int)ship->timePassed()) && rand() % spawnRate <= 1) {
			CULog("event queued");
			// ready up the event
			readyQueue.push_back(event);
			if (event->isOneTime()) {
				// If it's a one time event, we don't want to add it again next frame
				events.erase(events.begin() + i);
				i--;
			}
		}
	}

	for (int i = 0; i < readyQueue.size(); i++) {
		CULog("nonempty ready queue!");
		shared_ptr<EventModel> event = readyQueue.at(i);
		shared_ptr<BuildingBlockModel> block = blocks.at(event->getBlock());
		int numBreachesFree = count(breachFree.begin(), breachFree.end(), true);
		int numDoorsFree = std::count(doorFree.begin(), doorFree.end(), true);
		vector<BuildingBlockModel::Object> objects = block->getObjects();
		int breachesNeeded = count_if(
			objects.begin(), objects.end(),
			[](BuildingBlockModel::Object o) { return o.type == BuildingBlockModel::Breach; });
		int doorsNeeded = count_if(
			objects.begin(), objects.end(),
			[](BuildingBlockModel::Object o) { return o.type == BuildingBlockModel::Door; });

		// If we don't have enough resources for this event, skip it
		if (doorsNeeded > numDoorsFree || breachesNeeded > numBreachesFree) continue;
		float angle = (float)(rand() % (int)(ship->getSize()));
		bool goodAngle = true;
		for (int j = 0; j < ship->getDonuts().size(); j++) {
			float diff =
				ship->getSize() / 2 -
				abs(abs(ship->getDonuts().at(j)->getAngle() - angle) - ship->getSize() / 2);
			if (diff < (float)block->getRange() / 2) {
				goodAngle = false;
				break;
			}
		}
		// Make sure it's not too close to other breaches
		for (unsigned int k = 0; k < ship->getBreaches().size(); k++) {
			float breachAngle = ship->getBreaches()[k]->getAngle();
			float diff = ship->getSize() / 2 - abs(abs(breachAngle - angle) - ship->getSize() / 2);
			if (breachAngle != -1 && diff < (float)block->getRange() / 2.0f) {
				goodAngle = false;
				break;
			}
		}
		if (!goodAngle) continue;
		// set angle to where zero is
		angle = angle - (float)block->getRange() / 2.0f - (float)block->getMin();
		// assign the relative player ids
		vector<int> ids;
		for (int j = 0; j < ship->getDonuts().size(); j++) {
			ids.push_back(j);
		}
		random_shuffle(ids.begin(), ids.end());
		for (int j = 0; j < objects.size(); j++) {
			CULog("placing objects...");
			placeObject(objects.at(j), angle, ids);
		}
		readyQueue.erase(readyQueue.begin() + i);
		break;
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
