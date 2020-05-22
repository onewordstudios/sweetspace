#ifndef __SHIP_MODEL_H__
#define __SHIP_MODEL_H__
#include <cugl/cugl.h>

#include "BreachModel.h"
#include "ButtonModel.h"
#include "DonutModel.h"
#include "DoorModel.h"
#include "Globals.h"
#include "Unopenable.h"

class ShipModel {
   private:
   protected:
	/** Current list of breaches on ship*/
	std::vector<std::shared_ptr<DonutModel>> donuts;
	/** Current list of breaches on ship*/
	std::vector<std::shared_ptr<BreachModel>> breaches;
	/** Current list of doors on ship*/
	std::vector<std::shared_ptr<DoorModel>> doors;
	/** Current list of doors on ship*/
	std::vector<std::shared_ptr<Unopenable>> unopenable;
	/** Current list of doors on ship*/
	std::vector<std::shared_ptr<ButtonModel>> buttons;
	/** Initial health of the ship*/
	float initHealth;
	/** Current health of the ship*/
	float health;
	/** Size of the ship. Minimum value should be 360. Default value 360 */
	float shipSize;
	/** Roll direction of all player challenge*/
	int rollDir;
	/** If a all player challenge is in effect*/
	bool challenge;
	/** If this level has no time limit*/
	bool timeless;
	/** Challenge progress*/
	int challengeProg;
	/** Time at which stabilizer challenge ends */
	float endTime;
	/** Total level time*/
	float totalTime;
	/** If is in tutorial level*/
	int levelNum;
	static constexpr int MIN_DISTANCE = 15;

   public:
	enum StabilizerStatus { INACTIVE, ACTIVE, FAILURE, SUCCESS, ANIMATING };
	/** StabilizerStatus of all player challenge. 0 = no challenge, 1 = challenge, 2 = challenge
	 * failed, 3 = challenge success*/
	StabilizerStatus stabilizerStatus;
	/** Game countdown timer, contains time left in level */
	float timeLeftInTimer;
	/** Time elapsed since level start */
	float canonicalTimeElapsed;
#pragma mark Constructors
	/*
	 * Creates a ship.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	ShipModel(void)
		: donuts(0),
		  breaches(0),
		  doors(0),
		  initHealth(0),
		  health(0),
		  shipSize(0),
		  rollDir(0),
		  challenge(false),
		  timeless(false),
		  challengeProg(0),
		  endTime(0),
		  totalTime(0),
		  levelNum(0),
		  stabilizerStatus(INACTIVE),
		  timeLeftInTimer(0),
		  canonicalTimeElapsed(0) {}

	/**
	 * Destroys this breach, releasing all resources.
	 */
	~ShipModel(void) { dispose(); }

	/**
	 * Disposes all resources and assets of this breach
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a breach may not be used until it is initialized again.
	 */
	void dispose();

	/**
	 * Initializes ship model.
	 *
	 * @param numPlayers  The number of players in this ship
	 * @param numBreaches The number of breaches in this ship
	 * @param numDoors    The number of doors in this ship
	 * @param playerID    The ID of the current local player
	 * @param initHealth    The initial health of the ship
	 * @param numButtons    The number of buttons on the ship
	 *
	 * @return true if the model is initialized properly, false otherwise.
	 */
	bool init(unsigned int numPlayers, unsigned int numBreaches, unsigned int numDoors,
			  unsigned int playerID, int initHealth, unsigned int numButtons) {
		return init(numPlayers, numBreaches, numDoors, playerID, globals::DEG_ORIG_CIRCLE,
					initHealth, numButtons);
	}

	/**
	 * Initializes ship model.
	 *
	 * @param numPlayers  The number of players in this ship
	 * @param numBreaches The number of breaches in this ship
	 * @param numDoors    The number of doors in this ship
	 * @param playerID    The ID of the current local player
	 * @param shipSize		  The size of the level
	 * @param initHealth    The initial health of the ship
	 * @param numButtons    The number of buttons on the ship
	 *
	 * @return true if the model is initialized properly, false otherwise.
	 */
	bool init(unsigned int numPlayers, unsigned int numBreaches, unsigned int numDoors,
			  unsigned int playerID, float shipSize, int initHealth, unsigned int numButtons);

	bool init(unsigned int numPlayers, unsigned int numBreaches, unsigned int numDoors,
			  unsigned int playerID, float shipSize, int initHealth, unsigned int numButtons,
			  int numUnop) {
		// Instantiate door models
		for (unsigned int i = 0; i < numUnop; i++) {
			unopenable.push_back(std::make_shared<Unopenable>());
		}
		return init(numPlayers, numBreaches, numDoors, playerID, shipSize, initHealth, numButtons);
	}

