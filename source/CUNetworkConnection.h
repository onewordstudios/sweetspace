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
#include <string>
#include <thread>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_set>

#include <slikenet/BitStream.h>
#include <slikenet/MessageIdentifiers.h>
#include <slikenet/NatPunchthroughClient.h>

// Forward declarations
namespace SLNet {
	class RakPeerInterface;
}

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
			/** Maximum number of players allowed per game (including host) */
			uint32_t maxNumPlayers;
			/**
			 * API version number; clients with mismatched versions will be prevented
			 * from connecting to each other. Start this at 0 and increment it every
			 * time a backwards incompatible API change happens.
			 */
			uint8_t apiVersion;

			ConnectionConfig(const char* punchthroughServerAddr, uint16_t punchthroughServerPort, uint32_t maxPlayers, uint8_t apiVer) {
				this->punchthroughServerAddr = punchthroughServerAddr;
				this->punchthroughServerPort = punchthroughServerPort;
				this->maxNumPlayers = maxPlayers;
				this->apiVersion = apiVer;
			}
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
		 */
		explicit NetworkConnection(ConnectionConfig config);

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
		 */
		NetworkConnection(ConnectionConfig config, std::string roomID);

		/** Delete and cleanup this connection. */
		~NetworkConnection();
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
		void send(const std::vector<uint8_t>& msg);

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
		void sendOnlyToHost(const std::vector<uint8_t>& msg);

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
		void receive(const std::function<void(const std::vector<uint8_t>&)>& dispatcher);
#pragma endregion

#pragma region State Management
		/**
		 * Mark the game as started and ban incoming connections except for reconnects.
		 * 
		 * PRECONDITION: Can only be called by the host.
		 */
		void startGame();

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
		NetStatus getStatus();

		/**
		 * Returns the player ID or empty.
		 * 
		 * If this player is the host, this is guaranteed to be 0, even before a connection is established.
		 * 
		 * Otherwise, as client, this will return empty until connected to host and a player ID is assigned.
		 */
		std::optional<uint8_t> getPlayerID() { return playerID; }

		/**
		 * Returns the room ID or empty string.
		 * 
		 * If this player is a client, this will return the room ID this object was constructed with.
		 * 
		 * Otherwise, as host, this will return the empty string until connected to the punchthrough server
		 * and a room ID is assigned.
		 */
		std::string getRoomID() { return roomID; }

		/**
		 * Returns true if the given player ID is currently connected to the game.
		 * 
		 * Does not return meaningful data until a connection is established.
		 * 
		 * As a client, if disconnected from host, player ID 0 will return disconnected.
		 */
		bool isPlayerActive(uint8_t playerID) { return connectedPlayers.test(playerID); }

		/** Return the number of players currently connected to this game */
		uint8_t getNumPlayers() { return numPlayers; }

		/** Return the number of players present when the game was started
		 *  (including players that may have disconnected) */
		uint8_t getTotalPlayers() { return maxPlayers;  }
#pragma endregion

	private:
		/** Connection object */
		std::unique_ptr<SLNet::RakPeerInterface> peer;

#pragma region State
		/** Current status */
		NetStatus status;
		/** API version number */
		const uint8_t apiVer;
		/** Number of players currently connected */
		uint8_t numPlayers;
		/** Number of players connected when the game started */
		uint8_t maxPlayers;
		/** Current player ID */
		std::optional<uint8_t> playerID;
		/** Connected room ID */
		std::string roomID;
		/** Which players are active */
		std::bitset<256> connectedPlayers;
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

			HostPeers() : started(false), maxPlayers(6) {
				for (uint8_t i = 0; i < 5; i++) {
					peers.push_back(nullptr);
				}
			};
			explicit HostPeers(uint32_t max) : started(false), maxPlayers(max) {
				for (uint8_t i = 0; i < max - 1; i++) {
					peers.push_back(nullptr);
				}
			};
		};

		/** Connection to host and room ID for client */
		struct ClientPeer {
			std::unique_ptr<SLNet::SystemAddress> addr;
			std::string room;

			explicit ClientPeer(std::string roomID) { room = std::move(roomID); }
		};

		/**
		 * Collection of peers for the host, or the host for clients
		 */
		std::variant<HostPeers, ClientPeer> remotePeer;
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
		void cc7HostGetClientData(HostPeers& h, SLNet::Packet* packet, const std::vector<uint8_t>& msgConverted);

		/** Reconnect Step 1: Picks up after client step 5; host sent reconn data to client */
		void cr1ClientReceivedInfo(ClientPeer& c, const std::vector<uint8_t>& msgConverted);
		/** Reconnect Step 2: Host received confirmation of game data from client */
		void cr2HostGetClientResp(HostPeers& h, SLNet::Packet* packet, const std::vector<uint8_t>& msgConverted);

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
		void directSend(const std::vector<uint8_t>& msg, CustomDataPackets packetType, SLNet::SystemAddress dest);

		/** Last reconnection attempt time, or none if n/a */
		std::optional<time_t> lastReconnAttempt;
		/** Time when disconnected, or none if connected */
		std::optional<time_t> disconnTime;

		/**
		 * Attempt to reconnect to the host.
		 * 
		 * PRECONDITION: Must be called by client when in reconnecting phase.
		 * A successful connection must have previously been established.
		 */
		void attemptReconnect();

	};
}

#endif // CU_NETWORK_CONNECTION_H