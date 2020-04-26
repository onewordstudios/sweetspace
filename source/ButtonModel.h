#ifndef __BUTTON_MODEL_H__
#define __BUTTON_MODEL_H__
#include <cugl/cugl.h>

#include <bitset>

#include "Globals.h"

class ButtonModel {
   private:
	/** The height of the button */
	int height = 0;

   protected:
	/** The angle at which the button exists */
	float angle;
	/** The state of the button */
	uint8_t playersOn;
	bool jumped;
	std::shared_ptr<ButtonModel> pairButton;
	int pairID;
	bool resolved = false;

   public:
#pragma mark Constructors
	/*
	 * Creates a new button, unused button.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	ButtonModel(void) : angle(-1), playersOn(0), jumped(false), pairID(-1) {}

	/**
	 * Initializes a new button at an unassigned angle (-1) and without a defined pair.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	bool init() { return init(-1.0f, nullptr, -1); }

	/**
	 * Initializes a new button with the given angle and pair.
	 *
	 * @param a   The angle at which the button exists
	 * @param pair Pointer to the pair of this button
	 * @param pairID ID of the pair of this button
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	bool init(const float a, std::shared_ptr<ButtonModel> pair, int pairID);

	/**
	 * Allocate and return a pointer to a new, empty button model.
	 */
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
		std::bitset<globals::MAX_PLAYERS> ids(playersOn);
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
	bool jumpedOn() { return jumped && getPlayersOn() > 0; }

	std::shared_ptr<ButtonModel> getPair() { return pairButton; }
	int getPairID() { return pairID; }

	/**
	 * Resets this button.
	 */
	void clear();
};
#endif /* __BUTTON_MODEL_H__ */
