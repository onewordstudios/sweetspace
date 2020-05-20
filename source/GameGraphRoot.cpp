#include "GameGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Globals.h"
#include "TutorialConstants.h"
#include "Tween.h"

using namespace cugl;
using namespace std;

// NOLINTNEXTLINE Simple 4-vectors are unlikely to throw an exception
const std::vector<Color4> GameGraphRoot::BREACH_COLOR{
	cugl::Color4(219, 197, 52), cugl::Color4(227, 100, 159), cugl::Color4(152, 95, 204),
	cugl::Color4(158, 212, 87), cugl::Color4(244, 150, 40),	 cugl::Color4(47, 206, 197)};

#pragma mark -
#pragma mark Level Layout

/** Offset of donut sprites from the radius of the ship */
constexpr int DONUT_OFFSET = 195;

/** The scale of the ship segments. */
constexpr float SEG_SCALE = 0.33f;

/** Number of animation frames of doors */
constexpr int DOOR_FRAMES = 32;

/** Number of animation rows of doors */
constexpr int DOOR_ROWS = 5;

/** Number of animation cols of doors */
constexpr int DOOR_COLS = 7;

/** Loop range of the background image */
constexpr int BG_SCROLL_LIMIT = 256;

/** Parallax speed of background image */
constexpr float BG_SCROLL_SPEED = 0.5;

/** Animation cycle length of ellipses */
constexpr int MAX_ELLIPSES_FRAMES = 180;

/** Presumable number of frames per second */
constexpr int FRAMES_PER_SECOND = 60;

/** Ratio of spin on reconnect donut */
constexpr float RECONNECT_SPIN_RATIO = 0.26f;

/** Milliseconds before connection timeout */
constexpr int CONNECTION_TIMEOUT = 15000;

/** Milliseconds in a second */
constexpr int MILLISECONDS_IN_SECONDS = 1000;

/** Animation cycle length of ship red flash */
constexpr int MAX_HEALTH_WARNING_FRAMES = 150;

/** Maximum alpha value for health warning overlay */
constexpr int MAX_HEALTH_WARNING_ALPHA = 100;

/** Size of ship segment label */
constexpr int SEG_LABEL_SIZE = 100;

/** Y position of ship segment label */
constexpr int SEG_LABEL_Y = 1113;

/** Maximum number of health labels */
constexpr int MAX_HEALTH_LABELS = 10;

/** Percentage of ship health to start showing decrease in green */
constexpr float SHIP_HEALTH_HIGH_GREEN_CUTOFF = 0.9f;

/** Percentage of ship health to start showing more decrease in green */
constexpr float SHIP_HEALTH_LOW_GREEN_CUTOFF = 0.7f;

/** Percentage of ship health to start showing yellow */
constexpr float SHIP_HEALTH_YELLOW_CUTOFF = 0.5f;

/** Percentage of ship health to start showing red */
constexpr float SHIP_HEALTH_RED_CUTOFF = 0.3f;

/** Time to stop showing health tutorial */
constexpr int HEALTH_TUTORIAL_CUTOFF = 10;

/** Time to stop showing move tutorial */
constexpr int MOVE_TUTORIAL_CUTOFF = 5;

/** Time to show breach tutorial */
constexpr int BREACH_TUTORIAL_CUTOFF = 10;

/** Time stop showing timer */
constexpr int TIMER_TUTORIAL_CUTOFF = 18;

/** Red health position */
constexpr float RED_POS_X = -100;

/** Red health position */
constexpr float RED_POS_Y = 176;

/** Yellow health position */
constexpr float YELLOW_POS_X = -120;

/** Yellow health position */
constexpr float YELLOW_POS_Y = 118;

/** Low green health position */
constexpr float LOW_GREEN_POS_X = -100;

/** Low green health position */
constexpr float LOW_GREEN_POS_Y = 60;

/** High green health position */
constexpr float HIGH_GREEN_POS_X = -75;

/** High green health position */
constexpr float HIGH_GREEN_POS_Y = 30;

/** Red health angle */
constexpr float RED_ANGLE = 240 * globals::PI_180;

/** Yellow health angle */
constexpr float YELLOW_ANGLE = 270 * globals::PI_180;

/** Low green health angle */
constexpr float LOW_GREEN_ANGLE = 300 * globals::PI_180;

/** High green health angle */
constexpr float HIGH_GREEN_ANGLE = 320 * globals::PI_180;

/** Tutorial asset scale */
constexpr float TUTORIAL_SCALE = 0.4f;

