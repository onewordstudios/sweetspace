//
// CUAdHocNetworkConnection.h
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
#ifndef ADHOC_NETWORK_CONNECTION_H
#define ADHOC_NETWORK_CONNECTION_H

#include "CUNetworkConnection.h"
#include "libraries/SLikeNet/slikenet/BitStream.h"
#include "libraries/SLikeNet/slikenet/MessageIdentifiers.h"
#include "libraries/SLikeNet/slikenet/NatPunchthroughClient.h"

// Forward declarations
namespace SLNet {
class RakPeerInterface;
}; // namespace SLNet

namespace cugl {
/**
 * Network connection to other players with an adhoc implementation.
 */
class AdHocNetworkConnection : public NetworkConnection {
   public:
#pragma region Setup
	explicit AdHocNetworkConnection(ConnectionConfig config);

	AdHocNetworkConnection(ConnectionConfig config, std::string roomID);

	~AdHocNetworkConnection() override;
#pragma endregion

#pragma region Main Networking Methods
	void send(const std::vector<uint8_t>& msg) override;

	void sendOnlyToHost(const std::vector<uint8_t>& msg) override;

	void receive(const std::function<void(const std::vector<uint8_t>&)>& dispatcher) override;

	void manualDisconnect() override;
#pragma endregion

#pragma region State Management
	void startGame() override;
#pragma endregion

#pragma region Getters
	NetStatus getStatus() const override;

	tl::optional<uint8_t> getPlayerID() const override { return playerID; }

	std::string getRoomID() const override { return roomID; }

	bool isPlayerActive(uint8_t playerID) const override { return connectedPlayers.test(playerID); }

	uint8_t getNumPlayers() const override { return numPlayers; }

	uint8_t getTotalPlayers() const override { return maxPlayers; }
#pragma endregion

   private:
	static constexpr uint8_t DEFAULT_MAX_PLAYERS = 6;
	static constexpr size_t ONE_BYTE = 256;

	/** Connection object */
	std::unique_ptr<SLNet::RakPeerInterface> peer;

#pragma region State
	/** Current status */
	NetStatus status;
	/** API version number */
	const uint8_t apiVer; // NOLINT
	/** Number of players currently connected */
	uint8_t numPlayers;
	/** Number of players connected when the game started */
	uint8_t maxPlayers;
	/** Current player ID */
	tl::optional<uint8_t> playerID;
	/** Connected room ID */
	std::string roomID;
	/** Which players are active */
	std::bitset<ONE_BYTE> connectedPlayers;
#pragma endregion

#pragma region Punchthrough
	/** Address of punchthrough server */
	std::unique_ptr<SLNet::SystemAddress> natPunchServerAddress;
	/** NAT Punchthrough Client */
	SLNet::NatPunchthroughClient natPunchthroughClient;
#pragma endregion

#pragma region Connection Data Structures
	struct HostPeers {
		/** Whether the game has started */
		bool started;
		/** Maximum number of players to allow in this game (NOT the max that was in this room) */
		uint32_t maxPlayers;
		/** Addresses of all connected players */
		std::vector<std::unique_ptr<SLNet::SystemAddress>> peers;
		/** Addresses of all players to reject */
		std::unordered_set<std::string> toReject;

		HostPeers() : started(false), maxPlayers(DEFAULT_MAX_PLAYERS) {
			for (uint8_t i = 0; i < DEFAULT_MAX_PLAYERS - 1; i++) {
				peers.push_back(nullptr);
			}
		};
		explicit HostPeers(uint32_t max) : started(false), maxPlayers(max) {
			for (uint8_t i = 0; i < max - 1; i++) {
				peers.push_back(nullptr);
			}
		};
	};

	/** Connection to host and room ID for client NOLINTNEXTLINE */
	struct ClientPeer {
		std::unique_ptr<SLNet::SystemAddress> addr;
		std::string room;

		// NOLINTNEXTLINE
		explicit ClientPeer(std::string roomID) : room(std::move(roomID)) {}
	};

	/**
	 * Collection of peers for the host, or the host for clients
	 */
	mapbox::util::variant<HostPeers, ClientPeer> remotePeer;
#pragma endregion

