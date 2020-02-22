//
//  SDGameScene.h
//  Ship Demo
//
//  This is the most important class in this demo.  This class manages the
//  gameplay for this demo.  It is a relativeluy simple class as we are not
//  worried about collisions.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design.  This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together.  However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  Author: Walker White
//  Version: 1/10/17
//
#include "SDGameScene.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

/** This is adjusted by screen aspect ratio to get the height */
constexpr unsigned int SCENE_WIDTH = 1024;

/** The parallax for each layer */
constexpr float PARALLAX_AMT = 0.1f;

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
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= SCENE_WIDTH / dimen.width;	 // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	} else if (!Scene::init(dimen)) {
		return false;
	}

	// Start up the input handler
	this->assets = assets;
	input.init();

	// Acquire the scene built by the asset loader and resize it the scene
	auto scene = assets->get<Node>("game");
	scene->setContentSize(dimen);
	scene->doLayout();	// Repositions the HUD

	// Get the scene components.
	allSpace = assets->get<Node>("game_field");
	farSpace = assets->get<Node>("game_field_far");
	nearSpace = assets->get<Node>("game_field_near");
	shipNode = std::dynamic_pointer_cast<AnimationNode>(assets->get<Node>("game_field_player"));
	coordHUD = std::dynamic_pointer_cast<Label>(assets->get<Node>("game_hud"));

	// Create the ship model
	Vec2 shipPos = shipNode->getPosition();
	shipModel = ShipModel::alloc(shipPos);
	shipModel->setSprite(shipNode);

	addChild(scene);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
	if (_active) {
		removeAllChildren();
		input.dispose();
		allSpace = nullptr;
		farSpace = nullptr;
		nearSpace = nullptr;
		shipNode = nullptr;
		shipModel = nullptr;
		_active = false;
	}
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void GameScene::reset() {
	// Reset the ships and input
	shipModel->reset();
	input.clear();

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
void GameScene::update(float timestep) {
	input.update(timestep);

	// Reset the game if necessary
	if (input.didReset()) {
		reset();
	}

	Vec2 thrust = input.getThrust();

	// Move the ship (MODEL ONLY)
	shipModel->setForward(thrust.y);
	shipModel->setTurning(thrust.x);
	shipModel->update(timestep);

	// "Drawing" code.  Move everything BUT the ship
	// Update the HUD
	coordHUD->setText(positionText(shipModel->getPosition()));

	Vec2 offset = shipModel->getPosition() - farSpace->getPosition();

	// Anchor points are in texture coordinates (0 to 1). Scale it.
	offset.x = offset.x / allSpace->getContentSize().width;
	offset.y = offset.y / allSpace->getContentSize().height;

	// Reanchor the node at the center of the screen and rotate about center.
	Vec2 position = farSpace->getPosition();
	farSpace->setAnchor(offset * PARALLAX_AMT + Vec2::ANCHOR_CENTER);
	farSpace->setPosition(position);  // Reseting the anchor changes the position
	farSpace->setAngle(shipModel->getAngle());

	// Reanchor the node at the center of the screen and rotate about center.
	position = nearSpace->getPosition();
	nearSpace->setAnchor(offset + Vec2::ANCHOR_CENTER);
	nearSpace->setPosition(position);  // Reseting the anchor changes the position
	nearSpace->setAngle(shipModel->getAngle());
}

/**
 * Returns an informative string for the position
 *
 * This function is for writing the current ship position to the HUD.
 *
 * @param coords The current ship coordinates
 *
 * @return an informative string for the position
 */
std::string GameScene::positionText(const cugl::Vec2& coords) {
	stringstream ss;
	constexpr unsigned int COORD_SHIFT = 10;
	ss << "Coords: (" << (int)coords.x / COORD_SHIFT << "," << (int)coords.y / COORD_SHIFT << ")";
	return ss.str();
}