	/**
	 * Create and return a shared pointer to a new ship model.
	 *
	 * @param numPlayers  The number of players in this ship
	 * @param numBreaches The number of breaches in this ship
	 * @param numDoors    The number of doors in this ship
	 * @param playerID    The ID of the current local player
	 * @param initHealth    The initial health of the ship
	 * @param numButtons    The number of buttons on the ship
	 *
	 * @return A smart pointer to a newly initialized ship model
	 */
	static std::shared_ptr<ShipModel> alloc(unsigned int numPlayers, unsigned int numBreaches,
											unsigned int numDoors, unsigned int playerID,
											int initHealth, unsigned int numButtons) {
		std::shared_ptr<ShipModel> result = std::make_shared<ShipModel>();
		return (result->init(numPlayers, numBreaches, numDoors, playerID, initHealth, numButtons)
					? result
					: nullptr);
	}

	/**
	 * Create and return a shared pointer to a new ship model.
	 *
	 * @param numPlayers  The number of players in this ship
	 * @param numBreaches The number of breaches in this ship
	 * @param numDoors    The number of doors in this ship
	 * @param playerID    The ID of the current local player
	 * @param shipSize	  The size of the level
	 * @param initHealth    The initial health of the ship
	 * @param numButtons    The number of buttons on the ship
	 *
	 * @return A smart pointer to a newly initialized ship model
	 */
	static std::shared_ptr<ShipModel> alloc(unsigned int numPlayers, unsigned int numBreaches,
											unsigned int numDoors, unsigned int playerID,
											float shipSize, int initHealth,
											unsigned int numButtons) {
		std::shared_ptr<ShipModel> result = std::make_shared<ShipModel>();
		return (result->init(numPlayers, numBreaches, numDoors, playerID, shipSize, initHealth,
							 numButtons)
					? result
					: nullptr);
	}

#pragma mark -
#pragma mark Accessors
	/**
	 * Returns the list of donuts.
	 *
	 * @return the list of donuts.
	 */
	std::vector<std::shared_ptr<DonutModel>>& getDonuts() { return donuts; }

	/**
	 * Returns the current list of breaches.
	 *
	 * @return the current list of breaches.
	 */
	std::vector<std::shared_ptr<BreachModel>>& getBreaches() { return breaches; }

	/**
	 * Returns the current list of doors.
	 *
	 * @return the current list of doors.
	 */
	std::vector<std::shared_ptr<DoorModel>>& getDoors() { return doors; }

	/**
	 * Returns the current list of doors.
	 *
	 * @return the current list of doors.
	 */
	std::vector<std::shared_ptr<Unopenable>>& getUnopenable() { return unopenable; }

	/**
	 * Returns the current list of buttons.
	 *
	 * @return the current list of buttons.
	 */
	std::vector<std::shared_ptr<ButtonModel>>& getButtons() { return buttons; }

	/**
	 * Create breach.
	 *
	 * @param angle	   the location to create the breach.
	 * @param health   the health of the breach.
	 * @param player   the player assigned to the breach.
	 */
	bool createBreach(float angle, int health, int player);

	/**
	 * Create breach with given id.
	 *
	 * @param angle	   the location to create the breach.
	 * @param health   the health of the breach.
	 * @param player   the player assigned to the breach.
	 * @param id   	   the id of breach to be created.
	 */
	bool createBreach(float angle, int health, int player, int id);

	/**
	 * Decrement the health of a breach with given id.
	 *
	 * @param id   the id of breach to be resolved.
	 */
	bool resolveBreach(int id);

	/**
	 * Create door with given id.
	 *
	 * @param angle	   the location to create the door.
	 * @param id   	   the id of door to be created.
	 */
	bool createDoor(float angle, int id);

	/**
	 * Create door with given id.
	 *
	 * @param angle	   the location to create the door.
	 * @param id   	   the id of door to be created.
	 */
	bool createUnopenable(float angle, int id);

	/**
	 * Flag door with given id.
	 *
	 * @param id   the id of door to be opened.
	 * @param player   the player id flagging the door.
	 * @param flag   the flag to set (on or off, 1 or 0)
	 */
	bool flagDoor(int id, int player, int flag);

	/**
	 * Set health of the ship
	 *
	 * @param health   the health of the ship
	 */
	void setHealth(float health) { this->health = health < 0 ? 0 : health; }

	/**
	 * Get health of the ship
	 *
	 * @return health  the health of the ship
	 */
	float getHealth() { return health; }

	void setTimeless(bool t) { timeless = t; }
	bool getTimeless() { return timeless; }

