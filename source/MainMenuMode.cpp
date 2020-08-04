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

	animations.registerNode("matchmaking_studiologo", assets);
	animations.registerNode("matchmaking_mainmenubg-glow", assets);
	animations.registerNode("matchmaking_mainmenubg-ship", assets);
	animations.registerNode("matchmaking_mainmenubg-land", assets);
	animations.registerNode("matchmaking_mainmenubg-landnoship", assets);

	animations.fadeOut("matchmaking_studiologo", TRANSITION_DURATION * 2);
	animations.animateY("matchmaking_mainmenubg-glow", AnimationManager::TweenType::EaseOut,
						screenHeight / 2, OPEN_TRANSITION);
	animations.animateY("matchmaking_mainmenubg-ship", AnimationManager::TweenType::EaseOut,
						screenHeight / 2, OPEN_TRANSITION);
	animations.animateY("matchmaking_mainmenubg-land", AnimationManager::TweenType::EaseOut,
						screenHeight / 2, OPEN_TRANSITION);

	for (auto e : mainScreen) {
		animations.registerNode(e, assets);
		animations.fadeIn(e, TRANSITION_DURATION, OPEN_TRANSITION_FADE);
	}

	animations.registerNode("matchmaking_backbtn", assets);

	animations.registerNode("matchmaking_host", assets);
	animations.registerNode("matchmaking_client", assets);

	animations.registerNode("matchmaking_credits", assets);

	animations.registerNode("matchmaking_levelselect", assets);
	animations.registerNode("matchmaking_tutorialbtn", assets);

	animations.registerNode("matchmaking_clienterr", assets);

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
}

#pragma endregion

#pragma region Scene Transitions

void MainMenuMode::triggerCredits() {
	currState = Credits;
	credits->setVisible(true);
	credits->setColor(Color4::WHITE);
	credits->setPositionY(0);
	creditsScrollFrame = 0;

	animateOutMainMenu();
	animations.fadeOut("matchmaking_mainmenubg-glow", TRANSITION_DURATION);

	animations.animateY("matchmaking_mainmenubg-land", AnimationManager::TweenType::EaseInOut,
						screenHeight / CREDITS_BG_POS, TRANSITION_DURATION);
	animations.fadeOut("matchmaking_mainmenubg-land", TRANSITION_DURATION);

	animations.animateY("matchmaking_mainmenubg-landnoship", AnimationManager::TweenType::EaseInOut,
						screenHeight / CREDITS_BG_POS, TRANSITION_DURATION);
	animations.fadeIn("matchmaking_mainmenubg-landnoship", 1);

	animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION);
}

void MainMenuMode::animateOutMainMenu() {
	for (auto e : mainScreen) {
		animations.fadeOut(e, TRANSITION_DURATION);
	}
	animations.animateY("matchmaking_mainmenubg-ship", AnimationManager::TweenType::EaseIn,
						screenHeight / SHIP_FLY_POS, TRANSITION_DURATION);
	animations.fadeOut("matchmaking_mainmenubg-ship", TRANSITION_DURATION);
}

void MainMenuMode::returnToMainMenu() {
	for (auto e : mainScreen) {
		animations.fadeIn(e, TRANSITION_DURATION);
	}
	animations.animateY("matchmaking_mainmenubg-ship", AnimationManager::TweenType::EaseIn,
						screenHeight / 2, 1);
	animations.fadeIn("matchmaking_mainmenubg-ship", TRANSITION_DURATION);
	animations.fadeOut("matchmaking_backbtn", TRANSITION_DURATION);

	switch (currState) {
		case HostScreen:
		case ClientScreenDone:
			animations.animateY("matchmaking_host", AnimationManager::TweenType::EaseIn,
								-screenHeight, TRANSITION_DURATION);
			animations.fadeOut("matchmaking_host", 1, TRANSITION_DURATION);
			break;
		case ClientScreen:
			animations.animateY("matchmaking_client", AnimationManager::TweenType::EaseIn,
								-screenHeight, TRANSITION_DURATION);
			animations.fadeOut("matchmaking_client", 1, TRANSITION_DURATION);
			break;
		case Credits:
			animations.fadeIn("matchmaking_mainmenubg-glow", TRANSITION_DURATION);

			animations.animateY("matchmaking_mainmenubg-landnoship",
								AnimationManager::TweenType::EaseInOut, screenHeight / 2,
								TRANSITION_DURATION);
			animations.fadeOut("matchmaking_mainmenubg-landnoship", 1, TRANSITION_DURATION);

			animations.animateY("matchmaking_mainmenubg-land",
								AnimationManager::TweenType::EaseInOut, screenHeight / 2,
								TRANSITION_DURATION);
			animations.fadeIn("matchmaking_mainmenubg-land", TRANSITION_DURATION);

			animations.fadeOut("matchmaking_credits", TRANSITION_DURATION);
			break;
		default:
			break;
	}
}

