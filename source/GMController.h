#ifndef __GM_CONTROLLER_H__
#define __GM_CONTROLLER_H__

#include <cugl/cugl.h>

#include "BreachModel.h"

/**
 * This class represents the GM of the game
 */
class GMController {
   private:
	/** Whether or not this input is active */
	bool active;

	/** Current number of breaches on ship */
	unsigned int numEvents;

	/** Current list of breaches on ship*/
	std::vector<std::shared_ptr<BreachModel>> breaches;

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
	bool init(std::vector<std::shared_ptr<BreachModel>> breaches);

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
};
#endif /* __GM_CONTROLLER_H__ */
