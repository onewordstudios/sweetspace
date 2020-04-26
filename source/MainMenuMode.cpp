#include "MainMenuMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "LevelConstants.h"
#include "Tween.h"

using namespace cugl;

/** Number of buttons for room ID entry */
constexpr unsigned int NUM_DIGITS = 10;

#pragma region Animation Constants
/** Maximum rotation */
constexpr int ROTATION_MAX = 360 * 100;

/** Duration of a standard transition */
constexpr int TRANSITION_DURATION = 30;

/** Duration of opening transition */
constexpr int OPEN_TRANSITION = 120;

/** When during opening transition to fade in stuff */
constexpr int OPEN_TRANSITION_FADE = 90;

/** How much the host player count needle is offset by */
constexpr float NEEDLE_OFFSET = 0.9f;
#pragma endregion

#pragma region Initialization Logic
bool MainMenuMode::init(const std::shared_ptr<AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}
	// Set network controller
	net = MagicInternetBox::getInstance();
	input = InputController::getInstance();

	screenHeight = dimen.height;
	// Initialize the scene to a locked width
	if (!Scene::init(dimen)) {
		return false;
	}

	// Acquire the scene built by the asset loader and resize it the scene
	auto scene = assets->get<Node>("matchmaking");
	scene->setContentSize(dimen);
	scene->doLayout(); // Repositions the HUD

#pragma region Scene Graph Components
	bg0stars = assets->get<Node>("matchmaking_mainmenubg2");
	bg1land = assets->get<Node>("matchmaking_mainmenubg3");
	bg2ship = assets->get<Node>("matchmaking_mainmenubg4");
	bg9studio = assets->get<Node>("matchmaking_studiologo");

	backBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_backbtn"));

	hostBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_hostbtn"));
	clientBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_clientbtn"));

	mainScreen = assets->get<Node>("matchmaking_home");
	hostScreen = assets->get<Node>("matchmaking_host");
	clientScreen = assets->get<Node>("matchmaking_client");
	connScreen = std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_connscreen"));

	hostLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_host_wrap_plate_room"));
	clientLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_client_wrap_plate_room"));

	hostBeginBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_host_wrap_startbtn"));
	hostNeedle = assets->get<Node>("matchmaking_host_dial_hand");

	clientJoinBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_wrap_joinbtn"));
	clientClearBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_buttons_btnclear"));

	levelSelect = assets->get<Node>("matchmaking_levelselect");
	easyBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_levelselect_easybtn"));
	medBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_levelselect_medbtn"));
	hardBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_levelselect_hardbtn"));

	buttonManager.registerButton(backBtn);
	buttonManager.registerButton(hostBtn);
	buttonManager.registerButton(clientBtn);
	buttonManager.registerButton(hostBeginBtn);
	buttonManager.registerButton(clientJoinBtn);
	buttonManager.registerButton(clientClearBtn);
	buttonManager.registerButton(easyBtn);
	buttonManager.registerButton(medBtn);
	buttonManager.registerButton(hardBtn);
	for (unsigned int i = 0; i < NUM_DIGITS; i++) {
		clientRoomBtns.push_back(std::dynamic_pointer_cast<Button>(
			assets->get<Node>("matchmaking_client_buttons_btn" + std::to_string(i))));
		buttonManager.registerButton(clientRoomBtns[i]);
	}
#pragma endregion

	transitionFrame = 0;
	currState = NA;
	transitionState = StartScreen;

	updateClientLabel();

	addChild(scene);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MainMenuMode::dispose() {
	removeAllChildren();
	backBtn = nullptr;
	hostBtn = nullptr;
	clientBtn = nullptr;
	mainScreen = nullptr;
	hostScreen = nullptr;
	clientScreen = nullptr;
	connScreen = nullptr;
	hostLabel = nullptr;
	hostBeginBtn = nullptr;
	hostNeedle = nullptr;
	clientLabel = nullptr;
	clientJoinBtn = nullptr;
	clientClearBtn = nullptr;
	levelSelect = nullptr;
	easyBtn = nullptr;
	medBtn = nullptr;
	hardBtn = nullptr;
	clientRoomBtns.clear();
}
#pragma endregion

#pragma region Internal Helpers
void MainMenuMode::updateClientLabel() {
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

void MainMenuMode::setRoomID() {
	if (roomID == net->getRoomID()) {
		return;
	}
	roomID = net->getRoomID();

	if (roomID == "") {
		hostLabel->setText("_ _ _ _ _");
		clientEnteredRoom.clear();
		updateClientLabel();
		return;
	}

	std::ostringstream disp;
	for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
		disp << roomID.at(i);
		if (i < globals::ROOM_LENGTH - 1) {
			disp << ' ';
		}
	}
	hostLabel->setText(disp.str());
}

