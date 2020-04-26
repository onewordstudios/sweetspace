#ifndef __GAME_GRAPH_ROOT_H__
#define __GAME_GRAPH_ROOT_H__
#include <cugl/cugl.h>

#include <vector>

#include "BreachModel.h"
#include "BreachNode.h"
#include "ButtonManager.h"
#include "ButtonNode.h"
#include "DonutModel.h"
#include "DonutNode.h"
#include "DoorNode.h"
#include "Globals.h"
#include "HealthNode.h"
#include "InputController.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"

class GameGraphRoot : public cugl::Scene {
   public:
	/** Enum for determining drawing state */
	enum DrawStatus {
		/** Reconnecting */
		Reconnecting = -1,
		/** Normal Gameplay */
		Normal,
		/** Win Screen */
		Win,
		/** Loss Screen */
		Loss,
		/** Game Ended Unexpectedly */
		Ended
	};

   protected:
	/** The asset manager for this game mode. */
	std::shared_ptr<cugl::AssetManager> assets;
	/** The Screen's Height. */
	float screenHeight;

	/** Helper object to make the buttons go up and down */
	ButtonManager buttonManager;

	// VIEW COMPONENTS
	/** Filmstrip representing the player's animated donut */
	std::shared_ptr<cugl::PolygonNode> donutNode;
	/** Label for on-screen coordinate HUD */
	std::shared_ptr<cugl::Label> coordHUD;
	/** Node to hold all of our graphics. Necesary for resolution indepedence. */
	std::shared_ptr<cugl::Node> allSpace;
	/** Background in animation parallax. Stores the field of stars */
	std::shared_ptr<cugl::Node> farSpace;
	/** Foreground in animation parallax. Stores the planets. */
	std::shared_ptr<cugl::Node> nearSpace;
	/** Parent node of all breaches, is child of nearSpace */
	std::shared_ptr<cugl::Node> breachesNode;
	/** Parent node of all ship segments, is child of nearSpace */
	std::shared_ptr<cugl::Node> shipSegsNode;
	/** Parent node of all doors, is child of nearSpace */
	std::shared_ptr<cugl::Node> doorsNode;
	/** Parent node of all external donuts, is child of nearSpace */
	std::shared_ptr<cugl::Node> externalDonutsNode;

	std::shared_ptr<cugl::PolygonNode> challengePanelHanger;
	std::shared_ptr<cugl::PolygonNode> challengePanel;
	std::shared_ptr<cugl::PolygonNode> challengePanelText;
	std::vector<std::shared_ptr<cugl::PolygonNode>> challengePanelArrows;
	/** Filmstrip representing the player's animated donut */
	std::shared_ptr<cugl::PolygonNode> healthNode;

	// Reconnection Textures
	/** Node to hold all of the Reconnect Overlay.*/
	std::shared_ptr<cugl::Node> reconnectOverlay;
	/** Label for second ellipsis point */
	std::shared_ptr<cugl::Label> reconnectE2;
	/** Label for third ellipsis point */
	std::shared_ptr<cugl::Label> reconnectE3;
	/** Current animation frame for ellipses */
	int currentEllipsesFrame;

	// Pause Textures
	/** Button to Open Pause */
	std::shared_ptr<cugl::Button> pauseBtn;
	/** Node to hold all of the Loss Screen.*/
	std::shared_ptr<cugl::Node> pauseScreen;
	/** Button to Mute Music */
	std::shared_ptr<cugl::Button> musicBtn;
	/** Button to Mute Sound Effects */
	std::shared_ptr<cugl::Button> soundBtn;
	/** Button to Leave */
	std::shared_ptr<cugl::Button> leaveBtn;
	/** The node containing the player count needle*/
	std::shared_ptr<cugl::Node> needle;

	/** Ship red overlay node */
	std::shared_ptr<cugl::PolygonNode> shipOverlay;

	// Loss Screen Textures
	/** Node to hold all of the Loss Screen.*/
	std::shared_ptr<cugl::Node> lossScreen;
	/** Button to restart game */
	std::shared_ptr<cugl::Button> restartBtn;
	/** Button to levels */
	std::shared_ptr<cugl::Button> levelsBtn;