	/**
	 * Get health of the ship
	 *
	 * @return health  the health of the ship
	 */
	float getInitHealth() { return initHealth; }

	/**
	 * Decrement health of the ship
	 *
	 * @param health   the amount of health to be decremented
	 */
	void decHealth(float h) { setHealth(health - h); }

	/**

	 * Initialize the timer for the ship
	 *
	 * @param startTime the initial time on the timer
	 */
	void initTimer(float startTime) {
		timeLeftInTimer = startTime;
		totalTime = startTime;
		canonicalTimeElapsed = 0;
	}

	/**
	 * Update timer of the ship
	 *
	 * @param time amount of time to detract from timer
	 */
	void updateTimer(float time, bool isTimeLeftUpdated) {
		canonicalTimeElapsed += time;
		if (!timeless && isTimeLeftUpdated) {
			timeLeftInTimer -= time;
		}
	}

	/**
	 * Get if timer has ended
	 *
	 * @return if timer has ended
	 */
	bool timerEnded() { return timeLeftInTimer < 1; }

	/**
	 * Returns whether the level has ended (won or lost)
	 */
	bool isLevelOver() { return timerEnded() || health <= 0; }

	/**
	 * Get the amount of time that has passed in the level
	 *
	 * @return the time that has passed
	 */
	float timePassed() { return canonicalTimeElapsed; }

	/**
	 * Set size of the ship
	 *
	 * @param the size of the ship
	 */
	void setSize(float s) { shipSize = s; };

	/**
	 * Get size of the ship
	 *
	 * @return the size of the ship
	 */
	float getSize() { return shipSize; }

	/**
	 * Set roll direction of challenge
	 *
	 * @param direction of roll
	 */
	void setRollDir(int dir) { rollDir = dir; }

	/**
	 * Get roll direction of challenge
	 *
	 * @return direction of roll
	 */
	int getRollDir() { return rollDir; }

	/**
	 * Set true if challenge.
	 *
	 * @param if challenge
	 */
	void setChallenge(bool c) { challenge = c; }

	/**
	 * Get if in challenge.
	 *
	 * @return if in challenge
	 */
	bool getChallenge() { return challenge; }

	/**
	 * Set challenge progress.
	 *
	 * @param starting progress
	 */
	void setChallengeProg(int p) { challengeProg = p; }

	/**
	 * Get challenge progress
	 *
	 * @return challenge progress
	 */
	int getChallengeProg() { return challengeProg; }

	/**
	 * Update challenge progress
	 */
	void updateChallengeProg() { challengeProg = challengeProg + 1; }
	/**
	 * Set data for challenge
	 */
	bool createAllTask(int data);

	/**
	 * Set fail challenge
	 */
	bool failAllTask();

	/**
	 * Set challenge status
	 */
	void setStabilizerStatus(StabilizerStatus s);

	/**
	 * Set end time for challenge
	 */
	void setEndTime(float time) { endTime = time; };

	/**
	 * Get end time for challenge
	 */
	float getEndTime() { return endTime; }

	/**
	 * Create button with given id.
	 *
	 * @param angle1	   the location to create the button.
	 * @param id1  	       the id of button to be created.
	 * @param angle2       the location to create the button's pair.
	 * @param id2  	       the id of button's pair to be created.
	 */
	bool createButton(float angle1, int id1, float angle2, int id2);

	/**
	 * Flag button with given id.
	 *
	 * @param id   the id of button to press.
	 *
	 * @return true iff the button was successfully
	 * triggered (ie was not called during the
	 * i-frames after the last call to trigger)
	 */
	bool flagButton(int id);

	/**
	 * Resolve a button pair
	 *
	 * @param id The ID of one of the two buttons
	 */
	void resolveButton(int id);

	/**
	 * Gets if level is tutorial
	 */
	int getLevelNum() { return levelNum; }

	/**
	 * Gets status of challenge
	 */
	StabilizerStatus getStabilizerStatus() { return stabilizerStatus; }

	/**
	 * Sets if level is tutorial
	 */
	void setLevelNum(int l) { levelNum = l; }

	/**
	 * Separates each donut into their own section
	 */
	void separateDonuts() {
		for (int i = 0; i < getDonuts().size(); i++) {
			float angle = getSize() * (float)i / getDonuts().size();
			getDonuts().at(i)->setAngle(angle);
		}
	}

#pragma mark -
#pragma mark Helpers
	float getAngleDifference(float angle1, float angle2) {
		return shipSize / 2 - abs(abs(angle1 - angle2) - shipSize / 2);
	}
};
#endif /* __SHIP_MODEL_H__ */
