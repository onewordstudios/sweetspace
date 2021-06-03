#include "MainMenuMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "LevelConstants.h"
#include "MainMenuTransitions.h"
#include "NeedleAnimator.h"
#include "Tween.h"

using namespace cugl;

/** Number of buttons for room ID entry */
constexpr unsigned int NUM_DIGITS = 10;

#pragma region Animation Constants
/** Maximum rotation */
constexpr int ROTATION_MAX = 360 * 100;

/** Height of the credits scroll */
constexpr float CREDITS_HEIGHT = 2000;

/** Duration of credits scroll (in frames) */
constexpr float CREDITS_DURATION = 4500;

/** How much more to increment the credit scroll frame when tapping to go faster */
constexpr unsigned int FAST_CREDITS_SCROLL_INCREMENT = 5;

#pragma endregion

#pragma region Initialization Logic

MainMenuMode::MainMenuMode()
	: net(MagicInternetBox::getInstance()),
	  startHostThread(nullptr),
	  screenHeight(0),
	  gameReady(false),
	  rotationFrame(0),
	  creditsScrollFrame(0),
	  currState(StartScreen) {
	transition = std::make_unique<MainMenuTransitions>(this);
}

MainMenuMode::~MainMenuMode() { dispose(); }

bool MainMenuMode::init(const std::shared_ptr<AssetManager>& assets, bool toCredits) {
	// Initialize the scene to a locked width
	cugl::Size dimen = Application::get()->getDisplaySize();
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

	transition->init(assets, toCredits);

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
	transition->reset();
}

#pragma endregion

#pragma region Internal Helpers

