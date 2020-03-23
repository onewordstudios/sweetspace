#include "DonutModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Constructors

/**
 * Initializes a new donut with the given position
 *
 * This is an initializer.  It, combined with the constructor, produces the static
 * constructor create().  The initializer and normal constructor are private while
 * the static constructor is not.
 *
 * @param  pos  Initial position in world coordinates
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool DonutModel::init(const Vec2& pos) {
	sgPos = pos;
	// Set Initial jump Velocity based on gravity and max jump height
	jumpVelocity = sqrt(2 * GRAVITY * JUMP_HEIGHT);
	return true;
}

/**
 * Disposes all resources and assets of this donut
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a donut may not be used until it is initialized again.
 */
void DonutModel::dispose() {}

#pragma mark -
#pragma mark Animation

/**
 * Applies a force to the donut.
 *
 * @param value The donut turning force
 */
void DonutModel::applyForce(float value) { velocity += DONUT_MAX_FORCE * value; }

/**
 * Starts a fixed height jump for the donut.
 *
 * @param value The donut turning force
 */
void DonutModel::startJump() {
	jumping = true;
	jumpTime = 0.0f;
}

void DonutModel::updateJump(float timestep) {
	// Update jump offset depending on time passed since start of jump
	if (jumping) {
		jumpOffset = -GRAVITY / 2 * jumpTime * jumpTime + jumpVelocity * jumpTime;

		// Check for end of jump
		if (jumpTime > 0.0f && jumpOffset <= 0.0f) {
			jumpOffset = 0.0f;
			jumping = false;
		}
		jumpTime += timestep;
	}
}

/**
 * Resets the donut back to its original settings
 */
void DonutModel::reset() {
	angle = 0.0f;
	velocity = 0.0f;
	jumpOffset = 0.0f;
	jumpTime = 0.0f;
	jumpVelocity = 0.0f;
	jumping = false;
}
