#ifndef DOOR_MODEL_H
#define DOOR_MODEL_H
#include <cugl/cugl.h>

#include <bitset>

#include "Globals.h"

class DoorModel {
   private:
	/** The height of the door */
	unsigned int height = 0;
	/** Whether or not this object is active */
	bool isActive;

   protected:
	/** The angle at which the door exists */
	float angle;
	/** The state of the door */
	std::bitset<globals::MAX_PLAYERS> playersOn;

   public:
#pragma mark Constructors
	/*
	 * Creates a new door at angle 0.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	DoorModel() : isActive(false), angle(0) {}

	DoorModel(const DoorModel&) = delete;

	/**
	 * Destroys this door, releasing all resources.
	 */
	~DoorModel() { dispose(); }

	/**
	 * Disposes all resources and assets of this door
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a door may not be used until it is initialized again.
	 */
	void dispose();

	/**
	 * Initializes a new door with the given angle
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param a   The angle at which the door exists
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(float a);

#pragma mark -
#pragma mark Accessors
	/**
	 * Returns the current angle of the door in degrees.
	 *
	 * @return the current angle of the door in degrees.
	 */
	float getAngle() const { return angle; }

	/**
	 * Returns whether the breach is currently active.
	 *
	 * @return whether the breach is currently active.
	 */
	bool getIsActive() const { return isActive; }

	/**
	 * Returns the current height of the door.
	 *
	 * @return the current height of the door.
	 */
	unsigned int getHeight() const { return height; }

	/**
	 * Returns the number of players in range of the door.
	 *
	 * @return the number of players in range of the door.
	 */
	uint8_t getPlayersOn() const { return static_cast<uint8_t>(playersOn.count()); }

	/**
	 * Adds the given player's flag from the door.
	 */
	void addPlayer(uint8_t id) { playersOn.set(id); }

	/**
	 * Removes the given player's flag from the door. Requires that this player is on the door
	 */
	void removePlayer(uint8_t id);

	/**
	 * Raises the door.
	 */
	void update(float timestep);

	/**
	 * Returns whether this door can be passed under.
	 */
	bool halfOpen() const;

	/**
	 * Returns whether this door has been resolved and opened.
	 */
	bool resolvedAndRaised() const;

	/**
	 * Returns whether this player is on the door.
	 */
	bool isPlayerOn(uint8_t id) const { return playersOn.test(id); }

	/**
	 * Returns whether this door is resolved.
	 */
	bool resolved() const { return getPlayersOn() >= 2; }

	/**
	 * Resets this door.
	 */
	void reset();
};
#endif /* DOOR_MODEL_H */
