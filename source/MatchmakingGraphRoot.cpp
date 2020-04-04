#include "MatchmakingGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Globals.h"

using namespace cugl;

/** Number of buttons for room ID entry */
constexpr unsigned int NUM_DIGITS = 10;

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
	hostBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_hostbtn"));
	clientBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_clientbtn"));
	hostLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_host_wrap_plate_room"));

	mainScreen = assets->get<Node>("matchmaking_home");
	hostScreen = assets->get<Node>("matchmaking_host");
	clientScreen = assets->get<Node>("matchmaking_client");

	clientLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_client_wrap_plate_room"));
	clientJoinBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_wrap_joinbtn"));

	clientClearBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_buttons_btnclear"));

	for (unsigned int i = 0; i < NUM_DIGITS; i++) {
		clientRoomBtns.push_back(std::dynamic_pointer_cast<Button>(
			assets->get<Node>("matchmaking_client_buttons_btn" + std::to_string(i))));
	}

	updateClientLabel();

	addChild(scene);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MatchmakingGraphRoot::dispose() {
	if (_active) {
		removeAllChildren();
		hostBtn = nullptr;
		clientBtn = nullptr;
		hostLabel = nullptr;
		_active = false;
	}
}

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
void MatchmakingGraphRoot::update(float timestep) {
	if (hostScreen->isVisible()) {
		if (roomID.length() == globals::ROOM_LENGTH &&
			hostLabel->getText().length() != (2 * globals::ROOM_LENGTH - 1)) {
			std::ostringstream disp;
			for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
				disp << roomID.at(i);
				if (i < globals::ROOM_LENGTH - 1) {
					disp << ' ';
				}
			}
			hostLabel->setText(disp.str());
		}
	}
}

/**
 * Returns integers representing which button has been tapped if any
 *
 * @param position The screen coordinates of the tap
 *
 * @return -1 if no button pressed 0 for host creation, 1 for client creation
 */
MatchmakingGraphRoot::PressedButton MatchmakingGraphRoot::checkButtons(const cugl::Vec2& position) {
	if (position == Vec2::ZERO) {
		return None;
	}

	if (mainScreen->isVisible()) {
		if (hostBtn->containsScreen(position)) {
			playerID = 0;
			hostScreen->setVisible(true);
			mainScreen->setVisible(false);
			return StartHost;
		}
		if (clientBtn->containsScreen(position)) {
			hostLabel->setVisible(true);
			clientJoinBtn->setVisible(true);

			mainScreen->setVisible(false);
			clientScreen->setVisible(true);
			return StartClient;
		}
	} else if (clientScreen->isVisible()) {
		if (clientJoinBtn->containsScreen(position)) {
			if (clientEnteredRoom.size() != globals::ROOM_LENGTH) {
				return None;
			}

			std::ostringstream room;
			for (int i = 0; i < globals::ROOM_LENGTH; i++) {
				room << clientEnteredRoom[i];
			}

			roomID = room.str();

			return ClientConnect;
		}

		for (unsigned int i = 0; i < NUM_DIGITS; i++) {
			if (clientRoomBtns[i]->containsScreen(position)) {
				if (clientEnteredRoom.size() < globals::ROOM_LENGTH) {
					clientEnteredRoom.push_back(i);
					updateClientLabel();
				}
				return None;
			}
		}

		if (clientClearBtn->containsScreen(position)) {
			clientEnteredRoom.clear();
			updateClientLabel();
			return None;
		}
	}

	return None;
}

/**
 * Returns an informative string for the room id
 *
 * @return an informative string for the room id
 */
std::string MatchmakingGraphRoot::positionText() { return roomID; }

void MatchmakingGraphRoot::updateClientLabel() {
	std::vector<char> room;
	for (unsigned int i = 0; i < clientEnteredRoom.size(); i++) {
		room.push_back('0' + clientEnteredRoom[i]);
	}
	for (unsigned int i = clientEnteredRoom.size(); i < globals::ROOM_LENGTH; i++) {
		room.push_back('_');
	}

	std::ostringstream disp;
	for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
		disp << room[i];
		if (i < globals::ROOM_LENGTH - 1) {
			disp << ' ';
		}
	}

	clientLabel->setText(disp.str());
}
