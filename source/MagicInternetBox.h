#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

#include <cugl/cugl.h>

#include "libraries/easywsclient.hpp"

class MagicInternetBox {
   private:
	/**
	 * The actual websocket connection
	 */
	easywsclient::WebSocket::pointer ws;

   public:
	/**
	 * Create an empty Network Controller instance. Does no initialization.
	 * Call one of the init methods to connect and stuff.
	 */
	MagicInternetBox(){};

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
	 * Disconnect from the room and cleanup this object.
	 */
	void leaveRoom();

	/**
	 * Returns the room ID this controller is connected to.
	 * Is the empty string if this controller is uninitialized.
	 */
	std::string getRoomID();

	/**
	 * Returns the current player ID, or -1 if uninitialized.
	 * 0 is the host player.
	 */
	int getPlayerID();

	/**
	 * Update method called every frame.
	 * This controller will batch and handle network communication as long as this method is called.
	 * TODO: Pass ship model into this method after it is created.
	 */
	void update();

	/**
	 * Inform other players that a new breach has been created.
	 *
	 * @param angle The angle of the ship to spawn the breach
	 * @param player The player ID that can resolve this breach
	 * @param id The breach ID used to identify this breach in the future
	 */
	void createBreach(float angle, int player, int id);

	/**
	 * Inform other players that a breach has been reseolved
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
	 */
	void flagDualTask(int id);
};

#endif /* __NETWORK_CONTROLLER_H__ */