#pragma endregion

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
	switch (currState) {
		case HostScreenWait: {
			if (net->getRoomID() != "") {
				setRoomID();
				connScreen->setVisible(false);

				currState = HostScreen;
				animations.animateY("matchmaking_host", AnimationManager::TweenType::EaseOut, 0,
									TRANSITION_DURATION);
				animations.fadeIn("matchmaking_host", 1);
				animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION);

				startHostThread->detach();
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
				net->forceDisconnect();
				// Intentional fall-through
			case ClientScreen:
			case Credits:
				CULog("Going Back");
				returnToMainMenu();
				currState = StartScreen;
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
				startHostThread = std::unique_ptr<std::thread>(new std::thread([]() {
					MagicInternetBox::getInstance()->initHost();
					CULog("SEPARATE THREAD FINISHED INIT HOST");
				}));
				currState = HostScreenWait;
				hostNeedle->setAngle(0);
				needlePos = 0;

				animateOutMainMenu();
			} else if (buttonManager.tappedButton(clientBtn, tapData)) {
				currState = ClientScreen;

				clientEnteredRoom.clear();
				updateClientLabel();

				animateOutMainMenu();

				animations.animateY("matchmaking_client", AnimationManager::TweenType::EaseOut, 0,
									TRANSITION_DURATION);
				animations.fadeIn("matchmaking_client", 1);
				animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION);
			} else if (buttonManager.tappedButton(creditsBtn, tapData)) {
				triggerCredits();
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
				startHostThread->detach();
				net->forceDisconnect();

				returnToMainMenu();
				currState = StartScreen;
			}
			break;
		}
		case HostScreen: {
			if (net->getNumPlayers() >= globals::MIN_PLAYERS) {
				if (buttonManager.tappedButton(hostBeginBtn, tapData)) {
					animations.animateY("matchmaking_host", AnimationManager::TweenType::EaseIn,
										-screenHeight, TRANSITION_DURATION);
					animations.fadeOut("matchmaking_host", 1, TRANSITION_DURATION);
					animations.fadeIn("matchmaking_levelselect", TRANSITION_DURATION,
									  TRANSITION_DURATION / 2);
					animations.fadeIn("matchmaking_tutorialbtn", TRANSITION_DURATION,
									  TRANSITION_DURATION / 2);

					currState = HostLevelSelect;
				}
			} else {
				if (buttonManager.tappedButton(backBtn, tapData)) {
					CULog("Going Back");
					net->forceDisconnect();

					returnToMainMenu();
					currState = StartScreen;
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
				returnToMainMenu();
				currState = StartScreen;
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
				currState = ClientScreen;

				animations.fadeOut("matchmaking_clienterr", 1, TRANSITION_DURATION);
				animations.animateY("matchmaking_clienterr", AnimationManager::TweenType::EaseIn,
									-screenHeight, TRANSITION_DURATION);

				animations.animateY("matchmaking_client", AnimationManager::TweenType::EaseOut, 0,
									TRANSITION_DURATION, TRANSITION_DURATION);
				animations.fadeIn("matchmaking_client", 1, TRANSITION_DURATION);
				animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION, TRANSITION_DURATION);

				clientEnteredRoom.clear();
				updateClientLabel();
			}
			break;
		}
		case ClientScreenDone: {
			if (buttonManager.tappedButton(backBtn, tapData)) {
				CULog("Going Back");
				net->forceDisconnect();
				returnToMainMenu();
				currState = StartScreen;
			}
		}
		case Credits: {
			if (buttonManager.tappedButton(backBtn, tapData)) {
				CULog("Going Back");
				returnToMainMenu();
				currState = StartScreen;
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

	if (!animations.step()) {
		processUpdate();
		processButtons();
	}

	switch (net->matchStatus()) {
		case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
		case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
		case MagicInternetBox::MatchmakingStatus::ClientError:
			if (currState == ClientScreenSubmitted) {
				currState = ClientScreenError;

				animations.animateY("matchmaking_client", AnimationManager::TweenType::EaseIn,
									-screenHeight, TRANSITION_DURATION);
				animations.fadeOut("matchmaking_client", 1, TRANSITION_DURATION);
				animations.fadeOut("matchmaking_backbtn", TRANSITION_DURATION);

				animations.fadeIn("matchmaking_clienterr", 1, TRANSITION_DURATION);
				animations.animateY("matchmaking_clienterr", AnimationManager::TweenType::EaseOut,
									0, TRANSITION_DURATION, TRANSITION_DURATION);

				switch (net->matchStatus()) {
					case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
						clientErrorLabel->setText("That ship ID doesn't seem to exist.");
						break;
					case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
						clientErrorLabel->setText("That ship is full.");
						break;
					case MagicInternetBox::MatchmakingStatus::ClientError:
						clientErrorLabel->setText("Your app is out of date. Please update.");
						break;
					default:
						break;
				}
			}
			return;
		case MagicInternetBox::MatchmakingStatus::Uninitialized:
		case MagicInternetBox::MatchmakingStatus::HostError:
			return;
		case MagicInternetBox::MatchmakingStatus::GameStart:
			gameReady = true;
			return;
		case MagicInternetBox::MatchmakingStatus::ClientWaitingOnOthers:
			if (currState == ClientScreenSubmitted) {
				currState = ClientScreenDone;

				animations.animateY("matchmaking_client", AnimationManager::TweenType::EaseIn,
									-screenHeight, TRANSITION_DURATION);
				animations.fadeOut("matchmaking_client", 1, TRANSITION_DURATION);

				animations.fadeIn("matchmaking_host", 1, TRANSITION_DURATION);
				animations.animateY("matchmaking_host", AnimationManager::TweenType::EaseOut, 0,
									TRANSITION_DURATION, TRANSITION_DURATION);
				clientWaitHost->setVisible(true);

				setRoomID();
				setNumPlayers();
			}
			// Intentional fall-through
		default:
			net->update();
			break;
	}
}

/**
 * Draws the game.
 */
void MainMenuMode::draw(const std::shared_ptr<SpriteBatch>& batch) { render(batch); }
