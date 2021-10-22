#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include <array>
#include <functional>
#include <mapbox/variant.hpp>
#include <string>
#include <thread>
#include <vector>

#include "Globals.h"
#include "libraries/SLikeNet/slikenet/BitStream.h"
#include "libraries/SLikeNet/slikenet/MessageIdentifiers.h"
#include "libraries/SLikeNet/slikenet/NatPunchthroughClient.h"
#include "libraries/SLikeNet/slikenet/peerinterface.h"

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

	/**
	 * Mark the game as started and ban incoming connections except for reconnects.
	 * PRECONDITION: Should only be called by host.
	 */
	void startGame();

   private:
	/** Connection object */
	std::unique_ptr<SLNet::RakPeerInterface> peer;

#pragma region Punchthrough
	/** Address of punchthrough server */
	std::unique_ptr<SLNet::SystemAddress> natPunchServerAddress;
	/** NAT Punchthrough Client */
	SLNet::NatPunchthroughClient natPunchthroughClient;
#pragma endregion

#pragma region Connection Data Structures
	struct HostPeers {
		bool started;
		uint8_t numPlayers;
		std::array<std::unique_ptr<SLNet::SystemAddress>, globals::MAX_PLAYERS - 1> peers;

		HostPeers() : started(false), numPlayers(1){};
	};

	/** Connection to host and room ID for client */
	struct ClientPeer {
		std::unique_ptr<SLNet::SystemAddress> addr;
		std::string room;

		explicit ClientPeer(std::string roomID) : room(roomID) {}
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
	void broadcast(const std::vector<uint8_t>& msg, SLNet::SystemAddress& ignore);
};

#endif /* NETWORK_CONNECTION_H */
