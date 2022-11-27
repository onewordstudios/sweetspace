//
// CUNetworkConnection.h
//
// Author: Michael Xing
// Version: 2/8/2022
//
// With help from onewordstudios:
// - Demi Chang
// - Aashna Saxena
// - Sam Sorenson
// - Michael Xing
// - Jeffrey Yao
// - Wendy Zhang
// https://onewordstudios.com/
//
// With thanks to the students of CS 4152 Spring 2021
// for beta testing this class.
//
#ifndef CU_NETWORK_CONNECTION_H
#define CU_NETWORK_CONNECTION_H

#include <array>
#include <bitset>
#include <ctime>
#include <functional>
#include <mapbox/variant.hpp>
#include <string>
#include <thread>
#include <tl/optional.hpp>
#include <unordered_set>
#include <vector>

// Forward declarations
namespace SLNet {
class RakPeerInterface;
}; // namespace SLNet

namespace cugl {
/**
 * Network connection to other players with a peer-to-peer interface.
 *
 * The premise of this class is to make networking as simple as possible. Simply call
 * send() with a byte vector, and then all others will receive it when they call receive().
 *
 * This class maintains a networked game using an ad-hoc server setup, but provides
 * an interface that acts like it is peer-to-peer.
 *
 * The "host" is the server, and is the one all others are connected to. The "clients" are
 * other players connected to the ad-hoc server. However, any messages sent are relayed
 * by the host to all other players too, so the interface appears peer-to-peer.
 *
 * You can instead choose to use this as a true client-server by just checking the player ID.
 * Player ID 0 is the host, and all others are clients connected to the host. Calling send()
 * from the host works as usual; as a client, you may use sendOnlyToHost() in lieu of send()
 * to only send a message to the host that will not be broadcast to other players. Both the host
 * and clients receive messages via receive() as usual. Note that the host on the receiving end
 * does not know if a message was sent via send() or sendOnlyToHost().
 *
 * This class does support automatic reconnections, but does NOT support host migration.
 * If the host drops offline, the connection is closed.
 */
class NetworkConnection {
   public:
#pragma region Setup
	/**
	 * Basic data needed to setup a connection.
	 *
	 * To setup a NAT punchthrough server of your own, see:
	 * https://github.com/mt-xing/nat-punchthrough-server
	 */
	struct ConnectionConfig {
		/** Address of the NAT Punchthrough server */
		const char* punchthroughServerAddr;
		/** Port to connect on the NAT Punchthrough server */
		uint16_t punchthroughServerPort;
		/** Port to connect on the backup server */
		uint16_t fallbackServerPort;
		/** Maximum number of players allowed per game (including host) */
		uint32_t maxNumPlayers;
		/**
		 * API version number; clients with mismatched versions will be prevented
		 * from connecting to each other. Start this at 0 and increment it every
		 * time a backwards incompatible API change happens.
		 */
		uint8_t apiVersion;

		constexpr ConnectionConfig(const char* punchthroughServerAddr,
								   uint16_t punchthroughServerPort, uint16_t fallbackServerPort,
								   uint32_t maxPlayers, uint8_t apiVer) noexcept
			: punchthroughServerAddr(punchthroughServerAddr),
			  punchthroughServerPort(punchthroughServerPort),
			  fallbackServerPort(fallbackServerPort),
			  maxNumPlayers(maxPlayers),
			  apiVersion(apiVer) {}
	};

	/**
	 * Start a new network connection as host.
	 *
	 * This will automatically connect to the NAT punchthrough server and request a room ID.
	 * This process is NOT instantaneous. Wait for getStatus() to return CONNECTED.
	 * Once it does, getRoomID() will return your assigned room ID.
	 *
	 * You will likely want to access this class through a smart pointer, which you can
	 * easily make by calling std::make_shared<cugl::NetworkConnection>(config);
	 *
	 * @param setup Connection config
	 * @returns A pointer to the network connection
	 */
	static std::unique_ptr<NetworkConnection> newHostConnection(ConnectionConfig config);

