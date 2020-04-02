#include "GameMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "PlayerDonutModel.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout
/** The maximum number of events on ship at any one time. This will probably need to scale with the
 * number of players*/
constexpr unsigned int MAX_EVENTS = 3;
/** The maximum number of doors on ship at any one time. This will probably need to scale with the
 * number of players*/
constexpr unsigned int MAX_DOORS = 1;
/** The Angle in degrees for fixing a breach*/
constexpr float EPSILON_ANGLE = 5.2f;
/** The Angle in degrees for which a collision occurs*/
constexpr float DOOR_WIDTH = 7.0f;
/** The Angle in degrees for which a breach donut collision occurs*/
constexpr float BREACH_WIDTH = 11.0f;
/** The Angle in degrees for which a door can be activated*/
constexpr float DOOR_ACTIVE_ANGLE = 15.0f;
/** Force to push back during collision */
constexpr float REBOUND_FORCE = -6;
/** Starting time for the round */
constexpr unsigned int TIME = 30;

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
bool GameMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	auto source = assets->get<Sound>("theme");
	AudioChannels::get()->playMusic(source, true, source->getVolume());
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	input.init();

	net = MagicInternetBox::getInstance();
	playerID = net->getPlayerID();

	float shipSize = 360; // TODO level size comes from level file
	ship = ShipModel::alloc(net->getNumPlayers(), MAX_EVENTS, MAX_DOORS, playerID, shipSize);
	gm.init(ship);

	donutModel = ship->getDonuts().at(static_cast<unsigned long>(playerID));
	ship->initTimer(TIME);

	// Scene graph setup
	sgRoot.init(assets, ship, playerID);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameMode::dispose() {
	input.dispose();
	gm.dispose();
	sgRoot.dispose();
	donutModel = nullptr;
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void GameMode::reset() {
	donutModel->reset();
	sgRoot.reset();
	input.clear();
	gm.clear();
}

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameMode::update(float timestep) {
	input.update(timestep);

	net->update(ship);

	if (!(ship->timerEnded())) {
		ship->updateTimer(timestep);
	}

	// Breach health depletion
	for (int i = 0; i < MAX_EVENTS; i++) {
		std::shared_ptr<BreachModel> breach = ship->getBreaches().at(i);
		if (breach == nullptr) {
			continue;
		}
		float diff =
			(float)DonutModel::HALF_CIRCLE -
			abs(abs(donutModel->getAngle() - breach->getAngle()) - (float)DonutModel::HALF_CIRCLE);

		if (!donutModel->isJumping() && playerID != breach->getPlayer() && diff < BREACH_WIDTH &&
			breach->getHealth() != 0) {
			donutModel->applyForce(REBOUND_FORCE * donutModel->getVelocity());
		} else if (playerID == breach->getPlayer() && diff < EPSILON_ANGLE &&
				   !breach->isPlayerOn() && donutModel->getJumpOffset() == 0.0f &&
				   breach->getHealth() > 0) {
			breach->decHealth(1);
			breach->setIsPlayerOn(true);

			if (breach->getHealth() == 0) {
				net->resolveBreach(i);
			}

		} else if (diff > EPSILON_ANGLE && breach->isPlayerOn()) {
			breach->setIsPlayerOn(false);
		}
	}

	for (int i = 0; i < MAX_DOORS; i++) {
		if (ship->getDoors().at(i) == nullptr || ship->getDoors().at(i)->halfOpen() ||
			ship->getDoors().at(i)->getAngle() < 0) {
			continue;
		}
		float diff = (float)DonutModel::HALF_CIRCLE -
					 abs(abs(donutModel->getAngle() - ship->getDoors().at(i)->getAngle()) -
						 (float)DonutModel::HALF_CIRCLE);

		if (diff < DOOR_WIDTH) {
			// TODO: Real physics...
			donutModel->applyForce(REBOUND_FORCE * donutModel->getVelocity());
		}
		if (diff < DOOR_ACTIVE_ANGLE) {
			ship->getDoors().at(i)->addPlayer(playerID);
			net->flagDualTask(i, playerID, 1);
		} else {
			if (ship->getDoors().at(i)->isPlayerOn(playerID)) {
				ship->getDoors().at(i)->removePlayer(playerID);
				net->flagDualTask(i, playerID, 0);
			}
		}
	}

	if ((ship->getBreaches().size()) == 0) {
		ship->setHealth(globals::INITIAL_SHIP_HEALTH);
	} else {
		int h = 0;
		for (int i = 0; i < ship->getBreaches().size(); i++) {
			h = h + ship->getBreaches().at(i)->getHealth();
		}
		ship->setHealth(globals::INITIAL_SHIP_HEALTH + 1 - h);
	}

	gm.update(timestep);
	float thrust = input.getRoll();

	// Move the donut (MODEL ONLY)
	donutModel->applyForce(thrust);
	// Jump Logic
	if (input.getTapLoc() != Vec2::ZERO && !donutModel->isJumping()) {
		donutModel->startJump();
		net->jump(playerID);
	}

	for (unsigned int i = 0; i < ship->getDonuts().size(); i++) {
		ship->getDonuts()[i]->update(timestep);
	}

	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
