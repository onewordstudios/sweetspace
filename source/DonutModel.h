#ifndef __DONUT_MODEL_H__
#define __DONUT_MODEL_H__
#include <cugl/cugl.h>
class DonutModel {
   private:
	/**
	 * Determines the next animation frame for the ship and applies it to the sprite.
	 *
	 * This method includes some dampening of the turn, and should be called before
	 * moving the ship.
	 */
	void advanceFrame();
   protected:
	/** INITIAL position of the ship in world space */
	cugl::Vec2 initial;
	/** Position of the ship in world space */
	cugl::Vec2 position;
	/** Current ship velocity */
	cugl::Vec2 velocity;
	/** Angle of the ship in the world space */
	float angle;
	/** Current forward thrust (stored to facilitate decay) */
	float forward;
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
		DonutModel(void) : angle(0), forward(0) {}

		/**
		 * Destroys this ship, releasing all resources.
		 */
		~DonutModel(void) { dispose(); }

		/**
		 * Disposes all resources and assets of this ship
		 *
		 * Any assets owned by this object will be immediately released.  Once
		 * disposed, a ship may not be used until it is initialized again.
		 */
		void dispose();

		/**
		 * Initializes a new ship at the origin.
		 *
		 * An initializer does the real work that the constructor does not.  It
		 * initializes all assets and makes the object read for use.  By separating
		 * them, we allow ourselfs non-pointer references to complex objects.
		 *
		 * @return true if the obstacle is initialized properly, false otherwise.
		 */
		virtual bool init() { return init(cugl::Vec2::ZERO); }

		/**
		 * Initializes a new ship with the given position
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
		 * Returns a newly allocated ship at the origin.
		 *
		 * This is a static constructor. You call it with the DonutModel::alloc().
		 * We prefer static constructors as they make the usage of shared pointers
		 * much simpler (and prevent the temptation of making a weak pointer on
		 * the heap).
		 *
		 * @return a newly allocated ship at the origin.
		 */
		static std::shared_ptr<DonutModel> alloc() {
			std::shared_ptr<DonutModel> result = std::make_shared<DonutModel>();
			return (result->init() ? result : nullptr);
		}

		/**
		 * Returns a newly allocated ship at the given position.
		 *
		 * This is a static constructor. You call it with the DonutModel::alloc().
		 * We prefer static constructors as they make the usage of shared pointers
		 * much simpler (and prevent the temptation of making a weak pointer on
		 * the heap).
		 *
		 * @param pos   Initial position in world coordinates
		 *
		 * @return a newly allocated ship at the given position.
		 */
		static std::shared_ptr<DonutModel> alloc(const cugl::Vec2& pos) {
			std::shared_ptr<DonutModel> result = std::make_shared<DonutModel>();
			return (result->init(pos) ? result : nullptr);
		}

	#pragma mark -
	#pragma mark Accessors
		// SHORT METHODS CAN BE IN-LINED IN C++

		/**
		 * Returns the ship velocity as a reference.
		 *
		 * This allows us to modify the value.
		 *
		 * @return the ship velocity as a reference.
		 */
		cugl::Vec2& getVelocity() { return velocity; }

		/**
		 * Returns the ship position as a reference.
		 *
		 * This allows us to modify the value.
		 *
		 * @return the ship position as a reference.
		 */
		cugl::Vec2& getPosition() { return position; }
		/**
		 * Returns the current roll amount of the donut
		 *
		 * @return The current roll amount of the donut
		 */
		float getForward() { return rollAmount; }

		/**
		 * Sets the current forward force on the ship
		 *
		 * @param value The ship forward force
		 */
		void setForward(float value) { forward = value; }

	#pragma mark -
	#pragma mark Animation
		/**
		 * Returns a reference to film strip representing this ship.
		 *
		 * It returns nullptr if there is no active film strip.
		 *
		 * @return a reference to film strip representing this ship.
		 */
		std::shared_ptr<cugl::AnimationNode>& getSprite() { return sprite; }

		/**
		 * Sets the film strip representing this ship.
		 *
		 * Setting this to nullptr clears the value.
		 *
		 * @param value The ship film strip.
		 */
		void setSprite(const std::shared_ptr<cugl::AnimationNode>& value);

		/**
		 * Updates the state of the model
		 *
		 * This method moves the ship forward, dampens the forces (if necessary)
		 * and updates the sprite if it exists.
		 *
		 * @param timestep  Time elapsed since last called.
		 */
		void update(float timestep = 0.0f);

		/**
		 * Resets the ship back to its original settings
		 */
		void reset();
	
};


#endif /* __DONUT_MODEL_H__ */
