#include "DonutModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Animation Constants and Functions
/** Factor to multiply the forward thrust */
constexpr unsigned int FULL_CIRCLE = 360;
/** The max angular velocity (in degrees) per frame */
constexpr float DONUT_MAX_TURN = 2.0f;
/** The max force to apply to the donut */
constexpr float DONUT_MAX_FORCE = 0.5f;
/** The amount the angular velocity decays by each frame */
constexpr float DONUT_FRICTION_FACTOR = 0.9f;
/** The threshold below which the donut has effectively stopped rolling */
constexpr float DONUT_STOP_THRESHOLD = 0.01f;

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
	velocity = RANGE_CLAMP(velocity, -DONUT_MAX_TURN, DONUT_MAX_TURN);

	if (sprite != nullptr) {
		advanceFrame();
	}

	// Adjust the angle by the change in angle
	angle += velocity;
	// INVARIANT: -360 < ang < 720
	if (angle > FULL_CIRCLE) {
		angle -= FULL_CIRCLE;
	} else if (angle < 0) {
		angle += FULL_CIRCLE;
	}

	velocity *= DONUT_FRICTION_FACTOR;
	if (abs(velocity) < DONUT_STOP_THRESHOLD) {
		velocity = 0;
	}
}

/**
 * Applies a force to the donut.
 *
 * @param value The donut turning force
 */
void DonutModel::applyForce(float value) { velocity += DONUT_MAX_FORCE * value; }

/**
 * Determines the next animation frame for the donut and applies it to the sprite.
 */
void DonutModel::advanceFrame() {
	// Our animation depends on the current frame.
	unsigned int frame = sprite->getFrame();
	float velocity = RANGE_CLAMP(this->velocity, -DONUT_MAX_TURN, DONUT_MAX_TURN);
	// Process the donut turning.
	if (velocity < 0.0f) {
		unsigned int offset =
			(unsigned int)((velocity / DONUT_MAX_TURN) * (SHIP_IMG_FLAT - SHIP_IMG_RIGHT));
		unsigned int goal = SHIP_IMG_FLAT + offset;
		if (frame != goal) {
			frame += (frame < goal ? 1 : -1);
		}
	} else if (velocity > 0.0f) {
		unsigned int offset =
			(unsigned int)((velocity / DONUT_MAX_TURN) * (SHIP_IMG_FLAT - SHIP_IMG_LEFT));
		unsigned int goal = SHIP_IMG_FLAT - offset;
		if (frame != goal) {
			frame += (frame < goal ? 1 : -1);
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
	velocity = 0.0f;
	if (sprite != nullptr) {
		sprite->setFrame(SHIP_IMG_FLAT);
	}
}
