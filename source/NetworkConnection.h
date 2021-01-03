#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include <array>
#include <functional>
#include <mapbox/variant.hpp>
#include <string>
#include <thread>
#include <vector>

#include "Globals.h"
#include "libraries/RakNet/BitStream.h"
#include "libraries/RakNet/MessageIdentifiers.h"
#include "libraries/RakNet/NatPunchthroughClient.h"
#include "libraries/RakNet/RakNetTypes.h"
#include "libraries/RakNet/RakPeerInterface.h"

class NetworkConnection {
   public:
	/**
	 * Start a new network connection as host.
	 */
	NetworkConnection();

	/**
	 * Start a new network connection as client.
	 *
	 * @param roomID The RakNet GUID of the host.
	 */
	explicit NetworkConnection(std::string roomID);

	/** Delete and cleanup this connection. */
	~NetworkConnection();

	void send(const std::vector<uint8_t>& msg);

	void receive(const std::function<void(const std::vector<uint8_t>&)>& dispatcher);

   private:
	/** Connection object */
	std::unique_ptr<RakNet::RakPeerInterface> peer;

#pragma region Punchthrough
	/** Address of punchthrough server */
	std::unique_ptr<RakNet::SystemAddress> natPunchServerAddress;
	/** NAT Punchthrough Client */
	RakNet::NatPunchthroughClient natPunchthroughClient;
#pragma endregion

#pragma region Connection Data Structures
	struct HostPeers {
		uint8_t numPlayers;
		std::array<std::unique_ptr<RakNet::SystemAddress>, globals::MAX_PLAYERS - 1> peers;

		HostPeers() { numPlayers = 1; };
	};

	/** Connection to host and room ID for client */
	struct ClientPeer {
		std::unique_ptr<RakNet::SystemAddress> addr;
		std::string room;

		explicit ClientPeer(std::string roomID) { room = std::move(roomID); }
	};

	/**
	 * Collection of peers for the host, or the host for clients
	 */
	mapbox::util::variant<HostPeers, ClientPeer> remotePeer;
#pragma endregion

	/** Initialize the connection */
	void startupConn();

	/**
	 * Broadcast a message to everyone except the specified connection.
	 *
	 * PRECONDITION: This player MUST be the host
	 *
	 * @param msg The message to send
	 * @param ignore The address to not send to
	 */
	void broadcast(const std::vector<uint8_t>& msg, RakNet::SystemAddress& ignore);
};

#endif /* NETWORK_CONNECTION_H */
