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
	/** The asset manager for this game mode. */
	std::shared_ptr<cugl::AssetManager> assets;

	// VIEW

	// MODEL
	/** RoomId for host display */
	std::string roomID;
	/** Num players connected */
	unsigned int numPlayers;
	/** Whether an error has occured */
	bool isError;

   public:
	/**
	 * The method called to update the game mode.
	 *
	 * This method contains any gameplay code that is not an OpenGL call.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep) override;

	/** An enum representing buttons that have been pressed */
	enum PressedButton {
		None,
		StartHost,
		StartClient,
		HostBegin,
		StartGame1,
		StartGame2,
		StartGame3,
		ClientConnect
	};

	/**
	 * Processes button presses. Should be called AFTER update() every frame.
	 *
	 * @return The button pressed
	 */
	PressedButton checkButtons();

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

	/** Force the level select screen to be shown. TODO remove after refactoring matchmaking */
	void startLevelSelect();
};
#endif /* __MATCHMAKING_GRAPH_ROOT_H__ */
