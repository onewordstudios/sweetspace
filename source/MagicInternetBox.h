#ifndef MIB_H
#define MIB_H

#include <cugl/cugl.h>

#include <array>
#include <tl/optional.hpp>

#include "NetworkConnection.h"
#include "NetworkDataType.h"
#include "ShipModel.h"
#include "StateReconciler.h"

/**
 * The controller that handles all communication between clients and the server.
 *
 * This class is a singleton. It provides initialization methods to be called
 * before use, and update methods to be called on appropriate frames.
 *
 * Pass information into this class by calling the appropriate methods; this information will be
 * broadcast to other players. Information from other players are automatically dispatched to the
 * Ship Model during the call to update.
 */
class MagicInternetBox {
   private:
	class Mimpl;
	std::unique_ptr<Mimpl> impl;

	/**
	 * Create an empty Network Controller instance. Does no initialization.
	 * Call one of the init methods to connect and stuff.
	 * This constructor is private, as this class is a singleton.
	 */
	MagicInternetBox();

	~MagicInternetBox();

   public:
	MagicInternetBox(MagicInternetBox const&) = delete;
	void operator=(MagicInternetBox const&) = delete;

	/**
	 * Status of whether the game is ready to start
	 */
	enum MatchmakingStatus {
		Disconnected = -2,
		Uninitialized = -1,
		/** Connecting to server; room ID not assigned yet */
		HostConnecting = 0,
		/** Connected and room ID assigned; waiting for other players */
		HostWaitingOnOthers,
		/** Client API is too old for server */
		HostApiMismatch,
		/** Unknown error as host */
		HostError,
		/** Connecting to server; player ID not assigned yet */
		ClientConnecting = 100,
		/** Connected and player ID assigned; waiting for other players */
		ClientWaitingOnOthers,
		/** Room ID does not exist */
		ClientRoomInvalid,
		/** Room ID is full already */
		ClientRoomFull,
		/** Client API does not match host */
		ClientApiMismatch,
		/** Unknown error as client */
		ClientError,
		/** Game has started */
		GameStart = 200,
		/** Attempting to reconnect to a room after dropping */
		Reconnecting = 500,
		/** Unknown error when reconnecting */
		ReconnectError,
		/** Game has ended */
		GameEnded = 900
	};

	/**
	 * Important events from the network that the root controller needs to know about
	 */
	enum NetworkEvents { None, LoadLevel, EndGame };

	/**
	 * Grab a pointer to the singleton instance of this class
	 */
	static MagicInternetBox& getInstance() {
		static MagicInternetBox m;
		return m;
	}

	/**
	 * Initialize this controller class as a host.
	 * Will establish a connection to the server and create a room.
	 * Use the {@link getRoomID()} method to get the ID of the formed room.
	 *
	 * @returns Whether a connection was successfully established
	 */
	bool initHost();

	/**
	 * Initialize this controller class as a client.
	 * Will establish a connection to the server and attempt to join the specified room.
	 *
	 * @param id The room ID
	 * @returns Whether a connection was successfully established
	 */
	bool initClient(const std::string& id);

	/**
	 * Reconnect to a game that you lost connection from.
	 * Will attempt to rejoin the room. Query {@link matchStatus()} over the next few frames to see
	 * the progress. If {@code GameEnded} is returned, then the room does not have a valid game
	 * going at this time.
	 * Should only be called when this controller has a cached playerID and roomID. Otherwise, will
	 * fail.
	 *
	 * @returns Whether a connection was successfully established
	 */
	bool reconnect();

	/**
	 * Query the current matchmaking status
	 */
	MatchmakingStatus matchStatus();

	/**
	 * Get the last major unacknowledged network event; does NOT acknowledge the event.
	 */
	NetworkEvents lastNetworkEvent();

	/**
	 * Acknowledge the last network event, causing {@link lastNetworkEvent()} to return None until
	 * the next event.
	 */
	void acknowledgeNetworkEvent();

	/**
	 * Returns the room ID this controller is connected to.
	 * Is the empty string if this controller is uninitialized.
	 */
	std::string getRoomID();

	/**
	 * Returns the current level number, or -1 if uninitialized.
	 */
	tl::optional<uint8_t> getLevelNum();

	/**
	 * Returns the current player ID, or -1 if uninitialized.
	 * 0 is the host player.
	 */
	tl::optional<uint8_t> getPlayerID();

	/**
	 * Returns the number of connected players, or 0 if uninitialized.
	 */
	uint8_t getNumPlayers() const;

