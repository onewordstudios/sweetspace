﻿#include "GameMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "PlayerDonutModel.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout
/** The Angle in degrees for fixing a breach*/
constexpr float EPSILON_ANGLE = 5.2f;
/** The Angle in degrees for which a door can be activated*/
constexpr float DOOR_ACTIVE_ANGLE = 15.0f;
/** Angles to adjust per frame to prevent door tunneling */
constexpr float ANGLE_ADJUST = 0.5f;

// Friction
/** The friction factor while fixing a breach */
constexpr float FIX_BREACH_FRICTION = 0.5f;
/** The friction factor applied when moving through other players breaches */
constexpr float OTHER_BREACH_FRICTION = 0.2f;

#pragma mark -
#pragma mark Constructors

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
	// Music Initialization
	auto source = assets->get<Sound>("theme");
	AudioChannels::get()->playMusic(source, true, source->getVolume());

	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	// Input Initialization
	input = InputController::getInstance();
	input->clear();

	// Network Initialization
	net = MagicInternetBox::getInstance();
	playerID = net->getPlayerID();
	roomId = net->getRoomID();

	std::shared_ptr<LevelModel> level = assets->get<LevelModel>(LEVEL_ONE_KEY);
	ship = ShipModel::alloc(net->getNumPlayers(), level->getMaxBreaches(), level->getMaxDoors(),
							playerID, (float)level->getShipSize((int)net->getNumPlayers()),
							level->getInitHealth());
	gm.init(ship, level);

	donutModel = ship->getDonuts().at(static_cast<unsigned long>(playerID));
	ship->initTimer(level->getTime());
	ship->setHealth(globals::INITIAL_SHIP_HEALTH);

	// Scene graph Initialization
	sgRoot.init(assets, ship, playerID);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameMode::dispose() {
	gm.dispose();
	sgRoot.dispose();
	donutModel = nullptr;
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameMode::update(float timestep) {
	// Connection Status Checks
	switch (net->matchStatus()) {
		case MagicInternetBox::Disconnected:
		case MagicInternetBox::ClientRoomInvalid:
		case MagicInternetBox::ReconnectError:
			if (net->reconnect(roomId)) {
				net->update();
			}
			sgRoot.setStatus(MagicInternetBox::Disconnected);
			sgRoot.update(timestep);
			return;
		case MagicInternetBox::Reconnecting:
			// Still Reconnecting
			net->update();
			sgRoot.setStatus(MagicInternetBox::Reconnecting);
			sgRoot.update(timestep);
			return;
		case MagicInternetBox::ClientRoomFull:
		case MagicInternetBox::GameEnded:
			// Insert Game End
			net->update(ship);
			sgRoot.setStatus(MagicInternetBox::GameEnded);
			sgRoot.update(timestep);
			return;
		case MagicInternetBox::GameStart:
			net->update(ship);
			sgRoot.setStatus(MagicInternetBox::GameStart);
			break;
		default:
			CULog("ERROR: Uncaught MatchmakingStatus Value Occurred");
	}

	// Only process game logic if properly connected to game
	input->update(timestep);

	if (!(ship->timerEnded())) {
		ship->updateTimer(timestep);
	}

	// Breach Checks
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breach = ship->getBreaches().at(i);
		if (breach == nullptr) {
			continue;
		}
		float diff = ship->getSize() / 2 -
					 abs(abs(donutModel->getAngle() - breach->getAngle()) - ship->getSize() / 2);
		if (!donutModel->isJumping() && playerID != breach->getPlayer() &&
			diff < globals::BREACH_WIDTH && breach->getHealth() != 0) {
			// Slow player by drag factor
			donutModel->setFriction(OTHER_BREACH_FRICTION);
		} else if (playerID == breach->getPlayer() && diff < EPSILON_ANGLE &&
				   donutModel->getJumpOffset() == 0.0f && breach->getHealth() > 0) {
			if (!breach->isPlayerOn()) {
				// Decrement Health
				breach->decHealth(1);
				breach->setIsPlayerOn(true);
			}

			// Slow player by friction factor if not already slowed more
			if (donutModel->getFriction() > FIX_BREACH_FRICTION) {
				donutModel->setFriction(FIX_BREACH_FRICTION);
			}

			// Resolve Breach if health is 0
			if (breach->getHealth() == 0) {
				net->resolveBreach(i);
			}

		} else if (diff > EPSILON_ANGLE && breach->isPlayerOn()) {
			breach->setIsPlayerOn(false);
		}
	}

	// Door Checks
	for (int i = 0; i < ship->getDoors().size(); i++) {
		if (ship->getDoors().at(i) == nullptr || ship->getDoors().at(i)->halfOpen() ||
			ship->getDoors().at(i)->getAngle() < 0) {
			continue;
		}
		float diff = donutModel->getAngle() - ship->getDoors().at(i)->getAngle();
		float a = diff + ship->getSize() / 2;
		diff = a - floor(a / ship->getSize()) * ship->getSize() - ship->getSize() / 2;

		if (abs(diff) < globals::DOOR_WIDTH) {
			// Stop donut and push it out if inside
			donutModel->setVelocity(0);
			if (diff < 0) {
				donutModel->setAngle(donutModel->getAngle() - ANGLE_ADJUST < 0.0f
										 ? ship->getSize()
										 : donutModel->getAngle() - ANGLE_ADJUST);
			} else {
				donutModel->setAngle(donutModel->getAngle() + ANGLE_ADJUST > ship->getSize()
										 ? 0.0f
										 : donutModel->getAngle() + ANGLE_ADJUST);
			}
		}
		if (abs(diff) < DOOR_ACTIVE_ANGLE) {
			ship->getDoors().at(i)->addPlayer(playerID);
			net->flagDualTask(i, playerID, 1);
		} else {
			if (ship->getDoors().at(i)->isPlayerOn(playerID)) {
				ship->getDoors().at(i)->removePlayer(playerID);
				net->flagDualTask(i, playerID, 0);
			}
		}
	}

	for (int i = 0; i < ship->getBreaches().size(); i++) {
		// this should be adjusted based on the level and number of players
		if (ship->getBreaches().at(i) != nullptr &&
			trunc(ship->getBreaches().at(i)->getTimeCreated()) - trunc(ship->timer) > 15) {
			ship->decHealth(0.01);
		}
	}

	gm.update(timestep);

	// Move the donut (MODEL ONLY)
	float thrust = input->getRoll();
	donutModel->applyForce(thrust);
	// Jump Logic
	if (input->hasJumped() && !donutModel->isJumping()) {
		donutModel->startJump();
		net->jump(playerID);
	}

	for (unsigned int i = 0; i < ship->getDonuts().size(); i++) {
		ship->getDonuts()[i]->update(timestep);
	}

	if (ship->getChallenge() && trunc(ship->timer) <= globals::ROLL_CHALLENGE_LENGTH) {
		ship->setChallenge(false);
	}

	if (ship->getChallenge() && trunc(ship->timer) > globals::ROLL_CHALLENGE_LENGTH) {
		for (unsigned int i = 0; i < ship->getDonuts().size(); i++) {
			if (ship->getRollDir() == 0) {
				if (ship->getDonuts()[i]->getVelocity() < 0) {
					ship->updateChallengeProg();
				}
			} else {
				if (ship->getDonuts()[i]->getVelocity() > 0) {
					ship->updateChallengeProg();
				}
			}
		}
		if (ship->getChallengeProg() > 30 || trunc(ship->timer) == trunc(ship->getEndTime())) {
			if (ship->getChallengeProg() < 10) {
				net->failAllTask();
				float h = ship->getHealth();
				ship->setHealth(h - 3);
			}
			ship->setChallenge(false);
			ship->setChallengeProg(0);
		}
	}

	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
