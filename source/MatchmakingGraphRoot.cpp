#include "MatchmakingGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Globals.h"
#include "Tween.h"

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

	transitionFrame = -1;

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

	mainScreen = assets->get<Node>("matchmaking_home");
	hostScreen = assets->get<Node>("matchmaking_host");
	clientScreen = assets->get<Node>("matchmaking_client");

	hostLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_host_wrap_plate_room"));
	clientLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_client_wrap_plate_room"));

	hostBeginBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_host_wrap_startbtn"));

	clientJoinBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_wrap_joinbtn"));
	clientClearBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_buttons_btnclear"));

	buttonManager.registerButton(hostBtn);
	buttonManager.registerButton(clientBtn);
	buttonManager.registerButton(hostBeginBtn);
	buttonManager.registerButton(clientJoinBtn);
	buttonManager.registerButton(clientClearBtn);

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
		mainScreen = nullptr;
		hostScreen = nullptr;
		clientScreen = nullptr;
		hostLabel = nullptr;
		hostBeginBtn = nullptr;
		clientLabel = nullptr;
		clientJoinBtn = nullptr;
		clientClearBtn = nullptr;
		clientRoomBtns.clear();
		_active = false;
	}
}

/**
 * Resets the status of the game so that we can play again.
 */
void MatchmakingGraphRoot::reset() {}

void MatchmakingGraphRoot::update(float timestep) {
	if (transitionState != NA) {
		processTransition();
		return;
	}

	switch (currState) {
		case HostScreenWait: {
			if (roomID != "") {
				hostScreen->setVisible(true);
				hostScreen->setPositionY(-screenHeight);
				transitionState = HostScreen;
			}
		}
		default: {
			break;
		}
	}
}

MatchmakingGraphRoot::PressedButton MatchmakingGraphRoot::checkButtons(const cugl::Vec2& position) {
	// Do not process inputs if a) nothing was pressed, or b) currently transitioning
	if (position == Vec2::ZERO || transitionState != NA) {
		return None;
	}

	buttonManager.process(position);

	switch (currState) {
		case StartScreen: {
			if (hostBtn->containsScreen(position)) {
				transitionState = HostScreenWait;
				hostBtn->setDown(true);
				return StartHost;
			}
			if (clientBtn->containsScreen(position)) {
				mainScreen->setVisible(false);
				clientScreen->setVisible(true);
				currState = ClientScreen;
				return StartClient;
			}
		}
		case HostScreen: {
			if (hostBeginBtn->containsScreen(position)) {
				hostBeginBtn->setDown(true);
				return HostBegin;
			} else {
				return None;
			}
		}
		case ClientScreen: {
			if (clientJoinBtn->containsScreen(position)) {
				if (clientEnteredRoom.size() != globals::ROOM_LENGTH) {
					return None;
				}

				std::ostringstream room;
				for (int i = 0; i < globals::ROOM_LENGTH; i++) {
					room << clientEnteredRoom[i];
				}

				roomID = room.str();
				currState = ClientScreenDone;
				clientJoinBtn->setDown(true);

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
				if (clientEnteredRoom.size() > 0) {
					clientEnteredRoom.pop_back();
					updateClientLabel();
				}
				return None;
			}
		}
		default: {
			break;
		}
	}

	return None;
}

void MatchmakingGraphRoot::setRoomID(std::string roomID) {
	if (this->roomID == roomID) {
		return;
	}
	this->roomID = roomID;
	std::ostringstream disp;
	for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
		disp << roomID.at(i);
		if (i < globals::ROOM_LENGTH - 1) {
			disp << ' ';
		}
	}
	hostLabel->setText(disp.str());
}

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

#pragma region Animation Constants
/** Duration of a standard transition */
constexpr int TRANSITION_DURATION = 30;
#pragma endregion

void MatchmakingGraphRoot::processTransition() {
	transitionFrame++;
	switch (currState) {
		case StartScreen: {
			switch (transitionState) {
				case HostScreenWait: {
					if (transitionFrame >= TRANSITION_DURATION) {
						currState = HostScreenWait;
						transitionState = NA;
						transitionFrame = -1;
						mainScreen->setVisible(false);
					} else {
						mainScreen->setColor(Tween::fade(
							Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));
					}
					break;
				}
				default:
					break;
			}
			break;
		}
		case HostScreenWait: {
			if (transitionState == HostScreen) {
				if (transitionFrame >= TRANSITION_DURATION) {
					currState = HostScreen;
					transitionState = NA;
					transitionFrame = -1;
					hostScreen->setPositionY(0);
				} else {
					hostScreen->setPositionY(
						Tween::easeOut(-screenHeight, 0, transitionFrame, TRANSITION_DURATION));
				}
			}
			break;
		}
		default:
			break;
	}
}