	/** Returns the total number of players in this ship (including disconnected players), or 0 if
	 * uninitialized. */
	uint8_t getMaxNumPlayers() const;

	/**
	 * Returns whether the specified player ID is active.
	 *
	 * PRECONDITION: The playerID must be valid.
	 */
	bool isPlayerActive(uint8_t playerID);

	/**
	 * Set whether or not the tutorial should be skipped.
	 */
	void setSkipTutorial(bool skip);

	/**
	 * Start the game with the current number of players.
	 * Should only be called when the matchmaking status is waiting on others
	 *
	 * @param levelNum The level number to start
	 */
	void startGame(uint8_t levelNum);

	/**
	 * Restart the current level.
	 * Should only be called by the host when a level is in progress.
	 */
	void restartGame();

	/**
	 * Move to the current level number + 1.
	 * Should only be called by the host after winning a level.
	 */
	void nextLevel();

	/**
	 * Update method called every frame during matchmaking phase.
	 * Should be called as long as {@link matchStatus()} is not returning {@code GameStart} and
	 * should not be called after.
	 */
	void update();

	/**
	 * Update method called every frame during gameplay.
	 * This controller will batch and handle network communication as long as this method is called.
	 */
	void update(std::shared_ptr<ShipModel> state);

	/**
	 * Inform other players that a new breach has been created.
	 *
	 * @param angle The angle of the ship to spawn the breach
	 * @param player The player ID that can resolve this breach
	 * @param id The breach ID used to identify this breach in the future
	 */
	void createBreach(float angle, uint8_t player, uint8_t id);

	/**
	 * Inform other players that a breach has shrunk in size by 1.
	 *
	 * @param id The breach ID
	 */
	void resolveBreach(uint8_t id);

	/**
	 * Inform other players that a task requiring two players has been created
	 * (eg: locked doors from nondigital)
	 *
	 * @param angle The angle of the ship to spawn the task
	 * @param id The dual-task ID used to identify this task in the future
	 */
	void createDualTask(float angle, uint8_t id);

	/**
	 * Inform other players that at least one of the two required players has
	 * done their part to resolve a dual-task.
	 * This method does not keep track of whether one or both players are
	 * resolving this task. It merely broadcasts to all other players that
	 * one more person is now on this task; when both players are here, it
	 * is the responsibility of the receivers of this message to resolve the task.
	 *
	 * @param id The dual-task ID
	 * @param player The player ID who is activating the door
	 * @param flag Whether the player is on or off the door (1 or 0)
	 */
	void flagDualTask(uint8_t id, uint8_t player, uint8_t flag);

	/**
	 * Inform other players that two buttons requiring two players has been created
	 *
	 * @param angle1 The angle of the ship to spawn the button
	 * @param id1 The button ID
	 * @param angle2 The angle of the ship to spawn the button's pair
	 * @param id2 The ID for the button's pair
	 */
	void createButtonTask(float angle1, uint8_t id1, float angle2, uint8_t id2);

	/**
	 * Inform other players that one person is on the button.
	 * This method does not keep track of whether one or both players are resolving this task. It
	 * merely broadcasts to all other players that one more person is now on this task. It is the
	 * responsibility of the second player to jump on the button to call {@link resolveButton()} to
	 * inform others that the task has been resolved.
	 *
	 * @param id The button ID
	 * @param player The player ID who is activating the button
	 * @param flag Whether the player is on or off the door (1 or 0)
	 */
	void flagButton(uint8_t id);

	/**
	 * Inform other players that a pair of buttons have been resolved.
	 *
	 * @param id The ID of one of the two buttons.
	 */
	void resolveButton(uint8_t id);

	/**
	 * Inform other players that a stabilizer malfunction has been created
	 *
	 * @param player The player whose screen this message will appear on
	 */
	void createAllTask(uint8_t player);

	/**
	 * Inform the host that a task requiring all members of the ship has failed, and thus to deduct
	 * appropriate health
	 */
	void failAllTask();

	/**
	 * Inform the host that a task requiring all members of the ship has succeeded
	 */
	void succeedAllTask();

	/**
	 * Inform the clients that the level has been won
	 */
	void forceWinLevel();

	/**
	 * Inform other players that a player has initiated a jump.
	 *
	 * @param player The player ID who is jumping
	 */
	void jump(uint8_t player);

	/**
	 * Disconnect this player from the server, by force.
	 */
	void forceDisconnect();

	/**
	 * Reset the controller entirely; useful when leaving a game.
	 */
	void reset();
};

#endif /* MIB_H */
