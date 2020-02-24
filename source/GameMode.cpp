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

/** The parallax for each layer */
constexpr float PARALLAX_AMT = 0.1f;

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

	// Create the ship model
	donutModel = sgRoot.getDonutModel();

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameMode::dispose() {
	input.dispose();
	sgRoot.dispose();
	donutModel = nullptr;
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void GameMode::reset() {
	// Reset the ships and input
	donutModel->reset();
	sgRoot.reset();
	input.clear();
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

	// Reset the game if necessary
	// if (input.didReset()) {
	//	reset();
	//}

	float thrust = input.getRoll();

	// Move the ship (MODEL ONLY)
	donutModel->setTurning(thrust);
	donutModel->update(timestep);

	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
