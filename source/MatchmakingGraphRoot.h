#ifndef __MATCHMAKING_GRAPH_ROOT_H__
#define __MATCHMAKING_GRAPH_ROOT_H__
#include <cugl/cugl.h>

#include <vector>

#include "BreachModel.h"
#include "DonutModel.h"
#include "DonutNode.h"
#include "InputController.h"

class MatchmakingGraphRoot : public cugl::Scene {
   private:
	/** The asset manager for this game mode. */
	std::shared_ptr<cugl::AssetManager> assets;
	/** The Screen's Height. */
	float screenHeight;

	// VIEW
	/** Button to create host */
	std::shared_ptr<cugl::Button> hostBtn;
	/** Button to create client */
	std::shared_ptr<cugl::Button> clientBtn;

	/** The node containing all UI for the starting splash screen */
	std::shared_ptr<cugl::Node> mainScreen;
	/** The node containing all UI for the host screen */
	std::shared_ptr<cugl::Node> hostScreen;
	/** The node containing all UI for the client screen */
	std::shared_ptr<cugl::Node> clientScreen;

	/** Label for room ID (host) */
	std::shared_ptr<cugl::Label> hostLabel;
	/** Label for room ID (client) */
	std::shared_ptr<cugl::Label> clientLabel;
	/** Button to confirm room ID (client) */
	std::shared_ptr<cugl::Button> clientJoinBtn;
	/** Vector of 0-9 buttons used to enter room ID (client) */
	std::vector<std::shared_ptr<cugl::Button>> clientRoomBtns;
	/** Clear button from client */
	std::shared_ptr<cugl::Button> clientClearBtn;
	/** The room ID the client is currently entering */
	std::vector<unsigned int> clientEnteredRoom;

	// MODEL
	int playerID;
	/** RoomId for host display */
	std::string roomID;

	/**
	 * Returns an informative string for the position
	 *
	 * This function is for writing the current donut position to the HUD.
	 *
	 * @param coords The current donut coordinates
	 *
	 * @return an informative string for the position
	 */
	std::string positionText();

	/**
	 * Update the client room display using the contents of {@link clientEnteredRoom}
	 */
	void updateClientLabel();

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	MatchmakingGraphRoot() : Scene(), screenHeight(0), playerID(-1), roomID("") {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 *
	 * This method is different from dispose() in that it ALSO shuts off any
	 * static resources, like the input controller.
	 */
	~MatchmakingGraphRoot() { dispose(); }

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	void dispose() override;

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
	bool init(const std::shared_ptr<cugl::AssetManager>& assets);

#pragma mark -
#pragma mark Matchmaking Handling
	/**
	 * The method called to update the game mode.
	 *
	 * This method contains any gameplay code that is not an OpenGL call.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep) override;

	/**
	 * Resets the status of the game so that we can play again.
	 */
	void reset() override;

	enum PressedButton { None, StartHost, StartClient, HostBegin, ClientConnect };

	/**
	 * Returns integers representing which button has been tapped if any
	 *
	 * @param position The screen coordinates of the tap
	 *
	 * @return The button pressed
	 */
	PressedButton checkButtons(const cugl::Vec2& position);

	/**
	 * Sets roomID
	 *
	 * @param roomID The host room id
	 */
	void setRoomID(std::string roomID) { this->roomID = roomID; }

	/**
	 * Gets roomID
	 *
	 * @param roomID The room id
	 */
	std::string getRoomID() { return roomID; }

	/**
	 * Sets playerID
	 *
	 * @param playerID The new player id
	 */
	void setPlayerID(int playerID) { this->playerID = playerID; }

	/**
	 * Gets playerID
	 *
	 * @param roomID The player id
	 */
	int getPlayerID() { return playerID; }
};
#endif /* __MATCHMAKING_GRAPH_ROOT_H__ */
