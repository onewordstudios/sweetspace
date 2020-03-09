#ifndef __SHIP_MODEL_H__
#define __SHIP_MODEL_H__
#include <cugl/cugl.h>

#include "DonutModel.h"
#include "BreachModel.h"
class ShipModel {
   private:
   protected:
	/** Current list of breaches on ship*/
	std::vector<std::shared_ptr<DonutModel>> donuts;
	/** Current list of breaches on ship*/
	std::vector<std::shared_ptr<BreachModel>> breaches;
//	/** Current list of doors on ship*/
//	std::vector<std::shared_ptr<DoorModel>> doors;

   public:
#pragma mark Constructors
	/*
	 * Creates a ship.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	ShipModel(void): donuts(0), breaches(0) {}

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
	 * Initializes ship.
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(std::vector<std::shared_ptr<DonutModel>>& donuts, std::vector<std::shared_ptr<BreachModel>>& breaches);
	static std::shared_ptr<ShipModel> alloc(std::vector<std::shared_ptr<DonutModel>>& donuts, std::vector<std::shared_ptr<BreachModel>>& breaches) {
		std::shared_ptr<ShipModel> result = std::make_shared<ShipModel>();
		return (result->init(donuts, breaches) ? result : nullptr);
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
 * Returns the list of donuts.
 *
 * @return the list of donuts.
 */
	void setDonuts(const std::vector<std::shared_ptr<DonutModel>>& d) { donuts = d; }

	/**
	 * Returns the current list of breaches.
	 *
	 * @return the current list of breaches.
	 */
	void setBreaches(const std::vector<std::shared_ptr<BreachModel>>& b) { breaches = b; }

	/**
	 * Create breach.
	 */
	bool createBreach();

	/**
	 * Create breach with given id.
	 *
	 * @param id   the id of breach to be created.
	 */
	bool createBreach(int id);

	/**
	 * Resolve breach with given id.
	 *
	 * @param id   the id of breach to be resolved.
	 */
	bool resolveBreach(int id);

	/**
	 * Create door.
	 */
	bool createDoor();

	/**
	 * Create door with given id.
	 *
	 * @param id   the id of door to be created.
	 */
	bool createDoor(int id);

	/**
	 * Open door with given id.
	 *
	 * @param id   the id of door to be opened.
	 */
	bool openDoor(int id);

	/**
	 * Close door with given id.
	 *
	 * @param id   the id of door to be closed.
	 */
	bool closeDoor(int id);
};
#endif /* __SHIP_MODEL_H__ */
