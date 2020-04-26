#ifndef __BUTTON_MODEL_H__
#define __BUTTON_MODEL_H__
#include <cugl/cugl.h>

#include <bitset>

#include "Globals.h"

class ButtonModel {
   private:
	/** The height of the button */
	int height = 0;
	/** The angle at which the button exists */
	float angle;
	/** Bitset of players on this button */
	std::bitset<globals::MAX_PLAYERS> playersOn;
	/** Pointer to the pair of this button */
	std::shared_ptr<ButtonModel> pairButton;
	/** ID of the pair of this button */
	int pairID;
	/** Whether this button is resolved */
	bool resolved;

   public:
#pragma mark Constructors
	/*
	 * Creates a new, uninitialized, and unused button.
	 *
	 * Do not call this constructor using new. These models should exclusively be allocated into an
	 * object pool by {@code ShipModel} and accessed from there.
	 */
	ButtonModel(void) : angle(-1), pairID(-1), resolved(false) {}

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
	int getPlayersOn() { return (int)playersOn.count(); }

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
	void addPlayer(int id) { playersOn.set(id); }

	/**
	 * Removes the given player's flag from the button. Requires that this player is on the button
	 */
	void removePlayer(int id) {
		if (!isResolved()) {
			playersOn.reset(id);
		}
	}

	/**
	 * Returns whether this player is on the button.
	 */
	bool isPlayerOn(int id) { return playersOn.test(id); }

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
	bool jumpedOn() { return playersOn.any(); }

	/**
	 * Return a pointer to the pair of this button
	 */
	std::shared_ptr<ButtonModel> getPair() { return pairButton; }

	/**
	 * Return the ID of the pair of this button
	 */
	int getPairID() { return pairID; }

	/**
	 * Resets this button.
	 */
	void clear();
};
#endif /* __BUTTON_MODEL_H__ */
