#ifndef MAIN_MENU_MODE_H
#define MAIN_MENU_MODE_H
#include <cugl/cugl.h>

#include <array>
#include <vector>

#include "AnimationManager.h"
#include "ButtonManager.h"
#include "InputController.h"
#include "LevelConstants.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"

/**
 * The primary controller for the main menu / matchmaking mode
 */
class MainMenuMode : public cugl::Scene {
   private:
#pragma region Controllers
	/** Controller for abstracting out input across multiple platforms */
	std::shared_ptr<InputController> input;
	/** Networking controller*/
	std::shared_ptr<MagicInternetBox> net;
#pragma endregion

	/** An extra thread used to connect to the server from the host, in case the server is down. */
	std::unique_ptr<std::thread> startHostThread;

	/** Helper object to make the buttons go up and down */
	ButtonManager buttonManager;

	/** The Screen's Height. */
	float screenHeight;

#pragma region State Variables
	/** True if game is ready to start */
	bool gameReady;

	/** The room ID the client is currently entering */
	std::vector<unsigned int> clientEnteredRoom;

	/** Current room ID */
	std::string roomID;

	/** Current frame of the rotating stars */
	int rotationFrame;

	/** Position of credits */
	unsigned int creditsScrollFrame;

	/** Current position of the needle */
	float needlePos;

	/**
	 * An enum with the current state of the matchmaking mode
	 *
	 *
	 * User Flow:
	 *
	 * NA - This mode is initialized here, and matches the loading screen exit point
	 * StartScreen - Loading mode transitions automatically to here; main splash menu
	 *             - Contains buttons Create, Join, and Credits
	 *	-	HostScreenWait - Clicking Create takes users here; waiting for room ID from server
	 *			HostScreen - Transitions here after room ID assigned; waiting for others to join
	 *			HostLevelSelect - Transitions here after clicking start; level select screen
	 *	-	ClientScreen - Clicking Join takes users here; waiting for room ID input
	 *			ClientScreenSubmitted - Client entered room ID; waiting to hear from server
	 *					ClientScreenDone - Room ID valid; join complete
	 *					ClientScreenError - Something went wrong; returns to ClientScreen
	 *	-	Credits - Credits Screen
	 *
	 * Back button exists on screens HostScreen, ClientScreen, and Credits. Once joined into a game,
	 * back button is no longer available. In addition, HostScreenWait will provide a back button if
	 * there is an error connecting to server. Back button always returns to StartScreen.
	 */
	enum MatchState {
		/** Main menu splash screen */
		StartScreen,
		/** Hosting a game; waiting on ship ID */
		HostScreenWait,
		/** Hosting a game; ship ID received */
		HostScreen,
		/** Host; level select screen */
		HostLevelSelect,
		/** Joining a game; waiting on ship ID */
		ClientScreen,
		/** Joining a game; hit submit */
		ClientScreenSubmitted,
		/** Joining a game but something went wrong */
		ClientScreenError,
		/** Joining a game; actually connected */
		ClientScreenDone,
		/** Matchmaking complete */
		Done,

		/** Credits screen */
		Credits
	};

	/** The current state */
	MatchState currState;
#pragma endregion

#pragma region Scene Graph Nodes

	class MainMenuTransitions;

	/** Transitions helper object */
	std::unique_ptr<MainMenuTransitions> transition;

	/** Background asset with stars */
	std::shared_ptr<cugl::Node> bg0stars;

	/** Back button */
	std::shared_ptr<cugl::Button> backBtn;

	/** Button to create host */
	std::shared_ptr<cugl::Button> hostBtn;
	/** Button to create client */
	std::shared_ptr<cugl::Button> clientBtn;

	/** Connection loading message */
	std::shared_ptr<cugl::Label> connScreen;

	/** Level select buttons */
	std::array<std::shared_ptr<cugl::Button>, NUM_LEVEL_BTNS> levelBtns;

	/** Label for room ID (host) */
	std::shared_ptr<cugl::Label> hostLabel;
	/** Button to begin game (host) */
	std::shared_ptr<cugl::Button> hostBeginBtn;
	/** The node containing the player count needle for the host */
	std::shared_ptr<cugl::Node> hostNeedle;
	/** The toggle to skip the tutorial; not managed by the btn manager */
	std::shared_ptr<cugl::Button> hostTutorialSkipBtn;

	/** Label for room ID (client) */
	std::shared_ptr<cugl::Label> clientLabel;
	/** Button to confirm room ID (client) */
	std::shared_ptr<cugl::Button> clientJoinBtn;
	/** Vector of 0-9 buttons used to enter room ID (client) */
	std::vector<std::shared_ptr<cugl::Button>> clientRoomBtns;
	/** Clear button from client */
	std::shared_ptr<cugl::Button> clientClearBtn;
	/** The label on the host screen shown to the client after joining */
	std::shared_ptr<cugl::Node> clientWaitHost;
	/** The label on the client error screen */
	std::shared_ptr<cugl::Label> clientErrorLabel;
	/** The label on the client error screen */
	std::shared_ptr<cugl::Button> clientErrorBtn;

	/** Button to credits */
	std::shared_ptr<cugl::Button> creditsBtn;
	/** Credits scroll */
	std::shared_ptr<cugl::Node> credits;

#pragma endregion
	/**
	 * Update the client room display using the contents of {@link clientEnteredRoom}
	 */
	void updateClientLabel();

	/**
	 * Query mib and update the room ID for the host accordingly
	 */
	void setRoomID();

	/** Query mib and update the needle for number of players */
	void setNumPlayers();

#pragma region Update Handlers

	/**
	 * Handle normal updates during a frame. Process state updates that happen each frame.
	 */
	void processUpdate();

	/**
	 * Handle button presses during a frame. Update button states and handle when buttons are
	 * clicked.
	 */
	void processButtons();
#pragma endregion

   public:
#pragma region Instantiation Logic
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	MainMenuMode();

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	~MainMenuMode();

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
	 * @param toCredits Whether to jump directly to credits
	 *
	 * @return true if the controller is initialized properly, false otherwise.
	 */
	bool init(const std::shared_ptr<cugl::AssetManager>& assets, bool toCredits = false);

#pragma endregion

	/**
	 * The method called to update the game mode.
	 *
	 * This method contains any gameplay code that is not an OpenGL call.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep) override;

	/**
	 * Draws the game.
	 *
	 * @param batch The sprite batch.
	 */
	void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);

	/**
	 * Checks if game is ready to start
	 *
	 * @return True if game is ready to start, false otherwise
	 */
	bool isGameReady() const { return gameReady; }
};

#endif /* MAIN_MENU_MODE_H */
