#include "GameMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "PlayerDonutModel.h"

using namespace cugl;
using namespace std;

#pragma region Constants
/** The Angle in degrees for fixing a breach*/
constexpr float EPSILON_ANGLE = 5.2f;
/** The Angle in degrees for which a door can be activated*/
constexpr float DOOR_ACTIVE_ANGLE = 15.0f;

/** Jump height to trigger button press */
constexpr float BUTTON_JUMP_HEIGHT = 0.1f;

// Friction
/** The friction factor applied when moving through other players breaches */
constexpr float OTHER_BREACH_FRICTION = 0.2f;

// Health
/** Grace period for a breach before it starts deducting health */
constexpr float BREACH_HEALTH_GRACE_PERIOD = 5.0f;
/** Amount of health to decrement each frame per breach */
constexpr float BREACH_HEALTH_PENALTY = 0.003f;

#pragma endregion
#pragma region Constructors

/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GameMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	isBackToMainMenu = false;
	gm.reset();

	// Music Initialization
	auto source = assets->get<Sound>("theme");
	if (AudioChannels::get()->currentMusic() == nullptr ||
		AudioChannels::get()->currentMusic()->getFile() != source->getFile()) {
		AudioChannels::get()->stopMusic(globals::MUSIC_FADE_OUT);
		AudioChannels::get()->queueMusic(source, true, source->getVolume(), globals::MUSIC_FADE_IN);
	}
	// Initialize the scene to a locked width
	cugl::Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	// Input Initialization
	input = InputController::getInstance();
	input->clear();

	// Sound (should already be initialized)
	soundEffects = SoundEffectController::getInstance();
	soundEffects->reset();

	// Network Initialization
	uint8_t playerID = net.getPlayerID().value();
	uint8_t levelID = net.getLevelNum().value();

	if (levelID >= MAX_NUM_LEVELS) {
		// Reached end of game

		// Return to main menu next frame
		isBackToMainMenu = true;
		AudioChannels::get()->stopMusic(1);

		// Initialize dummy crap so we don't crash this frame
		ship = ShipModel::alloc(0, 0, 0, 0, 0, 0);
	} else if (!tutorial::IS_TUTORIAL_LEVEL(levelID)) {
		const char* levelName = LEVEL_NAMES.at(levelID);

		CULog("Loading level %s b/c mib gave level num %d", levelName, levelID);
		uint8_t shipNumPlayers = net.getMaxNumPlayers();

		std::shared_ptr<LevelModel> level = assets->get<LevelModel>(levelName);
		unsigned int maxEvents = level->getMaxBreaches() * shipNumPlayers / globals::MIN_PLAYERS;
		unsigned int maxDoors =
			std::min(level->getMaxDoors() * shipNumPlayers / globals::MIN_PLAYERS,
					 static_cast<int>(shipNumPlayers) * 2 - 1);
		unsigned int maxButtons = level->getMaxButtons() * shipNumPlayers / globals::MIN_PLAYERS;
		if (maxButtons % 2 != 0) {
			maxButtons += 1;
		}
		ship = ShipModel::alloc(
			shipNumPlayers, maxEvents, maxDoors, playerID, level->getShipSize(shipNumPlayers),
			level->getInitHealth() * static_cast<float>(shipNumPlayers) / globals::MIN_PLAYERS,
			maxButtons);
		ship->initTimer(level->getTime());
		if (playerID == 0) {
			gm.emplace();
			gm->init(ship, level);
		}
	} else {
		// Tutorial Mode. Allocate an empty ship and let the gm do the rest.
		// Prepare for maximum hardcoding
		ship = ShipModel::alloc(0, 0, 0, 0, 0, 0);
		gm.emplace();
		gm->init(ship, levelID);

		// Ugly hack for the fact that GLaDOS is responsible for tutorial initialization
		if (playerID != 0) {
			gm.reset();
		}
	}

	donutModel = ship->getDonuts()[playerID];
	ship->setLevelNum(levelID);

	// Scene graph Initialization
	sgRoot.init(assets, ship, playerID);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameMode::dispose() {
	if (gm.has_value()) {
		gm->dispose();
		gm.reset();
	}
	sgRoot.dispose();
	donutModel = nullptr;
}

#pragma endregion
#pragma region Update Helpers

#pragma region Collision Handlers

