#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

#include <cugl/cugl.h>

#include "ShipModel.h"
#include "libraries/easywsclient.hpp"

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
   public:
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

   private:
	/**
	 * The singleton instance of this class.
	 */
	static std::shared_ptr<MagicInternetBox> instance;

	/**
	 * The actual websocket connection
	 */
	easywsclient::WebSocket::pointer ws;

	/**
	 * The current status
	 */
	MatchmakingStatus status;

	/**
	 * The current frame, modulo the network tick rate.
	 */
	unsigned int currFrame;

	/**
	 * ID of the current player, or -1 if unassigned
	 */
	int playerID;

	/**
	 * The ID of the current room, or "" if unassigned
	 */
	std::string roomID;

	/** Current level number, or -1 if unassigned */
	int levelNum;

	/**
	 * Number of connected players
	 */
	unsigned int numPlayers;

	/**
	 * Maximum number of players for this ship
	 */
	unsigned int maxPlayers;

	/**
	 * Number of frames since the last inbound server message
	 */
	unsigned int lastConnection;

	/**
	 * The type of data being sent during a network packet
	 */
	enum NetworkDataType {
		// Gameplay messages
		PositionUpdate = 0,
		Jump,
		BreachCreate,
		BreachShrink,
		DualCreate,
		DualResolve,
		ButtonCreate,
		ButtonFlag,
		ButtonResolve,
		AllCreate,
		AllFail,
		StateSync,

		// Connection messages that can be received during gameplay
		PlayerJoined = 50, // Doubles for both matchmaking and reconnect
		PlayerDisconnect,  // Doubles for manually disconnecting
		StartGame,

		// Matchmaking messages only
		AssignedRoom = 100, // Doubles for both creating and created
		JoinRoom			// Doubles for both joining and join response
	};

	/**
	 * Initialize the network connection.
	 * Will establish a connection to the server.
	 *
	 * @returns Whether the connection was successfully established
	 */
	bool initConnection();

	/**
	 * Send data over the network as described in the architecture specification.
	 *
	 * Angle field is for the angle, if applicable.
	 * ID field is for the ID of the object being acted on, if applicable.
	 * Remaining data fields should be filled from first applicable data type back in the same order
	 * that arguments are passed to the calling method in this class.
	 *
	 * Any unused fields should be set to -1.
	 *
	 * For example, create dual task passes angle to angle, task id to id, the two players to data1
	 * and data2 respectively, and sets data3 to -1.
	 */
	void sendData(NetworkDataType type, float angle, int id, int data1, int data2, float data3);

	/**
	 * Broadcast the state of the ship as host to all other players. Will broadcast status and
	 * location of all breaches and doors. Does NOT broadcast location and jump status of other
	 * players, as this data will self-resolve over time and de-syncing on it will not cause issues
	 * with the gameplay.
	 *
	 * Should only be called as host.
	 *
	 * @param state The current, definitive state of the ship
	 */
	void syncState(std::shared_ptr<ShipModel> state);

	/**
	 * Compare the current state of the ship with the state given by the host, and will resolve any
	 * discrepancies in favor of the host.
	 *
	 * @param state The current, potentially de-synced state of the ship
	 * @param message The actual state of the ship message from the host
	 */
	void resolveState(std::shared_ptr<ShipModel> state, const std::vector<uint8_t>& message);

	/**
	 * Create an empty Network Controller instance. Does no initialization.
	 * Call one of the init methods to connect and stuff.
	 * This constructor is private, as this class is a singleton.
	 */
	MagicInternetBox() {
		ws = nullptr;
		status = Uninitialized;
		levelNum = -1;
		currFrame = 0;
		playerID = -1;
		numPlayers = 0;
		maxPlayers = 0;
		lastConnection = 0;
	};

   public:
	/**
	 * Grab a pointer to the singleton instance of this class
	 */
	static std::shared_ptr<MagicInternetBox> getInstance() {
		if (instance == nullptr) {
			// clang-tidy doesn't like this raw pointer assignment, but it's a singleton so it
			// should be fine NOLINTNEXTLINE
			MagicInternetBox* temp = new MagicInternetBox();
			instance = std::shared_ptr<MagicInternetBox>(temp);
		}
		return instance;
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
	bool initClient(std::string id);

	/**
	 * Reconnect to a game that you lost connection from.
	 * Will attempt to rejoin the room. Query {@link matchStatus()} over the next few frames to see
	 * the progress. If {@code GameEnded} is returned, then the room does not have a valid game
	 * going at this time.
	 *
	 * @param id The room ID
	 * @returns Whether a connection was successfully established
	 */
	bool reconnect(std::string id);

	/**
	 * Query the current matchmaking status
	 */
	MatchmakingStatus matchStatus();

	/**
	 * Disconnect from the room and cleanup this object.
	 */
	void leaveRoom();

	/**
	 * Returns the room ID this controller is connected to.
	 * Is the empty string if this controller is uninitialized.
	 */
	std::string getRoomID();

	/**
	 * Returns the current level number, or -1 if uninitialized.
	 */
	int getLevelNum() { return levelNum; }

	/**
	 * Returns the current player ID, or -1 if uninitialized.
	 * 0 is the host player.
	 */
	int getPlayerID();

	/**
	 * Returns the number of connected players, or 0 if uninitialized.
	 */
	unsigned int getNumPlayers();

	/** Returns the total number of players in this ship (including disconnected players), or 0 if
	 * uninitialized. */
	unsigned int getMaxNumPlayers() { return maxPlayers; }

	/**
	 * Start the game with the current number of players.
	 * Should only be called when the matchmaking status is waiting on others
	 *
	 * @param levelNum The level number to start
	 */
	void startGame(int levelNum);

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
	void createBreach(float angle, int player, int id);

	/**
	 * Inform other players that a breach has shrunk in size by 1.
	 *
	 * @param id The breach ID
	 */
	void resolveBreach(int id);

	/**
	 * Inform other players that a task requiring two players has been created
	 * (eg: locked doors from nondigital)
	 *
	 * @param angle The angle of the ship to spawn the task
	 * @param player1 The player ID of one player that can resolve this task or -1 for anyone
	 * @param player2 The player ID of the second player that can resolve this task or -1 for anyone
	 * @param id The dual-task ID used to identify this task in the future
	 */
	void createDualTask(float angle, int player1, int player2, int id);

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
	void flagDualTask(int id, int player, int flag);

	/**
	 * Inform other players that two buttons requiring two players has been created
	 *
	 * @param angle1 The angle of the ship to spawn the button
	 * @param id1 The button ID
	 * @param angle2 The angle of the ship to spawn the button's pair
	 * @param id2 The ID for the button's pair
	 */
	void createButtonTask(float angle1, int id1, float angle2, int id2);

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
	void flagButton(int id, int player, int flag);

	/**
	 * Inform other players that a pair of buttons have been resolved.
	 *
	 * @param id The ID of one of the two buttons.
	 */
	void resolveButton(int id);

	/**
	 * Inform other players that a task requiring all members of the ship has been created (eg:
	 * stabilizer malfunction)
	 *
	 * @param player The player whose screen this message will appear on
	 * @param data Data representing the nature of this task. This data will be passed directly into
	 *             ShipModel on the receiving player's end.
	 */
	void createAllTask(int player, int data);

	/**
	 * Inform the host that a task requiring all members of the ship has failed, and thus to deduct
	 * appropriate health
	 */
	void failAllTask();

	/**
	 * Inform other players that a player has initiated a jump.
	 *
	 * @param player The player ID who is jumping
	 */
	void jump(int player);

	/**
	 * Disconnect this player from the server, by force.
	 */
	void forceDisconnect();
};

#endif /* __NETWORK_CONTROLLER_H__ */
