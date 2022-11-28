#include "GameGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "AdUtils.h"
#include "Globals.h"
#include "ShipSegmentNode.h"
#include "TutorialConstants.h"
#include "Tween.h"

using namespace cugl;
using namespace std;

// NOLINTNEXTLINE Simple 4-vectors are unlikely to throw an exception
const std::vector<Color4> GameGraphRoot::BREACH_COLOR{
	cugl::Color4(219, 197, 52), cugl::Color4(227, 100, 159), cugl::Color4(158, 212, 87),
	cugl::Color4(244, 150, 40), cugl::Color4(47, 206, 197),	 cugl::Color4(152, 95, 204)};

#pragma mark -
#pragma mark Level Layout

/** Loop range of the background image */
constexpr int BG_SCROLL_LIMIT = 256;

/** Parallax speed of background image */
constexpr float BG_SCROLL_SPEED = 0.5;

/** Animation cycle length of ship red flash */
constexpr int MAX_HEALTH_WARNING_FRAMES = 150;

/** Maximum alpha value for health warning overlay */
constexpr int MAX_HEALTH_WARNING_ALPHA = 100;

/** Percentage of ship health to start showing yellow */
constexpr float SHIP_HEALTH_YELLOW_CUTOFF = 0.5f;

/** Percentage of ship health to start showing red */
constexpr float SHIP_HEALTH_RED_CUTOFF = 0.2f;

/** Portion of health bar shown on screen */
constexpr float HEALTH_RANGE = 100;

/** Offset of health bar (angle of health bar when health = 0) */
constexpr float HEALTH_OFFSET = 217;

/** Time to stop showing health tutorial */
constexpr int HEALTH_TUTORIAL_CUTOFF = 20;

/** Time to stop showing move tutorial */
constexpr int MOVE_TUTORIAL_CUTOFF = 10;

/** Time to show breach tutorial */
constexpr int BREACH_TUTORIAL_CUTOFF = 10;

/** Time to start showing timer */
constexpr int TIMER_TUTORIAL_CUTOFF = 13;

/** Tutorial asset scale */
constexpr float TUTORIAL_SCALE = 0.4f;

/** Timer offset */
constexpr float TIMER_OFFSET_X = -30;

