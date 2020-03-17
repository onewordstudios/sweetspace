#include "MatchmakingGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** This is adjusted by screen aspect ratio to get the height */
constexpr unsigned int SCENE_WIDTH = 1024;

/** 2 pi */
constexpr float TWO_PI = (float)(2 * M_PI);

/** Pi over 180 for converting between degrees and radians */
constexpr float PI_180 = (float)(M_PI / 180);

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS = 550;

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
bool MatchmakingGraphRoot::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	screenHeight = dimen.height;
	// Initialize the scene to a locked width
	if (assets == nullptr) {
		return false;
	}
	else if (!Scene::init(dimen)) {
		return false;
	}

	// Start up the input handler
	this->assets = assets;

	// Acquire the scene built by the asset loader and resize it the scene
	auto scene = assets->get<Node>("matchmaking");
	scene->setContentSize(dimen);

	// Get the scene components.
	host = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_host"));
	client = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client"));

	addChild(scene);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MatchmakingGraphRoot::dispose() {
	if (_active) {
		removeAllChildren();
		_active = false;
	}
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void MatchmakingGraphRoot::reset() {

}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void MatchmakingGraphRoot::update(float timestep) {
	// "Drawing" code.  Move everything BUT the donut
	// Update the HUD
	coordHUD->setText(positionText());


}

/**
 * Returns an informative string for the position
 *
 * This function is for writing the current donut position to the HUD.
 *
 * @param coords The current donut coordinates
 *
 * @return an informative string for the position
 */
std::string MatchmakingGraphRoot::positionText() {
	stringstream ss;
	ss << "Angle: (" << (float)donuts.at(playerId)->getAngle() / PI_180 << ")";
	return ss.str();
}