void MainMenuMode::endTransition() {
	currState = transitionState;
	transitionState = NA;
	transitionFrame = -1;
	input->clear();
}

void MainMenuMode::processTransition() {
	transitionFrame++;
	switch (currState) {
		case NA: {
			if (transitionFrame == 1) {
				bg1land->setVisible(true);
				bg2ship->setVisible(true);
			}
			if (transitionFrame > OPEN_TRANSITION) {
				bg9studio->setVisible(false);
				mainScreen->setColor(Color4::WHITE);
				endTransition();
				return;
			}

			// Fade out studio logo from loading screen
			if (transitionFrame <= TRANSITION_DURATION * 2) {
				bg9studio->setColor(Tween::fade(
					Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION * 2)));
			}

			// Fade in main menu at end
			if (transitionFrame > OPEN_TRANSITION_FADE) {
				mainScreen->setVisible(true);
				int i = transitionFrame - OPEN_TRANSITION_FADE;
				mainScreen->setColor(Tween::fade(
					Tween::linear(0.0f, 1.0f, i, OPEN_TRANSITION - OPEN_TRANSITION_FADE)));
			}

			// Background pans up into view
			bg1land->setPositionY(
				Tween::easeOut(-screenHeight, screenHeight / 2, transitionFrame, OPEN_TRANSITION));
			bg2ship->setPositionY(
				Tween::easeOut(-screenHeight, screenHeight / 2, transitionFrame, OPEN_TRANSITION));

			return;
		}
		case StartScreen: {
			if (transitionFrame > TRANSITION_DURATION) {
				endTransition();
				mainScreen->setVisible(false);
			} else {
				mainScreen->setColor(
					Tween::fade(Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));
				if (transitionState == ClientScreen) {
					if (transitionFrame == 1) {
						backBtn->setVisible(true);
					}
					clientScreen->setPositionY(
						Tween::easeOut(-screenHeight, 0, transitionFrame, TRANSITION_DURATION));
					backBtn->setColor(Tween::fade(
						Tween::linear(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));
				}
			}
			break;
		}
		case HostScreenWait: {
			if (transitionState == HostScreen) {
				if (transitionFrame >= TRANSITION_DURATION) {
					endTransition();
					hostScreen->setPositionY(0);
				} else {
					hostScreen->setPositionY(
						Tween::easeOut(-screenHeight, 0, transitionFrame, TRANSITION_DURATION));

					if (transitionFrame == 1) {
						backBtn->setVisible(true);
					}
					backBtn->setColor(Tween::fade(
						Tween::linear(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));
				}
			}
			break;
		}
		case HostScreen:
		case ClientScreen: {
			if (transitionState == StartScreen) {
				// Start transition
				if (transitionFrame == 1) {
					mainScreen->setVisible(true);
				}

				// Transition over
				if (transitionFrame > TRANSITION_DURATION) {
					endTransition();
					hostScreen->setVisible(false);
					clientScreen->setVisible(false);
					backBtn->setVisible(false);
					return;
				}

				// Make current screen go down
				if (currState == HostScreen) {
					hostScreen->setPositionY(
						Tween::easeIn(0, -screenHeight, transitionFrame, TRANSITION_DURATION));
				} else {
					clientScreen->setPositionY(
						Tween::easeIn(0, -screenHeight, transitionFrame, TRANSITION_DURATION));
				}

				mainScreen->setColor(
					Tween::fade(Tween::linear(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));
				backBtn->setColor(
					Tween::fade(Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));

				return;
			}
		}
		default:
			break;
	}
}

void MainMenuMode::processUpdate() {
	switch (currState) {
		case HostScreenWait: {
			if (net->getRoomID() != "") {
				setRoomID();
				hostScreen->setVisible(true);
				hostScreen->setPositionY(-screenHeight);
				transitionState = HostScreen;
				connScreen->setVisible(false);

				startHostThread->detach();
			} else {
				connScreen->setVisible(true);
			}
			if (net->matchStatus() == MagicInternetBox::MatchmakingStatus::HostError) {
				connScreen->setText("Error Connecting :(");
				backBtn->setVisible(true);
				backBtn->setColor(cugl::Color4::WHITE);
			}
			break;
		}
		case HostScreen: {
			float percentage = (float)(net->getNumPlayers() - 1) / (float)globals::MAX_PLAYERS;
			hostNeedle->setAngle(-percentage * globals::TWO_PI * NEEDLE_OFFSET);
			if (backBtn->isVisible()) {
				if (percentage > 0) {
					backBtn->setVisible(false);
				}
			}
			break;
		}
		default: {
			break;
		}
	}
}

void MainMenuMode::processButtons() {
	if (currState != ClientScreenDone) {
		buttonManager.process();
	}

	if (InputController::getInstance()->hasPressedBack()) {
		switch (currState) {
			case HostScreenWait:
				startHostThread->detach();
				// Intentional fall-through
			case HostScreen:
				net->forceDisconnect();
				// Intentional fall-through
			case ClientScreen:
				CULog("Going Back");
				transitionState = StartScreen;
				return;
			default:
				break;
		}
	}

	// Do not process inputs if a) nothing was pressed, or b) currently transitioning
	if (!InputController::getInstance()->isTapEndAvailable() || transitionState != NA) {
		return;
	}

	std::tuple<Vec2, Vec2> tapData = InputController::getInstance()->getTapEndLoc();

	switch (currState) {
		case StartScreen: {
			if (buttonManager.tappedButton(hostBtn, tapData)) {
				startHostThread = std::unique_ptr<std::thread>(new std::thread([]() {
					MagicInternetBox::getInstance()->initHost();
					CULog("SEPARATE THREAD FINISHED INIT HOST");
				}));
				transitionState = HostScreenWait;
			} else if (buttonManager.tappedButton(clientBtn, tapData)) {
				transitionState = ClientScreen;
				clientScreen->setPositionY(-screenHeight);
				clientScreen->setVisible(true);
			}
			break;
		}
		case HostScreen: {
			if (buttonManager.tappedButton(hostBeginBtn, tapData)) {
				if (net->getNumPlayers() >= globals::MIN_PLAYERS) {
					currState = HostLevelSelect;
					hostScreen->setVisible(false);
					levelSelect->setVisible(true);
				}
			} else if (buttonManager.tappedButton(backBtn, tapData)) {
				CULog("Going Back");
				net->forceDisconnect();
				transitionState = StartScreen;
			}
			break;
		}
		case HostLevelSelect: {
			if (buttonManager.tappedButton(easyBtn, tapData)) {
				gameReady = true;
				net->startGame(1);
				return;
			}
			if (buttonManager.tappedButton(medBtn, tapData)) {
				gameReady = true;
				net->startGame(2);
				return;
			}
			if (buttonManager.tappedButton(hardBtn, tapData)) {
				gameReady = true;
				net->startGame(3);
				return;
			}
			break;
		}
		case ClientScreen: {
			if (buttonManager.tappedButton(clientJoinBtn, tapData)) {
				if (clientEnteredRoom.size() != globals::ROOM_LENGTH) {
					break;
				}

				std::ostringstream room;
				for (int i = 0; i < globals::ROOM_LENGTH; i++) {
					room << clientEnteredRoom[i];
				}

				currState = ClientScreenDone;
				clientJoinBtn->setDown(true);
				net->initClient(room.str());

				break;
			} else if (buttonManager.tappedButton(backBtn, tapData)) {
				transitionState = StartScreen;
				return;
			}

			for (unsigned int i = 0; i < NUM_DIGITS; i++) {
				if (buttonManager.tappedButton(clientRoomBtns[i], tapData)) {
					if (clientEnteredRoom.size() < globals::ROOM_LENGTH) {
						clientEnteredRoom.push_back(i);
						updateClientLabel();
					}
					break;
				}
			}

			if (buttonManager.tappedButton(clientClearBtn, tapData)) {
				if (clientEnteredRoom.size() > 0) {
					clientEnteredRoom.pop_back();
					clientJoinBtn->setDown(false);
					updateClientLabel();
				}
				break;
			}
		}
		default: {
			break;
		}
	}
}
#pragma endregion

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void MainMenuMode::update(float timestep) {
	input->update(timestep);

	rotationFrame = (rotationFrame + 1) % ROTATION_MAX;
	bg0stars->setAngle(globals::TWO_PI * (float)rotationFrame / ROTATION_MAX);

	if (transitionState != NA) {
		processTransition();
	} else {
		processUpdate();
		processButtons();
	}

	switch (net->matchStatus()) {
		case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
		case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
			if (currState == ClientScreenDone) {
				clientEnteredRoom.clear();
				updateClientLabel();
				currState = ClientScreen;
				clientJoinBtn->setDown(false);
			}
			return;
		case MagicInternetBox::MatchmakingStatus::Uninitialized:
		case MagicInternetBox::MatchmakingStatus::HostError:
			return;
		case MagicInternetBox::MatchmakingStatus::GameStart:
			gameReady = true;
			return;
		case MagicInternetBox::MatchmakingStatus::ClientWaitingOnOthers:
			if (backBtn->isVisible()) {
				backBtn->setVisible(false);
			}
		default:
			net->update();
			break;
	}
}

/**
 * Draws the game.
 */
void MainMenuMode::draw(const std::shared_ptr<SpriteBatch>& batch) { render(batch); }
