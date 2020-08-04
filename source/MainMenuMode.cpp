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

/** Height of the credits scroll */
constexpr float CREDITS_HEIGHT = 2000;

/** Duration of credits scroll (in frames) */
constexpr float CREDITS_DURATION = 4500;

/** How much more to increment the credit scroll frame when tapping to go faster */
constexpr unsigned int FAST_CREDITS_SCROLL_INCREMENT = 5;

/** Divisor of screen height to get credits bg position */
constexpr float CREDITS_BG_POS = 2.5;

/** Divisor of screen height to get ship flight destination position */
constexpr float SHIP_FLY_POS = 1.5;

/** How much the needle moves each frame to match its correct position */
constexpr float NEEDLE_SPEED = 0.3f;

/** When to just snap the needle to its correct position */
constexpr float NEEDLE_CUTOFF = 0.01f;

/**
 * Current frame of the credits scroll (there's only ever one credits screen, so it's safe to
 * stick this here)
 */
unsigned int creditsScrollFrame = 0;
#pragma endregion

#pragma region Initialization Logic
bool MainMenuMode::init(const std::shared_ptr<AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	// Music Initialization
	auto source = assets->get<Sound>("menu");
	if (AudioChannels::get()->currentMusic() == nullptr ||
		AudioChannels::get()->currentMusic()->getFile() != source->getFile()) {
		AudioChannels::get()->stopMusic(globals::MUSIC_FADE_OUT);
		AudioChannels::get()->queueMusic(source, true, source->getVolume(), globals::MUSIC_FADE_IN);
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
	transitions.init(assets);

	creditsBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_creditsbtn"));
	credits = assets->get<Node>("matchmaking_credits");

	backBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_backbtn"));

	hostBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_hostbtn"));
	clientBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_clientbtn"));

	connScreen = std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_connscreen"));

	hostLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_host_wrap_plate_room"));
	clientLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_client_wrap_plate_room"));

	hostBeginBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_host_wrap_startbtn"));
	hostNeedle = assets->get<Node>("matchmaking_host_dial_hand");
	hostTutorialSkipBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_tutorialbtn"));

	clientJoinBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_wrap_joinbtn"));
	clientClearBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_buttons_btnclear"));
	clientWaitHost = assets->get<Node>("matchmaking_host_wrap_waittext");

	clientErrorLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_clienterr_errortext"));
	clientErrorBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_clienterr_retrybtn"));

	for (unsigned int i = 0; i < NUM_LEVEL_BTNS; i++) {
		levelBtns.at(i) = std::dynamic_pointer_cast<Button>(
			assets->get<Node>("matchmaking_levelselect_lvl" + std::to_string(i)));
		buttonManager.registerButton(levelBtns.at(i));
	}

	buttonManager.registerButton(backBtn);
	buttonManager.registerButton(hostBtn);
	buttonManager.registerButton(clientBtn);
	buttonManager.registerButton(hostBeginBtn);
	buttonManager.registerButton(clientJoinBtn);
	buttonManager.registerButton(clientClearBtn);
	buttonManager.registerButton(creditsBtn);
	buttonManager.registerButton(clientErrorBtn);
	for (unsigned int i = 0; i < NUM_DIGITS; i++) {
		clientRoomBtns.push_back(std::dynamic_pointer_cast<Button>(
			assets->get<Node>("matchmaking_client_buttons_btn" + std::to_string(i))));
		buttonManager.registerButton(clientRoomBtns[i]);
	}
#pragma endregion

	currState = StartScreen;

	// Reset state in case coming back from other place
	gameReady = false;
	hostBeginBtn->setVisible(false);
	clientJoinBtn->setDown(false);
	clientJoinBtn->setVisible(true);
	credits->setVisible(false);
	clientEnteredRoom.clear();
	clientWaitHost->setVisible(false);
	backBtn->setVisible(false);
	hostTutorialSkipBtn->setVisible(false);

	updateClientLabel();
	addChild(scene);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MainMenuMode::dispose() {
	removeAllChildren();

	bg0stars = nullptr;

	backBtn = nullptr;
	hostBtn = nullptr;
	clientBtn = nullptr;
	connScreen = nullptr;
	hostLabel = nullptr;
	hostBeginBtn = nullptr;
	hostNeedle = nullptr;
	clientLabel = nullptr;
	clientJoinBtn = nullptr;
	clientClearBtn = nullptr;
	clientWaitHost = nullptr;
	clientErrorLabel = nullptr;
	clientErrorBtn = nullptr;
	credits = nullptr;
	creditsBtn = nullptr;
	levelBtns.fill(nullptr);
	buttonManager.clear();
	clientRoomBtns.clear();
	transitions.reset();
}

#pragma endregion

void MainMenuMode::triggerCredits() { transitions.go(Credits); }

#pragma region Internal Helpers

void MainMenuMode::updateClientLabel() {
	std::vector<char> room;
	for (unsigned int i = 0; i < clientEnteredRoom.size(); i++) {
		room.push_back('0' + clientEnteredRoom[i]);
	}
	for (unsigned int i = (unsigned int)clientEnteredRoom.size(); i < globals::ROOM_LENGTH; i++) {
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

void MainMenuMode::setNumPlayers() {
	float percentage = (float)(net->getNumPlayers() - 1) / (float)globals::MAX_PLAYERS;
	float diff = percentage - needlePos;
	if (abs(diff) < NEEDLE_CUTOFF) {
		needlePos = percentage;
	} else {
		needlePos += NEEDLE_SPEED * diff;
	}
	hostNeedle->setAngle(-needlePos * globals::TWO_PI * globals::NEEDLE_OFFSET);
}

void MainMenuMode::processUpdate() {
	switch (net->matchStatus()) {
		case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
		case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
		case MagicInternetBox::MatchmakingStatus::ClientError:
		case MagicInternetBox::MatchmakingStatus::Uninitialized:
		case MagicInternetBox::MatchmakingStatus::HostError:
			break;
		case MagicInternetBox::MatchmakingStatus::GameStart:
			gameReady = true;
			return;
		default:
			net->update();
			break;
	}

	switch (currState) {
		case HostScreenWait: {
			if (net->getRoomID() != "") {
				transitions.go(HostScreen);
			} else {
				if (!connScreen->isVisible()) {
					switch (net->matchStatus()) {
						case MagicInternetBox::MatchmakingStatus::HostError:
							connScreen->setText("Error Connecting :(");
							backBtn->setVisible(true);
							backBtn->setColor(cugl::Color4::WHITE);
							break;
						case MagicInternetBox::MatchmakingStatus::HostApiMismatch:
							connScreen->setText("Update required :(");
							backBtn->setVisible(true);
							backBtn->setColor(cugl::Color4::WHITE);
							break;
						default:
							break;
					}
					connScreen->setVisible(true);
				}
			}
			break;
		}
		case HostScreen: {
			setNumPlayers();
			if (backBtn->isVisible()) {
				if (net->getNumPlayers() > 1) {
					backBtn->setVisible(false);
					hostBeginBtn->setVisible(true);
				}
			} else {
				if (net->getNumPlayers() == 1) {
					backBtn->setVisible(true);
					hostBeginBtn->setVisible(false);
				}
			}
			break;
		}
		case ClientScreenSubmitted: {
			switch (net->matchStatus()) {
				case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
					clientErrorLabel->setText("That ship ID doesn't seem to exist.");
					transitions.go(ClientScreenError);
					break;
				case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
					clientErrorLabel->setText("That ship is full.");
					transitions.go(ClientScreenError);
					break;
				case MagicInternetBox::MatchmakingStatus::ClientError:
					clientErrorLabel->setText("Your app is out of date. Please update.");
					transitions.go(ClientScreenError);
					break;
				case MagicInternetBox::MatchmakingStatus::ClientWaitingOnOthers:
					transitions.go(ClientScreenDone);
					break;
				default:
					break;
			}
		}
		case ClientScreenDone: {
			setNumPlayers();
			break;
		}
		case Credits: {
			float pos = ((float)(CREDITS_HEIGHT + screenHeight) *
						 ((float)(creditsScrollFrame++) / CREDITS_DURATION));

			if (InputController::getInstance()->getCurrTapLoc() != Vec2::ZERO) {
				creditsScrollFrame += FAST_CREDITS_SCROLL_INCREMENT;
			}

			credits->setPositionY(pos);
			if ((float)creditsScrollFrame > CREDITS_DURATION) {
				creditsScrollFrame = 0;
			}
			break;
		}
		default: {
			break;
		}
	}
}

void MainMenuMode::processButtons() {
	if (currState != ClientScreenSubmitted) {
		buttonManager.process();
	}

	if (InputController::getInstance()->hasPressedBack()) {
		switch (currState) {
			case HostScreenWait:
				startHostThread->detach();
				// Intentional fall-through
			case HostScreen:
				if (net->getNumPlayers() > 1) {
					break;
				}
				// Intentional fall-through
			case ClientScreenDone:
				net->reset();
				// Intentional fall-through
			case ClientScreen:
			case Credits:
				CULog("Going Back");
				transitions.go(StartScreen);
				return;
			default:
				break;
		}
	}

	// Do not process inputs if a) nothing was pressed, or b) currently transitioning
	if (!InputController::getInstance()->isTapEndAvailable()) {
		return;
	}

	std::tuple<Vec2, Vec2> tapData = InputController::getInstance()->getTapEndLoc();

	switch (currState) {
		case StartScreen: {
			if (buttonManager.tappedButton(hostBtn, tapData)) {
				transitions.go(HostScreenWait);
			} else if (buttonManager.tappedButton(clientBtn, tapData)) {
				transitions.go(ClientScreen);
			} else if (buttonManager.tappedButton(creditsBtn, tapData)) {
				transitions.go(Credits);
			}
			break;
		}
		case HostScreenWait: {
			auto status = net->matchStatus();
			if (status != MagicInternetBox::MatchmakingStatus::HostError &&
				status != MagicInternetBox::MatchmakingStatus::HostApiMismatch) {
				break;
			}
			if (buttonManager.tappedButton(backBtn, tapData)) {
				CULog("Going Back");
				transitions.go(StartScreen);
			}
			break;
		}
		case HostScreen: {
			if (net->getNumPlayers() >= globals::MIN_PLAYERS) {
				if (buttonManager.tappedButton(hostBeginBtn, tapData)) {
					transitions.go(HostLevelSelect);
				}
			} else {
				if (buttonManager.tappedButton(backBtn, tapData)) {
					CULog("Going Back");
					transitions.go(StartScreen);
				}
			}
			break;
		}
		case HostLevelSelect: {
			for (unsigned int i = 0; i < NUM_LEVEL_BTNS; i++) {
				if (buttonManager.tappedButton(levelBtns.at(i), tapData)) {
					gameReady = true;
					net->startGame(LEVEL_ENTRY_POINTS.at(i));
					return;
				}
			}
			if (buttonManager.tappedButton(hostTutorialSkipBtn, tapData)) {
				bool isDown = hostTutorialSkipBtn->isDown();
				hostTutorialSkipBtn->setDown(!isDown);
				net->setSkipTutorial(!isDown);
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

				currState = ClientScreenSubmitted;
				clientJoinBtn->setDown(true);
				net->initClient(room.str());

				break;
			} else if (buttonManager.tappedButton(backBtn, tapData)) {
				transitions.go(StartScreen);
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
			break;
		}
		case ClientScreenError: {
			if (buttonManager.tappedButton(clientErrorBtn, tapData)) {
				transitions.go(ClientScreen);
			}
			break;
		}
		case ClientScreenDone:
		case Credits: {
			if (buttonManager.tappedButton(backBtn, tapData)) {
				transitions.go(StartScreen);
			}
			break;
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

	if (transitions.step()) {
		net->update();
		return;
	}
	processUpdate();
	processButtons();
}

/**
 * Draws the game.
 */
void MainMenuMode::draw(const std::shared_ptr<SpriteBatch>& batch) { render(batch); }
