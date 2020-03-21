﻿#ifndef __GAME_GRAPH_ROOT_H__
#define __GAME_GRAPH_ROOT_H__
#include <cugl/cugl.h>

#include <vector>

#include "DonutNode.h"
#include "DoorNode.h"
#include "InputController.h"
#include "ShipModel.h"

class GameGraphRoot : public cugl::Scene {
   protected:
	/** The asset manager for this game mode. */
	std::shared_ptr<cugl::AssetManager> assets;
	/** The donut's base position. */
	cugl::Vec2 donutPos;
	/** The Screen's Height. */
	float screenHeight;

	// VIEW
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

	// MODEL
	/** Id of the current client */
	unsigned int playerID;
	/** The ship */
	std::shared_ptr<ShipModel> ship;

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
#pragma mark Public Consts
	/** Possible colors for player representations */
	const std::vector<string> playerColor{"yellow", "red", "purple", "green", "orange"};

#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	GameGraphRoot() : Scene() {}

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

	std::shared_ptr<cugl::Node> getDonutNode();
};
#endif /* __GAME_GRAPH_ROOT_H__ */
