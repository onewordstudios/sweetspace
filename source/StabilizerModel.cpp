#include "StabilizerModel.h"

#include "Globals.h"

/** Number of frames to roll to be successful */
constexpr unsigned int SUCCESS_CUTOFF = 60;

StabilizerModel::StabilizerModel()
	: rand(static_cast<unsigned int>(time(nullptr))), // NOLINT Good enough for us
	  currState(StabilizerModel::StabilizerState::Inactive),
	  progress(0),
	  endTime(0) {}

bool StabilizerModel::getIsActive() const {
	switch (currState) {
		case StabilizerState::Inactive:
		case StabilizerState::Fail:
		case StabilizerState::Win:
			return false;
		case StabilizerState::Left:
		case StabilizerState::Right:
			return true;
	}
}

float StabilizerModel::getProgress() const {
	CULog("Progress %d", progress);
	if (progress > SUCCESS_CUTOFF) {
		return 1;
	}
	return static_cast<float>(progress) / static_cast<float>(SUCCESS_CUTOFF);
}

void StabilizerModel::startChallenge(float currTime) {
	endTime = currTime + globals::ROLL_CHALLENGE_LENGTH;
	progress = 0;
	currState = (rand() % 2) != 0 ? StabilizerState::Left : StabilizerState::Right;
}

void StabilizerModel::finish() {
	bool won = getIsWin();
	reset();
	currState = won ? StabilizerState::Win : StabilizerState::Fail;
}

void StabilizerModel::forceWin() { currState = StabilizerState::Win; }

void StabilizerModel::reset() {
	currState = StabilizerState::Inactive;
	progress = 0;
	endTime = 0;
}
