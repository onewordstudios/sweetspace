#include "MatchmakingMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Constants

/** This is adjusted by screen aspect ratio to get the height */
constexpr unsigned int SCENE_WIDTH = 1024;
/** The maximum number of events on ship at any one time. This will probably need to scale with the
 * number of players*/
constexpr unsigned int MAX_EVENTS = 3;

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
bool MatchmakingMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	input.init();
	for (int i = 0; i < MAX_EVENTS; i++) {
		breaches.push_back(BreachModel::alloc());
	}
	for (int i = 0; i < 3; i++) {
		donuts.push_back(DonutModel::alloc());
	}

	shipModel = ShipModel::alloc(donuts, breaches);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MatchmakingMode::dispose() { input.dispose(); }

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void MatchmakingMode::reset() { input.clear(); }

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void MatchmakingMode::update(float timestep) {
	input.update(timestep);
	if (net != nullptr) {
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
			float diff =
				(float)M_PI -
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
		float thrust = input.getRoll();
	}

	/**
	 * Draws the game.
	 */
	void MatchmakingMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
		sgRoot.render(batch);
	}
