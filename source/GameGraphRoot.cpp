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
	donutPos = donutNode->getPosition();
	coordHUD = std::dynamic_pointer_cast<Label>(assets->get<Node>("game_hud"));
	// Reconnect Overlay
	reconnectDim =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_reconnect_dim"));
	reconnectBg =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_reconnect_bg"));
	reconnectText =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_reconnect_text"));
	reconnectEllipses =
		dynamic_pointer_cast<cugl::PolygonNode>(assets->get<Node>("game_field_reconnect_ellipses"));

	// Initialize Players
	for (int i = 0; i < ship->getDonuts().size(); i++) {
		std::shared_ptr<DonutModel> donutModel = ship->getDonuts().at(i);
		string donutColor = playerColor.at(static_cast<unsigned long>(donutModel->getColorId()));
		std::shared_ptr<Texture> image = assets->get<Texture>("donut_" + donutColor);
		// Player node is handled separately
		if (i == playerID) {
			donutNode->setTexture(image);
		} else {
			std::shared_ptr<DonutNode> newDonutNode = DonutNode::allocWithTexture(image);
			newDonutNode->setModel(donutModel);
			newDonutNode->setScale(DONUT_SCALE);
			nearSpace->addChild(newDonutNode);

			Vec2 donutPos = Vec2((globals::RADIUS + DONUT_OFFSET) * sin(donutModel->getAngle()),
								 -(globals::RADIUS + DONUT_OFFSET) * cos(donutModel->getAngle()));
			newDonutNode->setPosition(donutPos);
		}
	}

	// Initialize Breaches
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at(i);
		string breachColor = playerColor.at(
			static_cast<unsigned long>(ship->getDonuts()
										   .at(static_cast<unsigned long>(breachModel->getPlayer()))
										   ->getColorId()));
		std::shared_ptr<Texture> image = assets->get<Texture>("breach_" + breachColor);
		std::shared_ptr<BreachNode> breachNode = BreachNode::allocWithTexture(image);
		breachNode->setModel(breachModel);
		breachNode->setTag(i + 1);
		breachNode->setScale(BREACH_SCALE);
		// Start position is off screen
		Vec2 breachPos = Vec2(0, 0);
		breachNode->setPosition(breachPos);
		// Add the breach node
		breachesNode->addChild(breachNode);
	}

	for (int i = 0; i < ship->getDoors().size(); i++) {
		std::shared_ptr<DoorModel> doorModel = ship->getDoors().at(i);
		std::shared_ptr<Texture> image = assets->get<Texture>("door");
		std::shared_ptr<DoorNode> doorNode = DoorNode::alloc(image, 1, 32, 32);
		doorNode->setModel(doorModel);
		doorNode->setFrame(0);
		doorNode->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
		doorNode->setScale(0.3f);
		nearSpace->addChild(doorNode);
	}

	for (int i = 0; i < 8; i++) {
		std::shared_ptr<Texture> image = assets->get<Texture>("health_glow");
		std::shared_ptr<HealthNode> healthNode = HealthNode::alloc(image, 1, 12);
		healthNode->setModel(ship);
		healthNode->setFrame(11);
		healthNode->setSection(i);
		healthNode->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
		healthNode->setScale(0.55f);
		nearSpace->addChild(healthNode);
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

	float angle = (float)(fmod(ship->getSize() - ship->getDonuts().at(playerID)->getAngle(), 360));

	// Reanchor the node at the center of the screen and rotate about center.
	Vec2 position = farSpace->getPosition();
	farSpace->setAnchor(Vec2::ANCHOR_CENTER);
	if (position.x == -256) {
		farSpace->setPositionX(0);
	} else {
		farSpace->setPosition(position - Vec2(0.5, 0)); // Reseting the anchor changes the position
	}

	// Rotate about center.
	nearSpace->setAngle(globals::PI_180 * angle);

	double radiusRatio = globals::RADIUS / (donutNode->getWidth() / 2.0);

	angle = (float)(donutNode->getAngle() -
					ship->getDonuts().at(playerID)->getVelocity() * globals::PI_180 * radiusRatio);
	donutNode->setAnchor(Vec2::ANCHOR_CENTER);
	donutNode->setAngle(angle);
	// Draw Jump Offset
	float donutNewY = donutPos.y + ship->getDonuts().at(playerID)->getJumpOffset() * screenHeight;
	donutNode->setPositionY(donutNewY);

	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breachModel = ship->getBreaches().at(i);
		if (breachModel->getHealth() > 0 && breachModel->getNeedSpriteUpdate()) {
			string breachColor = playerColor.at(static_cast<unsigned long>(
				ship->getDonuts()
					.at(static_cast<unsigned long>(breachModel->getPlayer()))
					->getColorId()));
			std::shared_ptr<Texture> image = assets->get<Texture>("breach_" + breachColor);
			shared_ptr<BreachNode> breachNode =
				dynamic_pointer_cast<BreachNode>(breachesNode->getChildByTag(i + 1));
			breachNode->setTexture(image);
			breachModel->setNeedSpriteUpdate(false);
		}
	}

	// Draw Client Reconnection Overlay
	switch (status) {
		case MagicInternetBox::GameEnded:
			// Insert Game Ended Screen
			break;
		case MagicInternetBox::Reconnecting:
			// Still Reconnecting
			reconnectDim->setVisible(true);
			reconnectBg->setVisible(true);
			reconnectText->setVisible(true);
			reconnectEllipses->setVisible(true);
			break;
		case MagicInternetBox::GameStart:
			reconnectDim->setVisible(false);
			reconnectBg->setVisible(false);
			reconnectText->setVisible(false);
			reconnectEllipses->setVisible(false);
			break;
		default:
			CULog("ERROR: Uncaught MatchmakingStatus Value Occurred");
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
	if (ship->timerEnded() && ship->getHealth() > 10) {
		ss << "You Win!";
	} else if (ship->timerEnded()) {
		ss << "You Lose.";
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
