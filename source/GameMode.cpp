#include "GameMode.h"

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
/** The Angle in degrees for which a collision occurs*/
constexpr float DOOR_WIDTH = 7.0f;
/** The Angle in degrees for which a breach donut collision occurs*/
constexpr float BREACH_WIDTH = 11.0f;
/** The Angle in degrees for which a door can be activated*/
constexpr float DOOR_ACTIVE_ANGLE = 15.0f;
/** Force to push back during collision */
constexpr float REBOUND_FORCE = -6;
/** Starting time for the round */
constexpr unsigned int TIME = 45;
/** Initial health of the ship */
int initHealth;

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
	input.init();

	// Network Initialization
	net = MagicInternetBox::getInstance();
	playerID = net->getPlayerID();
	roomId = net->getRoomID();

	std::shared_ptr<LevelModel> level = assets->get<LevelModel>(LEVEL_ONE_KEY);
	float shipSize = level->getShipSize();
	initHealth = level->getInitHealth();
	ship = ShipModel::alloc(net->getNumPlayers(), level->getMaxBreaches(), level->getMaxDoors(),
							playerID, shipSize, initHealth);
	gm.init(ship, level);

	donutModel = ship->getDonuts().at(static_cast<unsigned long>(playerID));
	ship->initTimer(level->getTime());

	// Scene graph Initialization
	sgRoot.init(assets, ship, playerID);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameMode::dispose() {
	input.dispose();
	gm.dispose();
	sgRoot.dispose();
	donutModel = nullptr;
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void GameMode::reset() {
	donutModel->reset();
	sgRoot.reset();
	input.clear();
	gm.clear();
}

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
	input.update(timestep);

	if (!(ship->timerEnded())) {
		ship->updateTimer(timestep);
	}

	// Breach health depletion
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breach = ship->getBreaches().at(i);
		if (breach == nullptr) {
			continue;
		}
		float diff = ship->getSize() / 2 -
					 abs(abs(donutModel->getAngle() - breach->getAngle()) - ship->getSize() / 2);

		if (!donutModel->isJumping() && playerID != breach->getPlayer() && diff < BREACH_WIDTH &&
			breach->getHealth() != 0) {
			donutModel->applyForce(REBOUND_FORCE * donutModel->getVelocity());
		} else if (playerID == breach->getPlayer() && diff < EPSILON_ANGLE &&
				   !breach->isPlayerOn() && donutModel->getJumpOffset() == 0.0f &&
				   breach->getHealth() > 0) {
			breach->decHealth(1);
			breach->setIsPlayerOn(true);

			if (breach->getHealth() == 0) {
				net->resolveBreach(i);
			}

		} else if (diff > EPSILON_ANGLE && breach->isPlayerOn()) {
			breach->setIsPlayerOn(false);
		}
	}

	for (int i = 0; i < ship->getDoors().size(); i++) {
		if (ship->getDoors().at(i) == nullptr || ship->getDoors().at(i)->halfOpen() ||
			ship->getDoors().at(i)->getAngle() < 0) {
			continue;
		}
		float diff = ship->getSize() / 2 -
					 abs(abs(donutModel->getAngle() - ship->getDoors().at(i)->getAngle()) -
						 ship->getSize() / 2);

		if (diff < DOOR_WIDTH) {
			// TODO: Real physics...
			donutModel->applyForce(REBOUND_FORCE * donutModel->getVelocity());
		}
		if (diff < DOOR_ACTIVE_ANGLE) {
			ship->getDoors().at(i)->addPlayer(playerID);
			net->flagDualTask(i, playerID, 1);
		} else {
			if (ship->getDoors().at(i)->isPlayerOn(playerID)) {
				ship->getDoors().at(i)->removePlayer(playerID);
				net->flagDualTask(i, playerID, 0);
			}
		}
	}

	if ((ship->getBreaches().size()) == 0) {
		ship->setHealth(initHealth);
	} else {
		int h = 0;
		for (int i = 0; i < ship->getBreaches().size(); i++) {
			h = h + ship->getBreaches().at(i)->getHealth();
		}
		ship->setHealth(initHealth + 1 - h);
	}

	gm.update(timestep);

	// Move the donut (MODEL ONLY)
	float thrust = input.getRoll();
	donutModel->applyForce(thrust);
	// Jump Logic
	if (input.getTapLoc() != Vec2::ZERO && !donutModel->isJumping()) {
		donutModel->startJump();
		net->jump(playerID);
	}

	for (unsigned int i = 0; i < ship->getDonuts().size(); i++) {
		ship->getDonuts()[i]->update(timestep);
	}

	if(ship->getChallenge() && trunc(ship->timer) <= 6) {
		ship->setChallenge(false);

	}

	if (gm.allPlayerChallenge() && trunc(ship->timer) > 6) {
		if (!(ship->getChallenge())) {
			ship->setChallengeProg(0);
			endTime = trunc(ship->timer) - 6;
			ship->setChallenge(true);

		}
        for (unsigned int i = 0; i < ship->getDonuts().size(); i++) {
            if(ship->getRollDir() == -1){
                if (ship->getDonuts()[i]->getVelocity() < 0) {
                	ship->updateChallengeProg();
                }
            } else {
                if (ship->getDonuts()[i]->getVelocity() > 0) {
                	ship->updateChallengeProg();
                }
            }
        }
		if (trunc(ship->timer) == endTime) {
			CULog("End Challenge");
			if (ship->getChallengeProg() > 10) {
				CULog("Challenge Completed");
			} else {
				CULog("Challenge Failed");
				net->failAllTask();
			}
			gm.setAllPlayerChallenge(false);
			ship->setChallenge(false);
			ship->setChallengeProg(0);
		}
	} else {
		gm.setAllPlayerChallenge(false);
	}

	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
