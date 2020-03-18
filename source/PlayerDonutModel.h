#ifndef __PLAYER_DONUT_MODEL_H__
#define __PLAYER_DONUT_MODEL_H__
#include <cugl/cugl.h>

#include "DonutModel.h"

/**
 * The model for the donut representing the current player.
 */
class PlayerDonutModel : public DonutModel {
   public:
#pragma region Static Constructors
	/**
	 * Returns a newly allocated donut at the origin.
	 *
	 * This is a static constructor. You call it with the DonutModel::alloc().
	 * We prefer static constructors as they make the usage of shared pointers
	 * much simpler (and prevent the temptation of making a weak pointer on
	 * the heap).
	 *
	 * @return a newly allocated donut at the origin.
	 */
	static std::shared_ptr<DonutModel> alloc() {
		std::shared_ptr<PlayerDonutModel> result = std::make_shared<PlayerDonutModel>();
		return (result->DonutModel::init() ? result : nullptr);
	}

	/**
	 * Returns a newly allocated donut at the given position.
	 *
	 * This is a static constructor. You call it with the DonutModel::alloc().
	 * We prefer static constructors as they make the usage of shared pointers
	 * much simpler (and prevent the temptation of making a weak pointer on
	 * the heap).
	 *
	 * @param pos   Initial position in world coordinates
	 *
	 * @return a newly allocated donut at the given position.
	 */
	static std::shared_ptr<DonutModel> alloc(const cugl::Vec2& pos) {
		std::shared_ptr<PlayerDonutModel> result = std::make_shared<PlayerDonutModel>();
		return (result->init(pos) ? result : nullptr);
	}
#pragma endregion
	/**
	 * Updates the state of the model
	 *
	 * This method moves the donut forward, dampens the forces (if necessary)
	 * and updates the sprite if it exists.
	 *
	 * @param timestep  Time elapsed (in seconds) since last called.
	 */
	void update(float timestep = 0.0f) override;
};

#endif