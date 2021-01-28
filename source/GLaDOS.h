#ifndef GM_CONTROLLER_H
#define GM_CONTROLLER_H

#include <cugl/cugl.h>

#include <random>

#include "BreachModel.h"
#include "DonutModel.h"
#include "DoorModel.h"
#include "Globals.h"
#include "LevelModel.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"
#include "TutorialConstants.h"

/**
 * Game Logic and Distribution Operations Synthesizer
 * The controller class responsible for generating the challenges in the game.
 */
class GLaDOS {
   private:
	/** Whether or not this input is active */
	bool active;

	/** Random number generator */
	std::minstd_rand rand;

	/** The state of the ship */
	std::shared_ptr<ShipModel> ship;

	/** Network Controller for outbound calls */
	MagicInternetBox& mib;

	/** The maximum number of events on ship at any one time. This will probably need to scale with
	 * the number of players*/
	unsigned int maxEvents;
	/** The level number, for tutorial only*/
	int levelNum;
	/** The "things" (custom events) in this level, for tutorial only*/
	int customEventCtr;
	/** sections for tutorial only */
	int sections;
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
	/** Time we started the stabilizer (for tutorial only) */
	float stabilizerStart;

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
	bool init(const std::shared_ptr<ShipModel>& ship, const std::shared_ptr<LevelModel>& level);

	/**
	 * Initializes the GM for tutorial levels
	 *
	 * This method works like a proper constructor, initializing the GM
	 * controller and allocating memory.
	 *
	 * @return true if the controller was initialized successfully
	 */
	bool init(const std::shared_ptr<ShipModel>& ship, int levelNum);

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
	 * Places an object in the game. Requires that enough resources are present.
	 *
	 * @param obj the object to place
	 * @param zeroAngle the angle corresponding to the relative angle zero
	 * @param p the id to use for the player
	 */
	void placeObject(BuildingBlockModel::Object obj, float zeroAngle, int p);

	void placeButtons(float angle1, float angle2);

	/**
	 * Processes the GM.
	 *
	 * This method is used to generate and manage current events
	 */
	void update(float dt);

	void tutorialLevels(float dt);

#pragma mark -
};
#endif /* GM_CONTROLLER_H */
