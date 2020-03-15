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
constexpr float DOOR_WIDTH = 0.15f;
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
bool GameMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	input.init();
	net.initHost();

	for (int i = 0; i < MAX_EVENTS; i++) {
		breaches.push_back(BreachModel::alloc());
	}
	for (int i = 0; i < 3; i++) {
		donuts.push_back(DonutModel::alloc());
	}
	for (int i = 0; i < MAX_DOORS; i++) {
		doors.push_back(DoorModel::alloc());
		doors.at(i)->setAngle(3.14);
	}

	shipModel = ShipModel::alloc(donuts, breaches);
	gm.init(donuts, breaches, net, -1);
	while (net.getPlayerID() == -1) {
		net.update(shipModel);
	}
	playerId = net.getPlayerID();
	gm.setPlayerId(playerId);
	// gm.setDonuts(shipModel);
	donutModel = donuts.at(playerId);
	// Scene graph setup
	sgRoot.setBreaches(breaches);
	sgRoot.setDoors(doors);
	sgRoot.setDonuts(donuts);
	sgRoot.setPlayerId(playerId);
	sgRoot.init(assets);

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
	net.update(shipModel);
	// Reset the game if necessary
	// if (input.didReset()) {
	//	reset();
	//}

	// Breach health depletion
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (breaches.at(i) == nullptr) {
			continue;
		}
		float diff = (float)M_PI -
					 abs(abs(donutModel->getAngle() - breaches.at(i)->getAngle()) - (float)M_PI);

		if (playerId == breaches.at(i)->getPlayer() && diff < EPSILON_ANGLE &&
			!breaches.at(i)->isPlayerOn() && donutModel->getJumpOffset() == 0.0f) {
			breaches.at(i)->decHealth(1);
			breaches.at(i)->setIsPlayerOn(true);

			if (breaches.at(i)->getHealth() == 0) {
				net.resolveBreach(i);
			}

		} else if (diff > EPSILON_ANGLE && breaches.at(i)->isPlayerOn()) {
			breaches.at(i)->setIsPlayerOn(false);
		}
	}

	for (int i = 0; i < MAX_DOORS; i++) {
		if (doors.at(i) == nullptr || doors.at(i)->resolved()) {
			continue;
		}
		float diff =
			(float)M_PI - abs(abs(donutModel->getAngle() - doors.at(i)->getAngle()) - (float)M_PI);

		if (diff < DOOR_WIDTH) {
			// TODO: Real physics...
			donutModel->applyForce(-10 * donutModel->getVelocity());
		}
		if (diff < DOOR_ACTIVE_ANGLE) {
			doors.at(i)->addPlayer(playerId);
		} else {
			if (doors.at(i)->isPlayerOn(playerId)) {
				doors.at(i)->removePlayer(playerId);
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
	donutModel->update(timestep);

	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
