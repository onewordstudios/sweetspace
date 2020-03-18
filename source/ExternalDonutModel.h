#ifndef __EXTERN_DONUT_MODEL_H__
#define __EXTERN_DONUT_MODEL_H__
#include <cugl/cugl.h>

#include "DonutModel.h"

class ExternalDonutModel : public DonutModel {
   private:
	/**
	 * Struct describing data needed to interpolate between network ticks
	 */
	struct NetworkMovementData {
		/**
		 * Number of frames passed since the last network update.
		 * If greater than or equal to NETWORK_TICK, then position should be aligned.
		 */
		unsigned int framesSinceUpdate;
		/**
		 * The actual angle of the donut, computed from the last network update position.
		 */
		float angle;

		/**
		 * The angle of the donut computed from its local position during the last network update.
		 */
		float oldAngle;

		// The angle of the donut exposed to the world is linearly interpolated between oldAngle and
		// angle as framesSinceUpdate increases.
	};

	/**
	 * Data used by the network controller to ease movement for non-player donuts.
	 */
	NetworkMovementData networkMove;

   public:
	ExternalDonutModel(void) : DonutModel(), networkMove(){};

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
		std::shared_ptr<ExternalDonutModel> result = std::make_shared<ExternalDonutModel>();
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
		std::shared_ptr<ExternalDonutModel> result = std::make_shared<ExternalDonutModel>();
		return (result->init(pos) ? result : nullptr);
	}
#pragma endregion

	bool init(const cugl::Vec2& pos) override;
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