/** Timer offset */
constexpr float TIMER_OFFSET_Y = 50;

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
bool GameGraphRoot::init( // NOLINT Yeah it's a big function; we'll live with it for now
	const std::shared_ptr<cugl::AssetManager>& assets, const std::shared_ptr<ShipModel>& ship,
	unsigned int playerID) {
	this->playerID = playerID;
	this->ship = ship;
	this->prevPlayerAngle = ship->getDonuts().at(playerID)->getAngle();
	isBackToMainMenu = false;
	status = Normal;

	// Initialize the scene to a locked width
	cugl::Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	screenHeight = dimen.height;
	// Initialize the scene to a locked width
	if (assets == nullptr) {
		return false;
	}
	if (!Scene::init(dimen)) {
		return false;
	}

	// Start up the input handler
	this->assets = assets;

	// Acquire the scene built by the asset loader and resize it the scene
	auto scene = assets->get<Node>("game");
	scene->setContentSize(dimen);
	scene->doLayout(); // Repositions the HUD

	// Game Scene Components.
	allSpace = assets->get<Node>("game_field");
	farSpace = assets->get<Node>("game_field_far");
	nearSpace = assets->get<Node>("game_field_near");
	std::shared_ptr<PolygonNode> tempDonutNode =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_player1"));
	donutPos = tempDonutNode->getPosition();
	breachesNode = assets->get<Node>("game_field_near_breaches");
	breachSparklesNode = assets->get<Node>("game_field_near_breachsparkles");
	buttonSparklesNode = assets->get<Node>("game_field_near_buttonsparkles");
	doorsNode = assets->get<Node>("game_field_near_doors");
	unopsNode = assets->get<Node>("game_field_near_unops");
	externalDonutsNode = assets->get<Node>("game_field_near_externaldonuts");
	healthNode =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_healthBase"));
	healthNodeOverlay =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_health"));
	healthNodeOverlay->setVisible(true);
	healthNodeNumbers =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_healthNumbers"));
	healthNodeNumbers->setVisible(true);
	coordHUD = std::dynamic_pointer_cast<Label>(assets->get<Node>("game_hud"));
	timerBorder =
		std::dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_timerBorder"));
	timerBorder->setVisible(true);
	coordHUD->setVisible(true);
	currentHealthWarningFrame = 0;
	moveTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_moveTutorial"));
	moveTutorial->setVisible(false);
	if (ship->getLevelNum() == tutorial::BREACH_LEVEL) {
		moveTutorial->setVisible(true);
	}
	healthTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_healthTutorial"));
	healthTutorial->setVisible(false);
	communicateTutorial = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_communicateTutorial"));
	communicateTutorial->setVisible(false);
	timerTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_timerTutorial"));
	timerTutorial->setVisible(false);
	if (ship->getLevelNum() == tutorial::REAL_LEVELS.at(0)) {
		healthTutorial->setVisible(false);
		communicateTutorial->setVisible(false);
		timerTutorial->setVisible(true);
	}
	rollTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_rollTutorial"));
	rollTutorial->setVisible(false);

	tutorialNode = assets->get<Node>("game_field_near_tutorial");
	buttonsNode = assets->get<Node>("game_field_near_button");
	nearSpace->setAngle(0.0f);

	// Initialize Roll Challenge
	stabilizerNode = std::make_shared<StabilizerNode>(assets, ship->getStabilizer());
	blackoutOverlay =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_blackoutOverlay"));
	blackoutOverlay->setColor(Tween::fade(0));
	currentTeleportationFrame = 0;
	prevIsStabilizerFail = false;

	// Initialize Ship Segments
	shipSegsNode = ShipSegmentWrap::alloc(assets);
	nearSpace->addChild(shipSegsNode);
	nearSpace->sortZOrder();

	const std::shared_ptr<DonutModel> playerModel = ship->getDonuts()[playerID];

	// Initialize Players
	for (uint8_t i = 0; i < ship->getDonuts().size(); i++) {
		const std::shared_ptr<DonutModel> donutModel = ship->getDonuts().at(i);
		const string donutColor = PLAYER_COLOR.at(donutModel->getColorId());
		// Player node is handled separately
		if (i == playerID) {
			donutNode = PlayerDonutNode::alloc(playerModel, screenHeight, assets, donutColor,
											   tempDonutNode->getPosition());
			allSpace->addChild(donutNode);
			tempDonutNode = nullptr;
		} else {
			const std::shared_ptr<ExternalDonutNode> newDonutNode = ExternalDonutNode::alloc(
				donutModel, playerModel, ship->getSize(), assets, donutColor);
			externalDonutsNode->addChild(newDonutNode);
		}
	}

	// Initialize Breaches
	const std::shared_ptr<cugl::Texture> breachFilmstrip = assets->get<Texture>("breach_filmstrip");
	const std::shared_ptr<cugl::Texture> breachSparkleBig =
		assets->get<Texture>("breach_sparklebig");
	const std::shared_ptr<cugl::Texture> breachSparkleSmall =
		assets->get<Texture>("breach_sparklesmall");
	for (uint8_t i = 0; i < ship->getBreaches().size(); i++) {
		const std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at(i);
		const string breachColor =
			PLAYER_COLOR.at(ship->getDonuts().at(breachModel->getPlayer())->getColorId());
		const std::shared_ptr<cugl::Texture> pattern =
			assets->get<Texture>("breach_" + breachColor);
		cugl::Color4 const color =
			BREACH_COLOR.at(ship->getDonuts().at(breachModel->getPlayer())->getColorId());
		// Initialize sparkle nodes
		const std::shared_ptr<SparkleNode> sparkleNodeBig =
			SparkleNode::alloc(playerModel, ship->getSize(), breachSparkleBig, Color4::WHITE,
							   SparkleNode::SparkleType::Big);
		breachSparklesNode->addChild(sparkleNodeBig);
		const std::shared_ptr<SparkleNode> sparkleNodeSmall =
			SparkleNode::alloc(playerModel, ship->getSize(), breachSparkleSmall, Color4::WHITE,
							   SparkleNode::SparkleType::Small);
		breachSparklesNode->addChild(sparkleNodeSmall);

		const std::shared_ptr<BreachNode> breachNode =
			BreachNode::alloc(breachModel, playerModel, ship->getSize(), breachFilmstrip, pattern,
							  color, sparkleNodeBig, sparkleNodeSmall);
		breachNode->setTag(static_cast<unsigned int>(i + 1));

		// Add the breach node
		breachesNode->addChild(breachNode);
		if (ship->getLevelNum() == tutorial::BREACH_LEVEL) {
			const std::shared_ptr<Texture> image = assets->get<Texture>("jump_tutorial0");
			const std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			tutorial->setScale(TUTORIAL_SCALE);
			tutorial->setBreachNode(breachNode);
			tutorialNode->addChildWithTag(tutorial, static_cast<unsigned int>(i + 1));
		}
	}

	// Initialize Doors
	for (uint8_t i = 0; i < ship->getDoors().size(); i++) {
		const std::shared_ptr<DoorModel> doorModel = ship->getDoors().at(i);
		const std::shared_ptr<DoorNode> doorNode =
			DoorNode::alloc(doorModel, playerModel, ship->getSize(), assets);
		doorsNode->addChildWithTag(doorNode, i + 1);
	}

	// Initialize unopenable doors
	for (uint8_t i = 0; i < ship->getUnopenable().size(); i++) {
		const std::shared_ptr<Unopenable> unopModel = ship->getUnopenable().at(i);
		const std::shared_ptr<Texture> image = assets->get<Texture>("unop");
		const std::shared_ptr<UnopenableNode> unopNode =
			UnopenableNode::alloc(unopModel, playerModel, ship->getSize(), image);
		unopsNode->addChildWithTag(unopNode, i + 1);
	}

	// Initialize Buttons
	for (uint8_t i = 0; i < ship->getButtons().size(); i++) {
		const std::shared_ptr<ButtonModel> buttonModel = ship->getButtons().at(i);
		const std::shared_ptr<SparkleNode> sparkleNode =
			SparkleNode::alloc(playerModel, ship->getSize(), breachSparkleBig, Color4::WHITE,
							   SparkleNode::SparkleType::Big);
		buttonSparklesNode->addChild(sparkleNode);
		const std::shared_ptr<ButtonNode> buttonNode =
			ButtonNode::alloc(buttonModel, playerModel, ship->getSize(), assets, sparkleNode);
		buttonsNode->addChildWithTag(buttonNode, i + 1);
	}

	if (ship->getLevelNum() == tutorial::DOOR_LEVEL) {
		for (int i = 0; i < doorsNode->getChildCount(); i++) {
			const std::shared_ptr<Texture> image = assets->get<Texture>("door_tutorial");
			const std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			const shared_ptr<DoorNode> doorNode = dynamic_pointer_cast<DoorNode>(
				doorsNode->getChildByTag(static_cast<unsigned int>(i + 1)));
			tutorial->setDoorNode(doorNode);
			tutorial->setScale(TUTORIAL_SCALE);
			tutorialNode->addChildWithTag(tutorial, i + 1);
		}
	} else if (ship->getLevelNum() == tutorial::BUTTON_LEVEL) {
		for (int i = 0; i < buttonsNode->getChildCount(); i++) {
			const std::shared_ptr<Texture> image = assets->get<Texture>("engine_tutorial");
			const std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			const shared_ptr<ButtonNode> buttonNode = dynamic_pointer_cast<ButtonNode>(
				buttonsNode->getChildByTag(static_cast<unsigned int>(i + 1)));
			tutorial->setButtonNode(buttonNode);
			tutorial->setScale(TUTORIAL_SCALE);
			tutorialNode->addChildWithTag(tutorial, i + 1);
		}
	} else if (ship->getLevelNum() == tutorial::REAL_LEVELS.at(4)) {
		const std::shared_ptr<Texture> image = assets->get<Texture>("timer_tutorial1");
		timerTutorial->setTexture(image);
		const float posY = timerTutorial->getPositionY() + TIMER_OFFSET_Y;
		const float posX = timerTutorial->getPositionX() + TIMER_OFFSET_X;
		timerTutorial->setPosition(posX, posY);
	}

	// Overlay Components

	// Initialize Reconnect Overlay
	reconnectScreen = std::make_shared<ReconnectScreen>(assets);

	// Initialize Pause Screen Componenets
	pauseMenu = std::make_shared<PauseMenu>(assets);

	// Initialize Loss Screen Componenets
	lossScreen = assets->get<Node>("game_overlay_loss");
	restartBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("game_overlay_loss_restartBtn"));
	lostWaitText =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_loss_waitText"));

	// Initialize Win Screen Componenets
	winScreen = std::make_shared<WinScreen>(assets);

	lossScreen->setVisible(false);
	nearSpace->setVisible(true);
	healthNode->setVisible(true);
	lostWaitText->setVisible(false);
	restartBtn->setVisible(true);

	lastButtonPressed = None;

	// Register Regular Buttons
	buttonManager.registerButton(restartBtn);

	addChild(scene);
	addChild(stabilizerNode);
	addChild(winScreen);
	addChild(reconnectScreen);
	addChild(pauseMenu);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameGraphRoot::dispose() {
	if (_active) {
		removeAllChildren();
		buttonManager.clear();
		allSpace->removeChild(donutNode);
		allSpace = nullptr;
		farSpace = nullptr;
		nearSpace = nullptr;
		donutNode->removeAllChildren();
		donutNode = nullptr;

		coordHUD = nullptr;
		breachesNode->removeAllChildren();
		breachesNode = nullptr;
		shipSegsNode = nullptr;
		doorsNode->removeAllChildren();
		doorsNode = nullptr;
		tutorialNode->removeAllChildren();
		tutorialNode = nullptr;
		unopsNode->removeAllChildren();
		unopsNode = nullptr;
		externalDonutsNode->removeAllChildren();
		externalDonutsNode = nullptr;
		breachSparklesNode->removeAllChildren();
		breachSparklesNode = nullptr;

		stabilizerNode = nullptr;
		healthNode = nullptr;

		blackoutOverlay = nullptr;
		reconnectScreen = nullptr;
		pauseMenu = nullptr;

		lossScreen = nullptr;
		restartBtn = nullptr;
		lostWaitText = nullptr;

		buttonsNode->removeAllChildren();
		buttonsNode = nullptr;

		winScreen = nullptr;

		_active = false;
	}
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void GameGraphRoot::reset() {
	// Reset the parallax
	Vec2 position = farSpace->getPosition();
	farSpace->setAnchor(Vec2::ANCHOR_CENTER);
	farSpace->setPosition(position);
	farSpace->setAngle(0.0f);
	position = nearSpace->getPosition();
	nearSpace->setAnchor(Vec2::ANCHOR_CENTER);
	nearSpace->setPosition(position);
	nearSpace->setAngle(0.0f);
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameGraphRoot::update( // NOLINT Yeah it's a big function; we'll live with it for now
	float /*timestep*/) {
	// "Drawing" code.  Move everything BUT the donut
	// Update the HUD
	for (auto& i : ship->getButtons()) {
		if (i->getIsActive()) {
			coordHUD->setColor(cugl::Color4::RED);
			break;
		}
		coordHUD->setColor(cugl::Color4::WHITE);
	}
	std::string const time = timerText();
	if (time != coordHUD->getText()) {
		coordHUD->setText(time);
	}

	// State Check for Drawing
	switch (status) {
		case Normal:
			// Hide Unnecessary Overlays
			lossScreen->setVisible(false);
			reconnectScreen->deactivateStep();
			// Reset Timeout Counters to negative value
			pauseMenu->update();
			break;
		case Loss: {
			const bool justLost = !lossScreen->isVisible();
			// Show loss screen
			lossScreen->setVisible(true);
			pauseMenu->setVisible(false);
			if (playerID != 0) {
				lostWaitText->setVisible(true);
				restartBtn->setVisible(false);
			}
			if (justLost) {
				// Show an ad to the losers
				AdUtils::displayInterstitial();
			}
			break;
		}
		case Win:
			// Show Win Screen
			if (!winScreen->isVisible()) {
				nearSpace->setVisible(false);
				healthNode->setVisible(false);
				rollTutorial->setVisible(false);
				moveTutorial->setVisible(false);
				healthTutorial->setVisible(false);
				communicateTutorial->setVisible(false);
				timerBorder->setVisible(false);
				healthNodeOverlay->setVisible(false);
				healthNodeNumbers->setVisible(false);
				coordHUD->setVisible(false);
				winScreen->activate(*MagicInternetBox::getInstance().getLevelNum());
				pauseMenu->setVisible(false);
			}
			winScreen->update();
			break;
		case Reconnecting:
			// Still Reconnecting, Animation Frames
			if (reconnectScreen->activeStep()) {
				isBackToMainMenu = true;
			}
			break;
		default:
			CULog("ERROR: Uncaught DrawingStatus Value Occurred");
	}
	if (ship->getTimeless()) {
		coordHUD->setVisible(false);
		timerBorder->setVisible(false);
	}

	// Button Checks for Special Case Buttons
	processButtons();

	if (ship->getHealth() < 1) {
		healthNodeOverlay->setVisible(false);
	} else {
		const float percentHealth = ship->getHealth() / ship->getInitHealth();
		if (percentHealth == 1) {
			healthNodeOverlay->setAngle(((percentHealth * HEALTH_RANGE) + HEALTH_OFFSET + 3) *
										globals::PI_180);
			const std::shared_ptr<Texture> image = assets->get<Texture>("health_green");
			healthNodeOverlay->setTexture(image);
		} else {
			healthNodeOverlay->setAngle(((percentHealth * HEALTH_RANGE) + HEALTH_OFFSET) *
										globals::PI_180);
		}
		if (percentHealth < SHIP_HEALTH_RED_CUTOFF) {
			const std::shared_ptr<Texture> image = assets->get<Texture>("health_red");
			healthNodeOverlay->setTexture(image);
		} else if (percentHealth < SHIP_HEALTH_YELLOW_CUTOFF) {
			const std::shared_ptr<Texture> image = assets->get<Texture>("health_yellow");
			healthNodeOverlay->setTexture(image);
		}
	}

	if (ship->getLevelNum() == tutorial::BREACH_LEVEL) {
		if (trunc(ship->canonicalTimeElapsed) > BREACH_TUTORIAL_CUTOFF) {
			for (int i = 0; i < tutorialNode->getChildCount(); i++) {
				const shared_ptr<TutorialNode> tutorial =
					dynamic_pointer_cast<TutorialNode>(tutorialNode->getChildByTag(i + 1));
				if (tutorial != nullptr) {
					tutorial->setVisible(true);
					if (tutorial->getPlayer() == playerID) {
						const int breachHealth = tutorial->getBreachNode()->getModel()->getHealth();
						std::shared_ptr<Texture> image = assets->get<Texture>("fix_count3");
						if (breachHealth == 1) {
							image = assets->get<Texture>("fix_count1");
						} else if (breachHealth == 2) {
							image = assets->get<Texture>("fix_count2");
						}
						tutorial->setTexture(image);
					}
				}
			}
		}
	}

	if (ship->getLevelNum() == tutorial::BREACH_LEVEL) {
		if (trunc(ship->canonicalTimeElapsed) == MOVE_TUTORIAL_CUTOFF) {
			moveTutorial->setVisible(false);
		}
	} else if (ship->getLevelNum() == tutorial::REAL_LEVELS.at(0)) {
		if (trunc(ship->canonicalTimeElapsed) == HEALTH_TUTORIAL_CUTOFF) {
			communicateTutorial->setVisible(false);
			healthTutorial->setVisible(true);
		} else if (trunc(ship->canonicalTimeElapsed) == MOVE_TUTORIAL_CUTOFF) {
			timerTutorial->setVisible(false);
			healthTutorial->setVisible(false);
			communicateTutorial->setVisible(true);
		}
	} else if (ship->getLevelNum() == tutorial::REAL_LEVELS.at(4)) {
		if (trunc(ship->canonicalTimeElapsed) > TIMER_TUTORIAL_CUTOFF) {
			timerTutorial->setVisible(true);
		} else {
			timerTutorial->setVisible(false);
		}
	} else if (ship->getLevelNum() == tutorial::STABILIZER_LEVEL) {
		rollTutorial->setVisible(true);
		auto& stabilizer = ship->getStabilizer();
		if (stabilizer.getIsActive()) {
			const std::shared_ptr<Texture> image = assets->get<Texture>("stabilizer_tutorial1");
			rollTutorial->setTexture(image);
		} else {
			const std::shared_ptr<Texture> image = assets->get<Texture>("stabilizer_tutorial0");
			rollTutorial->setTexture(image);
		}
	}
	// Reanchor the node at the center of the screen and rotate about center.
	const Vec2 position = farSpace->getPosition();
	farSpace->setAnchor(Vec2::ANCHOR_CENTER);
	if (position.x == -BG_SCROLL_LIMIT) {
		farSpace->setPositionX(0);
	} else {
		farSpace->setPosition(position -
							  Vec2(BG_SCROLL_SPEED, 0)); // Reseting the anchor changes the position
	}

	// Rotate nearSpace about center.
	const float newPlayerAngle = ship->getDonuts().at(playerID)->getAngle();
	float delta = (prevPlayerAngle - newPlayerAngle) * globals::PI_180;
	delta = (delta < -globals::PI)
				? (delta + ship->getSize() * globals::PI_180)
				: (delta > globals::PI ? delta - ship->getSize() * globals::PI_180 : delta);
	if (std::abs(delta) > globals::SEG_SIZE) {
		delta = fmod(prevPlayerAngle, globals::SEG_SIZE / globals::PI_180) -
				fmod(newPlayerAngle, globals::SEG_SIZE / globals::PI_180);
		delta = delta * globals::PI_180;
	}
	nearSpace->setAngle(globals::remainderPos(nearSpace->getAngle() + delta, globals::TWO_PI));
	prevPlayerAngle = newPlayerAngle;

	// Update ship segments
	shipSegsNode->updateSegments(nearSpace->getAngle(), ship->getSize(),
								 ship->getDonuts().at(playerID)->getAngle());

	// Update breaches textures if recycled
	for (uint8_t i = 0; i < ship->getBreaches().size(); i++) {
		const std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at(i);
		const shared_ptr<BreachNode> breachNode =
			dynamic_pointer_cast<BreachNode>(breachesNode->getChildByTag(i + 1));
		if (!breachNode->getIsAnimatingShrink() && breachModel->getHealth() > 0 &&
			breachModel->getNeedSpriteUpdate()) {
			cugl::Color4 const color =
				BREACH_COLOR.at(ship->getDonuts().at(breachModel->getPlayer())->getColorId());
			const string breachColor =
				PLAYER_COLOR.at(ship->getDonuts().at(breachModel->getPlayer())->getColorId());
			const std::shared_ptr<Texture> image = assets->get<Texture>("breach_" + breachColor);
			breachNode->resetAppearance(image, color);
			breachModel->setNeedSpriteUpdate(false);
		}
	}

	stabilizerNode->update();

	// Animate health warning flashing
	if (currentHealthWarningFrame != 0) {
		currentHealthWarningFrame += 1;
		if (currentHealthWarningFrame == MAX_HEALTH_WARNING_FRAMES) {
			if (ship->getHealth() > SHIP_HEALTH_RED_CUTOFF * ship->getInitHealth()) {
				currentHealthWarningFrame = 0;
				setSegHealthWarning(0);
			} else {
				setSegHealthWarning(MAX_HEALTH_WARNING_ALPHA / MAX_HEALTH_WARNING_FRAMES * 2);
				currentHealthWarningFrame = 1;
			}
		} else {
			int alpha = 0;
			if (currentHealthWarningFrame < MAX_HEALTH_WARNING_FRAMES / 2) {
				alpha = MAX_HEALTH_WARNING_ALPHA * currentHealthWarningFrame /
						MAX_HEALTH_WARNING_FRAMES * 2;
			} else {
				alpha = MAX_HEALTH_WARNING_ALPHA *
						(MAX_HEALTH_WARNING_FRAMES - currentHealthWarningFrame) /
						MAX_HEALTH_WARNING_FRAMES * 2;
			}
			setSegHealthWarning(alpha);
		}
	} else if (ship->getHealth() <= SHIP_HEALTH_RED_CUTOFF * ship->getInitHealth()) {
		setSegHealthWarning(MAX_HEALTH_WARNING_ALPHA / MAX_HEALTH_WARNING_FRAMES * 2);
		currentHealthWarningFrame = 1;
	}

	// Handle teleportation
	doTeleportAnimation();
}

void GameGraphRoot::processButtons() {
	// Process normal button draw states
	buttonManager.process();

	// Do not process inputs if a) nothing was pressed, or b) currently transitioning
	if (!InputController::getInstance()->isTapEndAvailable()) {
		return;
	}

	const std::tuple<Vec2, Vec2> tapData = InputController::getInstance()->getTapEndLoc();

	// Pause menu
	if (pauseMenu->manageButtons(tapData)) {
		isBackToMainMenu = true;
	}

	if (playerID == 0) {
		if (winScreen->isVisible()) {
			if (winScreen->tappedNext(tapData)) {
				lastButtonPressed = NextLevel;
			}
		} else if (lossScreen->isVisible()) {
			// Is this loss?
			if (ButtonManager::tappedButton(restartBtn, tapData)) {
				lastButtonPressed = Restart;
			}
		}
	}
}

void GameGraphRoot::setSegHealthWarning(int alpha) {
	for (int i = 0; i < globals::VISIBLE_SEGS; i++) {
		const std::shared_ptr<cugl::PolygonNode> segment = dynamic_pointer_cast<cugl::PolygonNode>(
			shipSegsNode->getChildByTag(static_cast<unsigned int>(i + 1)));
		const std::shared_ptr<cugl::PolygonNode> segRed = dynamic_pointer_cast<cugl::PolygonNode>(
			segment->getChild(static_cast<unsigned int>(1)));
		segRed->setColor(Color4(globals::MAX_BYTE, globals::MAX_BYTE, globals::MAX_BYTE, alpha));
	}
}

void GameGraphRoot::doTeleportAnimation() {
	StabilizerModel& stabilizer = ship->getStabilizer();

	if (stabilizer.getState() == StabilizerModel::StabilizerState::Fail && !prevIsStabilizerFail) {
		currentTeleportationFrame = 1;
	}
	prevIsStabilizerFail = stabilizer.getState() == StabilizerModel::StabilizerState::Fail;

	if (currentTeleportationFrame != 0) {
		if (currentTeleportationFrame <= TELEPORT_FRAMECUTOFF_FIRST) {
			// Nothing needs to happen here but this branch is left in to keep the logic of the next
			// few more clear
		} else if (currentTeleportationFrame <= TELEPORT_FRAMECUTOFF_SECOND) {
			blackoutOverlay->setColor(Tween::fade(
				Tween::linear(0, 1, currentTeleportationFrame - TELEPORT_FRAMECUTOFF_FIRST,
							  TELEPORT_FRAMECUTOFF_SECOND - TELEPORT_FRAMECUTOFF_FIRST)));
		} else {
			if (currentTeleportationFrame == TELEPORT_FRAMECUTOFF_SECOND + 1) {
				// Teleport models
				const auto& playerDonut =
					ship->getDonuts().at(*MagicInternetBox::getInstance().getPlayerID());
				playerDonut->teleport();
				stabilizer.reset();
			} else if (currentTeleportationFrame == TELEPORT_FRAMECUTOFF_SECOND + 2) {
				CustomNode::recomputeAll();
			}
			blackoutOverlay->setColor(Tween::fade(
				Tween::linear(1, 0, currentTeleportationFrame - TELEPORT_FRAMECUTOFF_SECOND,
							  TELEPORT_FRAMECUTOFF_THIRD - TELEPORT_FRAMECUTOFF_SECOND)));
		}
		currentTeleportationFrame++;
		if (currentTeleportationFrame > TELEPORT_FRAMECUTOFF_THIRD) {
			currentTeleportationFrame = 0;
		}
	}
}
/**
 * Returns an informative string for the timer
 *
 * This function is for writing the current donut position to the HUD.
 *
 * @param coords The current donut coordinates
 *
 * @return an informative string for the position
 */
std::string GameGraphRoot::timerText() {
	stringstream ss;
	const int time = static_cast<int>(trunc(ship->timeLeftInTimer));
	if (time > SEC_IN_MIN - 1) {
		if (time % SEC_IN_MIN < TEN_SECONDS) {
			ss << "0" << time / SEC_IN_MIN << ":0" << time % SEC_IN_MIN;
		} else {
			ss << "0" << time / SEC_IN_MIN << ":" << time % SEC_IN_MIN;
		}
	} else {
		if (time < TEN_SECONDS) {
			ss << "00:0" << time;
		} else {
			ss << "00:" << time;
		}
	}

	return ss.str();
}
