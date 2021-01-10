#include "MainMenuTransitions.h"

#include <cugl/cugl.h>

#include <memory>

#include "MainMenuMode.h"

using namespace cugl;

#pragma region Animation Constants

/** Duration of a standard transition */
constexpr int TRANSITION_DURATION = 30;

/** Duration of opening transition */
constexpr int OPEN_TRANSITION = 120;

/** When during opening transition to fade in stuff */
constexpr int OPEN_TRANSITION_FADE = 90;

/** Divisor of screen height to get credits bg position */
constexpr float CREDITS_BG_POS = 2.5;

/** Divisor of screen height to get ship flight destination position */
constexpr float SHIP_FLY_POS = 1.5;

/** The nodes containing all UI for the starting splash screen */
constexpr std::array<const char *, 3> MAIN_SCREEN = {"matchmaking_home", "matchmaking_gamelogo",
													 "matchmaking_creditsbtn"};
#pragma endregion

MainMenuMode::MainMenuTransitions::MainMenuTransitions(MainMenuMode *parent) {
	this->parent = parent;
}

void MainMenuMode::MainMenuTransitions::init(const std::shared_ptr<AssetManager> &assets,
											 bool toCredits) {
	auto studioLogo = assets->get<Node>("matchmaking_studiologo");

	auto screenHeight = parent->screenHeight;

	animations.fadeOut(studioLogo, TRANSITION_DURATION * 2);

	animations.registerNode("matchmaking_mainmenubg-glow", assets);
	animations.registerNode("matchmaking_mainmenubg-ship", assets);
	animations.registerNode("matchmaking_mainmenubg-land", assets);
	animations.registerNode("matchmaking_mainmenubg-landnoship", assets);

	if (!toCredits) {
		animations.animateY("matchmaking_mainmenubg-glow", Tween::TweenType::EaseOut,
							screenHeight / 2, OPEN_TRANSITION);
		animations.animateY("matchmaking_mainmenubg-ship", Tween::TweenType::EaseOut,
							screenHeight / 2, OPEN_TRANSITION);
		animations.animateY("matchmaking_mainmenubg-land", Tween::TweenType::EaseOut,
							screenHeight / 2, OPEN_TRANSITION);
	}

	for (const auto *e : MAIN_SCREEN) {
		animations.registerNode(e, assets);
		if (!toCredits) {
			animations.fadeIn(e, TRANSITION_DURATION, OPEN_TRANSITION_FADE);
		}
	}

	animations.registerNode("matchmaking_backbtn", assets);

	animations.registerNode("matchmaking_host", assets);
	animations.registerNode("matchmaking_client", assets);

	animations.registerNode("matchmaking_credits", assets);

	animations.registerNode("matchmaking_levelselect", assets);
	animations.registerNode("matchmaking_tutorialbtn", assets);

	animations.registerNode("matchmaking_clienterr", assets);

	if (toCredits) {
		animations.animateY("matchmaking_mainmenubg-landnoship", Tween::TweenType::EaseInOut,
							screenHeight / CREDITS_BG_POS, TRANSITION_DURATION);
		animations.fadeIn("matchmaking_mainmenubg-landnoship", TRANSITION_DURATION);

		animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION);

		parent->credits->setVisible(true);
		parent->credits->setColor(Color4::WHITE);
		parent->credits->setPositionY(0);
		parent->creditsScrollFrame = 0;

		parent->currState = Credits;
	}
}

