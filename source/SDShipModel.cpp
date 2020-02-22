//
//  SDShipModel.cpp
//  Ship Demo
//
//  This model encapsulates all of the information for the ship demo. As there
//  are no collisions in ship demo, this class is pretty simple.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design.  This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together.  However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  Pay close attention to how this class designed. This class uses our standard
//  shared-pointer architecture which is common to the entire engine.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  Note that this object manages its own texture, but DOES NOT manage its own
//  scene graph node.  This is a very common way that we will approach complex
//  objects.
//
//  Author: Walker White
//  Version: 1/10/17
//
#include "SDShipModel.h"


using namespace cugl;

#pragma mark -
#pragma mark Animation Constants and Functions

/** The max turn (in degrees) per frame */
#define SHIP_MAX_TURN      1.0f
/** The max forward speed */
#define SHIP_MAX_SPEED    10.0f
/** Factor to multiply the forward thrust */
#define SHIP_THRUST_FACTOR 0.4f

/** Compute cos (in degrees) from 90 degrees */
#define DCOS_90(a)  (cos(M_PI*(a+90.0f)/180.0f))
/** Compute sin (in degrees) from 90 degrees */
#define DSIN_90(a)  (sin(M_PI*(a+90.0f)/180.0f))
/** Clamp x into the range [y,z] */
#define RANGE_CLAMP(x,y,z)  (x < y ? y : (x > z ? z : x))


#pragma mark -
#pragma mark Constructors

/**
 * Initializes a new ship with the given position
 *
 * This is an initializer.  It, combined with the constructor, produces the static
 * constructor create().  The initializer and normal constructor are private while
 * the static constructor is not.
 *
 * @param  pos  Initial position in world coordinates
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool ShipModel::init(const Vec2& pos) {
    _initial  = pos;
    _position = pos;
    return true;
}

/**
 * Disposes all resources and assets of this ship
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a ship may not be used until it is initialized again.
 */
void ShipModel::dispose() {
    _sprite = nullptr;
}


#pragma mark -
#pragma mark Animation
/**
 * Sets the film strip representing this ship.
 *
 * Setting this to nullptr clears the value.
 *
 * @param value The ship film strip.
 */
void ShipModel::setSprite(const std::shared_ptr<cugl::AnimationNode>& value) {
    _sprite = value;
    if (_sprite != nullptr) {
        _sprite->setFrame(SHIP_IMG_FLAT);
        _sprite->setPosition(_position);
        _sprite->setAnchor(Vec2::ANCHOR_CENTER);
    }
}

/**
 * Updates the state of the model
 *
 * This method moves the ship forward, dampens the forces (if necessary)
 * and updates the sprite if it exists.
 *
 * @param timestep  Time elapsed since last called.
 */
void ShipModel::update(float timestep) {
    // Adjust the active forces.
    _forward = RANGE_CLAMP(_forward, -SHIP_MAX_SPEED, SHIP_MAX_SPEED);
    _turning = RANGE_CLAMP(_turning, -SHIP_MAX_TURN, SHIP_MAX_TURN);
    
    if (_sprite != nullptr) {
        advanceFrame();
    }
    
    // Process the ship thrust.
    if (_forward != 0.0f) {
        // Thrust key pressed; increase the ship velocity.
        _velocity.x = (float)(_forward *  (-DCOS_90(_angle)) * SHIP_THRUST_FACTOR);
        _velocity.y = (float)(_forward *    DSIN_90(_angle)  * SHIP_THRUST_FACTOR);
    }
    
    // Move the ship, updating it.
    // Adjust the angle by the change in angle
    _angle += _turning;  // INVARIANT: -360 < ang < 720
    if (_angle > 360) _angle -= 360;
    if (_angle < 0)   _angle += 360;
    
    // Move the ship
    _position += _velocity;
}

/**
 * Determines the next animation frame for the ship and applies it to the sprite.
 *
 * This method includes some dampening of the turn, and should be called before
 * moving the ship.
 */
void ShipModel::advanceFrame() {
    // Our animation depends on the current frame.
    unsigned int frame = _sprite->getFrame();
    
    // Process the ship turning.
    if (_turning < 0.0f) {
        unsigned int offset = (unsigned int)((_turning/SHIP_MAX_TURN)*(SHIP_IMG_FLAT-SHIP_IMG_RIGHT));
        unsigned int goal  = SHIP_IMG_FLAT+offset;
        if (frame != goal) {
            frame += (frame < goal ? 1 : -1);
        }
        if (frame == SHIP_IMG_FLAT) {
            _turning = 0.0f;
        }
    } else if (_turning > 0.0f) {
		unsigned int offset = (unsigned int)((_turning/SHIP_MAX_TURN)*(SHIP_IMG_FLAT-SHIP_IMG_LEFT));
        unsigned int goal  = SHIP_IMG_FLAT-offset;
        if (frame != goal) {
            frame += (frame < goal ? 1 : -1);
        }
        if (frame == SHIP_IMG_FLAT) {
            _turning = 0.0f;
        }
    } else {
        if (frame < SHIP_IMG_FLAT) {
            frame++;
        } else if (frame > SHIP_IMG_FLAT) {
            frame--;
        }
    }
    
    _sprite->setFrame(frame);
}

/**
 * Resets the ship back to its original settings
 */
void ShipModel::reset() {
    _position = _initial;
    _velocity = Vec2::ZERO;
    _angle   = 0.0f;
    _turning = 0.0f;
    _forward = 0.0f;
    if (_sprite != nullptr) {
        _sprite->setFrame(SHIP_IMG_FLAT);
    }
}
