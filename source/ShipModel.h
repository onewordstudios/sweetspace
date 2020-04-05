#ifndef __SHIP_MODEL_H__
#define __SHIP_MODEL_H__
#include <cugl/cugl.h>

#include "BreachModel.h"
#include "DonutModel.h"
#include "DoorModel.h"
class ShipModel {
   private:
   protected:
	/** Current list of breaches on ship*/
	std::vector<std::shared_ptr<DonutModel>> donuts;
	/** Current list of breaches on ship*/
	std::vector<std::shared_ptr<BreachModel>> breaches;
	/** Current list of doors on ship*/
	std::vector<std::shared_ptr<DoorModel>> doors;
	/** Current health of the ship*/
	float health;
	/** Size of the ship. Minimum value should be 360. Default value 360 */
	float shipSize;

   public:
	/** Current health of the ship*/
	float timer;
#pragma mark Constructors
	/*
	 * Creates a ship.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	ShipModel(void) : donuts(0), breaches(0), doors(0), health(0) {}

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
	 *
	 * @return true if the model is initialized properly, false otherwise.
	 */
	bool init(unsigned int numPlayers, unsigned int numBreaches, unsigned int numDoors,
			  unsigned int playerID) {
		return init(numPlayers, numBreaches, numDoors, playerID, (float)360);
	}

	/**
	 * Initializes ship model.
	 *
	 * @param numPlayers  The number of players in this ship
	 * @param numBreaches The number of breaches in this ship
	 * @param numDoors    The number of doors in this ship
	 * @param playerID    The ID of the current local player
	 * @param shipSize		  The size of the level
	 *
	 * @return true if the model is initialized properly, false otherwise.
	 */
	bool init(unsigned int numPlayers, unsigned int numBreaches, unsigned int numDoors,
			  unsigned int playerID, float shipSize);

	/**
	 * Create and return a shared pointer to a new ship model.
	 *
	 * @param numPlayers  The number of players in this ship
	 * @param numBreaches The number of breaches in this ship
	 * @param numDoors    The number of doors in this ship
	 * @param playerID    The ID of the current local player
	 *
	 * @return A smart pointer to a newly initialized ship model
	 */
	static std::shared_ptr<ShipModel> alloc(unsigned int numPlayers, unsigned int numBreaches,
											unsigned int numDoors, unsigned int playerID) {
		std::shared_ptr<ShipModel> result = std::make_shared<ShipModel>();
		return (result->init(numPlayers, numBreaches, numDoors, playerID) ? result : nullptr);
	}

	/**
	 * Create and return a shared pointer to a new ship model.
	 *
	 * @param numPlayers  The number of players in this ship
	 * @param numBreaches The number of breaches in this ship
	 * @param numDoors    The number of doors in this ship
	 * @param playerID    The ID of the current local player
	 * @param shipSize	  The size of the level
	 *
	 * @return A smart pointer to a newly initialized ship model
	 */
	static std::shared_ptr<ShipModel> alloc(unsigned int numPlayers, unsigned int numBreaches,
											unsigned int numDoors, unsigned int playerID,
											float shipSize) {
		std::shared_ptr<ShipModel> result = std::make_shared<ShipModel>();
		return (result->init(numPlayers, numBreaches, numDoors, playerID, shipSize) ? result
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
	 * Resolve breach with given id.
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
	 * Flag door with given id.
	 *
	 * @param id   the id of door to be opened.
	 * @param player   the player id flagging the door.
	 * @param flag   the flag to set (on or off, 1 or 0)
	 */
	bool flagDoor(int id, int player, int flag);

	/**
	 * Close door with given id.
	 *
	 * @param id   the id of door to be closed.
	 */
	bool closeDoor(int id);

	/**
	 * Set health of the ship
	 *
	 * @param health   the health of the ship
	 */
	void setHealth(float health) { this->health = health; }

	/**
	 * Get health of the ship
	 *
	 * @return health  the health of the ship
	 */
	float getHealth() { return health; }

    /**
     * Decrement health of the ship
     *
     * @param health   the amount of health to be decremented
     */
    void decHealth(float h) { health = health - h; }

	/**

	 * Get health of the ship
	 *
	 * @return health the health of the ship
	 */
	void initTimer(float startTime) { timer = startTime; }

	/**
	 * Update timer of the ship
	 *
	 * @param time amount of time to detract from timer
	 */
	void updateTimer(float time) { timer = timer - time; }

	/**
	 * Get if timer has ended
	 *
	 * @return if timer has ended
	 */
	bool timerEnded() { return timer < 1; }
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
};
#endif /* __SHIP_MODEL_H__ */
