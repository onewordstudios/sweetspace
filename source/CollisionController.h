#ifndef COLLISION_CONTROLLER
#define COLLISION_CONTROLLER

#include <cugl/cugl.h>

#include "ShipModel.h"

class CollisionController {
   public:
	static void updateCollisions(ShipModel& ship, uint8_t playerID);
};
#endif // COLLISION_CONTROLLER
