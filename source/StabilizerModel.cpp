#include "StabilizerModel.h"

#include "Globals.h"

/** Number of frames to roll to be successful */
constexpr unsigned int SUCCESS_CUTOFF = 60;

StabilizerModel::StabilizerModel()
	: rand(static_cast<unsigned int>(time(nullptr))), // NOLINT Good enough for us
	  currState(StabilizerModel::StabilizerState::Inactive),
	  progress(0),
	  endTime(0) {}

float StabilizerModel::getProgress() const {
	if (progress > SUCCESS_CUTOFF) {
		return 1;
	}
	return static_cast<float>(progress) / static_cast<float>(SUCCESS_CUTOFF);
}

void StabilizerModel::startChallenge(float currTime) {
	endTime = currTime + globals::ROLL_CHALLENGE_LENGTH;
	progress = 0;
	currState = (rand() % 2) != 0 ? StabilizerModel::StabilizerState::Left
								  : StabilizerModel::StabilizerState::Right;
}

void StabilizerModel::reset() {
	currState = StabilizerModel::StabilizerState::Inactive;
	progress = 0;
	endTime = 0;
}
