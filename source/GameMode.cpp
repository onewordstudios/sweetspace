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
/** The Angle in radians for which a tap can registers as fixing a breach*/
constexpr float EPSILON_ANGLE = 0.09f;

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

	sgRoot.init(assets);
	Vec2 donutPos = sgRoot.getDonutNode()->getPosition();
	donutModel = DonutModel::alloc(donutPos);
	donutModel->setSprite(std::dynamic_pointer_cast<AnimationNode>(sgRoot.getDonutNode()));
	donuts.push_back(donutModel);
	for (int i = 0; i < MAX_EVENTS; i++) {
		breaches.push_back(BreachModel::alloc());
	}
	sgRoot.setBreaches(breaches);
	sgRoot.setDonutModel(donutModel);

	gm.init(donuts, breaches);

	CULog("Initializing Network");
	CULog("Network Response %d", network.initHost());

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
	network.update();

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

		if (diff < EPSILON_ANGLE && !breaches.at(i)->isPlayerOn() &&
			donutModel->getJumpOffset() == 0.0f) {
			breaches.at(i)->decHealth(1);
			breaches.at(i)->setIsPlayerOn(true);
		} else if (diff > EPSILON_ANGLE && breaches.at(i)->isPlayerOn()) {
			breaches.at(i)->setIsPlayerOn(false);
		}
	}

	// Exception thrown : read access violation.** array** was nullptr.occurred
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
