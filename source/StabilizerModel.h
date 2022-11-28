#ifndef STABILIZER_MODEL_H
#define STABILIZER_MODEL_H

#include <cugl/cugl.h>

#include "DonutModel.h"

class StabilizerModel {
   public:
	/** Current state of the stabilizer */
	enum class StabilizerState { Inactive, Left, Right, Fail };

   private:
	/** Random number generator */
	std::minstd_rand rand;

	/** Current state of the challenge */
	StabilizerModel::StabilizerState currState;

	/** Current progress */
	unsigned int progress;

	/** Time at which this challenge is scheduled to end */
	float endTime;

   public:
	/** Construct a new stabilizer model */
	StabilizerModel();

	StabilizerModel(const StabilizerModel&) = delete;

	virtual ~StabilizerModel() = default;

	/** Return whether this stabilizer is active */
	bool getIsActive() const;

	/** Return whether this stabilizer requires rolling left */
	bool isLeft() const { return currState == StabilizerState::Left; }

	/** Get the current state of the stabilizer */
	StabilizerState getState() const { return currState; }

	/** Return the end time of this stabilizer; requires this stabilizer be active */
	float getEndTime() const { return endTime; }

	/** Return the progress made in beating this stabilizer (in 0 to 1) */
	float getProgress() const;

	/** Return whether this stabilizer was completed; requires this stabilizer be active */
	bool getIsWin() const { return getProgress() >= 1; }

	/**
	 * Trigger the challenge, with the current player getting the prompt. Will pick a direction
	 * randomly. Requires the challenge currently be inactive.
	 *
	 * @param currTime The current canonical ship time
	 */
	void startChallenge(float currTime);

	/**
	 * Step the local state of the challenge.
	 *
	 * Is safe to call when inactive; simply no-ops if that's the case.
	 *
	 * Will automatically check for donuts rolling in correct direction and	update progress.
	 * Will NOT fail or finish the challenge after completion (though will cancel if time remaining
	 * is too low).
	 *
	 * @param timeRemaining Amount of time left as displayed on the ship's timer, or -1 for
	 * timeless levels
	 * @param donuts All donut models in ship
	 * @returns True if the model performed computations this frame. If this returns true, the ship
	 * should check to see if pass or fail happened this frame, and if so process accordingly.
	 */
	bool update(float timeRemaining, const std::vector<std::shared_ptr<DonutModel>>& donuts);

	/** Immediately fail this challenge (usually b/c we received the command over networking) */
	void fail();

	/** Complete the challenge, marking as win or loss depending on current status. */
	void finish();

	/** Reset the challenge */
	void reset();
};
#endif /* STABILIZER_MODEL_H */