	enum CustomDataPackets {
		Standard = 0,
		AssignedRoom,
		// Request to join, or success
		JoinRoom,
		// Couldn't find room
		JoinRoomFail,
		Reconnect,
		PlayerJoined,
		PlayerLeft,
		StartGame,
		DirectToHost
	};

#pragma region Connection Handshake
	ConnectionConfig config;

	/*
	===============================
	 Connection Handshake Overview
	===============================

			Host		Punchthrough Server			Client
			====		===================			======
	c0		Connect ------------->
	ch1		  <--------- Conn Req Accepted
			  <--------- Room ID Assigned
	ch2		Accept Req

	c0							 <----------------- Connect
						 Conn Req Accepted ------------>
	cc1							 <----------------- Try connect to host
			  <--------- Punch Succeeded -------------->
	cc2												Save host address
	cc3		Check hasRoom
			Connect ----------------------------------->
	cc4		  <------------------------------------ Incoming connection
	cc5		Request Accepted -------------------------->
	cc6												Join Room

	*/

	/** Step 0: Connect to punchthrough server (both client and host) */
	void c0StartupConn();

	/** Host Step 1: Server connection established */
	void ch1HostConnServer(HostPeers& h);
	/** Host Step 2: Server gave room ID to host; awaiting incoming connections */
	void ch2HostGetRoomID(HostPeers& h, SLNet::BitStream& bts);

	/** Client Step 1: Server connection established; request punchthrough to host from server */
	void cc1ClientConnServer(ClientPeer& c);
	/** Client Step 2: Client received successful punchthrough from server */
	void cc2ClientPunchSuccess(ClientPeer& c, SLNet::Packet* packet);
	/** Client Step 3: Host received successful punchthrough request passed through from server */
	void cc3HostReceivedPunch(HostPeers& h, SLNet::Packet* packet);
	/** Client Step 4: Client received direct connection request from host */
	void cc4ClientReceiveHostConnection(ClientPeer& c, SLNet::Packet* packet);
	/** Client Step 5: Host received confirmation of connection from client */
	void cc5HostConfirmClient(HostPeers& h, SLNet::Packet* packet);
	/** Client Step 6: Client received player ID from host and API */
	void cc6ClientAssignedID(ClientPeer& c, const std::vector<uint8_t>& msgConverted);
	/** Client Step 7: Host received confirmation of game data from client; connection finished */
	void cc7HostGetClientData(HostPeers& h, SLNet::Packet* packet,
							  const std::vector<uint8_t>& msgConverted);

	/** Reconnect Step 1: Picks up after client step 5; host sent reconn data to client */
	void cr1ClientReceivedInfo(ClientPeer& c, const std::vector<uint8_t>& msgConverted);
	/** Reconnect Step 2: Host received confirmation of game data from client */
	void cr2HostGetClientResp(HostPeers& h, SLNet::Packet* packet,
							  const std::vector<uint8_t>& msgConverted);

#pragma endregion

	/**
	 * Broadcast a message to everyone except the specified connection.
	 *
	 * PRECONDITION: This player MUST be the host
	 *
	 * @param packetType Packet type from RakNet
	 * @param msg The message to send
	 * @param ignore The address to not send to
	 */
	void broadcast(const std::vector<uint8_t>& msg, SLNet::SystemAddress& ignore,
				   CustomDataPackets packetType = Standard);

	void send(const std::vector<uint8_t>& msg, CustomDataPackets packetType);

	/**
	 * Send a message to just one connection.
	 *
	 * @param msg The message to send
	 * @param packetType The type of custom data packet
	 * @param dest Desination address
	 */
	void directSend(const std::vector<uint8_t>& msg, CustomDataPackets packetType,
					SLNet::SystemAddress dest);

	/** Last reconnection attempt time, or none if n/a */
	tl::optional<time_t> lastReconnAttempt;
	/** Time when disconnected, or none if connected */
	tl::optional<time_t> disconnTime;

	/**
	 * Attempt to reconnect to the host.
	 *
	 * PRECONDITION: Must be called by client when in reconnecting phase.
	 * A successful connection must have previously been established.
	 */
	void attemptReconnect();
};
}; // namespace cugl

#endif // ADHOC_NETWORK_CONNECTION_H