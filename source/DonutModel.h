#ifndef __DONUT_MODEL_H__
#define __DONUT_MODEL_H__
#include <cugl/cugl.h>
constexpr float HALF_CIRCLE = 180.0f;

/** Donut Frame Sprite numbers */
// Left bank frame NOLINTNEXTLINE Walker's code
#define SHIP_IMG_LEFT 0
// Neutral frame NOLINTNEXTLINE Walker's code
#define SHIP_IMG_FLAT 9
// Right bank frame NOLINTNEXTLINE Walker's code
#define SHIP_IMG_RIGHT 17

/** Number of rows and cols in film strip */
// NOLINTNEXTLINE
#define SHIP_IMG_ROWS 4
// NOLINTNEXTLINE
#define SHIP_IMG_COLS 5
// NOLINTNEXTLINE
#define SHIP_IMG_SIZE 18

#pragma mark -
#pragma mark Donut Model

class DonutModel {
   private:
	/** This macro disables the copy constructor (not allowed on models) */
	CU_DISALLOW_COPY_AND_ASSIGN(DonutModel);

	/**
	 * Determines the next animation frame for the donut and applies it to the sprite.
	 *
	 * This method includes some dampening of the turn, and should be called before
	 * moving the donut.
	 */
	void advanceFrame();

   protected:
	/** INITIAL position of the donut in world space */
	cugl::Vec2 initial;
	/** Current ship velocity */
	cugl::Vec2 velocity;
	/** Angle of the donut in the world space */
	float angle;
	/** Current turning thrust (stored to facilitate decay) */
	float turning;
	/** Reference to image in SceneGraph for animation */
	std::shared_ptr<cugl::AnimationNode> sprite;

   public:
#pragma mark Constructors
	/*
	 * Creates a new donut at the origin.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	DonutModel(void) : angle(0), turning(0) {}

	/**
	 * Destroys this donut, releasing all resources.
	 */
	~DonutModel(void) { dispose(); }

	/**
	 * Disposes all resources and assets of this donut
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a donut may not be used until it is initialized again.
	 */
	void dispose();

	/**
	 * Initializes a new donut at the origin.
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init() { return init(cugl::Vec2::ZERO); }

	/**
	 * Initializes a new donut with the given position
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param pos   Initial position in world coordinates
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const cugl::Vec2& pos);

#pragma mark Static Constructors
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
		std::shared_ptr<DonutModel> result = std::make_shared<DonutModel>();
		return (result->init() ? result : nullptr);
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
		std::shared_ptr<DonutModel> result = std::make_shared<DonutModel>();
		return (result->init(pos) ? result : nullptr);
	}

#pragma mark -
#pragma mark Accessors
	// SHORT METHODS CAN BE IN-LINED IN C++

	/**
	 * Returns the donut position as a reference.
	 *
	 * This allows us to modify the value.
	 *
	 * @return the donut position as a reference.
	 */
	cugl::Vec2& getPosition() { return initial; }

	/**
	 * Returns the donut velocity as a reference.
	 *
	 * This allows us to modify the value.
	 *
	 * @return the donut velocity as a reference.
	 */
	cugl::Vec2& getVelocity() { return velocity; }

	/**
	 * Returns the current angle of the donut in radians.
	 *
	 * @return the current angle of the donut in radians.
	 */
	float getAngle() { return (float)M_PI * angle / HALF_CIRCLE; }

	/**
	 * Sets the current angle of the donut in radians.
	 *
	 * @param value The donut angle in radians
	 */
	void setAngle(float value) { angle = HALF_CIRCLE * value / (float)M_PI; }

	/**
	 * Returns the current turning force on the donut
	 *
	 * @return the current turning force on the donut
	 */
	float getTurning() { return turning; }

	/**
	 * Sets the current turning force on the donut
	 *
	 * @param value The donut turning force
	 */
	void setTurning(float value) { turning = value; }

#pragma mark -
#pragma mark Animation
	/**
	 * Returns a reference to film strip representing this donut.
	 *
	 * It returns nullptr if there is no active film strip.
	 *
	 * @return a reference to film strip representing this donut.
	 */
	std::shared_ptr<cugl::AnimationNode>& getSprite() { return sprite; }

	/**
	 * Sets the film strip representing this donut.
	 *
	 * Setting this to nullptr clears the value.
	 *
	 * @param value The donut film strip.
	 */
	void setSprite(const std::shared_ptr<cugl::AnimationNode>& value);

	/**
	 * Updates the state of the model
	 *
	 * This method moves the donut forward, dampens the forces (if necessary)
	 * and updates the sprite if it exists.
	 *
	 * @param timestep  Time elapsed since last called.
	 */
	void update(float timestep = 0.0f);

	/**
	 * Resets the donut back to its original settings
	 */
	void reset();
};

#endif /* __DONUT_MODEL_H__ */
