#include "GameGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Globals.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** The scale of the donut textures. */
constexpr float DONUT_SCALE = 0.4f;

/** Offset of donut sprites from the radius of the ship */
constexpr int DONUT_OFFSET = 195;

/** Offset of donut sprites from the radius of the ship */
constexpr int CHALLENGE_OFFSET = 195;

/** The scale of the ship segments. */
constexpr float SEG_SCALE = 0.33f;

/** The scale of the doors. */
constexpr float DOOR_SCALE = 0.3f;

/** Number of animation frames of doors */
constexpr int DOOR_FRAMES = 32;

/** Loop range of the background image */
constexpr int BG_SCROLL_LIMIT = 256;

/** Parallax speed of background image */
constexpr float BG_SCROLL_SPEED = 0.5;

/** Number of health bar nodes */
constexpr int NUM_HEALTH_BAR = 8;

/** Number of health bar frames */
constexpr int HEALTH_BAR_FRAMES = 12;

/** Scaling factor of health nodes */
constexpr float HEALTH_NODE_SCALE = 0.55f;

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

	// Get the scene components.
	allSpace = assets->get<Node>("game_field");
	farSpace = assets->get<Node>("game_field_far");
	nearSpace = assets->get<Node>("game_field_near");
	donutNode = dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_player1"));
	breachesNode = assets->get<Node>("game_field_near_breaches");
	shipSegsNode = assets->get<Node>("game_field_near_shipsegments");
	doorsNode = assets->get<Node>("game_field_near_doors");
	externalDonutsNode = assets->get<Node>("game_field_near_externaldonuts");
	donutPos = donutNode->getPosition();
	healthNode = dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_health"));
	coordHUD = std::dynamic_pointer_cast<Label>(assets->get<Node>("game_hud"));
	shipOverlay =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_near_shipoverlay"));
	shipOverlay->setColor(Color4::CLEAR);
	currentHealthWarningFrame = 0;
	buttonNode = assets->get<Node>("game_field_near_button");

	challengePanelHanger = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_challengePanelParent_challengePanelHanger"));
	challengePanelHanger->setVisible(false);
	challengePanel = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_challengePanelParent_challengePanel"));
	challengePanel->setVisible(false);
	challengePanelText = dynamic_pointer_cast<cugl::PolygonNode>(
		assets->get<Node>("game_field_challengePanelParent_challengePanelText"));
	challengePanelText->setVisible(false);
	reconnectOverlay = assets->get<Node>("game_field_reconnect");

	for (int i = 0; i < 10; i++) {
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
		segLabel->setPosition(segment->getTexture()->getWidth() / 2, SEG_LABEL_Y);
		segLabel->setForeground(SHIP_LABEL_COLOR);
		segment->addChild(segLabel);
		shipSegsNode->addChildWithTag(segment, (unsigned int)(i + 1));
	}

	// Initialize Players
	for (int i = 0; i < ship->getDonuts().size(); i++) {
		std::shared_ptr<DonutModel> donutModel = ship->getDonuts().at((unsigned long)i);
		string donutColor = playerColor.at((unsigned long)donutModel->getColorId());
		std::shared_ptr<Texture> image = assets->get<Texture>("donut_" + donutColor);
		// Player node is handled separately
		if (i == playerID) {
			donutNode->setTexture(image);
		} else {
			std::shared_ptr<DonutNode> newDonutNode = DonutNode::allocWithTexture(image);
			newDonutNode->setModel(donutModel);
			newDonutNode->setScale(DONUT_SCALE);
			newDonutNode->setShipSize(ship->getSize());
			newDonutNode->setDonutModel(ship->getDonuts().at(playerID));
			externalDonutsNode->addChild(newDonutNode);

			Vec2 donutPos = Vec2(sin(donutModel->getAngle() * (globals::RADIUS + DONUT_OFFSET)),
								 -cos(donutModel->getAngle()) * (globals::RADIUS + DONUT_OFFSET));
			newDonutNode->setPosition(donutPos);
		}
	}

	// Initialize Breaches
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at((unsigned long)i);
		std::shared_ptr<BreachNode> breachNode = BreachNode::alloc();
		breachNode->setModel(breachModel);
		breachNode->setTag((unsigned int)(i + 1));
		breachNode->setScale(BREACH_SCALE);
		breachNode->setShipSize(ship->getSize());
		breachNode->setDonutModel(ship->getDonuts().at(playerID));
		breachNode->setPrevHealth(breachModel->getHealth());
		// Start position is off screen
		Vec2 breachPos = Vec2(0, 0);
		breachNode->setPosition(breachPos);
		// Add shape node
		cugl::Color4 color = breachColor.at((unsigned long)ship->getDonuts()
												.at((unsigned long)breachModel->getPlayer())
												->getColorId());
		std::shared_ptr<Texture> image = assets->get<Texture>("breach_filmstrip");
		std::shared_ptr<AnimationNode> shapeNode =
			AnimationNode::alloc(image, BreachNode::BREACH_H, BreachNode::BREACH_W);
		shapeNode->setColor(color);
		shapeNode->setAnchor(Vec2::ANCHOR_CENTER);
		shapeNode->setPosition(0, 0);
		breachNode->setShapeNode(shapeNode);
		breachNode->addChildWithName(shapeNode, "shape");
		// Add pattern node
		string breachColor = playerColor.at((unsigned long)ship->getDonuts()
												.at((unsigned long)breachModel->getPlayer())
												->getColorId());
		image = assets->get<Texture>("breach_" + breachColor);
		std::shared_ptr<PolygonNode> patternNode = PolygonNode::allocWithTexture(image);
		patternNode->setAnchor(Vec2::ANCHOR_CENTER);
		patternNode->setPosition(0, 0);
		breachNode->setPatternNode(patternNode);
		breachNode->addChildWithName(patternNode, "pattern");
		// Add the breach node
		breachNode->resetAnimation();
		breachesNode->addChild(breachNode);
	}

	// Initialize Doors
	for (int i = 0; i < ship->getDoors().size(); i++) {
		std::shared_ptr<DoorModel> doorModel = ship->getDoors().at((unsigned long)i);
		std::shared_ptr<Texture> image = assets->get<Texture>("door");
		std::shared_ptr<DoorNode> doorNode = DoorNode::alloc(image, 1, DOOR_FRAMES, DOOR_FRAMES);
		doorNode->setModel(doorModel);
		doorNode->setFrame(0);
		doorNode->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
		doorNode->setScale(DOOR_SCALE);
		doorNode->setShipSize(ship->getSize());
		doorNode->setDonutModel(ship->getDonuts().at(playerID));
		doorsNode->addChild(doorNode);
	}

	// Initialize Buttons
	for (int i = 0; i < ship->getButtons().size(); i++) {
		std::shared_ptr<ButtonModel> buttonModel = ship->getButtons().at((unsigned long)i);
		std::shared_ptr<Texture> image = assets->get<Texture>("challenge_btn_base_up");
		std::shared_ptr<Texture> buttonImage = assets->get<Texture>("challenge_btn_up");
		std::shared_ptr<ButtonNode> bNode = ButtonNode::alloc(image);
		std::shared_ptr<ButtonNode> subNode = ButtonNode::alloc(buttonImage);
		std::shared_ptr<cugl::Label> buttonLabel =
			std::dynamic_pointer_cast<Label>(assets->get<Node>("game_field_near_buttonText"));
		subNode->setModel(buttonModel);
		subNode->setScale(0.7);
		subNode->setDonutModel(ship->getDonuts().at(playerID));
		subNode->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
		subNode->setScale(DOOR_SCALE);
		subNode->setShipSize(ship->getSize());
		subNode->setButtonNodeType(1);
		subNode->setButtonBaseDown(assets->get<Texture>("challenge_btn_base_down"));
		subNode->setButtonBaseUp(assets->get<Texture>("challenge_btn_base_up"));
		subNode->setButtonDown(assets->get<Texture>("challenge_btn_down"));
		subNode->setButtonUp(assets->get<Texture>("challenge_btn_up"));
		bNode->setModel(buttonModel);
		bNode->setScale(0.7);
		bNode->setDonutModel(ship->getDonuts().at(playerID));
		bNode->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
		bNode->setScale(DOOR_SCALE);
		bNode->setShipSize(ship->getSize());
		bNode->setButtonNodeType(0);
		bNode->setButtonDown(assets->get<Texture>("challenge_btn_down"));
		bNode->setButtonUp(assets->get<Texture>("challenge_btn_up"));
		bNode->setButtonBaseDown(assets->get<Texture>("challenge_btn_base_down"));
		bNode->setButtonBaseUp(assets->get<Texture>("challenge_btn_base_up"));
		bNode->setButtonLabel(buttonLabel);
		buttonNode->addChild(subNode);
		buttonNode->addChild(bNode);
	}

	addChild(scene);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameGraphRoot::dispose() {
	if (_active) {
		removeAllChildren();
		allSpace = nullptr;
		farSpace = nullptr;
		nearSpace = nullptr;
		donutNode = nullptr;
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
	if (ship->getHealth() < 1) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_empty");
		healthNode->setTexture(image);
	} else if (ship->getHealth() < globals::INITIAL_SHIP_HEALTH * 0.5) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_red");
		healthNode->setTexture(image);
	} else if (ship->getHealth() < globals::INITIAL_SHIP_HEALTH * 0.8) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_yellow");
		healthNode->setTexture(image);
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
	if (std::abs(delta) > globals::SEG_SIZE / globals::PI_180) {
		delta = fmod(newPlayerAngle, globals::SEG_SIZE / globals::PI_180) -
				fmod(prevPlayerAngle, globals::SEG_SIZE / globals::PI_180);
	}
	nearSpace->setAngle(wrapAngle(nearSpace->getAngle() + delta));
	prevPlayerAngle = newPlayerAngle;

	double radiusRatio = (double)globals::RADIUS / (donutNode->getWidth() / 2);

	float angle = (float)(donutNode->getAngle() - ship->getDonuts().at(playerID)->getVelocity() *
													  globals::PI_180 * radiusRatio);
	donutNode->setAnchor(Vec2::ANCHOR_CENTER);
	donutNode->setAngle(angle);
	// Draw Jump Offset
	float donutNewY = donutPos.y + ship->getDonuts().at(playerID)->getJumpOffset() * screenHeight;
	donutNode->setPositionY(donutNewY);

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
			cugl::Color4 color = breachColor.at((unsigned long)ship->getDonuts()
													.at((unsigned long)breachModel->getPlayer())
													->getColorId());
			breachNode->getShapeNode()->setColor(color);
			breachNode->resetAnimation();
			string breachColor = playerColor.at((unsigned long)ship->getDonuts()
													.at((unsigned long)breachModel->getPlayer())
													->getColorId());
			std::shared_ptr<Texture> image = assets->get<Texture>("breach_" + breachColor);
			breachNode->getPatternNode()->setTexture(image);
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
				arrow->setAngle(180 * globals::PI_180);
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

	// Draw Client Reconnection Overlay
	switch (status) {
		case MagicInternetBox::GameEnded:
			// Insert Game Ended Screen
			break;
		case MagicInternetBox::Disconnected:
		case MagicInternetBox::ReconnectError:
		case MagicInternetBox::Reconnecting:
		case MagicInternetBox::ClientRoomInvalid:
			//// Still Reconnecting
			reconnectOverlay->setVisible(true);
			break;
		case MagicInternetBox::GameStart:
			reconnectOverlay->setVisible(false);
			break;
		default:
			CULog("ERROR: Uncaught MatchmakingStatus Value Occurred");
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
	if (ship->getHealth() < 1) {
		ss << "You Lose.";
	} else if (ship->timerEnded() && ship->getHealth() > 0) {
		ss << "You Win!";
	} else {
		ss << "Time Left: " << trunc(ship->timer);
	}
	return ss.str();
}

/**
 * Returns the donut node
 *
 */
std::shared_ptr<cugl::Node> GameGraphRoot::getDonutNode() { return donutNode; }
