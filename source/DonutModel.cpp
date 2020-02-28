#include "DonutModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Animation Constants and Functions
/** Factor to multiply the forward thrust */
constexpr unsigned int FULL_CIRCLE = 360;
/** The max turn (in degrees) per frame */
constexpr float DONUT_MAX_TURN = 1.0f;

/** Compute cos (in degrees) from 90 degrees */
#define DCOS_90(a) (cos(M_PI * (a + 90.0f) / 180.0f)) // NOLINT Walker's old code; no easy fix
/** Compute sin (in degrees) from 90 degrees */
#define DSIN_90(a) (sin(M_PI * (a + 90.0f) / 180.0f)) // NOLINT Walker's old code; no easy fix
/** Clamp x into the range [y,z] */
constexpr float RANGE_CLAMP(float x, float y, float z) { return (x < y ? y : (x > z ? z : x)); }

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
	return true;
}

/**
 * Disposes all resources and assets of this donut
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a donut may not be used until it is initialized again.
 */
void DonutModel::dispose() { sprite = nullptr; }

#pragma mark -
#pragma mark Animation
/**
 * Sets the film strip representing this donut.
 *
 * Setting this to nullptr clears the value.
 *
 * @param value The donut film strip.
 */
void DonutModel::setSprite(const std::shared_ptr<cugl::AnimationNode>& value) {
	sprite = value;
	if (sprite != nullptr) {
		sprite->setFrame(SHIP_IMG_FLAT);
		sprite->setAnchor(Vec2::ANCHOR_CENTER);
	}
}

/**
 * Updates the state of the model
 *
 * This method moves the donut forward, dampens the forces (if necessary)
 * This method moves the donut
 * and updates the sprite if it exists.
 *
 * @param timestep  Time elapsed since last called.
 */
void DonutModel::update(float timestep) {
	// Adjust the active forces.
	turning = RANGE_CLAMP(turning, -DONUT_MAX_TURN, DONUT_MAX_TURN);

	if (sprite != nullptr) {
		advanceFrame();
	}

	// Adjust the angle by the change in angle
	angle += turning;
	// INVARIANT: -360 < ang < 720
	if (angle > FULL_CIRCLE) {
		angle -= FULL_CIRCLE;
	} else if (angle < 0) {
		angle += FULL_CIRCLE;
	}
}

/**
 * Determines the next animation frame for the donut and applies it to the sprite.
 *
 * This method includes some dampening of the turn, and should be called before
 * moving the donut.
 */
void DonutModel::advanceFrame() {
	// Our animation depends on the current frame.
	unsigned int frame = sprite->getFrame();
	// Process the donut turning.
	if (turning < 0.0f) {
		unsigned int offset =
			(unsigned int)((turning / DONUT_MAX_TURN) * (SHIP_IMG_FLAT - SHIP_IMG_RIGHT));
		unsigned int goal = SHIP_IMG_FLAT + offset;
		if (frame != goal) {
			frame += (frame < goal ? 1 : -1);
		}
		if (frame == SHIP_IMG_FLAT) {
			turning = 0.0f;
		}
	} else if (turning > 0.0f) {
		unsigned int offset =
			(unsigned int)((turning / DONUT_MAX_TURN) * (SHIP_IMG_FLAT - SHIP_IMG_LEFT));
		unsigned int goal = SHIP_IMG_FLAT - offset;
		if (frame != goal) {
			frame += (frame < goal ? 1 : -1);
		}
		if (frame == SHIP_IMG_FLAT) {
			turning = 0.0f;
		}
	} else {
		if (frame < SHIP_IMG_FLAT) {
			frame++;
		} else if (frame > SHIP_IMG_FLAT) {
			frame--;
		}
	}

	sprite->setFrame((int)frame);
}

/**
 * Resets the donut back to its original settings
 */
void DonutModel::reset() {
	angle = 0.0f;
	if (sprite != nullptr) {
		sprite->setFrame(SHIP_IMG_FLAT);
	}
	angle = 0.0f;
	turning = 0.0f;
	if (sprite != nullptr) {
		sprite->setFrame(SHIP_IMG_FLAT);
	}
}
