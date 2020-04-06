#include "MatchmakingMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Constants

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
bool MatchmakingMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}
	// Set network controller
	net = MagicInternetBox::getInstance();
	input = InputController::getInstance();

	sgRoot.init(assets);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MatchmakingMode::dispose() { sgRoot.dispose(); }

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void MatchmakingMode::reset() { input->clear(); }

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void MatchmakingMode::update(float timestep) {
	input->update(timestep);
	// Update Scene Graph
	sgRoot.update(timestep);

	switch (sgRoot.checkButtons()) {
		case MatchmakingGraphRoot::StartHost: {
			startHostThread = std::unique_ptr<std::thread>(
				new std::thread([]() { MagicInternetBox::getInstance()->initHost(); }));
			break;
		}
		case MatchmakingGraphRoot::ClientConnect: {
			net->initClient(sgRoot.getRoomID());
			break;
		}
		case MatchmakingGraphRoot::HostBegin: {
			if (net->getNumPlayers() >= globals::MIN_PLAYERS) {
				gameReady = true;
				net->startGame();
			}
			return;
		}
		default:
			break;
	}

	switch (net->matchStatus()) {
		case MagicInternetBox::MatchmakingStatus::Uninitialized:
			return;
		case MagicInternetBox::MatchmakingStatus::HostError:
			sgRoot.signalError();
			return;
		default:
			break;
	}

	if (sgRoot.isConnected()) {
		net->update();
		switch (net->matchStatus()) {
			case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
			case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
			case MagicInternetBox::MatchmakingStatus::ClientError:
				sgRoot.setRoomID("");
				break;
			case MagicInternetBox::MatchmakingStatus::GameStart:
				gameReady = true;
				return;
			case MagicInternetBox::MatchmakingStatus::Uninitialized:
			case MagicInternetBox::MatchmakingStatus::HostError:
				break;
			default:
				sgRoot.setRoomID(net->getRoomID());
				sgRoot.setNumPlayers(net->getNumPlayers());
				break;
		}
	}
}

/**
 * Draws the game.
 */
void MatchmakingMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
	sgRoot.render(batch);
}