	/**
	 * Start a new network connection as client.
	 *
	 * This will automatically connect to the NAT punchthrough server and then try
	 * to connect to the host with the given ID.
	 * This process is NOT instantaneous. Wait for getStatus() to return CONNECTED.
	 * Once it does, getPlayerID() will return your assigned player ID.
	 *
	 * You will likely want to access this class through a smart pointer, which you can
	 * easily make by calling std::make_shared<cugl::NetworkConnection>(config, roomID);
	 *
	 * @param setup Connection config
	 * @param roomID Host's assigned Room ID
	 * @returns A pointer to the network connection
	 */
	static std::unique_ptr<NetworkConnection> newClientConnection(ConnectionConfig config,
																  std::string roomID);
#pragma endregion

#pragma region Main Networking Methods
	/**
	 * Sends a byte array to all other players.
	 *
	 * Within a few frames, other players should receive this via a call to receive().
	 *
	 * This requires a connection be established. Otherwise its behavior is undefined.
	 *
	 * You may choose to either send a byte array directly, or you can use the NetworkSerializer
	 * and NetworkDeserializer classes to encode more complex data.
	 *
	 * @param msg The byte array to send.
	 */
	virtual void send(const std::vector<uint8_t>& msg) = 0;

	/**
	 * Sends a byte array to the host only.
	 *
	 * Only useful when called from a client (player ID != 0). As host, this is a no-op.
	 *
	 * Within a few frames, the host should receive this via a call to receive().
	 *
	 * This requires a connection be established. Otherwise its behavior is undefined.
	 *
	 * You may choose to either send a byte array directly, or you can use the NetworkSerializer
	 * and NetworkDeserializer classes to encode more complex data.
	 *
	 * @param msg The byte array to send.
	 */
	virtual void sendOnlyToHost(const std::vector<uint8_t>& msg) = 0;

	/**
	 * Method to call every network frame to process incoming network messages.
	 *
	 * A network frame can, but need not be, the same as a render frame. However,
	 * during the network connection phase, before the game starts, this method should
	 * be called every frame. Otherwise, the NAT Punchthrough library may fail.
	 *
	 * This method must be called periodically EVEN BEFORE A CONNECTION IS ESTABLISHED.
	 * Otherwise, the library has no way to receive and process incoming connections
	 *
	 * @param dispatcher Function that will be called on every received byte array since the last
	 * call to receive(). However, if the original message was sent using NetworkSerializer,
	 * you should be using NetworkDeserializer to deserialize it.
	 */
	virtual void receive(const std::function<void(const std::vector<uint8_t>&)>& dispatcher) = 0;

	/**
	 * Manually disconnect from the server, while keeping the initialization state.
	 */
	virtual void manualDisconnect() = 0;
#pragma endregion

#pragma region State Management
	/**
	 * Mark the game as started and ban incoming connections except for reconnects.
	 *
	 * PRECONDITION: Can only be called by the host.
	 */
	virtual void startGame() = 0;

	/**
	 * Potential states the network connection could be in
	 */
	enum class NetStatus {
		// No connection
		Disconnected,
		// If host, waiting on Room ID from server; if client, waiting on Player ID from host
		Pending,
		// If host, accepting connections; if client, successfully connected to host
		Connected,
		// Lost connection, attempting to reconnect (failure causes disconnection)
		Reconnecting,
		// Room ID does not exist, or room is already full
		RoomNotFound,
		// API version numbers do not match between host, client, and Punchthrough Server
		// (when running your own punchthrough server, you can specify a minimum API version
		// that your server will require, or else it will reject the connection.
		// If you're using my demo server, that minimum is 0.
		ApiMismatch,
		// Something went wrong and IDK what :(
		GenericError
	};
#pragma endregion

#pragma region Getters
	/**
	 * The current status of this network connection.
	 */
	virtual NetStatus getStatus() const = 0;

	/**
	 * Returns the player ID or empty.
	 *
	 * If this player is the host, this is guaranteed to be 0, even before a connection is
	 * established.
	 *
	 * Otherwise, as client, this will return empty until connected to host and a player ID is
	 * assigned.
	 */
	virtual tl::optional<uint8_t> getPlayerID() const = 0;

	/**
	 * Returns the room ID or empty string.
	 *
	 * If this player is a client, this will return the room ID this object was constructed with.
	 *
	 * Otherwise, as host, this will return the empty string until connected to the punchthrough
	 * server and a room ID is assigned.
	 */
	virtual std::string getRoomID() const = 0;

	/**
	 * Returns true if the given player ID is currently connected to the game.
	 *
	 * Does not return meaningful data until a connection is established.
	 *
	 * As a client, if disconnected from host, player ID 0 will return disconnected.
	 */
	virtual bool isPlayerActive(uint8_t playerID) const = 0;

	/** Return the number of players currently connected to this game */
	virtual uint8_t getNumPlayers() const = 0;

	/** Return the number of players present when the game was started
	 *  (including players that may have disconnected) */
	virtual uint8_t getTotalPlayers() const = 0;
#pragma endregion
};
}; // namespace cugl

#endif // CU_NETWORK_CONNECTION_H