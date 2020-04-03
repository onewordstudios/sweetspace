#include "MatchmakingGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Globals.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

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
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	screenHeight = dimen.height;
	// Initialize the scene to a locked width
	if (assets == nullptr) {
		return false;
	} else if (!Scene::init(dimen)) {
		return false;
	}

	// Start up the input handler
	this->assets = assets;

	// Acquire the scene built by the asset loader and resize it the scene
	auto scene = assets->get<Node>("matchmaking");
	scene->setContentSize(dimen);
	scene->doLayout(); // Repositions the HUD

	// Get the scene components.
	host = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_hostbtn"));
	client =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_clientbtn"));
	roomLabel = std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_room"));
	roomInput = std::dynamic_pointer_cast<TextField>(assets->get<Node>("matchmaking_input"));
	textInput = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_inputbutton"));
	instrLabel = std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_instr"));

	mainScreen = assets->get<Node>("matchmaking_home");

	addChild(scene);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MatchmakingGraphRoot::dispose() {
	if (_active) {
		removeAllChildren();
		host = nullptr;
		client = nullptr;
		roomInput = nullptr;
		roomLabel = nullptr;
		_active = false;
	}
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void MatchmakingGraphRoot::reset() {}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void MatchmakingGraphRoot::update(float timestep) { roomLabel->setText(positionText()); }

/**
 * Returns integers representing which button has been tapped if any
 *
 * @param position The screen coordinates of the tap
 *
 * @return -1 if no button pressed 0 for host creation, 1 for client creation
 */
int MatchmakingGraphRoot::checkButtons(const cugl::Vec2& position) {
	if (position == Vec2::ZERO) {
		return -1;
	} else if (host->containsScreen(position)) {
		playerId = 0;
		roomLabel->setVisible(true);
		mainScreen->setVisible(false);
		return 0;
	} else if (client->containsScreen(position)) {
		roomInput->setVisible(true);
		roomInput->activate(2);
		roomLabel->setVisible(true);
		textInput->setVisible(true);
		instrLabel->setVisible(true);

		mainScreen->setVisible(false);
		return 1;
	} else {
		return -1;
	}
}

/**
 * Returns the text from inside the input field
 *
 * @return a string for the text
 */
std::string MatchmakingGraphRoot::getInput(const cugl::Vec2& position) {
	if (position == Vec2::ZERO) {
		return "";
	} else if (textInput->containsScreen(position)) {
		return roomInput->getText();
	}
	return "";
}

/**
 * Returns an informative string for the room id
 *
 * @return an informative string for the room id
 */
std::string MatchmakingGraphRoot::positionText() {
	stringstream ss;
	ss << "Room ID: " << roomId;
	return ss.str();
}