void GameMode::breachCollisions() {
	uint8_t playerID = net.getPlayerID().value();
	for (uint8_t i = 0; i < ship->getBreaches().size(); i++) {
		auto& breach = ship->getBreaches()[i];
		if (breach == nullptr || !breach->getIsActive()) {
			continue;
		}

		float diff = ship->getAngleDifference(donutModel->getAngle(), breach->getAngle());

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
				net.resolveBreach(i);
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

void GameMode::doorCollisions() {
	int playerID = net.getPlayerID().value();

	// Normal Door
	for (int i = 0; i < ship->getDoors().size(); i++) {
		auto& door = ship->getDoors()[i];
		if (door == nullptr || door->halfOpen() || !door->getIsActive()) {
			continue;
		}

		float diff = donutModel->getAngle() - door->getAngle();
		float a = diff + ship->getSize() / 2;
		diff = a - floor(a / ship->getSize()) * ship->getSize() - ship->getSize() / 2;

		// Stop donut and push it out if inside
		if (abs(diff) < globals::DOOR_WIDTH) {
			soundEffects->startEvent(SoundEffectController::DOOR, i);
			donutModel->setVelocity(0);
			if (diff < 0) {
				float proposedAngle = door->getAngle() - globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle < 0 ? ship->getSize() - 1 : proposedAngle);
			} else {
				float proposedAngle = door->getAngle() + globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle >= ship->getSize() ? 0 : proposedAngle);
			}
		}

		// Active Door
		if (abs(diff) < DOOR_ACTIVE_ANGLE) {
			door->addPlayer(playerID);
			net.flagDualTask(i, playerID, 1);
			donutModel->transitionFaceState(DonutModel::FaceState::Colliding);

			// Inactive Door
		} else if (door->isPlayerOn(playerID)) {
			soundEffects->endEvent(SoundEffectController::DOOR, i);
			door->removePlayer(playerID);
			net.flagDualTask(i, playerID, 0);
		}
	}

	// Unopenable Door
	for (int i = 0; i < ship->getUnopenable().size(); i++) {
		auto& door = ship->getUnopenable()[i];
		if (door == nullptr || !door->getIsActive()) {
			continue;
		}

		float diff = donutModel->getAngle() - door->getAngle();
		float shipSize = ship->getSize();
		float a = diff + shipSize / 2;
		diff = a - floor(a / shipSize) * shipSize - shipSize / 2;

		// Stop donut and push it out if inside
		if (abs(diff) < globals::DOOR_WIDTH) {
			soundEffects->startEvent(SoundEffectController::DOOR, i + globals::UNOP_MARKER);
			donutModel->setVelocity(0);
			if (diff < 0) {
				float proposedAngle = ship->getUnopenable()[i]->getAngle() - globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle < 0 ? ship->getSize() : proposedAngle);
			} else {
				float proposedAngle = ship->getUnopenable()[i]->getAngle() + globals::DOOR_WIDTH;
				donutModel->setAngle(proposedAngle > ship->getSize() ? 0 : proposedAngle);
			}

			// End sound effect otherwise
		} else {
			soundEffects->endEvent(SoundEffectController::DOOR, i + globals::UNOP_MARKER);
		}
	}
}

void GameMode::buttonCollisions() {
	for (int i = 0; i < ship->getButtons().size(); i++) {
		auto& button = ship->getButtons().at(i);

		if (button == nullptr || !button->getIsActive()) {
			continue;
		}

		button->update();

		float diff = donutModel->getAngle() - button->getAngle();
		float shipSize = ship->getSize();
		float a = diff + shipSize / 2;
		diff = a - floor(a / shipSize) * shipSize - shipSize / 2;

		if (abs(diff) > globals::BUTTON_ACTIVE_ANGLE) {
			continue;
		}

		if (!donutModel->isDescending() || donutModel->getJumpOffset() >= BUTTON_JUMP_HEIGHT) {
			continue;
		}

		if (ship->flagButton(i)) {
			net.flagButton(i);
			if (button->getPair()->isJumpedOn()) {
				CULog("Resolving button");
				ship->resolveButton(i);
				net.resolveButton(i);
			}
		}
	}
}

#pragma endregion

