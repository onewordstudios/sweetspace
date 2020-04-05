#ifndef __GM_CONTROLLER_H__
#define __GM_CONTROLLER_H__

#include <cugl/cugl.h>

#include "BreachModel.h"
#include "DonutModel.h"
#include "DoorModel.h"
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

	/** Current number of breaches on ship */
	unsigned int numEvents;

	/** Current player ID */
	unsigned int playerID;

	/** The state of the ship */
	std::shared_ptr<ShipModel> ship;

	/** Network Controller for outbound calls */
	shared_ptr<MagicInternetBox> mib;

	bool allChallenge;

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
	bool init(std::shared_ptr<ShipModel> ship);

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
	void setPlayerId(int value) { playerID = value; }

	/**
	 * Gets the current player id of this gm.
	 */
	int getPlayerId() { return (int)playerID; }

	/**
	 * Gets if all player challenge is in effect.
	 */
	bool allPlayerChallenge() { return allChallenge; }
	/**
	 * Sets if all player challenge is in effect.
	 */
	void setAllPlayerChallenge(bool b) { allChallenge = b; }
};
#endif /* __GM_CONTROLLER_H__ */
