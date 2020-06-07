#ifndef __MAIN_MENU_MODE_H__
#define __MAIN_MENU_MODE_H__
#include <cugl/cugl.h>

#include <array>
#include <vector>

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

	/** The current frame of a transition; -1 if not transitioning */
	int transitionFrame;

	/** Current frame of the rotating stars */
	int rotationFrame;

	/** An enum with the current state of the matchmaking mode */
	enum MatchState {
		/** Empty state; used mainly for transitions only; the main state should only be NA when
		   uninitialized */
		NA = -1,
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
		/** Joining a game; actually connected */
		ClientScreenDone,
		/** Matchmaking complete */
		Done,

		/** Credits screen */
		Credits
	};

	/** The current state */
	MatchState currState;
	/** The state we are transitioning into, or NA (-1) if not transitioning */
	MatchState transitionState;
#pragma endregion

#pragma region Scene Graph Nodes

	/** Background asset with stars */
	std::shared_ptr<cugl::Node> bg0stars;
	/** Background asset with ship glow */
	std::shared_ptr<cugl::Node> bg1glow;
	/** Background asset with donut ship */
	std::shared_ptr<cugl::Node> bg2ship;
	/** Background asset with land */
	std::shared_ptr<cugl::Node> bg3land;
	/** Background asset with studio name */
	std::shared_ptr<cugl::Node> bg9studio;

	/** Back button */
	std::shared_ptr<cugl::Button> backBtn;

	/** Button to create host */
	std::shared_ptr<cugl::Button> hostBtn;
	/** Button to create client */
	std::shared_ptr<cugl::Button> clientBtn;

	/** The nodes containing all UI for the starting splash screen */
	std::vector<std::shared_ptr<cugl::Node>> mainScreen;
	/** The node containing all UI for the host screen */
	std::shared_ptr<cugl::Node> hostScreen;
	/** The node containing all UI for the client screen */
	std::shared_ptr<cugl::Node> clientScreen;

	/** Connection loading message */
	std::shared_ptr<cugl::Label> connScreen;

	/** Level select */
	std::shared_ptr<cugl::Node> levelSelect;

	/** Level select buttons */
	std::array<std::shared_ptr<cugl::Button>, NUM_LEVEL_BTNS> levelBtns;

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
	/** The label on the host screen shown to the client after joining */
	std::shared_ptr<cugl::Node> clientWaitHost;

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

#pragma region Update Handlers
	/**
	 * Called during a state transition upon completing the transition.
	 */
	void endTransition();

	/**
	 * Handle update during a state transition. Animates the transition between states.
	 *
	 * PRECONDITION: transitionState != NA
	 */
	void processTransition();

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
	MainMenuMode()
		: Scene(),
		  net(nullptr),
		  startHostThread(nullptr),
		  screenHeight(0),
		  gameReady(false),
		  transitionFrame(-1),
		  rotationFrame(0),
		  currState(NA),
		  transitionState(StartScreen) {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	~MainMenuMode() { dispose(); }

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

#pragma endregion

	/** Go directly to the credits sequence */
	void triggerCredits();

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
	bool isGameReady() { return gameReady; }
};

#endif /* __MAIN_MENU_MODE_H__ */
