#ifndef __MATCHMAKING_GRAPH_ROOT_H__
#define __MATCHMAKING_GRAPH_ROOT_H__
#include <cugl/cugl.h>

#include <vector>

#include "BreachModel.h"
#include "DonutModel.h"
#include "DonutNode.h"
#include "InputController.h"

class MatchmakingGraphRoot : public cugl::Scene {
protected:
	/** The asset manager for this game mode. */
	std::shared_ptr<cugl::AssetManager> assets;
	/** The Screen's Height. */
	float screenHeight;

	// VIEW
	/** Button to create host */
	std::shared_ptr<cugl::Button> host;
	/** Button to create client */
	std::shared_ptr<cugl::Button> client;

	// MODEL
	int playerId;

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

public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	MatchmakingGraphRoot() : Scene() {}

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
};
#endif /* __MATCHMAKING_GRAPH_ROOT_H__ */
