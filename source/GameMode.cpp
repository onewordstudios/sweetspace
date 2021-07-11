#include "GameMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "PlayerDonutModel.h"

using namespace cugl;
using namespace std;

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

void GameMode::applyInputsToPlayerDonut(float timestep) {
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
	applyInputsToPlayerDonut(timestep);

	ship->update(timestep);

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
