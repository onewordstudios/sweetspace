﻿#ifndef __BUTTON_MODEL_H__
#define __BUTTON_MODEL_H__
#include <cugl/cugl.h>

#include <bitset>

#include "Globals.h"

class ButtonModel {
   private:
	/** The height of the button, as percentage down (0 = fully up) */
	float height = 0;
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
	/** Whether this model is active */
	bool isActive;

   public:
#pragma region Constructors
	/*
	 * Creates a new, uninitialized, and unused button.
	 *
	 * Do not call this constructor using new. These models should exclusively be allocated into an
	 * object pool by {@code ShipModel} and accessed from there.
	 */
	ButtonModel(void) : angle(-1), pairID(-1), resolved(false), isActive(false) {}

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

#pragma endregion
#pragma region Accessors
	/** Returns whether this model is active */
	bool getIsActive() { return isActive; }

	/**
	 * Returns the current angle of the button in degrees.
	 */
	float getAngle() { return angle; }

	/**
	 * Returns the section of the ship containing this button.
	 */
	int getSection();

	/**
	 * Returns the current height of the button, as percentage down, where 0 = fully up and 1 =
	 * fully down
	 */
	float getHeight() { return height; }

	/**
	 * Returns whether this button is resolved.
	 */
	bool isResolved() { return resolved; }

	/**
	 * Returns whether any players are jumping on this button.
	 */
	bool isJumpedOn() { return playersOn.any(); }

	/**
	 * Return a pointer to the pair of this button
	 */
	std::shared_ptr<ButtonModel> getPair() { return pairButton; }

	/**
	 * Return the ID of the pair of this button
	 */
	int getPairID() { return pairID; }

#pragma endregion
#pragma region Mutators

	/** Signals the current jump height of the player currently on the button. */
	void setPlayerHeight(float h);

	/**
	 * Adds the given player's flag to the button.
	 */
	void addPlayer(int id) { playersOn.set(id); }

	/**
	 * Removes the given player's flag from the button if present. Has no effect otherwise.
	 */
	void removePlayer(int id) { playersOn.reset(id); }

	/**
	 * Resolve this button
	 */
	void resolve() { resolved = true; }

	/**
	 * Resets this button.
	 */
	void clear();
#pragma endregion
};
#endif /* __BUTTON_MODEL_H__ */
