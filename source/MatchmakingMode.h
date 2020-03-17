﻿#ifndef __MATCHMAKING_MODE_H__
#define __MATCHMAKING_MODE_H__
#include <cugl/cugl.h>

#include <vector>

#include "InputController.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"

/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class MatchmakingMode {
   protected:
	// CONTROLLERS
	/** Controller for abstracting out input across multiple platforms */
	InputController input;
	/** Networking controller*/
	MagicInternetBox net;

	// VIEW
	/** Scenegraph root node */
	// GameGraphRoot sgRoot;

	// MODEL
	/** The list of breaches */
	std::vector<std::shared_ptr<DonutModel>> donuts;
	/** The list of breaches */
	std::vector<std::shared_ptr<BreachModel>> breaches;
	/** The Ship model */
	std::shared_ptr<ShipModel> shipModel;

	bool host = true;
	int playerId;

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	MatchmakingMode() {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 *
	 * This method is different from dispose() in that it ALSO shuts off any
	 * static resources, like the input controller.
	 */
	~MatchmakingMode() { dispose(); }

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	void dispose();

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
	void update(float timestep);

	/**
	 * Resets the status of the game so that we can play again.
	 */
	void reset();

	/**
	 * Draws the game.
	 */
	void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
};

#endif /* __MATCHMAKING_MODE_H__ */