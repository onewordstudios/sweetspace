#ifndef STABILIZER_MODEL_H
#define STABILIZER_MODEL_H
#include <cugl/cugl.h>
class StabilizerModel {
   private:
	/** Random number generator */
	std::minstd_rand rand;

	enum class StabilizerState { Inactive, Left, Right };

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
	bool getIsActive() const { return currState != StabilizerModel::StabilizerState::Inactive; }

	/** Return whether this stabilizer requires rolling left */
	bool isLeft() const { return currState == StabilizerModel::StabilizerState::Left; }

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

	/** Reset the challenge */
	void reset();
};
#endif /* STABILIZER_MODEL_H */
