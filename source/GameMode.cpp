//
//  SDGameScene.h
//  Ship Demo
//
//  This is the most important class in this demo.  This class manages the
//  gameplay for this demo.  It is a relativeluy simple class as we are not
//  worried about collisions.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design.  This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together.  However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  Author: Walker White
//  Version: 1/10/17
//
#include "GameMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "PlayerDonutModel.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** This is adjusted by screen aspect ratio to get the height */
constexpr unsigned int SCENE_WIDTH = 1024;
/** The maximum number of events on ship at any one time. This will probably need to scale with the
 * number of players*/
constexpr unsigned int MAX_EVENTS = 3;
/** The maximum number of doors on ship at any one time. This will probably need to scale with the
 * number of players*/
constexpr unsigned int MAX_DOORS = 1;
/** The Angle in radians for which a tap can registers as fixing a breach*/
constexpr float EPSILON_ANGLE = 0.09f;
/** The Angle in radians for which a collision occurs*/
constexpr float DOOR_WIDTH = 0.12f;
/** The Angle in radians for which a door can be activated*/
constexpr float DOOR_ACTIVE_ANGLE = 0.25f;

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
bool GameMode::init(const std::shared_ptr<cugl::AssetManager>& assets,
					std::shared_ptr<MagicInternetBox>& mib) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	input.init();
	net = mib;

	playerID = net->getPlayerID();
	shipModel = ShipModel::alloc(net->getNumPlayers(), MAX_EVENTS, MAX_DOORS, playerID);
	gm.init(shipModel, net);

	donutModel = shipModel->getDonuts().at(static_cast<unsigned long>(playerID));

	// Scene graph setup
	sgRoot.init(assets, shipModel, playerID);

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
	input.update(timestep);

	net->update(shipModel);

	// Breach health depletion
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (shipModel->getBreaches().at(i) == nullptr) {
			continue;
		}
		float diff = (float)M_PI -
					 abs(abs(donutModel->getAngle() - shipModel->getBreaches().at(i)->getAngle()) -
						 (float)M_PI);

		if (playerID == shipModel->getBreaches().at(i)->getPlayer() && diff < EPSILON_ANGLE &&
			!shipModel->getBreaches().at(i)->isPlayerOn() && donutModel->getJumpOffset() == 0.0f) {
			shipModel->getBreaches().at(i)->decHealth(1);
			shipModel->getBreaches().at(i)->setIsPlayerOn(true);

			if (shipModel->getBreaches().at(i)->getHealth() == 0) {
				net->resolveBreach(i);
			}

		} else if (diff > EPSILON_ANGLE && shipModel->getBreaches().at(i)->isPlayerOn()) {
			shipModel->getBreaches().at(i)->setIsPlayerOn(false);
		}
	}

	for (int i = 0; i < MAX_DOORS; i++) {
		if (shipModel->getDoors().at(i) == nullptr || shipModel->getDoors().at(i)->halfOpen() ||
			shipModel->getDoors().at(i)->getAngle() < 0) {
			continue;
		}
		float diff = (float)M_PI -
					 abs(abs(donutModel->getAngle() - shipModel->getDoors().at(i)->getAngle()) -
						 (float)M_PI);

		if (diff < DOOR_WIDTH) {
			// TODO: Real physics...
			donutModel->applyForce(-6 * donutModel->getVelocity());
		}
		if (diff < DOOR_ACTIVE_ANGLE) {
			shipModel->getDoors().at(i)->addPlayer(playerID);
			net->flagDualTask(i, playerID, 1);
		} else {
			if (shipModel->getDoors().at(i)->isPlayerOn(playerID)) {
				shipModel->getDoors().at(i)->removePlayer(playerID);
				net->flagDualTask(i, playerID, 0);
			}
		}
	}

	gm.update(timestep);
	float thrust = input.getRoll();

	// Move the donut (MODEL ONLY)
	donutModel->applyForce(thrust);
	// Jump Logic
	if (input.getTapLoc() != Vec2::ZERO && !donutModel->isJumping()) {
		donutModel->startJump();
	}

	for (unsigned int i = 0; i < shipModel->getDonuts().size(); i++) {
		shipModel->getDonuts()[i]->update(timestep);
	}

	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