	// Win Screen Textures
	/** Node to hold all of the Win Screen.*/
	std::shared_ptr<cugl::Node> winScreen;
	/** Button to next game */
	std::shared_ptr<cugl::Button> nextBtn;

	// DRAWING STATE VARIABLES
	/** The donut's base position. */
	cugl::Vec2 donutPos;

	/** Tag of the left most ship segment */
	unsigned int leftMostSeg;
	/** Tag of the right most ship segment */
	unsigned int rightMostSeg;
	/** Parent node of all buttons, is child of nearSpace */
	std::shared_ptr<cugl::Node> buttonsNode;

	// MODEL INFORMATION
	/** Id of the current client */
	unsigned int playerID;
	/** The ship */
	std::shared_ptr<ShipModel> ship;
	/** Angle of the player donut model from the last frame */
	float prevPlayerAngle;

	/** Current animation frame for ship flashing red */
	int currentHealthWarningFrame;
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
	 * The current Drawing status
	 */
	DrawStatus status;

	/**
	 * Returns the wrapped value of input around the ship size.
	 *
	 * @param f degree in radians
	 * @return Wrapped angle in radians
	 */
	float wrapAngle(float f) {
		float mod = fmod(f, globals::TWO_PI);
		return mod < 0 ? globals::TWO_PI + mod : mod;
	};

	/** Process Buttons in Special Screens */
	void processButtons();

	/** Whether to go back to main menu */
	bool isBackToMainMenu;

   public:
#pragma mark -
#pragma mark Public Consts
	/** Possible colors for player representations */
	const std::vector<string> PLAYER_COLOR{"yellow", "red", "purple", "green", "orange", "cyan"};
	/** Possible colors for breach representations */
	const std::vector<cugl::Color4> BREACH_COLOR{
		cugl::Color4(219, 197, 52), cugl::Color4(227, 100, 159), cugl::Color4(152, 95, 204),
		cugl::Color4(158, 212, 87), cugl::Color4(244, 150, 40),	 cugl::Color4(47, 206, 197)};
	/** Color of ship segment label text */
	const cugl::Color4 SHIP_LABEL_COLOR{255, 248, 161};
	/** Number of possible player colors */
	static constexpr int NUM_COLORS = 6;
	/** The scale of the breach textures. */
	static constexpr float BREACH_SCALE = 0.25;

#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	GameGraphRoot()
		: Scene(),
		  status(Normal),
		  currentEllipsesFrame(0),
		  currentHealthWarningFrame(0),
		  isBackToMainMenu(false) {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 *
	 * This method is different from dispose() in that it ALSO shuts off any
	 * static resources, like the input controller.
	 */
	~GameGraphRoot() { dispose(); }

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
	bool init(const std::shared_ptr<cugl::AssetManager>& assets, std::shared_ptr<ShipModel> ship,
			  unsigned int playerID);

#pragma mark -
#pragma mark Gameplay Handling
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

	/**
	 * Spin Dial in pause menu
	 * @param percentage  The percent of dial to spin
	 */
	void setNeedlePercentage(float percentage);

	std::shared_ptr<cugl::Node> getDonutNode();

#pragma mark -
#pragma mark Accessors
	/**
	 * Set Drawing status
	 *
	 * @param status the drawing status of the ship
	 */
	void setStatus(DrawStatus status) { this->status = status; }

	/**
	 * Get Drawing Status
	 *
	 * @return Drawing Status
	 */
	DrawStatus getStatus() { return status; }

	/**
	 * Set whether to go back to the main menu (should not be called)
	 *
	 * @param b whether to go back to the main menu
	 */
	void setIsBackToMainMenu(bool b) { isBackToMainMenu = b; }

	/**
	 * Get whether to go back to the main menu
	 *
	 * @return whether to go back to the main menu
	 */
	bool getIsBackToMainMenu() { return isBackToMainMenu; }
};
#endif /* __GAME_GRAPH_ROOT_H__ */
