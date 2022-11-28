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
			return false;
		case StabilizerState::Left:
		case StabilizerState::Right:
			return true;
		default:
			// Unreachable
			throw std::exception();
	}
}

float StabilizerModel::getProgress() const {
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

bool StabilizerModel::update(float timeRemaining,
							 const std::vector<std::shared_ptr<DonutModel>>& donuts) {
	if (!getIsActive()) {
		return false;
	}

	// If there's not enough time left in the level for the challenge, bail
	if (timeRemaining >= 0 && trunc(timeRemaining) <= globals::ROLL_CHALLENGE_LENGTH) {
		reset();
		return false;
	}

	bool allRoll = true;

	for (const auto& allDonut : donuts) {
		if (!allDonut->getIsActive()) {
			continue;
		}
		if (isLeft()) {
			if (allDonut->getVelocity() >= 0) {
				allRoll = false;
				break;
			}
		} else {
			if (allDonut->getVelocity() <= 0) {
				allRoll = false;
				break;
			}
		}
	}

	if (allRoll) {
		progress++;
	}

	return true;
}

void StabilizerModel::fail() { currState = StabilizerState::Fail; }

void StabilizerModel::finish() {
	const bool won = getIsWin();
	reset();
	currState = won ? StabilizerState::Inactive : StabilizerState::Fail;
}

void StabilizerModel::reset() {
	currState = StabilizerState::Inactive;
	progress = 0;
	endTime = 0;
}
