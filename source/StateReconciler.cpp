#include "StateReconciler.h"

/** Just the top bit in a byte */
constexpr uint8_t TOP_BIT_MASK = 1 << 7;

/** The precision to multiply floating point numbers by */
constexpr float FLOAT_PRECISION = 10.0f;

/** One byte */
constexpr uint16_t ONE_BYTE = 256;

/** How close to consider floating point numbers identical */
constexpr float FLOAT_EPSILON = 0.1f;

float StateReconciler::decodeFloat(uint8_t m1, uint8_t m2) {
	return static_cast<float>(m1 + ONE_BYTE * m2) / FLOAT_PRECISION;
}

void StateReconciler::encodeFloat(float f, std::vector<uint8_t>& out) {
	auto ff = static_cast<uint16_t>(f * FLOAT_PRECISION);
	out.push_back(static_cast<uint8_t>(ff % ONE_BYTE));
	out.push_back(static_cast<uint8_t>(ff / ONE_BYTE));
}

constexpr uint8_t StateReconciler::ENCODE_LEVEL_NUM(uint8_t level, bool parity) {
	return parity ? level : (level | TOP_BIT_MASK);
}

std::pair<uint8_t, bool> StateReconciler::decodeLevelNum(uint8_t encodedLevel) {
	if ((encodedLevel & TOP_BIT_MASK) > 0) {
		return {encodedLevel ^ TOP_BIT_MASK, false};
	}
	return {encodedLevel, true};
}

void StateReconciler::encode(const std::shared_ptr<ShipModel>& state, std::vector<uint8_t>& data,
							 uint8_t level, bool parity) {
	// Level data first
	data.push_back(ENCODE_LEVEL_NUM(level, parity));

	// Adding health and timer
	auto health = state->getHealth();
	encodeFloat(health < 0 ? 0 : health, data);
	encodeFloat(state->timeLeftInTimer, data);

	// Send Breaches
	const auto& breaches = state->getBreaches();
	data.push_back(static_cast<uint8_t>(breaches.size()));
	for (const auto& breach : breaches) {
		data.push_back(breach->getHealth());
		data.push_back(breach->getPlayer());
		encodeFloat(breach->getAngle(), data);
	}

	// Send Doors
	const auto& doors = state->getDoors();
	data.push_back(static_cast<uint8_t>(doors.size()));
	for (const auto& door : doors) {
		if (!door->getIsActive()) {
			data.push_back(0);
			data.push_back(0);
			data.push_back(0);
		} else {
			data.push_back(1);
			encodeFloat(door->getAngle(), data);
		}
	}

	// Send Buttons
	const auto& btns = state->getButtons();
	data.push_back(static_cast<uint8_t>(btns.size()));
	for (const auto& btn : btns) {
		if (!btn->getIsActive()) {
			data.push_back(0);
			data.push_back(0);
			data.push_back(0);
			data.push_back(0);
		} else {
			data.push_back(1);
			encodeFloat(btn->getAngle(), data);
			data.push_back(btn->getPairID());
		}
	}
}

bool StateReconciler::reconcile(const std::shared_ptr<ShipModel>& state,
								const std::vector<uint8_t>& message, uint8_t level, bool parity) {
	auto levelData = decodeLevelNum(message[1]);
	if (levelData.first != level || levelData.second != parity) {
		return false;
	}

	size_t index = 2;

	float health = decodeFloat(message[index], message[index + 1]);
	index += 2;
	float timer = decodeFloat(message[index], message[index + 1]);
	index += 2;

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
	for (uint8_t i = 0; i < breaches.size(); i++) {
		if (breaches[i]->getHealth() == 0 && message[index] > 0) {
			float angle = decodeFloat(message[index + 2], message[index + 3]);
			if (breachCache.count(i) != 0 && breachCache[i]) {
				CULog("Found resolved breach that should be unresolved, id %d", i);
				state->createBreach(angle, message[index], message[index + 1], static_cast<int>(i));
			} else {
				localBreach[i] = true;
			}
		} else if (breaches[i]->getHealth() > 0 && message[index] == 0) {
			if (breachCache.count(i) != 0 && !breachCache[i]) {
				CULog("Found unresolved breach that should be resolved, id %d", i);
				for (unsigned int j = breaches[i]->getHealth(); j > 0; j--) {
					state->resolveBreach(static_cast<int>(i));
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
	for (uint8_t i = 0; i < doors.size(); i++) {
		if (message[index] != 0u) {
			float angle = decodeFloat(message[index + 1], message[index + 2]);
			if (abs(doors[i]->getAngle() - angle) > FLOAT_EPSILON) {
				if (doorCache.count(i) != 0 && doorCache[i]) {
					CULog("Found open door that should be closed, id %d", i);
					state->createDoor(angle, static_cast<int>(i));
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
	for (uint8_t i = 0; i < btns.size(); i++) {
		if (message[index] != 0u) {
			float angle = decodeFloat(message[index + 1], message[index + 2]);
			if (abs(btns[i]->getAngle() - angle) > FLOAT_EPSILON) {
				CULog("Found fixed button that should be broken, id %d", i);
				if (localUnpairedBtn.count(message[index + 3]) == 0) {
					// Haven't found button yet
					localUnpairedBtn[message[index + 3]] = angle;
				} else {
					uint8_t pairID = message[index + 3];
					if (btnCache.count(i) != 0 && btnCache[i]) {
						state->createButton(localUnpairedBtn[pairID], pairID, angle, i);
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
					state->resolveButton(static_cast<int>(i));
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