// NOLINTNEXTLINE Bespoke UI code is gonna be big
void MainMenuMode::MainMenuTransitions::to(MatchState destination) {
	auto screenHeight = parent->screenHeight;
	switch (parent->currState) {
		case StartScreen: {
			switch (destination) {
				case HostScreenWait:
					parent->startHostThread = std::make_unique<std::thread>([]() {
						MagicInternetBox::getInstance()->initHost();
						CULog("SEPARATE THREAD FINISHED INIT HOST");
					});
					parent->connScreen->setText("Connecting to Server...");
					parent->hostNeedle->setAngle(0);
					parent->needlePos = 0;
					parent->clientWaitHost->setVisible(false);

					mainMenuOut();

					parent->currState = destination;
					break;
				case ClientScreen:
					parent->clientEnteredRoom.clear();
					parent->updateClientLabel();

					mainMenuOut();

					animations.animateY("matchmaking_client", Tween::TweenType::EaseOut, 0,
										TRANSITION_DURATION);
					animations.fadeIn("matchmaking_client", 1);
					animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION);

					parent->currState = destination;
					break;
				case Credits:
					parent->currState = Credits;
					parent->credits->setVisible(true);
					parent->credits->setColor(Color4::WHITE);
					parent->credits->setPositionY(0);
					parent->creditsScrollFrame = 0;

					mainMenuOut();
					animations.fadeOut("matchmaking_mainmenubg-glow", TRANSITION_DURATION);

					animations.animateY("matchmaking_mainmenubg-land", Tween::TweenType::EaseInOut,
										screenHeight / CREDITS_BG_POS, TRANSITION_DURATION);
					animations.fadeOut("matchmaking_mainmenubg-land", TRANSITION_DURATION);

					animations.animateY("matchmaking_mainmenubg-landnoship",
										Tween::TweenType::EaseInOut, screenHeight / CREDITS_BG_POS,
										TRANSITION_DURATION);
					animations.fadeIn("matchmaking_mainmenubg-landnoship", 1);

					animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION);
					break;
				default:
					break;
			}
			break;
		}
		case HostScreenWait: {
			switch (destination) {
				case HostScreen:
					parent->setRoomID();
					if (parent->connScreen->isVisible()) {
						animations.fadeOut(parent->connScreen, TRANSITION_DURATION);
					}
					parent->startHostThread->detach();

					parent->currState = HostScreen;

					animations.animateY("matchmaking_host", Tween::TweenType::EaseOut, 0,
										TRANSITION_DURATION);
					animations.fadeIn("matchmaking_host", 1);
					animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION);
					break;
				case StartScreen:
					mainMenuIn();
					parent->startHostThread->detach();
					parent->net->reset();
					animations.fadeOut(parent->connScreen, TRANSITION_DURATION);
					break;
				default:
					break;
			}
			break;
		}
		case HostScreen: {
			switch (destination) {
				case HostLevelSelect:
					animations.animateY("matchmaking_host", Tween::TweenType::EaseIn, -screenHeight,
										TRANSITION_DURATION);
					animations.fadeOut("matchmaking_host", 1, TRANSITION_DURATION);
					animations.fadeIn("matchmaking_levelselect", TRANSITION_DURATION,
									  TRANSITION_DURATION / 2);
					animations.fadeIn("matchmaking_tutorialbtn", TRANSITION_DURATION,
									  TRANSITION_DURATION / 2);

					parent->currState = HostLevelSelect;
					break;
				case StartScreen:
					parent->net->reset();
					animations.animateY("matchmaking_host", Tween::TweenType::EaseIn, -screenHeight,
										TRANSITION_DURATION);
					animations.fadeOut("matchmaking_host", 1, TRANSITION_DURATION);
					mainMenuIn();
					break;
				default:
					break;
			}
			break;
		}
		case ClientScreen: {
			mainMenuIn();
			parent->currState = StartScreen;
			animations.animateY("matchmaking_client", Tween::TweenType::EaseIn, -screenHeight,
								TRANSITION_DURATION);
			animations.fadeOut("matchmaking_client", 1, TRANSITION_DURATION);
			break;
		}
		case ClientScreenSubmitted: {
			switch (destination) {
				case ClientScreenError:
					parent->currState = ClientScreenError;

					animations.animateY("matchmaking_client", Tween::TweenType::EaseIn,
										-screenHeight, TRANSITION_DURATION);
					animations.fadeOut("matchmaking_client", 1, TRANSITION_DURATION);
					animations.fadeOut("matchmaking_backbtn", TRANSITION_DURATION);

					animations.fadeIn("matchmaking_clienterr", 1, TRANSITION_DURATION);
					animations.animateY("matchmaking_clienterr", Tween::TweenType::EaseOut, 0,
										TRANSITION_DURATION, TRANSITION_DURATION);
					break;
				case ClientScreenDone:
					parent->currState = ClientScreenDone;

					parent->hostNeedle->setAngle(0);
					parent->needlePos = 0;

					animations.animateY("matchmaking_client", Tween::TweenType::EaseIn,
										-screenHeight, TRANSITION_DURATION);
					animations.fadeOut("matchmaking_client", 1, TRANSITION_DURATION);

					animations.fadeIn("matchmaking_host", 1, TRANSITION_DURATION);
					animations.animateY("matchmaking_host", Tween::TweenType::EaseOut, 0,
										TRANSITION_DURATION, TRANSITION_DURATION);
					parent->clientWaitHost->setVisible(true);

					parent->setRoomID();
					parent->setNumPlayers();

					break;
				default:
					break;
			}
			break;
		}
		case ClientScreenDone: {
			parent->net->reset();

			animations.animateY("matchmaking_host", Tween::TweenType::EaseIn, -screenHeight,
								TRANSITION_DURATION);
			animations.fadeOut("matchmaking_host", 1, TRANSITION_DURATION);

			mainMenuIn();
			parent->currState = StartScreen;
			break;
		}
		case ClientScreenError: {
			parent->currState = ClientScreen;

			animations.fadeOut("matchmaking_clienterr", 1, TRANSITION_DURATION);
			animations.animateY("matchmaking_clienterr", Tween::TweenType::EaseIn, -screenHeight,
								TRANSITION_DURATION);

			animations.animateY("matchmaking_client", Tween::TweenType::EaseOut, 0,
								TRANSITION_DURATION, TRANSITION_DURATION);
			animations.fadeIn("matchmaking_client", 1, TRANSITION_DURATION);
			animations.fadeIn("matchmaking_backbtn", TRANSITION_DURATION, TRANSITION_DURATION);

			parent->clientEnteredRoom.clear();
			parent->updateClientLabel();
			break;
		}
		case Credits: {
			animations.fadeIn("matchmaking_mainmenubg-glow", TRANSITION_DURATION);

			animations.animateY("matchmaking_mainmenubg-landnoship", Tween::TweenType::EaseInOut,
								screenHeight / 2, TRANSITION_DURATION);
			animations.fadeOut("matchmaking_mainmenubg-landnoship", 1, TRANSITION_DURATION);

			animations.animateY("matchmaking_mainmenubg-land", Tween::TweenType::EaseInOut,
								screenHeight / 2, TRANSITION_DURATION);
			animations.fadeIn("matchmaking_mainmenubg-land", TRANSITION_DURATION);

			animations.fadeOut("matchmaking_credits", TRANSITION_DURATION);
			mainMenuIn();
			break;
		}
		default:
			break;
	}
}
void MainMenuMode::MainMenuTransitions::reset() { animations.reset(); }

bool MainMenuMode::MainMenuTransitions::step() { return animations.step(); }

void MainMenuMode::MainMenuTransitions::mainMenuOut() {
	auto screenHeight = parent->screenHeight;
	for (const auto *e : MAIN_SCREEN) {
		animations.fadeOut(e, TRANSITION_DURATION);
	}
	animations.animateY("matchmaking_mainmenubg-ship", Tween::TweenType::EaseIn,
						screenHeight / SHIP_FLY_POS, TRANSITION_DURATION);
	animations.fadeOut("matchmaking_mainmenubg-ship", TRANSITION_DURATION);
}

void MainMenuMode::MainMenuTransitions::mainMenuIn() {
	auto screenHeight = parent->screenHeight;
	for (const auto *e : MAIN_SCREEN) {
		animations.fadeIn(e, TRANSITION_DURATION);
	}
	animations.animateY("matchmaking_mainmenubg-ship", Tween::TweenType::EaseIn, screenHeight / 2,
						1);
	animations.fadeIn("matchmaking_mainmenubg-ship", TRANSITION_DURATION);
	animations.fadeOut("matchmaking_backbtn", TRANSITION_DURATION);

	parent->currState = StartScreen;
}