/** Timer offset */
constexpr float TIMER_OFFSET = 20;

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
bool GameGraphRoot::init(const std::shared_ptr<cugl::AssetManager>& assets,
						 std::shared_ptr<ShipModel> ship, unsigned int playerID) {
	this->playerID = playerID;
	this->ship = ship;
	this->prevPlayerAngle = ship->getDonuts().at(playerID)->getAngle();
	isBackToMainMenu = false;
	status = Normal;
	currentEllipsesFrame = 0;

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
	shipSegsNode = assets->get<Node>("game_field_near_shipsegments");
	doorsNode = assets->get<Node>("game_field_near_doors");
	unopsNode = assets->get<Node>("game_field_near_unops");
	externalDonutsNode = assets->get<Node>("game_field_near_externaldonuts");
	healthNode =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_healthBase"));
	healthNodeOverlay =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_health"));
	healthNodeOverlay->setVisible(true);
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
		communicateTutorial->setVisible(true);
		timerTutorial->setVisible(true);
	}
	rollTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_rollTutorial"));
	rollTutorial->setVisible(false);

	tutorialNode = assets->get<Node>("game_field_near_tutorial");
	buttonsNode = assets->get<Node>("game_field_near_button");
	nearSpace->setAngle(0.0f);
	// Initialize Roll Challenge
	challengePanelHanger = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_challengePanelParent_challengePanelHanger"));
	challengePanelHanger->setVisible(false);
	challengePanel = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_challengePanelParent_challengePanel"));
	challengePanel->setVisible(false);
	challengePanelText = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_challengePanelParent_challengePanelText"));
	challengePanelText->setVisible(false);

	for (int i = 0; i < MAX_HEALTH_LABELS; i++) {
		std::string s = std::to_string(i + 1);
		std::shared_ptr<cugl::PolygonNode> arrow = dynamic_pointer_cast<cugl::PolygonNode>(
			assets->get<Node>("game_field_challengePanelParent_challengePanelArrow" + s));
		challengePanelArrows.push_back(arrow);
	}

	stabilizerFailText = dynamic_pointer_cast<cugl::Label>(
		assets->get<Node>("game_field_challengePanelParent_challengePanelFailLabel"));
	stabilizerFailText->setVisible(false);
	stabilizerFailPanel = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_challengePanelParent_challengePanelFailPanel"));
	stabilizerFailPanel->setVisible(false);
	blackoutOverlay =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_blackoutOverlay"));
	blackoutOverlay->setColor(Tween::fade(0));
	currentTeleportationFrame = 0;
	prevIsStabilizerFail = false;

	// Initialize Ship Segments
	leftMostSeg = 0;
	rightMostSeg = globals::VISIBLE_SEGS - 1;
	std::shared_ptr<Texture> seg0 = assets->get<Texture>("shipseg0");
	std::shared_ptr<Texture> seg1 = assets->get<Texture>("shipseg1");
	std::shared_ptr<Texture> segRed = assets->get<Texture>("shipsegred");
	for (int i = 0; i < globals::VISIBLE_SEGS; i++) {
		std::shared_ptr<PolygonNode> segment =
			cugl::PolygonNode::allocWithTexture(i % 2 == 0 ? seg0 : seg1);
		segment->setAnchor(Vec2::ANCHOR_TOP_CENTER);
		segment->setScale(SEG_SCALE);
		segment->setPosition(Vec2(0, 0));
		segment->setAngle(globals::SEG_SIZE * ((float)i - 2));
		std::shared_ptr<cugl::Label> segLabel = cugl::Label::alloc(
			Size(SEG_LABEL_SIZE, SEG_LABEL_SIZE), assets->get<Font>("mont_black_italic_big"));
		segLabel->setAnchor(Vec2::ANCHOR_CENTER);
		segLabel->setHorizontalAlignment(Label::HAlign::CENTER);
		segLabel->setPosition((float)segment->getTexture()->getWidth() / 2, SEG_LABEL_Y);
		segLabel->setForeground(SHIP_LABEL_COLOR);
		segment->addChild(segLabel);
		std::shared_ptr<PolygonNode> segmentRed = cugl::PolygonNode::allocWithTexture(segRed);
		segmentRed->setColor(Color4::CLEAR);
		segment->addChild(segmentRed);
		shipSegsNode->addChildWithTag(segment, (unsigned int)(i + 1));
	}

	std::shared_ptr<DonutModel> playerModel = ship->getDonuts()[playerID];

	// Initialize Players
	std::shared_ptr<Texture> faceIdle = assets->get<Texture>("donut_face_idle");
	std::shared_ptr<Texture> faceDizzy = assets->get<Texture>("donut_face_dizzy");
	std::shared_ptr<Texture> faceWork = assets->get<Texture>("donut_face_work");
	for (int i = 0; i < ship->getDonuts().size(); i++) {
		std::shared_ptr<DonutModel> donutModel = ship->getDonuts().at((unsigned long)i);
		string donutColor = PLAYER_COLOR.at((unsigned long)donutModel->getColorId());
		std::shared_ptr<Texture> bodyTexture = assets->get<Texture>("donut_" + donutColor);
		// Player node is handled separately
		if (i == playerID) {
			donutNode = PlayerDonutNode::alloc(playerModel, screenHeight, bodyTexture, faceIdle,
											   faceDizzy, faceWork, tempDonutNode->getPosition());
			allSpace->addChild(donutNode);
			tempDonutNode = nullptr;
		} else {
			std::shared_ptr<ExternalDonutNode> newDonutNode =
				ExternalDonutNode::alloc(donutModel, playerModel, ship->getSize(), bodyTexture,
										 faceIdle, faceDizzy, faceWork);
			externalDonutsNode->addChild(newDonutNode);

			Vec2 donutPos = Vec2(sin(donutModel->getAngle() * (globals::RADIUS + DONUT_OFFSET)),
								 -cos(donutModel->getAngle()) * (globals::RADIUS + DONUT_OFFSET));
			newDonutNode->setPosition(donutPos);
		}
	}

	// Initialize Breaches
	std::shared_ptr<cugl::Texture> breachFilmstrip = assets->get<Texture>("breach_filmstrip");
	std::shared_ptr<cugl::Texture> breachSparkle = assets->get<Texture>("breach_sparkle");
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at((unsigned long)i);
		string breachColor = PLAYER_COLOR.at((unsigned long)ship->getDonuts()
												 .at((unsigned long)breachModel->getPlayer())
												 ->getColorId());
		std::shared_ptr<cugl::Texture> pattern = assets->get<Texture>("breach_" + breachColor);
		cugl::Color4 color = BREACH_COLOR.at((unsigned long)ship->getDonuts()
												 .at((unsigned long)breachModel->getPlayer())
												 ->getColorId());
		// Initialize sparkle node
		std::shared_ptr<SparkleNode> sparkleNode =
			SparkleNode::alloc(playerModel, ship->getSize(), breachSparkle, color);
		breachSparklesNode->addChildWithTag(sparkleNode, (unsigned int)(i + 1));

		std::shared_ptr<BreachNode> breachNode =
			BreachNode::alloc(breachModel, playerModel, ship->getSize(), breachFilmstrip, pattern,
							  color, sparkleNode);
		breachNode->setTag((unsigned int)(i + 1));

		// Add the breach node
		breachesNode->addChild(breachNode);
		if (ship->getLevelNum() == tutorial::BREACH_LEVEL) {
			std::shared_ptr<Texture> image = assets->get<Texture>("jump_tutorial0");
			std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			tutorial->setScale(TUTORIAL_SCALE);
			tutorial->setBreachNode(breachNode);
			tutorialNode->addChildWithTag(tutorial, (unsigned int)(i + 1));
		}
	}

	// Initialize Doors
	for (int i = 0; i < ship->getDoors().size(); i++) {
		std::shared_ptr<DoorModel> doorModel = ship->getDoors().at((unsigned long)i);
		std::shared_ptr<Texture> image = assets->get<Texture>("door");
		std::shared_ptr<DoorNode> doorNode = DoorNode::alloc(
			doorModel, playerModel, ship->getSize(), image, DOOR_ROWS, DOOR_COLS, DOOR_FRAMES);
		doorsNode->addChildWithTag(doorNode, i + 1);
	}

	// Initialize unopenable doors
	for (int i = 0; i < ship->getUnopenable().size(); i++) {
		std::shared_ptr<Unopenable> unopModel = ship->getUnopenable().at((unsigned long)i);
		std::shared_ptr<Texture> image = assets->get<Texture>("unop");
		std::shared_ptr<UnopenableNode> unopNode =
			UnopenableNode::alloc(unopModel, playerModel, ship->getSize(), image);
		unopsNode->addChildWithTag(unopNode, i + 1);
	}

	// Initialize Buttons
	for (int i = 0; i < ship->getButtons().size(); i++) {
		std::shared_ptr<ButtonModel> buttonModel = ship->getButtons().at((unsigned long)i);
		std::shared_ptr<ButtonNode> buttonNode = ButtonNode::alloc(
			buttonModel, playerModel, ship->getSize(),
			assets->get<Texture>("challenge_btn_base_down"),
			assets->get<Texture>("challenge_btn_base_up"),
			assets->get<Texture>("challenge_btn_down"), assets->get<Texture>("challenge_btn_up"),
			assets->get<Font>("mont_black_italic_big"));
		buttonsNode->addChildWithTag(buttonNode, i + 1);
	}

	if (ship->getLevelNum() == tutorial::DOOR_LEVEL) {
		for (int i = 0; i < doorsNode->getChildCount(); i++) {
			std::shared_ptr<Texture> image = assets->get<Texture>("door_tutorial");
			std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			shared_ptr<DoorNode> doorNode =
				dynamic_pointer_cast<DoorNode>(doorsNode->getChildByTag((unsigned int)(i + 1)));
			tutorial->setDoorNode(doorNode);
			tutorial->setScale(TUTORIAL_SCALE);
			tutorialNode->addChildWithTag(tutorial, i + 1);
		}
	} else if (ship->getLevelNum() == tutorial::BUTTON_LEVEL) {
		for (int i = 0; i < buttonsNode->getChildCount(); i++) {
			std::shared_ptr<Texture> image = assets->get<Texture>("engine_tutorial");
			std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			shared_ptr<ButtonNode> buttonNode =
				dynamic_pointer_cast<ButtonNode>(buttonsNode->getChildByTag((unsigned int)(i + 1)));
			tutorial->setButtonNode(buttonNode);
			tutorial->setScale(TUTORIAL_SCALE);
			tutorialNode->addChildWithTag(tutorial, i + 1);
		}
	} else if (ship->getLevelNum() == tutorial::REAL_LEVELS.at(2)) {
		std::shared_ptr<Texture> image = assets->get<Texture>("timer_tutorial1");
		timerTutorial->setTexture(image);
		float posY = timerTutorial->getPositionY() - TIMER_OFFSET;
		float posX = timerTutorial->getPositionX();
		timerTutorial->setPosition(posX, posY);
	}

	// Overlay Components
	// Initialize Reconnect Overlay
	reconnectOverlay = assets->get<Node>("game_overlay_reconnect");
	reconnectE2 =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_reconnect_ellipsis2"));
	reconnectE3 =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_reconnect_ellipsis3"));
	std::shared_ptr<PolygonNode> tempReconnectDonut =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_overlay_reconnect_donut"));
	reconnectDonut = cugl::PolygonNode::allocWithTexture(
		assets->get<Texture>("donut_" + PLAYER_COLOR.at(playerID)));
	reconnectOverlay->addChild(reconnectDonut);
	reconnectDonut->setAnchor(Vec2::ANCHOR_CENTER);
	reconnectDonut->setPosition(tempReconnectDonut->getPosition());
	reconnectDonut->setScale(DonutNode::DONUT_SCALE);
	reconnectDonut->setVisible(true);
	tempReconnectDonut = nullptr;

	// Initialize Timeout Display
	timeoutDisplay = assets->get<Node>("game_overlay_timeout");
	timeoutCounter =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_timeout_countdown"));
	timeoutCurrent.mark();
	timeoutStart.mark();

	// Initialize Pause Screen Componenets
	pauseBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("game_pauseBtn"));
	pauseScreen = assets->get<Node>("game_pause");
	pauseBtn->setDown(false);
	pauseScreen->setVisible(false);
	musicBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("game_pause_musicBtn"));
	soundBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("game_pause_soundBtn"));
	leaveBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("game_pause_leaveBtn"));
	needle = assets->get<Node>("game_pause_dial_hand");

	// Initialize Loss Screen Componenets
	lossScreen = assets->get<Node>("game_overlay_loss");
	restartBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("game_overlay_loss_restartBtn"));
	lostWaitText =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_loss_waitText"));

	// Initialize Win Screen Componenets
	winScreen = assets->get<Node>("game_overlay_win");
	nextBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("game_overlay_win_nextBtn"));
	winWaitText = std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_win_waitText"));

	reconnectOverlay->setVisible(false);
	timeoutDisplay->setVisible(false);
	lossScreen->setVisible(false);
	winScreen->setVisible(false);
	nearSpace->setVisible(true);
	healthNode->setVisible(true);
	lostWaitText->setVisible(false);
	winWaitText->setVisible(false);
	nextBtn->setVisible(true);
	restartBtn->setVisible(true);

	lastButtonPressed = None;

	// Register Regular Buttons
	buttonManager.registerButton(restartBtn);
	buttonManager.registerButton(nextBtn);
	buttonManager.registerButton(leaveBtn);

	addChild(scene);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameGraphRoot::dispose() {
	if (_active) {
		removeAllChildren();
		buttonManager.clear();
		allSpace = nullptr;
		farSpace = nullptr;
		nearSpace = nullptr;
		donutNode->removeAllChildren();
		donutNode = nullptr;

		coordHUD = nullptr;
		breachesNode->removeAllChildren();
		breachesNode = nullptr;
		shipSegsNode->removeAllChildren();
		shipSegsNode = nullptr;
		doorsNode->removeAllChildren();
		doorsNode = nullptr;
		tutorialNode->removeAllChildren();
		tutorialNode = nullptr;
		unopsNode->removeAllChildren();
		unopsNode = nullptr;
		externalDonutsNode->removeAllChildren();
		externalDonutsNode = nullptr;

		challengePanelHanger = nullptr;
		challengePanel = nullptr;
		challengePanelText = nullptr;
		challengePanelArrows.clear();
		healthNode = nullptr;

		stabilizerFailText = nullptr;
		stabilizerFailPanel = nullptr;
		blackoutOverlay = nullptr;

		reconnectOverlay = nullptr;
		reconnectE2 = nullptr;
		reconnectE3 = nullptr;
		reconnectDonut = nullptr;

		timeoutDisplay = nullptr;
		timeoutCounter = nullptr;

		pauseBtn = nullptr;
		pauseScreen = nullptr;
		musicBtn = nullptr;
		soundBtn = nullptr;
		leaveBtn = nullptr;
		needle = nullptr;

		lossScreen = nullptr;
		restartBtn = nullptr;
		lostWaitText = nullptr;

		buttonsNode->removeAllChildren();
		buttonsNode = nullptr;

		winScreen = nullptr;
		nextBtn = nullptr;
		winWaitText = nullptr;

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
void GameGraphRoot::update(float timestep) {
	// "Drawing" code.  Move everything BUT the donut
	// Update the HUD
	for (int i = 0; i < ship->getButtons().size(); i++) {
		if (ship->getButtons().at(i)->getIsActive()) {
			coordHUD->setColor(cugl::Color4::RED);
			break;
		} else {
			coordHUD->setColor(cugl::Color4::WHITE);
		}
	}
	coordHUD->setText(positionText());

	// State Check for Drawing
	switch (status) {
		case Normal:
			// Hide Unnecessary Overlays
			lossScreen->setVisible(false);
			winScreen->setVisible(false);
			reconnectOverlay->setVisible(false);
			timeoutDisplay->setVisible(false);
			// Reset Timeout Counters to negative value
			timeoutCurrent.mark();
			timeoutStart.mark();
			break;
		case Loss:
			// Show loss screen
			lossScreen->setVisible(true);
			if (playerID != 0) {
				lostWaitText->setVisible(true);
				restartBtn->setVisible(false);
			}
			break;
		case Win:
			// Show Win Screen
			winScreen->setVisible(true);
			nearSpace->setVisible(false);
			healthNode->setVisible(false);
			rollTutorial->setVisible(false);
			moveTutorial->setVisible(false);
			communicateTutorial->setVisible(false);
			timerBorder->setVisible(false);
			healthNodeOverlay->setVisible(false);
			coordHUD->setVisible(false);
			if (playerID != 0) {
				winWaitText->setVisible(true);
				nextBtn->setVisible(false);
			}
			break;
		case Reconnecting:
			// Still Reconnecting, Animation Frames
			// Check for initial reconnection attempt
			if (timeoutCurrent.ellapsedNanos(timeoutStart) < 0) {
				timeoutStart.mark();
			}
			if (timeoutCurrent.ellapsedMillis(timeoutStart) <
				CONNECTION_TIMEOUT - 3 * MILLISECONDS_IN_SECONDS) {
				// Regular Reconnect Display
				timeoutDisplay->setVisible(false);
				timeoutCurrent.mark();
				reconnectOverlay->setVisible(true);
				reconnectDonut->setAngle(
					(float)(reconnectDonut->getAngle() - globals::PI_180 * RECONNECT_SPIN_RATIO));
				currentEllipsesFrame++;
				if (currentEllipsesFrame > MAX_ELLIPSES_FRAMES) {
					currentEllipsesFrame = 0;
				} else if (currentEllipsesFrame % MAX_ELLIPSES_FRAMES < FRAMES_PER_SECOND) {
					reconnectE2->setVisible(false);
					reconnectE3->setVisible(false);
				} else if (currentEllipsesFrame % MAX_ELLIPSES_FRAMES < 2 * FRAMES_PER_SECOND) {
					reconnectE2->setVisible(true);
				} else if (currentEllipsesFrame % MAX_ELLIPSES_FRAMES < 3 * FRAMES_PER_SECOND) {
					reconnectE3->setVisible(true);
				}
			} else {
				// 3 Second Timeout Counter back to lobby
				timeoutDisplay->setVisible(true);
				if (timeoutCurrent.ellapsedMillis(timeoutStart) <
					CONNECTION_TIMEOUT - 2 * MILLISECONDS_IN_SECONDS) {
					timeoutCounter->setText("3");
				} else if (timeoutCurrent.ellapsedMillis(timeoutStart) <
						   CONNECTION_TIMEOUT - MILLISECONDS_IN_SECONDS) {
					timeoutCounter->setText("2");
				} else if (timeoutCurrent.ellapsedMillis(timeoutStart) < CONNECTION_TIMEOUT) {
					timeoutCounter->setText("1");
				} else {
					isBackToMainMenu = true;
				}
				timeoutCurrent.mark();
			}
			break;
		default:
			CULog("ERROR: Uncaught DrawingStatus Value Occurred");
	}
	if (ship->getTimeless()) {
		coordHUD->setVisible(false);
		timeoutCounter->setVisible(false);
		timeoutDisplay->setVisible(false);
		timerBorder->setVisible(false);
	}

	// Button Checks for Special Case Buttons
	processButtons();

	if (ship->getHealth() < 1) {
		healthNodeOverlay->setVisible(false);
	} else if (ship->getHealth() < ship->getInitHealth() * SHIP_HEALTH_RED_CUTOFF) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_red");
		healthNodeOverlay->setTexture(image);
		healthNodeOverlay->setPosition(RED_POS_X, RED_POS_Y);
		healthNodeOverlay->setAngle(RED_ANGLE);
	} else if (ship->getHealth() < ship->getInitHealth() * SHIP_HEALTH_YELLOW_CUTOFF) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_yellow");
		healthNodeOverlay->setTexture(image);
		healthNodeOverlay->setPosition(YELLOW_POS_X, YELLOW_POS_Y);
		healthNodeOverlay->setAngle(YELLOW_ANGLE);
	} else if (ship->getHealth() < ship->getInitHealth() * SHIP_HEALTH_LOW_GREEN_CUTOFF) {
		healthNodeOverlay->setPosition(LOW_GREEN_POS_X, LOW_GREEN_POS_Y);
		healthNodeOverlay->setAngle(LOW_GREEN_ANGLE);
	} else if (ship->getHealth() < ship->getInitHealth() * SHIP_HEALTH_HIGH_GREEN_CUTOFF) {
		healthNodeOverlay->setPosition(HIGH_GREEN_POS_X, HIGH_GREEN_POS_Y);
		healthNodeOverlay->setAngle(HIGH_GREEN_ANGLE);
	} else {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_green");
		healthNodeOverlay->setTexture(image);
		healthNodeOverlay->setPosition(0, 0);
		healthNodeOverlay->setAngle(0);
	}

	if (ship->getLevelNum() == tutorial::BREACH_LEVEL) {
		if (trunc(ship->timeCtr) > BREACH_TUTORIAL_CUTOFF) {
			for (int i = 0; i < tutorialNode->getChildCount(); i++) {
				shared_ptr<TutorialNode> tutorial =
					dynamic_pointer_cast<TutorialNode>(tutorialNode->getChildByTag(i + 1));
				if (tutorial != nullptr) {
					tutorial->setVisible(true);
					if (tutorial->getPlayer() == playerID) {
						int breachHealth = tutorial->getBreachNode()->getModel()->getHealth();
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
		if (trunc(ship->timeCtr) == MOVE_TUTORIAL_CUTOFF) {
			moveTutorial->setVisible(false);
		}
	} else if (ship->getLevelNum() == tutorial::REAL_LEVELS.at(0)) {
		if (trunc(ship->timeCtr) == HEALTH_TUTORIAL_CUTOFF) {
			healthTutorial->setVisible(false);
			communicateTutorial->setVisible(false);
		} else if (trunc(ship->timeCtr) == MOVE_TUTORIAL_CUTOFF) {
			timerTutorial->setVisible(false);
			healthTutorial->setVisible(true);
			std::shared_ptr<Texture> image = assets->get<Texture>("communicate_tutorial1");
			communicateTutorial->setTexture(image);
		}
	} else if (ship->getLevelNum() == tutorial::REAL_LEVELS.at(2)) {
		if (trunc(ship->timeCtr) > TIMER_TUTORIAL_CUTOFF) {
			timerTutorial->setVisible(false);
		} else {
			timerTutorial->setVisible(true);
		}
	} else if (ship->getLevelNum() == tutorial::STABILIZER_LEVEL) {
		rollTutorial->setVisible(true);
		if (ship->getChallenge()) {
			std::shared_ptr<Texture> image = assets->get<Texture>("stabilizer_tutorial1");
			rollTutorial->setTexture(image);
		} else {
			std::shared_ptr<Texture> image = assets->get<Texture>("stabilizer_tutorial0");
			rollTutorial->setTexture(image);
		}
	}
	// Reanchor the node at the center of the screen and rotate about center.
	Vec2 position = farSpace->getPosition();
	farSpace->setAnchor(Vec2::ANCHOR_CENTER);
	if (position.x == -BG_SCROLL_LIMIT) {
		farSpace->setPositionX(0);
	} else {
		farSpace->setPosition(position -
							  Vec2(BG_SCROLL_SPEED, 0)); // Reseting the anchor changes the position
	}

	// Rotate nearSpace about center.
	float newPlayerAngle = ship->getDonuts().at(playerID)->getAngle();
	float delta = (prevPlayerAngle - newPlayerAngle) * globals::PI_180;
	delta = delta < -globals::PI
				? delta + ship->getSize() * globals::PI_180
				: delta > globals::PI ? delta - ship->getSize() * globals::PI_180 : delta;
	if (std::abs(delta) > globals::SEG_SIZE) {
		delta = fmod(prevPlayerAngle, globals::SEG_SIZE / globals::PI_180) -
				fmod(newPlayerAngle, globals::SEG_SIZE / globals::PI_180);
		delta = delta * globals::PI_180;
	}
	nearSpace->setAngle(wrapAngle(nearSpace->getAngle() + delta));
	prevPlayerAngle = newPlayerAngle;

	// Update ship segments
	std::shared_ptr<Texture> seg0 = assets->get<Texture>("shipseg0");
	std::shared_ptr<Texture> seg1 = assets->get<Texture>("shipseg1");
	std::shared_ptr<PolygonNode> segment;
	for (int i = 0; i < globals::VISIBLE_SEGS; i++) {
		segment = dynamic_pointer_cast<cugl::PolygonNode>(
			shipSegsNode->getChildByTag((unsigned int)(i + 1)));
		// If segments rotate too far left, move left-most segment to the right side
		if (i == rightMostSeg &&
			wrapAngle(nearSpace->getAngle() + segment->getAngle()) < globals::SEG_CUTOFF_ANGLE) {
			rightMostSeg = (i + 1) % globals::VISIBLE_SEGS;
			leftMostSeg = (i + 2) % globals::VISIBLE_SEGS;
			std::shared_ptr<PolygonNode> newRightSegment = dynamic_pointer_cast<cugl::PolygonNode>(
				shipSegsNode->getChildByTag((unsigned int)(rightMostSeg + 1)));
			newRightSegment->setAngle(wrapAngle(segment->getAngle() + globals::SEG_SIZE));
		} else if (i == leftMostSeg && wrapAngle(nearSpace->getAngle() + segment->getAngle()) >
										   globals::TWO_PI - globals::SEG_CUTOFF_ANGLE) {
			leftMostSeg = (i + globals::VISIBLE_SEGS - 1) % globals::VISIBLE_SEGS;
			rightMostSeg = (i + globals::VISIBLE_SEGS - 2) % globals::VISIBLE_SEGS;
			std::shared_ptr<PolygonNode> newLeftSegment = dynamic_pointer_cast<cugl::PolygonNode>(
				shipSegsNode->getChildByTag((unsigned int)(leftMostSeg + 1)));
			newLeftSegment->setAngle(wrapAngle(segment->getAngle() - globals::SEG_SIZE));
		}
		// Update text label of segment
		float relSegAngle = wrapAngle(segment->getAngle() + nearSpace->getAngle());
		relSegAngle = relSegAngle >= 0 ? relSegAngle : globals::TWO_PI + relSegAngle;
		relSegAngle = relSegAngle > globals::PI ? relSegAngle - globals::TWO_PI : relSegAngle;
		float segAngle = (float)(ship->getDonuts().at(playerID)->getAngle() * globals::PI_180 +
								 relSegAngle + SEG_SCALE * globals::PI_180);
		segAngle = fmod(segAngle, ship->getSize() * globals::PI_180);
		segAngle = segAngle < 0 ? segAngle + ship->getSize() * globals::PI_180 : segAngle;
		unsigned int segNum = (unsigned int)(segAngle / globals::SEG_SIZE);
		std::shared_ptr<cugl::Label> segLabel =
			dynamic_pointer_cast<cugl::Label>(segment->getChild(static_cast<unsigned int>(0)));
		segLabel->setText(std::to_string(segNum));
	}

	// Update breaches textures if recycled
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at((unsigned long)i);
		shared_ptr<BreachNode> breachNode =
			dynamic_pointer_cast<BreachNode>(breachesNode->getChildByTag((unsigned int)(i + 1)));
		if (!breachNode->getIsAnimatingShrink() && breachModel->getHealth() > 0 &&
			breachModel->getNeedSpriteUpdate()) {
			cugl::Color4 color = BREACH_COLOR.at((unsigned long)ship->getDonuts()
													 .at((unsigned long)breachModel->getPlayer())
													 ->getColorId());
			string breachColor = PLAYER_COLOR.at((unsigned long)ship->getDonuts()
													 .at((unsigned long)breachModel->getPlayer())
													 ->getColorId());
			std::shared_ptr<Texture> image = assets->get<Texture>("breach_" + breachColor);
			breachNode->resetAppearance(image, color);
			breachModel->setNeedSpriteUpdate(false);
		}
	}

	if (ship->getChallenge()) {
		challengePanelHanger->setVisible(true);
		challengePanel->setVisible(true);
		challengePanelText->setVisible(true);
		std::shared_ptr<Texture> image = assets->get<Texture>("panel_progress_1");
		for (int i = 0; i < challengePanelArrows.size(); i++) {
			std::shared_ptr<cugl::PolygonNode> arrow = challengePanelArrows.at(i);
			if (ship->getRollDir() == 0) {
				arrow->setAngle(globals::PI);
			}
			if (i < (ship->getChallengeProg())) {
				arrow->setTexture(image);
			}
			arrow->setVisible(true);
		}
	} else {
		challengePanelHanger->setVisible(false);
		challengePanel->setVisible(false);
		challengePanelText->setVisible(false);
		std::shared_ptr<Texture> image = assets->get<Texture>("panel_progress_0");
		for (int i = 0; i < challengePanelArrows.size(); i++) {
			challengePanelArrows.at(i)->setVisible(false);
			challengePanelArrows.at(i)->setTexture(image);
		}
	}

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

	std::tuple<Vec2, Vec2> tapData = InputController::getInstance()->getTapEndLoc();
	// Pause button
	if (buttonManager.tappedButton(pauseBtn, tapData)) {
		if (pauseBtn->isDown()) {
			// Close Pause Screen
			pauseBtn->setDown(false);
			pauseScreen->setVisible(false);
		} else {
			// Open Pause Screen
			pauseBtn->setDown(true);
			pauseScreen->setVisible(true);
		}
	} else if (pauseScreen->isVisible()) {
		// Mute Music Button
		if (buttonManager.tappedButton(musicBtn, tapData)) {
			if (musicBtn->isDown()) {
				musicBtn->setDown(false);
				AudioChannels::get()->resumeMusic();
			} else {
				musicBtn->setDown(true);
				AudioChannels::get()->pauseMusic();
			}
		}
		// Mute Sound Button
		else if (buttonManager.tappedButton(soundBtn, tapData)) {
			if (soundBtn->isDown()) {
				soundBtn->setDown(false);
				AudioChannels::get()->resumeAllEffects();
			} else {
				soundBtn->setDown(true);
				AudioChannels::get()->pauseAllEffects();
			}
		}
		// Leave Button
		else if (buttonManager.tappedButton(leaveBtn, tapData)) {
			isBackToMainMenu = true;
		}
	} else if (playerID == 0) {
		if (winScreen->isVisible()) {
			if (buttonManager.tappedButton(nextBtn, tapData)) {
				lastButtonPressed = NextLevel;
			}
		} else if (lossScreen->isVisible()) {
			// Is this loss?
			if (buttonManager.tappedButton(restartBtn, tapData)) {
				lastButtonPressed = Restart;
			}
		}
	}
}

void GameGraphRoot::setNeedlePercentage(float percentage) {
	needle->setAngle(-percentage * globals::TWO_PI * globals::NEEDLE_OFFSET);
}

void GameGraphRoot::setSegHealthWarning(int alpha) {
	for (int i = 0; i < globals::VISIBLE_SEGS; i++) {
		std::shared_ptr<cugl::PolygonNode> segment = dynamic_pointer_cast<cugl::PolygonNode>(
			shipSegsNode->getChildByTag((unsigned int)(i + 1)));
		std::shared_ptr<cugl::PolygonNode> segRed = dynamic_pointer_cast<cugl::PolygonNode>(
			segment->getChild(static_cast<unsigned int>(1)));
		segRed->setColor(Color4(globals::MAX_BYTE, globals::MAX_BYTE, globals::MAX_BYTE, alpha));
	}
}

void GameGraphRoot::doTeleportAnimation() {
	if (ship->getStabilizerStatus() == ShipModel::StabilizerStatus::FAILURE &&
		!prevIsStabilizerFail) {
		// Start teleportation animation
		currentTeleportationFrame = 1;
		ship->setStabilizerStatus(ShipModel::StabilizerStatus::ANIMATING);
	}
	if (currentTeleportationFrame != 0) {
		// Continue teleportation animation
		if (currentTeleportationFrame <= TELEPORT_FRAMECUTOFF_FIRST) {
			stabilizerFailPanel->setVisible(true);
			stabilizerFailText->setVisible(true);
		} else if (currentTeleportationFrame <= TELEPORT_FRAMECUTOFF_SECOND) {
			blackoutOverlay->setColor(Tween::fade(
				Tween::linear(0, 1, currentTeleportationFrame - TELEPORT_FRAMECUTOFF_FIRST,
							  TELEPORT_FRAMECUTOFF_SECOND - TELEPORT_FRAMECUTOFF_FIRST)));
		} else {
			if (currentTeleportationFrame == TELEPORT_FRAMECUTOFF_SECOND + 1) {
				// Teleport models
				for (int i = 0; i < ship->getDonuts().size(); i++) {
					std::shared_ptr<DonutModel> donutModel = ship->getDonuts().at((unsigned long)i);
					donutModel->teleport();
				}
				ship->setStabilizerStatus(ShipModel::StabilizerStatus::INACTIVE);
			}
			stabilizerFailPanel->setVisible(false);
			stabilizerFailText->setVisible(false);
			blackoutOverlay->setColor(Tween::fade(
				Tween::linear(1, 0, currentTeleportationFrame - TELEPORT_FRAMECUTOFF_SECOND,
							  TELEPORT_FRAMECUTOFF_THIRD - TELEPORT_FRAMECUTOFF_SECOND)));
		}
		currentTeleportationFrame += 1;
		if (currentTeleportationFrame > TELEPORT_FRAMECUTOFF_THIRD) currentTeleportationFrame = 0;
	}
	prevIsStabilizerFail = ship->getStabilizerStatus() == ShipModel::StabilizerStatus::FAILURE;
}
/**
 * Returns an informative string for the position
 *
 * This function is for writing the current donut position to the HUD.
 *
 * @param coords The current donut coordinates
 *
 * @return an informative string for the position
 */
std::string GameGraphRoot::positionText() {
	stringstream ss;
	if (trunc(ship->timer) > SEC_IN_MIN - 1) {
		if ((int)trunc(ship->timer) % SEC_IN_MIN < tenSeconds) {
			ss << "0" << (int)trunc(ship->timer) / SEC_IN_MIN << " : 0"
			   << (int)trunc(ship->timer) % SEC_IN_MIN;
		} else {
			ss << "0" << (int)trunc(ship->timer) / SEC_IN_MIN << " : "
			   << (int)trunc(ship->timer) % SEC_IN_MIN;
		}
	} else {
		if (trunc(ship->timer) < tenSeconds) {
			ss << "00 : 0" << trunc(ship->timer);
		} else {
			ss << "00 : " << trunc(ship->timer);
		}
	}

	return ss.str();
}
