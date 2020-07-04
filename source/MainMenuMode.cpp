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
	bg1glow = assets->get<Node>("matchmaking_mainmenubg3");
	bg2ship = assets->get<Node>("matchmaking_mainmenubg4");
	bg3land = assets->get<Node>("matchmaking_mainmenubg5");
	bg4landNoShip = assets->get<Node>("matchmaking_mainmenubg6");
	bg9studio = assets->get<Node>("matchmaking_studiologo");

	creditsBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_creditsbtn"));
	credits = assets->get<Node>("matchmaking_credits");

	backBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_backbtn"));

	hostBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_hostbtn"));
	clientBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_clientbtn"));

	mainScreen.push_back(assets->get<Node>("matchmaking_home"));
	mainScreen.push_back(assets->get<Node>("matchmaking_gamelogo"));
	mainScreen.push_back(creditsBtn);
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
	clientWaitHost = assets->get<Node>("matchmaking_host_wrap_waittext");

	clientError = assets->get<Node>("matchmaking_clienterr");
	clientErrorLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_clienterr_errortext"));

	levelSelect = assets->get<Node>("matchmaking_levelselect");
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
	for (unsigned int i = 0; i < NUM_DIGITS; i++) {
		clientRoomBtns.push_back(std::dynamic_pointer_cast<Button>(
			assets->get<Node>("matchmaking_client_buttons_btn" + std::to_string(i))));
		buttonManager.registerButton(clientRoomBtns[i]);
	}
#pragma endregion

	transitionFrame = 0;
	currState = NA;
	transitionState = StartScreen;

	// Reset state in case coming back from other place
	gameReady = false;
	hostScreen->setVisible(false);
	clientScreen->setVisible(false);
	hostBeginBtn->setVisible(false);
	clientJoinBtn->setDown(false);
	clientJoinBtn->setVisible(true);
	levelSelect->setVisible(false);
	credits->setVisible(false);
	clientEnteredRoom.clear();
	clientWaitHost->setVisible(false);
	clientError->setVisible(false);
	bg2ship->setColor(Color4::WHITE);
	bg2ship->setPositionY(0);

	updateClientLabel();
	addChild(scene);

	return true;
}

