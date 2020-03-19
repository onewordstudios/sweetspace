#ifndef __GM_CONTROLLER_H__
#define __GM_CONTROLLER_H__

#include <cugl/cugl.h>

#include "BreachModel.h"
#include "DonutModel.h"
#include "DoorModel.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"

/**
 * This class represents the GM of the game
 */
class GMController {
   private:
	/** Whether or not this input is active */
	bool active;

	/** Current number of breaches on ship */
	unsigned int numEvents;

	/** PlayerId owning this GMController. -1 means no player id assigned yet. */
	unsigned int playerId;

	/** Current breaches on ship */
	std::vector<std::shared_ptr<DonutModel>> donuts;

	/** Current breaches on ship */
	std::vector<std::shared_ptr<BreachModel>> breaches;

	/** Current doors on ship */
	std::vector<std::shared_ptr<DoorModel>> doors;

	/** Network Controller for outbound calls */
	shared_ptr<MagicInternetBox> mib;

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new GM controller.
	 *
	 * This constructor does NOT do any initialization.  It simply allocates the
	 * object. This makes it safe to use this class without a pointer.
	 */
	GMController(); // Don't initialize.  Allow stack based

	/**
	 * Disposes of this GM controller, releasing all resources.
	 */
	~GMController() { dispose(); }

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
	bool init(std::vector<std::shared_ptr<DonutModel>> donuts,
			  std::vector<std::shared_ptr<BreachModel>> breaches,
			  std::vector<std::shared_ptr<DoorModel>> doors, shared_ptr<MagicInternetBox>& mib,
			  int playerId);

#pragma mark -
#pragma mark GM Handling
	/**
	 * Returns true if the GM is currently active
	 *
	 * @return true if the GM is currently active
	 */
	bool isActive() const { return active; }

	/**
	 * Processes the GM.
	 *
	 * This method is used to generate and manage current events
	 */
	void update(float dt);

	/**
	 * Clears all events
	 */
	void clear();

#pragma mark -
#pragma mark Accessors

	/**
	 * Sets the current player id of this gm.
	 *
	 * @param health New gm player id.
	 */
	void setPlayerId(int value) { playerId = value; }

	/**
	 * Gets the current player id of this gm.
	 *
	 */
	int getPlayerId() { return playerId; }

	/**
	 * Sets the donut vector to new donut vector
	 *
	 * @param donuts New donut vector.
	 */
	void setDonuts(std::vector<std::shared_ptr<DonutModel>> donuts) { this->donuts = donuts; }
};
#endif /* __GM_CONTROLLER_H__ */
