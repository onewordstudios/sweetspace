#include "GameGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Globals.h"

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

/** Loop range of the background image */
constexpr int BG_SCROLL_LIMIT = 256;

/** Parallax speed of background image */
constexpr float BG_SCROLL_SPEED = 0.5;

/** Animation cycle length of ellipses */
constexpr int MAX_ELLIPSES_FRAMES = 180;

/** Presumable number of frames per second */
constexpr int FRAMES_PER_SECOND = 60;

/** Animation cycle length of ship red flash */
constexpr int MAX_HEALTH_WARNING_FRAMES = 150;

/** Maximum alpha value for health warning overlay */
constexpr int MAX_HEALTH_WARNING_ALPHA = 100;

/** Value of ship health that triggers flashing */
constexpr int HEALTH_WARNING_THRESHOLD = 4;

/** Max value of a color4 channel */
constexpr int COLOR_CHANNEL_MAX = 255;

/** Size of ship segment label */
constexpr int SEG_LABEL_SIZE = 100;

/** Y position of ship segment label */
constexpr int SEG_LABEL_Y = 1113;

/** Maximum number of health labels */
constexpr int MAX_HEALTH_LABELS = 10;

/** Percentage of ship health to start showing yellow */
constexpr float SHIP_HEALTH_YELLOW_CUTOFF = 0.8f;

