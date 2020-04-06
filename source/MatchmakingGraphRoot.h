#ifndef __MATCHMAKING_GRAPH_ROOT_H__
#define __MATCHMAKING_GRAPH_ROOT_H__
#include <cugl/cugl.h>

#include <vector>

#include "BreachModel.h"
#include "ButtonManager.h"
#include "DonutModel.h"
#include "DonutNode.h"
#include "InputController.h"

class MatchmakingGraphRoot : public cugl::Scene {
   private:
	/** An enum with the current state of the matchmaking mode */
	enum MatchState {
		/** Empty state; used for transitions only; the main state should NEVER be NA */
		NA = -1,
		/** Main menu splash screen */
		StartScreen,
		/** Hosting a game; waiting on ship ID */
		HostScreenWait,
		/** Hosting a game; ship ID received */
		HostScreen,
		/** Joining a game; waiting on ship ID */
		ClientScreen,
		/** Joining a game; connected */
		ClientScreenDone,
		/** Matchmaking complete */
		Done
	};

	/** The current state */
	MatchState currState;
	/** The state we are transitioning into, or NA (-1) if not transitioning */
	MatchState transitionState;

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

	/** Connection loading message */
	std::shared_ptr<cugl::Label> connScreen;

	/** Label for room ID (host) */
	std::shared_ptr<cugl::Label> hostLabel;
	/** Button to begin game (host) */
	std::shared_ptr<cugl::Button> hostBeginBtn;
	/** The node containing the player count needle for the host */
	std::shared_ptr<cugl::Node> hostNeedle;

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

	/** Helper object to make the buttons go up and down */
	ButtonManager buttonManager;

	// MODEL
	/** RoomId for host display */
	std::string roomID;
	/** Num players connected */
	unsigned int numPlayers;
	/** Whether an error has occured */
	bool isError;

	/**
	 * Update the client room display using the contents of {@link clientEnteredRoom}
	 */
	void updateClientLabel();

	/** The current frame of a transition; -1 if not transitioning */
	int transitionFrame;

	/**
	 * Animate a transition between states.
	 *
	 * PRECONDITION: transitionState != NA
	 */
	void processTransition();

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	MatchmakingGraphRoot()
		: Scene(),
		  currState(StartScreen),
		  transitionState(NA),
		  screenHeight(0),
		  roomID(""),
		  numPlayers(0),
		  isError(false),
		  transitionFrame(-1) {}

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

	/** An enum representing buttons that have been pressed */
	enum PressedButton { None, StartHost, StartClient, HostBegin, ClientConnect };

	/**
	 * Processes button presses. Should be called AFTER update() every frame.
	 *
	 * @param position The screen coordinates of the tap
	 *
	 * @return The button pressed
	 */
	PressedButton checkButtons(InputController& position);

	/**
	 * Sets roomID (for the host)
	 *
	 * @param roomID The host room id
	 */
	void setRoomID(std::string roomID);

	/**
	 * Gets roomID (from client connection)
	 *
	 * @param roomID The room id
	 */
	std::string getRoomID() { return roomID; }

	/** Sets the number of players currently connected */
	void setNumPlayers(unsigned int num) { numPlayers = num; }

	/** Signal a catastrophic error has occured */
	void signalError() { isError = true; }

	/** Returns whether the graph is in a state where it is connected to the server (and thus mib
	 * needs to be updated every frame) */
	bool isConnected() {
		switch (currState) {
			case HostScreenWait:
			case HostScreen:
			case ClientScreenDone:
				return true;
			default:
				return false;
		}
	}
};
#endif /* __MATCHMAKING_GRAPH_ROOT_H__ */