void GameMode::updateStabilizer() {
	auto& stabilizer = ship->getStabilizer();

	if (!stabilizer.getIsActive()) {
		return;
	}

	// If there's not enough time left in the level for the challenge, bail
	if (stabilizer.getIsActive() && !ship->getTimeless() &&
		trunc(ship->timeLeftInTimer) <= globals::ROLL_CHALLENGE_LENGTH) {
		stabilizer.reset();
		return;
	}

	bool allRoll = true;
	const auto& allDonuts = ship->getDonuts();

	for (const auto& allDonut : allDonuts) {
		if (!allDonut->getIsActive()) {
			continue;
		}
		if (stabilizer.isLeft()) {
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
		stabilizer.incrementProgress();
	}

	if (stabilizer.getIsWin()) {
		net.succeedAllTask();
		ship->stabilizerTutorial = true;
		stabilizer.finish();
	} else if (trunc(ship->canonicalTimeElapsed) == trunc(stabilizer.getEndTime())) {
		net.failAllTask();
		ship->failAllTask();
	}
}

void GameMode::updateDonuts(float timestep) {
	uint8_t playerID = net.getPlayerID().value();

	// Jump logic check
	// We wanted donuts to be able to jump on the win screen, but in the absence of that happening
	// anytime soon, I'm sticking this here for cleanliness
	if (input->hasJumped() && !donutModel->isJumping()) {
		soundEffects->startEvent(SoundEffectController::JUMP, playerID);
		donutModel->startJump();
		net.jump(playerID);
	} else {
		soundEffects->endEvent(SoundEffectController::JUMP, playerID);
	}

	// Move the donut (MODEL ONLY)
	float thrust = input->getRoll();
	donutModel->applyForce(thrust);
	// Attempt to recover to idle animation
	donutModel->transitionFaceState(DonutModel::FaceState::Idle);

	for (auto& donut : ship->getDonuts()) {
		donut->update(timestep);
	}
}

void GameMode::updateTimer(float timestep) {
	if (ship->timerEnded()) {
		return;
	}

	bool allButtonsInactive = true;
	auto& buttons = ship->getButtons();

	for (auto& button : buttons) {
		if (button->getIsActive()) {
			allButtonsInactive = false;
			break;
		}
	}
	ship->updateTimer(timestep, allButtonsInactive);
}

void GameMode::updateHealth() {
	auto& breaches = ship->getBreaches();

	// Breach health drain
	for (const auto& breach : breaches) {
		// this should be adjusted based on the level and number of players
		if (breach->getIsActive() && trunc(ship->timePassed()) - trunc(breach->getTimeCreated()) >
										 BREACH_HEALTH_GRACE_PERIOD) {
			ship->decHealth(BREACH_HEALTH_PENALTY);
		}
	}
}

void GameMode::updateDoors() {
	for (const auto& door : ship->getDoors()) {
		if (door->getIsActive() && door->resolved()) {
			door->raiseDoor();
		}
	}
}

bool GameMode::lossCheck() {
	if (ship->getHealth() >= 1) {
		return false;
	}

	sgRoot.setStatus(GameGraphRoot::Loss);
	if (sgRoot.getAndResetLastButtonPressed() == GameGraphRoot::GameButton::Restart) {
		net.restartGame();
	}

	return true;
}

bool GameMode::winCheck() {
	if (!ship->timerEnded() || ship->getHealth() <= 0) {
		return false;
	}

	sgRoot.setStatus(GameGraphRoot::Win);
	if (sgRoot.getAndResetLastButtonPressed() == GameGraphRoot::GameButton::NextLevel) {
		CULog("Next Level Pressed");
		net.nextLevel();
	}

	return true;
}

bool GameMode::connectionUpdate(float timestep) {
	switch (net.matchStatus()) {
		case MagicInternetBox::Disconnected:
		case MagicInternetBox::ClientRoomInvalid:
		case MagicInternetBox::ReconnectError:
			if (net.reconnect()) {
				net.update();
			}
			sgRoot.setStatus(GameGraphRoot::Reconnecting);
			sgRoot.update(timestep);
			return false;
		case MagicInternetBox::Reconnecting:
		case MagicInternetBox::ReconnectPending:
			// Still Reconnecting
			net.update();
			sgRoot.setStatus(GameGraphRoot::Reconnecting);
			sgRoot.update(timestep);
			return false;
		case MagicInternetBox::ClientRoomFull:
		case MagicInternetBox::GameEnded:
			// Game Ended, Replace with Another Screen
			CULog("Game Ended");
			net.update(ship);
			sgRoot.update(timestep);
			return false;
		case MagicInternetBox::GameStart:
			net.update(ship);
			sgRoot.setStatus(GameGraphRoot::Normal);
			break;
		default:
			CULog("ERROR: Uncaught MatchmakingStatus Value Occurred");
			break;
	}
	return true;
}

#pragma endregion
#pragma region Gameplay

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameMode::update(float timestep) {
	// Check if need to go back to menu
	if (!isBackToMainMenu) {
		isBackToMainMenu = sgRoot.getIsBackToMainMenu();
		if (isBackToMainMenu) {
			AudioChannels::get()->stopMusic(1);
		}
	}

	// Connection Status Checks
	if (!connectionUpdate(timestep)) {
		return;
	}

	input->update(timestep);

	// Check for loss
	if (lossCheck()) {
		sgRoot.update(timestep);
		return;
	}

	// Check for Win
	if (winCheck()) {
		sgRoot.update(timestep);
		return;
	}

	updateTimer(timestep);
	updateDonuts(timestep);

	breachCollisions();
	doorCollisions();
	updateDoors();
	buttonCollisions();

	updateHealth();
	updateStabilizer();

	if (gm.has_value()) {
		gm->update(timestep);
	}

	sgRoot.update(timestep);
}
#pragma endregion

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
