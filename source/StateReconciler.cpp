#include "StateReconciler.h"

constexpr float StateReconciler::DECODE_FLOAT(uint8_t m1, uint8_t m2) {
	return (float)(m1 + oneByte * m2) / floatPrecision;
}

StateReconciler::StateReconciler(unsigned int oneByte, float floatPrecision, float floatEpsilon) {
	this->oneByte = oneByte;
	this->floatPrecision = floatPrecision;
	this->floatEpsilon = floatEpsilon;
}

bool StateReconciler::reconcile(std::shared_ptr<ShipModel> state,
								const std::vector<uint8_t>& message) {
	float health = DECODE_FLOAT(message[1], message[2]);
	float timer = DECODE_FLOAT(message[3], message[4]);
	int index = 5; // NOLINT

	if (abs(state->getHealth() - health) > 1.0f) {
		state->setHealth(health);
	}
	if (abs(state->timeLeftInTimer - timer) > 1.0f) {
		state->timeLeftInTimer = timer;
	}

	const auto& breaches = state->getBreaches();
	localBreach.clear();
	if (breaches.size() != message[index++]) {
		CULog("ERROR: BREACH ARRAY SIZE DISCREPANCY; local %lu but server %d", breaches.size(),
			  message[index - 1]);
		return false;
	}
	for (unsigned int i = 0; i < breaches.size(); i++) {
		if (breaches[i]->getHealth() == 0 && message[index] > 0) {
			float angle = DECODE_FLOAT(message[index + 2], message[index + 3]);
			if (breachCache.count(i) != 0 && breachCache[i]) {
				CULog("Found resolved breach that should be unresolved, id %d", i);
				state->createBreach(angle, message[index], message[index + 1], (int)i);
			} else {
				localBreach[i] = true;
			}
		} else if (breaches[i]->getHealth() > 0 && message[index] == 0) {
			if (breachCache.count(i) != 0 && !breachCache[i]) {
				CULog("Found unresolved breach that should be resolved, id %d", i);
				for (unsigned int j = breaches[i]->getHealth(); j > 0; j--) {
					state->resolveBreach((int)i);
				}
			} else {
				localBreach[i] = false;
			}
		}
		index += 4;
	}

	const auto& doors = state->getDoors();
	localDoor.clear();
	if (doors.size() != message[index++]) {
		CULog("ERROR: DOOR ARRAY SIZE DISCREPANCY; local %lu but server %d", doors.size(),
			  message[index - 1]);
		return false;
	}
	for (unsigned int i = 0; i < doors.size(); i++) {
		if (message[index]) {
			float angle = DECODE_FLOAT(message[index + 1], message[index + 2]);
			if (abs(doors[i]->getAngle() - angle) > floatEpsilon) {
				if (doorCache.count(i) != 0 && doorCache[i]) {
					CULog("Found open door that should be closed, id %d", i);
					state->createDoor(angle, (int)i);
				} else {
					localDoor[i] = true;
				}
			}
		} else {
			if (doors[i]->getIsActive()) {
				if (doorCache.count(i) != 0 && !doorCache[i]) {
					CULog("Found closed door that should be open, id %d", i);
					state->getDoors()[i]->reset();
				} else {
					localDoor[i] = false;
				}
			}
		}

		index += 3;
	}

	const auto& btns = state->getButtons();
	localBtn.clear();
	if (btns.size() != message[index++]) {
		CULog("ERROR: BUTTON ARRAY SIZE DISCREPANCY; local %lu but server %d", btns.size(),
			  message[index - 1]);
		return false;
	}
	// Map of ID of unpaired buttons to their angles
	localUnpairedBtn.clear();
	for (unsigned int i = 0; i < btns.size(); i++) {
		if (message[index]) {
			float angle = DECODE_FLOAT(message[index + 1], message[index + 2]);
			if (abs(btns[i]->getAngle() - angle) > floatEpsilon) {
				CULog("Found fixed button that should be broken, id %d", i);
				if (localUnpairedBtn.count(message[index + 3]) == 0) {
					// Haven't found button yet
					localUnpairedBtn[message[index + 3]] = angle;
				} else {
					int pairID = message[index + 3];
					if (btnCache.count(i) != 0 && btnCache[i]) {
						state->createButton(localUnpairedBtn[pairID], pairID, angle, (int)i);
					} else {
						localBtn[i] = true;
						localBtn[pairID] = true;
					}
				}
			}
		} else {
			if (btns[i]->getIsActive()) {
				if (btnCache.count(i) != 0 && !btnCache[i]) {
					CULog("Found active button that should be fixed, id %d; resolving both", i);
					state->resolveButton((int)i);
				} else {
					localBtn[i] = false;
				}
			}
		}

		index += 4;
	}

	reset();
	for (auto& t : localBreach) {
		breachCache[t.first] = t.second;
	}
	for (auto& t : localDoor) {
		doorCache[t.first] = t.second;
	}
	for (auto& t : localBtn) {
		btnCache[t.first] = t.second;
	}

	return true;
}

void StateReconciler::reset() {
	breachCache.clear();
	doorCache.clear();
	btnCache.clear();
}
