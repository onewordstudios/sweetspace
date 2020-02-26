#ifndef __BREACH_MODEL_H__
#define __BREACH_MODEL_H__
#include <cugl/cugl.h>
class BreachModel {
   private:
   protected:
	/** The angle at which the breach exists */
	float angle;
	/** The state of the breach: resolved or unresolved */
	bool resolved;
	/** Reference to image in SceneGraph for animation */
	std::shared_ptr<cugl::PolygonNode> sprite;

   public:
#pragma mark Constructors
	/*
	 * Creates a new breach at angle 0.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	BreachModel(void) : angle(0), resolved(false) {}

	/**
	 * Destroys this breach, releasing all resources.
	 */
	~BreachModel(void) { dispose(); }

	/**
	 * Disposes all resources and assets of this breach
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a breach may not be used until it is initialized again.
	 */
	void dispose();

	/**
	 * Initializes a new breach at angle 0.
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init() { return init(0.0f); }

	/**
	 * Initializes a new breach with the given angle
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param a   The angle at which the breach exists
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const float a);

	static std::shared_ptr<BreachModel> alloc() {
		std::shared_ptr<BreachModel> result = std::make_shared<BreachModel>();
		return (result->init() ? result : nullptr);
	}

#pragma mark -
#pragma mark Accessors
	/**
	 * Returns the current angle of the breach in radians.
	 *
	 * @return the current angle of the breach in radians.
	 */
	float getAngle() { return (float)M_PI * angle / 180.0f; }

	/**
	 * Returns the current state of the breach.
	 *
	 * @return the current state of the breach.
	 */
	bool getIsResolved() { return resolved; }

	/**
	 * Returns the current sprite of the breach.
	 *
	 * @return the current sprite of the breach.
	 */
	std::shared_ptr<cugl::PolygonNode> getSprite() { return sprite; }

	/**
	 * Sets the current angle of the breach in radians.
	 *
	 * @param value The breach angle in radians
	 */
	void setAngle(float value) { angle = 180.0f * value / (float)M_PI; }

	/**
	 * Sets the current state of the breach.
	 *
	 * @param isFixed If the breach is fixed.
	 */
	void setIsResolved(bool isResolved) { resolved = isResolved; }

	/**
	 * Sets the sprite of the breach.
	 *
	 * @param value The sprite
	 */
	void setSprite(const std::shared_ptr<cugl::PolygonNode> value) { sprite = value; }
};
#endif /* __BREACH_MODEL_H__ */
