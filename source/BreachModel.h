#ifndef __BREACH_MODEL_H__
#define __BREACH_MODEL_H__
#include <cugl/cugl.h>
class BreachModel {
   private:
   protected:
	/** The angle at which the breach exists */
	float angle;
	/** The state of the breach in health: 0 means its resolved */
	int health;
	/** Whether the player is currently on this breach */
	bool playerOn;
	/** Which player can clear this breach */
	int player;
	/** Reference to image in SceneGraph for animation */
	std::shared_ptr<cugl::PolygonNode> sprite;
	/** Set to true if sprite needs to be updated */
	bool needSpriteUpdate;

   public:
	/** Default Max Health of a Breach*/
	static constexpr unsigned int HEALTH_DEFAULT = 3;
#pragma mark Constructors
	/*
	 * Creates a new breach at angle 0.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	BreachModel(void) : angle(0), health(0), playerOn(false), player(0) {}

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
	 * Initializes a new breach at an unassigned angle (-1).
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init() { return init(-1.0f, 3, 0); }

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
	virtual bool init(const float a) { return init(a, 3, 0); };

	/**
	 * Initializes a new breach with the given angle and max health
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param a   The angle at which the breach exists
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const float a, const int b, const int c);

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
	 * Returns the current health of the breach.
	 *
	 * @return the current health of the breach.
	 */
	int getHealth() { return health; }

	/**
	 * Returns whether the player is currently on the breach.
	 *
	 * @return whether the player is currently on the breach.
	 */
	int isPlayerOn() { return playerOn; }

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
	 * Sets the current health of the breach.
	 *
	 * @param health New breach health.
	 */
	void setHealth(int value) { health = value; }

	/**
	 * Decrements the current health of the breach by value.
	 *
	 * @param value Amount to decrement health by.
	 */
	void decHealth(int value) { health = health - value; }

	/**
	 * Sets whether the player is currently on the breach.
	 *
	 * @param b Whether the player is currently on the breach.
	 */
	void setIsPlayerOn(bool b) { playerOn = b; }

	/**
	 * Gets which player is assigned to this breach.
	 *
	 * @return Which player is assigned to this breach.
	 */
	int getPlayer() { return player; }

	/**
	 * Sets which player is assigned to this breach.
	 *
	 * @param p The player to assign to the breach.
	 */
	void setPlayer(int p) { player = p; }

	/**
	 * Sets the sprite of the breach.
	 *
	 * @param value The sprite
	 */
	void setSprite(const std::shared_ptr<cugl::PolygonNode> value) { sprite = value; }

	/**
	 * Sets the needSpriteUpdate field.
	 * @return
	 */
	bool getNeedSpriteUpdate() { return needSpriteUpdate; }

	/**
	 * Gets the needSpriteUpdate field.
	 * @param b
	 */
	void setNeedSpriteUpdate(bool b) { needSpriteUpdate = b; }

	/**
	 * Resets the breach upon recycling.
	 * @param an
	 * @param he
	 * @param pl
	 */
	void reset(float an, int he, int pl) {
		setAngle(an);
		health = he;
		player = pl;
		needSpriteUpdate = true;
	}

	/**
	 * Resets the breach upon recycling.
	 * @param an
	 * @param he
	 * @param pl
	 */
	void reset(float an, int pl) { reset(an, HEALTH_DEFAULT, pl); }
};
#endif /* __BREACH_MODEL_H__ */