/** Percentage of ship health to start showing red */
constexpr float SHIP_HEALTH_RED_CUTOFF = 0.35f;

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
	shipSegsNode = assets->get<Node>("game_field_near_shipsegments");
	doorsNode = assets->get<Node>("game_field_near_doors");
	externalDonutsNode = assets->get<Node>("game_field_near_externaldonuts");
	healthNode = dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_health"));
	coordHUD = std::dynamic_pointer_cast<Label>(assets->get<Node>("game_hud"));
	shipOverlay =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_near_shipoverlay"));
	shipOverlay->setColor(Color4::CLEAR);
	currentHealthWarningFrame = 0;
	moveTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_moveTutorial"));
	moveTutorial->setVisible(false);
	if (ship->getLevelNum() == 0) {
		moveTutorial->setVisible(true);
	}
	healthTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_healthTutorial"));
	healthTutorial->setVisible(false);
	if (ship->getLevelNum() == 0) {
		healthTutorial->setVisible(true);
	}
	rollTutorial =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_rollTutorial"));
	rollTutorial->setVisible(false);
	tutorialNode = assets->get<Node>("game_field_near_tutorial");
	buttonsNode = assets->get<Node>("game_field_near_button");
	std::shared_ptr<Texture> image = assets->get<Texture>("health_green");
	healthNode->setTexture(image);
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

	// Initialize Ship Segments
	leftMostSeg = 0;
	rightMostSeg = globals::VISIBLE_SEGS - 1;
	std::shared_ptr<Texture> seg0 = assets->get<Texture>("shipseg0");
	std::shared_ptr<Texture> seg1 = assets->get<Texture>("shipseg1");
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
			tempDonutNode->setVisible(false);
		} else {
			std::shared_ptr<ExternalDonutNode> newDonutNode = ExternalDonutNode::alloc(
				donutModel, playerModel, ship->getSize(), bodyTexture, faceIdle, faceDizzy, faceWork);
			externalDonutsNode->addChild(newDonutNode);

			Vec2 donutPos = Vec2(sin(donutModel->getAngle() * (globals::RADIUS + DONUT_OFFSET)),
								 -cos(donutModel->getAngle()) * (globals::RADIUS + DONUT_OFFSET));
			newDonutNode->setPosition(donutPos);
		}
	}

	// Initialize Breaches
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at((unsigned long)i);
		string breachColor = PLAYER_COLOR.at((unsigned long)ship->getDonuts()
												 .at((unsigned long)breachModel->getPlayer())
												 ->getColorId());
		std::shared_ptr<cugl::Texture> filmstrip = assets->get<Texture>("breach_filmstrip");
		std::shared_ptr<cugl::Texture> pattern = assets->get<Texture>("breach_" + breachColor);
		cugl::Color4 color = BREACH_COLOR.at((unsigned long)ship->getDonuts()
												 .at((unsigned long)breachModel->getPlayer())
												 ->getColorId());
		std::shared_ptr<BreachNode> breachNode =
			BreachNode::alloc(breachModel, playerModel, ship->getSize(), filmstrip, pattern, color);
		breachNode->setTag((unsigned int)(i + 1));

		// Add the breach node
		breachesNode->addChild(breachNode);
		if (ship->getLevelNum() == 0) {
			std::shared_ptr<Texture> image = assets->get<Texture>("fix_breach_tutorial0");
			std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			tutorial->setBreachNode(breachNode);
			tutorialNode->addChild(tutorial);
		}
	}

	// Initialize Doors
	for (int i = 0; i < ship->getDoors().size(); i++) {
		std::shared_ptr<DoorModel> doorModel = ship->getDoors().at((unsigned long)i);
		std::shared_ptr<Texture> image = assets->get<Texture>("door");
		std::shared_ptr<DoorNode> doorNode = DoorNode::alloc(
			doorModel, playerModel, ship->getSize(), image, 1, DOOR_FRAMES, DOOR_FRAMES);
		doorsNode->addChildWithTag(doorNode, i + 1);
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

	if (ship->getLevelNum() == 1) {
		tutorialNode->removeAllChildren();
		for (int i = 0; i < doorsNode->getChildCount(); i++) {
			std::shared_ptr<Texture> image = assets->get<Texture>("door_tutorial");
			std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			shared_ptr<DoorNode> doorNode =
				dynamic_pointer_cast<DoorNode>(doorsNode->getChildByTag((unsigned int)(i + 1)));
			tutorial->setDoorNode(doorNode);
			tutorialNode->addChildWithTag(tutorial, i + 1);
		}
	} else if (ship->getLevelNum() == 2) {
		tutorialNode->removeAllChildren();
		for (int i = 0; i < buttonsNode->getChildCount(); i++) {
			std::shared_ptr<Texture> image = assets->get<Texture>("engine_tutorial0");
			std::shared_ptr<TutorialNode> tutorial = TutorialNode::alloc(image);
			shared_ptr<ButtonNode> buttonNode =
				dynamic_pointer_cast<ButtonNode>(buttonsNode->getChildByTag((unsigned int)(i + 1)));
			tutorial->setButtonNode(buttonNode);
			tutorialNode->addChildWithTag(tutorial, i + 1);
		}
	} else if (ship->getLevelNum() == 3) {
		tutorialNode->removeAllChildren();
	}

	// Overlay Components
	// Initialize Reconnect Overlay
	reconnectOverlay = assets->get<Node>("game_overlay_reconnect");
	reconnectE2 =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_reconnect_ellipsis2"));
	reconnectE3 =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("game_overlay_reconnect_ellipsis3"));

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
	levelsBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("game_overlay_loss_levelsBtn"));

	// Initialize Win Screen Componenets
	winScreen = assets->get<Node>("game_overlay_win");
	nextBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("game_overlay_win_nextBtn"));

	lossScreen->setVisible(false);
	winScreen->setVisible(false);
	nearSpace->setVisible(true);
	healthNode->setVisible(true);

	lastButtonPressed = None;

	// Register Regular Buttons
	buttonManager.registerButton(restartBtn);
	buttonManager.registerButton(levelsBtn);
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
		externalDonutsNode->removeAllChildren();
		externalDonutsNode = nullptr;

		challengePanelHanger = nullptr;
		challengePanel = nullptr;
		challengePanelText = nullptr;
		challengePanelArrows.clear();
		healthNode = nullptr;

		reconnectOverlay = nullptr;
		reconnectE2 = nullptr;
		reconnectE3 = nullptr;

		pauseBtn = nullptr;
		pauseScreen = nullptr;
		musicBtn = nullptr;
		soundBtn = nullptr;
		leaveBtn = nullptr;
		needle = nullptr;

		shipOverlay = nullptr;

		lossScreen = nullptr;
		restartBtn = nullptr;
		levelsBtn = nullptr;

		buttonsNode->removeAllChildren();
		buttonsNode = nullptr;

		winScreen = nullptr;
		nextBtn = nullptr;

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
	coordHUD->setText(positionText());

	// State Check for Drawing
	switch (status) {
		case Normal:
			// Hide Unnecessary Overlays
			lossScreen->setVisible(false);
			winScreen->setVisible(false);
			reconnectOverlay->setVisible(false);
			break;
		case Loss:
			// Show loss screen
			lossScreen->setVisible(true);
			break;
		case Win:
			// Show Win Screen
			winScreen->setVisible(true);
			nearSpace->setVisible(false);
			healthNode->setVisible(false);
			rollTutorial->setVisible(false);
			moveTutorial->setVisible(false);
			break;
		case Reconnecting:
			// Still Reconnecting
			reconnectOverlay->setVisible(true);
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
			break;
		default:
			CULog("ERROR: Uncaught DrawingStatus Value Occurred");
	}

	// Button Checks for Special Case Buttons
	processButtons();

	if (ship->getHealth() < 1) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_empty");
		healthNode->setTexture(image);
	} else if (ship->getHealth() < ship->getInitHealth() * SHIP_HEALTH_RED_CUTOFF) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_red");
		healthNode->setTexture(image);
	} else if (ship->getHealth() < ship->getInitHealth() * SHIP_HEALTH_YELLOW_CUTOFF) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_yellow");
		healthNode->setTexture(image);
	}

	if (ship->getLevelNum() == 0) {
		if (trunc(ship->timer) == 10) {
			healthTutorial->setVisible(false);
		} else if (trunc(ship->timer) == 15) {
			moveTutorial->setVisible(false);
		}
	} else if (ship->getLevelNum() == 3) {
		rollTutorial->setVisible(true);
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
		std::shared_ptr<cugl::Label> segLabel = dynamic_pointer_cast<cugl::Label>(
			segment->getChild(static_cast<unsigned int>(segment->getChildCount() - 1)));
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
			breachNode->getShapeNode()->setColor(color);
			breachNode->resetAnimation();
			string breachColor = PLAYER_COLOR.at((unsigned long)ship->getDonuts()
													 .at((unsigned long)breachModel->getPlayer())
													 ->getColorId());
			std::shared_ptr<Texture> image = assets->get<Texture>("breach_" + breachColor);
			breachNode->getPatternNode()->setTexture(image);
			breachNode->getPatternNode()->setColor(color);
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
			if (ship->getHealth() > HEALTH_WARNING_THRESHOLD) {
				currentHealthWarningFrame = 0;
				shipOverlay->setColor(Color4::CLEAR);
			} else {
				shipOverlay->setColor(
					Color4(COLOR_CHANNEL_MAX, COLOR_CHANNEL_MAX, COLOR_CHANNEL_MAX,
						   MAX_HEALTH_WARNING_ALPHA / MAX_HEALTH_WARNING_FRAMES * 2));
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
			shipOverlay->setColor(
				Color4(COLOR_CHANNEL_MAX, COLOR_CHANNEL_MAX, COLOR_CHANNEL_MAX, alpha));
		}
	} else if (ship->getHealth() <= HEALTH_WARNING_THRESHOLD) {
		shipOverlay->setColor(Color4(COLOR_CHANNEL_MAX, COLOR_CHANNEL_MAX, COLOR_CHANNEL_MAX,
									 MAX_HEALTH_WARNING_ALPHA / MAX_HEALTH_WARNING_FRAMES * 2));
		currentHealthWarningFrame = 1;
	}
}

void GameGraphRoot::processButtons() {
	// Process normal button draw states
	buttonManager.process();

	// Do not process inputs if a) nothing was pressed, or b) currently transitioning
	if (!InputController::getInstance()->isTapEndAvailable()) {
		return;
	}

	// TODO: Process Buttons for Win/Loss Screens

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
	} else {
		if (winScreen->isVisible()) {
			if (buttonManager.tappedButton(nextBtn, tapData)) {
				lastButtonPressed = NextLevel;
			}
		} else if (lossScreen->isVisible()) {
			// Is this loss?
			if (buttonManager.tappedButton(restartBtn, tapData)) {
				lastButtonPressed = Restart;
			} else if (buttonManager.tappedButton(levelsBtn, tapData)) {
				isBackToMainMenu = true;
			}
		}
	}
}

void GameGraphRoot::setNeedlePercentage(float percentage) {
	needle->setAngle(-percentage * globals::TWO_PI * globals::NEEDLE_OFFSET);
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
	ss << "Time Left: " << trunc(ship->timer);
	return ss.str();
}
