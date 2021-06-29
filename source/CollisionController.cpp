#include "CollisionController.h"

#include "MagicInternetBox.h"
#include "SoundEffectController.h"

/** The Angle in degrees for fixing a breach*/
constexpr float EPSILON_ANGLE = 5.2f;
/** The friction factor applied when moving through other players breaches */
constexpr float OTHER_BREACH_FRICTION = 0.2f;
/** The Angle in degrees for which a door can be activated*/
constexpr float DOOR_ACTIVE_ANGLE = 15.0f;

/** Jump height to trigger button press */
constexpr float BUTTON_JUMP_HEIGHT = 0.1f;

void breachCollisions(ShipModel& ship, uint8_t playerID) {
	const auto& donutModel = ship.getDonuts().at(playerID);
	const auto& soundEffects = SoundEffectController::getInstance();
	for (uint8_t i = 0; i < ship.getBreaches().size(); i++) {
		auto& breach = ship.getBreaches()[i];
		if (breach == nullptr || !breach->getIsActive()) {
			continue;
		}

		float diff = ship.getAngleDifference(donutModel->getAngle(), breach->getAngle());

		// Rolling over other player's breach
		if (!donutModel->isJumping() && playerID != breach->getPlayer() &&
			diff < globals::BREACH_WIDTH && breach->getHealth() != 0) {
			soundEffects->startEvent(SoundEffectController::SLOW, i);
			donutModel->setFriction(OTHER_BREACH_FRICTION);
			donutModel->transitionFaceState(DonutModel::FaceState::Dizzy);

			// Rolling over own breach
		} else if (playerID == breach->getPlayer() && diff < EPSILON_ANGLE &&
				   donutModel->getJumpOffset() == 0.0f && breach->getHealth() > 0) {
			if (!breach->isPlayerOn()) {
				soundEffects->startEvent(SoundEffectController::FIX, i);
				breach->decHealth(1);
				breach->setIsPlayerOn(true);
				MagicInternetBox::getInstance().resolveBreach(i);
			}
			donutModel->transitionFaceState(DonutModel::FaceState::Working);

			// Clearing breach flag
		} else if (breach->isPlayerOn() && diff > EPSILON_ANGLE) {
			breach->setIsPlayerOn(false);
			if (playerID == breach->getPlayer()) {
				soundEffects->endEvent(SoundEffectController::FIX, i);
			} else {
				soundEffects->endEvent(SoundEffectController::SLOW, i);
			}
		}
	}
}

void doorCollisions(ShipModel& ship, uint8_t playerID) {
	const auto& donutModel = ship.getDonuts().at(playerID);
	const auto& soundEffects = SoundEffectController::getInstance();

	// Normal Door
	for (int i = 0; i < ship.getDoors().size(); i++) {
		auto& door = ship.getDoors()[i];
		if (door == nullptr || door->halfOpen() || !door->getIsActive()) {
			continue;
		}

		float diff = donutModel->getAngle() - door->getAngle();
		float a = diff + ship.getSize() / 2;
		diff = a - floor(a / ship.getSize()) * ship.getSize() - ship.getSize() / 2;

		// Stop donut and push it out if inside
		if (abs(diff) < globals::DOOR_WIDTH) {
			soundEffects->startEvent(SoundEffectController::DOOR, i);
			donutModel->setVelocity(0);
			if (diff < 0) {
				float proposedAngle = door->getAngle() - globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle < 0 ? ship.getSize() - 1 : proposedAngle);
			} else {
				float proposedAngle = door->getAngle() + globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle >= ship.getSize() ? 0 : proposedAngle);
			}
		}

		// Active Door
		if (abs(diff) < DOOR_ACTIVE_ANGLE) {
			door->addPlayer(playerID);
			MagicInternetBox::getInstance().flagDualTask(i, playerID, 1);
			donutModel->transitionFaceState(DonutModel::FaceState::Colliding);

			// Inactive Door
		} else if (door->isPlayerOn(playerID)) {
			soundEffects->endEvent(SoundEffectController::DOOR, i);
			door->removePlayer(playerID);
			MagicInternetBox::getInstance().flagDualTask(i, playerID, 0);
		}
	}

	// Unopenable Door
	for (int i = 0; i < ship.getUnopenable().size(); i++) {
		auto& door = ship.getUnopenable()[i];
		if (door == nullptr || !door->getIsActive()) {
			continue;
		}

		float diff = donutModel->getAngle() - door->getAngle();
		float shipSize = ship.getSize();
		float a = diff + shipSize / 2;
		diff = a - floor(a / shipSize) * shipSize - shipSize / 2;

		// Stop donut and push it out if inside
		if (abs(diff) < globals::DOOR_WIDTH) {
			soundEffects->startEvent(SoundEffectController::DOOR, i + globals::UNOP_MARKER);
			donutModel->setVelocity(0);
			if (diff < 0) {
				float proposedAngle = ship.getUnopenable()[i]->getAngle() - globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle < 0 ? ship.getSize() : proposedAngle);
			} else {
				float proposedAngle = ship.getUnopenable()[i]->getAngle() + globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle > ship.getSize() ? 0 : proposedAngle);
			}

			// End sound effect otherwise
		} else {
			soundEffects->endEvent(SoundEffectController::DOOR, i + globals::UNOP_MARKER);
		}
	}
}

void buttonCollisions(ShipModel& ship, uint8_t playerID) {
	const auto& donutModel = ship.getDonuts().at(playerID);
	for (int i = 0; i < ship.getButtons().size(); i++) {
		auto& button = ship.getButtons().at(i);

		if (button == nullptr || !button->getIsActive()) {
			continue;
		}

		button->update();

		float diff = donutModel->getAngle() - button->getAngle();
		float shipSize = ship.getSize();
		float a = diff + shipSize / 2;
		diff = a - floor(a / shipSize) * shipSize - shipSize / 2;

		if (abs(diff) > globals::BUTTON_ACTIVE_ANGLE) {
			continue;
		}

		if (!donutModel->isDescending() || donutModel->getJumpOffset() >= BUTTON_JUMP_HEIGHT) {
			continue;
		}

		if (ship.flagButton(i)) {
			MagicInternetBox::getInstance().flagButton(i);
			if (button->getPair()->isJumpedOn()) {
				CULog("Resolving button");
				ship.resolveButton(i);
				MagicInternetBox::getInstance().resolveButton(i);
			}
		}
	}
}

void CollisionController::updateCollisions(ShipModel& ship, uint8_t playerID) {
	breachCollisions(ship, playerID);
	doorCollisions(ship, playerID);
	buttonCollisions(ship, playerID);
}
