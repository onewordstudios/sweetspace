#include "GameGraphRoot.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>
#include <vector>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** This is adjusted by screen aspect ratio to get the height */
constexpr unsigned int SCENE_WIDTH = 1024;

/** 2 pi */
constexpr float TWO_PI = (float)(2 * M_PI);

/** Pi over 180 for converting between degrees and radians */
constexpr float PI_180 = (float)(M_PI / 180);

/** The scale of the breach textures. */
constexpr float BREACH_SCALE = 0.25;

/** The scale of the donut textures. */
constexpr float DONUT_SCALE = 0.5;

/** Offset of donut sprites from the radius of the ship */
constexpr int DONUT_OFFSET = 200;

/** The diameter of the ship. Also the x coordinate of the center of the ship */
constexpr unsigned int DIAMETER = 1280;

/** The radius of the ship. Also the y coordinate of the center of the ship */
constexpr unsigned int RADIUS = 550;

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
bool GameGraphRoot::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
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
	donutNode = assets->get<Node>("game_field_player");
	donutPos = donutNode->getPosition();
	coordHUD = std::dynamic_pointer_cast<Label>(assets->get<Node>("game_hud"));

	for (int i = 0; i < donuts.size(); i++) {
		// Player node is handled separately
		if (i == playerId) {
			continue;
		}

		std::shared_ptr<DonutModel> donutModel = donuts.at(i);
		std::shared_ptr<Texture> image = assets->get<Texture>("donut_friend");
		std::shared_ptr<DonutNode> donutNode = DonutNode::allocWithTexture(image);
		donutNode->setModel(donutModel);
		donutNode->setScale(DONUT_SCALE);
		nearSpace->addChild(donutNode);

		Vec2 donutPos = Vec2(DIAMETER + (RADIUS+DONUT_OFFSET) * sin(donutModel->getAngle()),
							 DIAMETER / 2.0f - (RADIUS+DONUT_OFFSET) * cos(donutModel->getAngle()));
//		if (donutModel->getAngle() < 0) {
//			donutPos = Vec2(0, 0);
//		}
		donutNode->setPosition(donutPos);
		// For moving donut off screen
		// Vec2 breachPos = Vec2(0, 0);
		// donutPos->setPosition(breachPos);
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

	float angle = TWO_PI - donuts.at(playerId)->getAngle();

	// Reanchor the node at the center of the screen and rotate about center.
	Vec2 position = farSpace->getPosition();
	farSpace->setAnchor(Vec2::ANCHOR_CENTER);
	if (position == Vec2(1280 - 256, 1920)) {
		farSpace->setPosition(Vec2(1280, 1920));
	} else {
		farSpace->setPosition(position - Vec2(0.5, 0)); // Reseting the anchor changes the position
	}

	// Rotate about center.
	nearSpace->setAngle(angle);

	double radiusRatio = RADIUS / (donutNode->getWidth() / 2.0);

	angle = donutNode->getAngle() - donuts.at(playerId)->getVelocity() * PI_180 * radiusRatio;
	donutNode->setAnchor(Vec2::ANCHOR_CENTER);
	donutNode->setAngle(angle);
	// Draw Jump Offset
	float donutNewY = donutPos.y + donuts.at(playerId)->getJumpOffset() * screenHeight;
	donutNode->setPositionY(donutNewY);

	for (int i = 0; i < breaches.size(); i++) {
		std::shared_ptr<BreachModel> breachModel = breaches.at(i);
		if (breachModel->getHealth() > 0) {
			if (breachModel->getSprite() == nullptr) {
				std::shared_ptr<Texture> image = assets->get<Texture>("planet2");
				std::shared_ptr<PolygonNode> breachNode = PolygonNode::allocWithTexture(image);
				breachModel->setSprite(breachNode);
				breachNode->setScale(BREACH_SCALE);
				// Create the donut model
				nearSpace->addChild(breachNode);
			}
			Vec2 breachPos = Vec2(DIAMETER + RADIUS * sin(breachModel->getAngle()),
								  DIAMETER / 2.0f - RADIUS * cos(breachModel->getAngle()));
			if (breachModel->getAngle() < 0) {
				breachPos = Vec2(0, 0);
			}
			breachModel->getSprite()->setScale(BREACH_SCALE * breachModel->getHealth() / 3.0f);
			breachModel->getSprite()->setPosition(breachPos);
		} else {
			Vec2 breachPos = Vec2(0, 0);
			breachModel->getSprite()->setPosition(breachPos);
		}
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
	ss << "Angle: (" << (float)donuts.at(playerId)->getAngle() / PI_180 << ")";
	return ss.str();
}

/**
 * Returns the donut node
 *
 */
std::shared_ptr<cugl::Node> GameGraphRoot::getDonutNode() { return donutNode; }