void MainMenuMode::triggerCredits() {
	currState = StartScreen;
	transitionState = Credits;
	credits->setVisible(true);
	credits->setColor(Color4::WHITE);
	credits->setPositionY(0);
	creditsScrollFrame = 0;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MainMenuMode::dispose() {
	removeAllChildren();

	bg0stars = nullptr;
	bg1glow = nullptr;
	bg2ship = nullptr;
	bg3land = nullptr;
	bg4landNoShip = nullptr;
	bg9studio = nullptr;

	backBtn = nullptr;
	hostBtn = nullptr;
	clientBtn = nullptr;
	mainScreen.clear();
	hostScreen = nullptr;
	clientScreen = nullptr;
	connScreen = nullptr;
	hostLabel = nullptr;
	hostBeginBtn = nullptr;
	hostNeedle = nullptr;
	clientLabel = nullptr;
	clientJoinBtn = nullptr;
	clientClearBtn = nullptr;
	clientWaitHost = nullptr;
	clientError = nullptr;
	clientErrorLabel = nullptr;
	levelSelect = nullptr;
	credits = nullptr;
	creditsBtn = nullptr;
	levelBtns.fill(nullptr);
	buttonManager.clear();
	clientRoomBtns.clear();
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
	hostNeedle->setAngle(-percentage * globals::TWO_PI * globals::NEEDLE_OFFSET);
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
				bg1glow->setVisible(true);
				bg2ship->setVisible(true);
				bg3land->setVisible(true);
			}
			if (transitionFrame > OPEN_TRANSITION) {
				bg9studio->setVisible(false);
				for (auto e : mainScreen) {
					e->setColor(Color4::WHITE);
				}
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
				int i = transitionFrame - OPEN_TRANSITION_FADE;
				for (auto e : mainScreen) {
					e->setVisible(true);
					e->setColor(Tween::fade(
						Tween::linear(0.0f, 1.0f, i, OPEN_TRANSITION - OPEN_TRANSITION_FADE)));
				}
			}

			// Background pans up into view
			bg1glow->setPositionY(
				Tween::easeOut(-screenHeight, screenHeight / 2, transitionFrame, OPEN_TRANSITION));
			bg2ship->setPositionY(
				Tween::easeOut(-screenHeight, screenHeight / 2, transitionFrame, OPEN_TRANSITION));
			bg3land->setPositionY(
				Tween::easeOut(-screenHeight, screenHeight / 2, transitionFrame, OPEN_TRANSITION));

			return;
		}
		case StartScreen: {
			if (transitionFrame > TRANSITION_DURATION) {
				endTransition();
				for (auto e : mainScreen) {
					e->setVisible(false);
				}
				bg2ship->setPositionY(screenHeight / 2);
				bg2ship->setVisible(false);
			} else {
				for (auto e : mainScreen) {
					e->setColor(Tween::fade(
						Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));
				}
				bg2ship->setPositionY(Tween::easeIn(screenHeight / 2, screenHeight / SHIP_FLY_POS,
													transitionFrame, TRANSITION_DURATION));
				bg2ship->setColor(
					Tween::fade(Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));
				switch (transitionState) {
					// Host screen case unneeded b/c waiting for host room before playing transition
					case ClientScreen: {
						if (transitionFrame == 1) {
							backBtn->setVisible(true);
						}

						clientScreen->setPositionY(
							Tween::easeOut(-screenHeight, 0, transitionFrame, TRANSITION_DURATION));
						backBtn->setColor(Tween::fade(
							Tween::linear(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));

						break;
					}
					case Credits: {
						if (transitionFrame == 1) {
							backBtn->setVisible(true);
							bg4landNoShip->setVisible(true);
						}

						backBtn->setColor(Tween::fade(
							Tween::linear(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));

						bg1glow->setColor(Tween::fade(
							Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));

						bg3land->setPositionY(
							Tween::easeInOut(screenHeight / 2, screenHeight / CREDITS_BG_POS,
											 transitionFrame, TRANSITION_DURATION));
						bg4landNoShip->setPositionY(
							Tween::easeInOut(screenHeight / 2, screenHeight / CREDITS_BG_POS,
											 transitionFrame, TRANSITION_DURATION));
						bg3land->setColor(Tween::fade(
							Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));

						if (transitionFrame == TRANSITION_DURATION) {
							bg3land->setVisible(false);
						}

						break;
					}
					default:
						break;
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
				break;
			}
			// Intentional Fall-Through (all the way down to ClientScreen)
		}
		case HostScreen:
			if (transitionState == HostLevelSelect) {
				if (transitionFrame == 1) {
					levelSelect->setVisible(true);
					levelSelect->setColor(Tween::fade(0));
				}

				// Total transition duration is 1.5x standard length
				const int halfTransition = TRANSITION_DURATION / 2;

				// Finished transition
				if (transitionFrame > TRANSITION_DURATION + halfTransition) {
					endTransition();
					hostScreen->setVisible(false);
					levelSelect->setColor(Color4::WHITE);
					return;
				}

				// 0x - 1x, push down license plate
				if (transitionFrame <= TRANSITION_DURATION) {
					hostScreen->setPositionY(
						Tween::easeIn(0, -screenHeight, transitionFrame, TRANSITION_DURATION));
				}

				// 0.5x - 1.5x, fade in level select
				if (transitionFrame > halfTransition) {
					levelSelect->setColor(Tween::fade(Tween::linear(
						0, 1, transitionFrame - halfTransition, TRANSITION_DURATION)));
				}
				return;
			} else {
				connScreen->setVisible(false);
			}
			// Intentional fall-through
		case ClientScreen: {
			if (transitionState == StartScreen) {
				// Start transition
				if (transitionFrame == 1) {
					for (auto e : mainScreen) {
						e->setVisible(true);
					}
					bg2ship->setVisible(true);
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

				for (auto e : mainScreen) {
					e->setColor(Tween::fade(
						Tween::linear(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));
				}
				backBtn->setColor(
					Tween::fade(Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));

				bg2ship->setColor(
					Tween::fade(Tween::linear(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));

				return;
			}
			break;
		}
		case ClientScreenSubmitted: {
			if (transitionFrame == 1) {
				if (transitionState == ClientScreenDone) {
					hostScreen->setVisible(true);
					hostScreen->setPositionY(-screenHeight);
					clientWaitHost->setVisible(true);
				} else {
					clientError->setVisible(true);
					clientError->setPositionY(-screenHeight);
				}
				hostBeginBtn->setVisible(false);
			}

			if (transitionFrame > 2 * TRANSITION_DURATION) {
				backBtn->setVisible(false);
				clientScreen->setVisible(false);
				if (transitionState == ClientScreenDone) {
					hostScreen->setPositionY(0);
				} else {
					clientError->setPositionY(0);
					clientEnteredRoom.clear();
					updateClientLabel();
					clientJoinBtn->setDown(false);
				}

				endTransition();
			}

			if (transitionFrame >= TRANSITION_DURATION) {
				if (transitionState == ClientScreenDone) {
					hostScreen->setPositionY(Tween::easeOut(-screenHeight, 0,
															transitionFrame - TRANSITION_DURATION,
															TRANSITION_DURATION));
				} else {
					clientError->setPositionY(Tween::easeOut(-screenHeight, 0,
															 transitionFrame - TRANSITION_DURATION,
															 TRANSITION_DURATION));
				}
			} else {
				clientScreen->setPositionY(
					Tween::easeIn(0, -screenHeight, transitionFrame, TRANSITION_DURATION));
				backBtn->setColor(
					Tween::fade(Tween::linear(1, 0, transitionFrame, TRANSITION_DURATION)));
			}

			break;
		}
		case ClientScreenError: {
			if (transitionFrame == 1) {
				backBtn->setVisible(true);
				clientScreen->setVisible(true);
				clientScreen->setPositionY(-screenHeight);
			}

			if (transitionFrame > 2 * TRANSITION_DURATION) {
				clientError->setVisible(false);
				endTransition();
			}

			if (transitionFrame < TRANSITION_DURATION) {
				clientError->setPositionY(
					Tween::easeIn(0, -screenHeight, transitionFrame, TRANSITION_DURATION));

			} else {
				clientScreen->setPositionY(Tween::easeOut(
					-screenHeight, 0, transitionFrame - TRANSITION_DURATION, TRANSITION_DURATION));
				backBtn->setColor(Tween::fade(Tween::linear(
					0, 1, transitionFrame - TRANSITION_DURATION, TRANSITION_DURATION)));
			}
			break;
		}
		case Credits: {
			if (transitionFrame == 1) {
				for (auto e : mainScreen) {
					e->setVisible(true);
				}
				bg3land->setVisible(true);
				bg2ship->setVisible(true);
			}

			// Transition over
			if (transitionFrame > TRANSITION_DURATION) {
				endTransition();
				credits->setVisible(false);
				backBtn->setVisible(false);
				bg4landNoShip->setVisible(false);
				return;
			}

			credits->setColor(
				Tween::fade(Tween::easeInOut(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));
			backBtn->setColor(
				Tween::fade(Tween::easeInOut(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));
			for (auto e : mainScreen) {
				e->setColor(Tween::fade(
					Tween::easeInOut(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));
			}

			bg1glow->setColor(
				Tween::fade(Tween::easeInOut(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));
			bg2ship->setColor(
				Tween::fade(Tween::easeInOut(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));

			bg3land->setPositionY(Tween::easeInOut(screenHeight / CREDITS_BG_POS, screenHeight / 2,
												   transitionFrame, TRANSITION_DURATION));
			bg4landNoShip->setPositionY(Tween::easeInOut(screenHeight / CREDITS_BG_POS,
														 screenHeight / 2, transitionFrame,
														 TRANSITION_DURATION));
			bg3land->setColor(
				Tween::fade(Tween::easeInOut(0.0f, 1.0f, transitionFrame, TRANSITION_DURATION)));
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
			switch (net->matchStatus()) {
				case MagicInternetBox::MatchmakingStatus::HostError:
					connScreen->setText("Error Connecting :(");
					backBtn->setVisible(true);
					backBtn->setColor(cugl::Color4::WHITE);
					break;
				case MagicInternetBox::MatchmakingStatus::HostApiMismatch:
					connScreen->setText("Please update");
					backBtn->setVisible(true);
					backBtn->setColor(cugl::Color4::WHITE);
					break;
				default:
					break;
			}
			break;
		}
		case HostScreen:
		case ClientScreenDone: {
			setNumPlayers();
			if (backBtn->isVisible()) {
				if (net->getNumPlayers() > 1) {
					backBtn->setVisible(false);
					hostBeginBtn->setVisible(true);
				}
			}
			break;
		}
		default: {
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
				net->forceDisconnect();
				// Intentional fall-through
			case ClientScreen:
			case Credits:
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
				hostNeedle->setAngle(0);
			} else if (buttonManager.tappedButton(clientBtn, tapData)) {
				transitionState = ClientScreen;
				clientScreen->setPositionY(-screenHeight);
				clientScreen->setVisible(true);
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
				transitionState = StartScreen;
			}
			break;
		}
		case HostScreen: {
			if (buttonManager.tappedButton(backBtn, tapData)) {
				CULog("Going Back");
				net->forceDisconnect();
				transitionState = StartScreen;
			} else if (net->getNumPlayers() >= globals::MIN_PLAYERS) {
				if (buttonManager.tappedButton(hostBeginBtn, tapData)) {
					transitionState = HostLevelSelect;
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
			break;
		}
		case ClientScreenError: {
			transitionState = ClientScreen;
			break;
		}
		case Credits: {
			if (buttonManager.tappedButton(backBtn, tapData)) {
				CULog("Going Back");
				transitionState = StartScreen;
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

	if (transitionState != NA) {
		processTransition();
	} else {
		processUpdate();
		processButtons();
	}

	switch (net->matchStatus()) {
		case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
		case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
		case MagicInternetBox::MatchmakingStatus::ClientError:
			if (currState == ClientScreenSubmitted) {
				transitionState = ClientScreenError;
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
			if (backBtn->isVisible()) {
				transitionState = ClientScreenDone;
				setRoomID();
				setNumPlayers();
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
