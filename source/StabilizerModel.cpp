#include "StabilizerModel.h"

#include "Globals.h"

/** Number of frames to roll to be successful */
constexpr unsigned int SUCCESS_CUTOFF = 60;

StabilizerModel::StabilizerModel()
	: currState(StabilizerModel::StabilizerState::Inactive), progress(0), endTime(0) {
	srand(time(nullptr));
}

float StabilizerModel::getProgress() {
	if (progress > SUCCESS_CUTOFF) {
		return 1;
	}
	return (float)progress / (float)SUCCESS_CUTOFF;
}

void StabilizerModel::startChallenge(float currTime) {
	endTime = currTime + globals::ROLL_CHALLENGE_LENGTH;
	progress = 0;
	currState = rand() % 2 ? StabilizerModel::StabilizerState::Left
						   : StabilizerModel::StabilizerState::Right;
}

void StabilizerModel::reset() {
	currState = StabilizerModel::StabilizerState::Inactive;
	progress = 0;
	endTime = 0;
}
