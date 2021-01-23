#ifndef STABILIZER_MODEL_H
#define STABILIZER_MODEL_H
#include <cugl/cugl.h>
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

	/** Mark a single frame with everyone rolling together */
	void incrementProgress() { progress++; }

	/** Immediately fail this challenge (usually b/c we received the command over networking) */
	void fail();

	/** Complete the challenge, marking as win or loss depending on current status. */
	void finish();

	/** Reset the challenge */
	void reset();
};
#endif /* STABILIZER_MODEL_H */
