#ifndef __UNOPENABLE_H__
#define __UNOPENABLE_H__
#include <cugl/cugl.h>

class Unopenable {
   private:
	/** Whether or not this object is active */
	bool isActive;

   protected:
	/** The angle at which the unopenable door exists */
	float angle;

   public:
#pragma mark Constructors
	/*
	 * Creates a new unopenable door at angle 0.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	Unopenable(void) : angle(0), isActive(false) {}

	/**
	 * Destroys this unopenable door, releasing all resources.
	 */
	~Unopenable(void) { dispose(); }

	/**
	 * Disposes all resources and assets of this unopenable door
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a unopenable door may not be used until it is initialized again.
	 */
	void dispose(){};

	/**
	 * Initializes a new unopenable door.
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init() { return init(0.0f); }

	/**
	 * Initializes a new unopenable door with the given angle
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param a   The angle at which the unopenable door exists
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const float a) {
		this->angle = a;
		isActive = true;
		return true;
	};

#pragma mark -
#pragma mark Accessors
	/**
	 * Returns the current angle of the unopenable door in degrees.
	 *
	 * @return the current angle of the unopenable door in degrees.
	 */
	float getAngle() { return angle; }

	/**
	 * Returns whether the unopenable door is currently active.
	 *
	 * @return whether the unopenable door is currently active.
	 */
	bool getIsActive() { return isActive; }

	/**
	 * Sets the current angle of the unopenable door in degrees.
	 *
	 * @param value The unopenable door angle in degrees
	 */
	void setAngle(float value) { angle = value; }
};
#endif /* __UNOPENABLE_H__ */
