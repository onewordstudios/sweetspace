#ifndef __BUTTON_MODEL_H__
#define __BUTTON_MODEL_H__
#include <cugl/cugl.h>

#include <bitset>
/** The size to make the bitset */
constexpr unsigned int PLAYERS = 6;

class ButtonModel {
   private:
	/** The height of the button */
	int height = 0;

   protected:
	/** The angle at which the button exists */
	float angle;
	/** The state of the button */
	unsigned char playersOn;
	bool jumped;
	std::shared_ptr<ButtonModel> pairButton;
	int pairId;
	bool resolved = false;

   public:
#pragma mark Constructors
	/*
	 * Creates a new button, unused button.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	ButtonModel(void) : angle(-1), playersOn(0), jumped(false), pairId(-1) {}

	/**
	 * Destroys this button, releasing all resources.
	 */
	~ButtonModel(void) { dispose(); }

	/**
	 * Disposes all resources and assets of this button
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a button may not be used until it is initialized again.
	 */
	void dispose();

	/**
	 * Initializes a new button at an unassigned angle (-1).
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init() { return init(-1.0f); }

	/**
	 * Initializes a new button with the given angle
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param a   The angle at which the button exists
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const float a) {
		this->angle = a;
		return true;
	};

	static std::shared_ptr<ButtonModel> alloc() {
		std::shared_ptr<ButtonModel> result = std::make_shared<ButtonModel>();
		return (result->init() ? result : nullptr);
	}

#pragma mark -
#pragma mark Accessors
	/**
	 * Returns the current angle of the button in degrees.
	 *
	 * @return the current angle of the button in degrees.
	 */
	float getAngle() { return angle; }

	/**
	 * Returns the current section of the button.
	 *
	 * @return the current section of the button.
	 */
	int getSection();

	/**
	 * Returns the current height of the button.
	 *
	 * @return the current height of the button.
	 */
	int getHeight() { return height; }

	/**
	 * Returns the number of players in range of the button.
	 *
	 * @return the number of players in range of the button.
	 */
	int getPlayersOn() {
		std::bitset<PLAYERS> ids(playersOn);
		return (int)ids.count();
	}

	/**
	 * Sets the current angle of the button in degrees.
	 *
	 * @param value The button angle in degrees
	 */
	void setAngle(float value) { angle = value; }

	/**
	 * Adds the given player's flag from the button.
	 *
	 */
	void addPlayer(int id) { playersOn = playersOn | (unsigned char)pow(2, id); }

	/**
	 * Removes the given player's flag from the button. Requires that this player is on the button
	 */
	void removePlayer(int id) {
		if (!isResolved()) {
			playersOn = playersOn ^ (unsigned char)pow(2, id);
		}
	}

	/**
	 * Returns whether this player is on the button.
	 */
	bool isPlayerOn(int id) { return (playersOn & (unsigned char)pow(2, id)) > 0; }

	/**
	 * Returns whether this button is resolved.
	 */
	bool isResolved() { return resolved; }

	/**
	 * Sets whether this button is resolved.
	 */
	void setResolved(bool r) { resolved = r; }
	/**
	 * Returns whether this button can be passed under.
	 */
	void setJumpedOn(bool jump) { jumped = jump; }
	/**
	 * Returns whether this button can be passed under.
	 */
	bool jumpedOn() {
		return jumped;
		// &&getPlayersOn() == 1;
	}

	void setPair(std::shared_ptr<ButtonModel> b, int id) {
		pairButton = b;
		pairId = id;
	}
	std::shared_ptr<ButtonModel> getPair() { return pairButton; }
	int getPairID() { return pairId; }

	/**
	 * Resets this button.
	 */
	void clear() {
		playersOn = 0;
		height = 0;
		resolved = false;
	}
};
#endif /* __BUTTON_MODEL_H__ */