void MainMenuMode::updateClientLabel() {
	std::vector<char> room;
	for (uint8_t i : clientEnteredRoom) {
		room.push_back(static_cast<char>('0' + i));
	}
	for (auto i = clientEnteredRoom.size(); i < globals::ROOM_LENGTH; i++) {
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
	if (roomID == net.getRoomID()) {
		return;
	}
	roomID = net.getRoomID();

	if (roomID.empty()) {
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

void MainMenuMode::processUpdate() {
	switch (net.matchStatus()) {
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
			net.update();
			break;
	}

	switch (currState) {
		case HostScreenWait: {
			if (!net.getRoomID().empty()) {
				transition->to(HostScreen);
			} else {
				switch (net.matchStatus()) {
					case MagicInternetBox::MatchmakingStatus::HostError:
						connScreen->setText("Error Connecting :(");
						backBtn->setVisible(true);
						backBtn->setColor(cugl::Color4::WHITE);
						break;
					case MagicInternetBox::MatchmakingStatus::HostApiMismatch:
						connScreen->setText("Update Required :(");
						backBtn->setVisible(true);
						backBtn->setColor(cugl::Color4::WHITE);
						break;
					default:
						break;
				}
				if (!connScreen->isVisible()) {
					connScreen->setVisible(true);
					connScreen->setColor(cugl::Color4::WHITE);
				}
			}
			break;
		}
		case HostScreen: {
			NeedleAnimator::updateNeedle(hostNeedle);
			if (backBtn->isVisible()) {
				if (net.getNumPlayers() > 1) {
					backBtn->setVisible(false);
					hostBeginBtn->setVisible(true);
				}
			} else {
				if (net.getNumPlayers() == 1) {
					backBtn->setVisible(true);
					hostBeginBtn->setVisible(false);
				}
			}
			break;
		}
		case ClientScreenSubmitted: {
			switch (net.matchStatus()) {
				case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
					clientErrorLabel->setText("That ship ID doesn't seem to exist.");
					transition->to(ClientScreenError);
					break;
				case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
					clientErrorLabel->setText("That ship is full.");
					transition->to(ClientScreenError);
					break;
				case MagicInternetBox::MatchmakingStatus::ClientApiMismatch:
					clientErrorLabel->setText("Your app is outdated. Please update.");
					transition->to(ClientScreenError);
					break;
				case MagicInternetBox::MatchmakingStatus::ClientError:
					clientErrorLabel->setText("Check your internet?");
					transition->to(ClientScreenError);
					break;
				case MagicInternetBox::MatchmakingStatus::ClientWaitingOnOthers:
					transition->to(ClientScreenDone);
					break;
				default:
					break;
			}
		}
		case ClientScreenDone: {
			NeedleAnimator::updateNeedle(hostNeedle);
			break;
		}
		case Credits: {
			float pos = ((CREDITS_HEIGHT + screenHeight) *
						 (static_cast<float>(creditsScrollFrame++) / CREDITS_DURATION));

			if (InputController::getInstance()->getCurrTapLoc() != Vec2::ZERO) {
				creditsScrollFrame += FAST_CREDITS_SCROLL_INCREMENT;
			}

			credits->setPositionY(pos);
			if (static_cast<float>(creditsScrollFrame) > CREDITS_DURATION) {
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
				if (net.getNumPlayers() > 1) {
					break;
				}
				// Intentional fall-through
			case ClientScreenDone:
				net.reset();
				// Intentional fall-through
			case ClientScreen:
			case Credits:
				transition->to(StartScreen);
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
			if (ButtonManager::tappedButton(hostBtn, tapData)) {
				transition->to(HostScreenWait);
			} else if (ButtonManager::tappedButton(clientBtn, tapData)) {
				transition->to(ClientScreen);
			} else if (ButtonManager::tappedButton(creditsBtn, tapData)) {
				transition->to(Credits);
			}
			break;
		}
		case HostScreenWait: {
			auto status = net.matchStatus();
			if (status == MagicInternetBox::MatchmakingStatus::HostError ||
				status == MagicInternetBox::MatchmakingStatus::HostApiMismatch) {
				if (ButtonManager::tappedButton(backBtn, tapData)) {
					transition->to(StartScreen);
				}
			}
			break;
		}
		case HostScreen: {
			if (net.getNumPlayers() >= globals::MIN_PLAYERS) {
				if (ButtonManager::tappedButton(hostBeginBtn, tapData)) {
					transition->to(HostLevelSelect);
				}
			} else {
				if (ButtonManager::tappedButton(backBtn, tapData)) {
					CULog("Going Back");
					transition->to(StartScreen);
				}
			}
			break;
		}
		case HostLevelSelect: {
			for (unsigned int i = 0; i < NUM_LEVEL_BTNS; i++) {
				if (ButtonManager::tappedButton(levelBtns.at(i), tapData)) {
					gameReady = true;
					net.startGame(LEVEL_ENTRY_POINTS.at(i));
					return;
				}
			}
			if (ButtonManager::tappedButton(hostTutorialSkipBtn, tapData)) {
				bool isDown = hostTutorialSkipBtn->isDown();
				hostTutorialSkipBtn->setDown(!isDown);
				net.setSkipTutorial(!isDown);
			}
			break;
		}
		case ClientScreen: {
			if (ButtonManager::tappedButton(clientJoinBtn, tapData)) {
				if (clientEnteredRoom.size() != globals::ROOM_LENGTH) {
					break;
				}

				std::ostringstream room;
				for (int i = 0; i < globals::ROOM_LENGTH; i++) {
					room << static_cast<char>('0' + clientEnteredRoom[i]);
				}

				currState = ClientScreenSubmitted;
				clientJoinBtn->setDown(true);
				net.initClient(room.str());

				break;
			}
			if (ButtonManager::tappedButton(backBtn, tapData)) {
				transition->to(StartScreen);
				return;
			}

			for (unsigned int i = 0; i < NUM_DIGITS; i++) {
				if (ButtonManager::tappedButton(clientRoomBtns[i], tapData)) {
					if (clientEnteredRoom.size() < globals::ROOM_LENGTH) {
						clientEnteredRoom.push_back(i);
						updateClientLabel();
					}
					break;
				}
			}

			if (ButtonManager::tappedButton(clientClearBtn, tapData)) {
				if (!clientEnteredRoom.empty()) {
					clientEnteredRoom.pop_back();
					clientJoinBtn->setDown(false);
					updateClientLabel();
				}
				break;
			}
			break;
		}
		case ClientScreenError: {
			if (ButtonManager::tappedButton(clientErrorBtn, tapData)) {
				transition->to(ClientScreen);
			}
			break;
		}
		case ClientScreenDone:
		case Credits: {
			if (ButtonManager::tappedButton(backBtn, tapData)) {
				transition->to(StartScreen);
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
	bg0stars->setAngle(globals::TWO_PI * static_cast<float>(rotationFrame) / ROTATION_MAX);

	if (transition->step()) {
		net.update();
		return;
	}
	processUpdate();
	processButtons();
}

/**
 * Draws the game.
 */
void MainMenuMode::draw(const std::shared_ptr<SpriteBatch>& batch) { render(batch); }
