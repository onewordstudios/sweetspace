#ifndef __GM_CONTROLLER_H__
#define __GM_CONTROLLER_H__

#include <cugl/cugl.h>

#include "BreachModel.h"
#include "DonutModel.h"
#include "DoorModel.h"
#include "Globals.h"
#include "LevelModel.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"

/**
 * Game Logic and Distribution Operations Synthesizer
 * The controller class responsible for generating the challenges in the game.
 */
class GLaDOS {
   private:
	/** Whether or not this input is active */
	bool active;

	/** Current player ID */
	unsigned int playerID;

	/** The state of the ship */
	std::shared_ptr<ShipModel> ship;

	/** Network Controller for outbound calls */
	shared_ptr<MagicInternetBox> mib;

	bool fail;

	/** The maximum number of events on ship at any one time. This will probably need to scale with
	 * the number of players*/
	unsigned int maxEvents;
	/** The maximum number of events on ship at any one time. This will probably need to scale with
	 * the number of players*/
	unsigned int maxDoors;
	/** The maximum number of buttons on ship at any one time. This will probably need to scale with
	 * the number of players*/
	unsigned int maxButtons;
	/** queue recording which breaches are free. */
	queue<int> breachFree;
	/** queue recording which doors are free. */
	queue<int> doorFree;
	/** queue recording which buttons are free. */
	queue<int> buttonFree;
	/** List of building blocks for this level*/
	map<std::string, std::shared_ptr<BuildingBlockModel>> blocks;
	/** List of events for this level*/
	vector<std::shared_ptr<EventModel>> events;
	/** List of events that are ready to be executed*/
	vector<std::shared_ptr<EventModel>> readyQueue;

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new GM controller.
	 *
	 * This constructor does NOT do any initialization.  It simply allocates the
	 * object. This makes it safe to use this class without a pointer.
	 */
	GLaDOS(); // Don't initialize.  Allow stack based

	/**
	 * Disposes of this GM controller, releasing all resources.
	 */
	~GLaDOS() { dispose(); }

	/**
	 * Deactivates this GM controller.
	 *
	 * This method will not dispose of the GM controller. It can be reused
	 * once it is reinitialized.
	 */
	void dispose();

	/**
	 * Initializes the GM
	 *
	 * This method works like a proper constructor, initializing the GM
	 * controller and allocating memory.
	 *
	 * @return true if the controller was initialized successfully
	 */
	bool init(std::shared_ptr<ShipModel> ship, std::shared_ptr<LevelModel> level);

#pragma mark -
#pragma mark GM Handling
	/**
	 * Returns true if the GM is currently active
	 *
	 * @return true if the GM is currently active
	 */
	bool isActive() const { return active; }

	/**
	 * Places an object in the game. Requires that enough resources are present.
	 *
	 * @param obj the object to place
	 * @param zeroAngle the angle corresponding to the relative angle zero
	 * @param ids a vector of relative ids, scrambled by the caller
	 */
	void placeObject(BuildingBlockModel::Object obj, float zeroAngle, vector<int> ids);

	/**
	 * Processes the GM.
	 *
	 * This method is used to generate and manage current events
	 */
	void update(float dt);

#pragma mark -
#pragma mark Accessors

	/**
	 * Sets the current player id of this gm.
	 *
	 * @param health New gm player id.
	 */
	void setPlayerId(int value) { playerID = value; }

	/**
	 * Gets the current player id of this gm.
	 */
	int getPlayerId() { return (int)playerID; }

	/**
	 * Gets if all player challenge failed.
	 */
	bool challengeFail() { return fail; }
	/**
	 * Sets if all player challenge failed.
	 */
	void setChallengeFail(bool b) { fail = b; }

};
#endif /* __GM_CONTROLLER_H__ */
