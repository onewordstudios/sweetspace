#include <cugl/net/CUNetworkConnection.h>

#include <cugl/cugl.h>

#include <utility>


#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <fcntl.h>
#pragma comment(lib, "ws2_32")
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef _SSIZE_T_DEFINED
typedef int ssize_t;
#define _SSIZE_T_DEFINED
#endif
#ifndef _SOCKET_T_DEFINED
typedef SOCKET socket_t;
#define _SOCKET_T_DEFINED
#endif
#endif

#include <slikenet/peerinterface.h>



using namespace cugl;

template <class... Fs>
struct overload;

template <class F0, class... Frest>
struct overload<F0, Frest...> : F0, overload<Frest...>
{
	overload(F0 f0, Frest... rest) : F0(f0), overload<Frest...>(rest...) {}

	using F0::operator();
	using overload<Frest...>::operator();
};

template <class F0>
struct overload<F0> : F0
{
	overload(F0 f0) : F0(f0) {}

	using F0::operator();
};

template <class... Fs>
auto make_visitor(Fs... fs)
{
	return overload<Fs...>(fs...);
}

/** How long to block on shutdown */
constexpr unsigned int SHUTDOWN_BLOCK = 10;

/** Length of room IDs */
constexpr uint8_t ROOM_LENGTH = 5;

/** How long to wait before considering ourselves disconnected (ms) */
constexpr size_t DISCONN_TIME = 5000;

/** How long to wait between reconnection attempts (seconds) */
constexpr size_t RECONN_GAP = 3;

/** How long to wait before giving up on reconnection (seconds) */
constexpr size_t RECONN_TIMEOUT = 15;

NetworkConnection::NetworkConnection(ConnectionConfig config)
	: status(NetStatus::Pending), apiVer(config.apiVersion), numPlayers(1), maxPlayers(1), playerID(0), config(config) {
	c0StartupConn();
	remotePeer = HostPeers(config.maxNumPlayers);
}

NetworkConnection::NetworkConnection(ConnectionConfig config, std::string roomID)
	: status(NetStatus::Pending), apiVer(config.apiVersion), numPlayers(1), maxPlayers(0), config(config) {
	c0StartupConn();
	remotePeer = ClientPeer(std::move(roomID));
	peer->SetMaximumIncomingConnections(1);
}

NetworkConnection::~NetworkConnection() {
	peer->Shutdown(SHUTDOWN_BLOCK);
	SLNet::RakPeerInterface::DestroyInstance(peer.release());
}

/**
 * Read the message from a bitstream into a byte vector.
 *
 * Only works if the BitStream was encoded in the standard format used by this clas.
 */
std::vector<uint8_t> readBs(SLNet::BitStream& bts) {
	uint8_t ignored;
	bts.Read(ignored);
	uint8_t length;
	bts.Read(length);

	std::vector<uint8_t> msgConverted;
	msgConverted.resize(length, 0);

	bts.ReadAlignedBytes(msgConverted.data(), length);
	return msgConverted;
}

#pragma region Connection Handshake

void NetworkConnection::c0StartupConn() {
	peer = std::unique_ptr<SLNet::RakPeerInterface>(SLNet::RakPeerInterface::GetInstance());

	peer->SetTimeoutTime(DISCONN_TIME, SLNet::UNASSIGNED_SYSTEM_ADDRESS);

	peer->AttachPlugin(&(natPunchthroughClient));
	natPunchServerAddress = std::make_unique<SLNet::SystemAddress>(
		SLNet::SystemAddress(config.punchthroughServerAddr, config.punchthroughServerPort));

	// Use the default socket descriptor
	// This will make the OS assign us a random port.
	SLNet::SocketDescriptor socketDescriptor;
	// Allow connections for each player and one for the NAT server.
	peer->Startup(config.maxNumPlayers, &socketDescriptor, 1);

	CULog("Your GUID is: %s",
		peer->GetGuidFromSystemAddress(SLNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());

	// Connect to the NAT Punchthrough server
	CULog("Connecting to punchthrough server");
	peer->Connect(this->natPunchServerAddress->ToString(false),
		this->natPunchServerAddress->GetPort(), nullptr, 0);
}

void cugl::NetworkConnection::ch1HostConnServer(HostPeers& h) {
	CULog("Connected to punchthrough server; awaiting room ID");
}

void cugl::NetworkConnection::ch2HostGetRoomID(HostPeers& h, SLNet::BitStream& bts) {
	auto msgConverted = readBs(bts);
	std::stringstream newRoomId;
	for (size_t i = 0; i < ROOM_LENGTH; i++) {
		newRoomId << static_cast<char>(msgConverted[i]);
	}
	connectedPlayers.set(0);
	roomID = newRoomId.str();
	CULog("Got room ID: %s; Accepting Connections Now", roomID.c_str());
	status = NetStatus::Connected;
}

void cugl::NetworkConnection::cc1ClientConnServer(ClientPeer& c) {
	CULog("Connected to punchthrough server");
	CULog("Trying to connect to %s", c.room.c_str());
	SLNet::RakNetGUID remote;
	remote.FromString(c.room.c_str());
	this->natPunchthroughClient.OpenNAT(remote,
		*(this->natPunchServerAddress));
}

void cugl::NetworkConnection::cc2ClientPunchSuccess(ClientPeer& c, SLNet::Packet* packet) {
	c.addr = std::make_unique<SLNet::SystemAddress>(packet->systemAddress);
}

void cugl::NetworkConnection::cc3HostReceivedPunch(HostPeers& h, SLNet::Packet* packet) {
	auto p = packet->systemAddress;
	CULog("Host received punchthrough; curr num players %d", peer->NumberOfConnections());



	bool hasRoom = false;
	if (!h.started || numPlayers < maxPlayers) {
		for (uint8_t i = 0; i < h.peers.size(); i++) {
			if (h.peers.at(i) == nullptr) {
				hasRoom = true;
				h.peers.at(i) = std::make_unique<SLNet::SystemAddress>(p);
				break;
			}
		}
	}

	if (!hasRoom) {
		// Client is still waiting for a response at this stage,
		// so we need to connect to them first before telling them no.
		// Store address to reject so we know this connection is invalid.
		h.toReject.insert(p.ToString());
		CULog("Client attempted to join but room was full");
	}

	CULog("Connecting to client now");
	peer->Connect(p.ToString(false), p.GetPort(), nullptr, 0);
}

void cugl::NetworkConnection::cc4ClientReceiveHostConnection(ClientPeer& c, SLNet::Packet* packet) {
	if (packet->systemAddress == *c.addr) {
		CULog("Connected to host :D");
	}
}

void cugl::NetworkConnection::cc5HostConfirmClient(HostPeers& h, SLNet::Packet* packet) {

	if (h.toReject.count(packet->systemAddress.ToString()) > 0) {
		CULog("Rejecting player connection - bye :(");

		h.toReject.erase(packet->systemAddress.ToString());

		directSend({}, JoinRoomFail, packet->systemAddress);

		peer->CloseConnection(packet->systemAddress, true);
		return;
	}

	for (uint8_t i = 0; i < h.peers.size(); i++) {
		if (*h.peers.at(i) == packet->systemAddress) {
			uint8_t pID = i + 1;
			CULog("Player %d accepted connection request", pID);

			if (h.started) {
				// Reconnection attempt
				directSend({ static_cast<uint8_t>(numPlayers + 1), maxPlayers, pID, apiVer }, Reconnect, packet->systemAddress);
			}
			else {
				// New player connection
				maxPlayers++;
				directSend(
					{ static_cast<uint8_t>(numPlayers + 1), maxPlayers, pID, apiVer },
					JoinRoom,
					packet->systemAddress
				);
			}
			break;
		}
	}

	CULog("Host confirmed players; curr connections %d", peer->NumberOfConnections());
}

void cugl::NetworkConnection::cc6ClientAssignedID(ClientPeer& c, const std::vector<uint8_t>& msgConverted) {
	bool apiMatch = msgConverted[3] == apiVer;
	if (!apiMatch) {
		CULogError("API version mismatch; currently %d but host was %d", apiVer,
			msgConverted[3]);
		status = NetStatus::ApiMismatch;
	} else {
		numPlayers = msgConverted[0];
		maxPlayers = msgConverted[1];
		playerID = msgConverted[2];
		status = NetStatus::Connected;
	}

	peer->CloseConnection(*natPunchServerAddress, true);

	directSend({ *playerID, (uint8_t)(apiMatch ? 1 : 0) }, JoinRoom, *c.addr);
}

void cugl::NetworkConnection::cc7HostGetClientData(
	HostPeers& h, SLNet::Packet* packet, const std::vector<uint8_t>& msgConverted
) {

	for (uint8_t i = 0; i < h.peers.size(); i++) {
		if (*h.peers.at(i) == packet->systemAddress) {
			uint8_t pID = i + 1;
			CULog("Host verifying player %d connection info", pID);

			if (pID != msgConverted[0]) {
				CULog("Player ID mismatch; client reported id %d; disconnecting", msgConverted[0]);
				peer->CloseConnection(packet->systemAddress, true);
				return;
			}

			if (msgConverted[1] == 0) {
				CULog("Client %d reported outdated API or other issue; disconnecting", pID);
				peer->CloseConnection(packet->systemAddress, true);
				return;
			}

			CULog("Player id %d was successfully verified; connection handshake complete", pID);
			connectedPlayers.set(pID);
			std::vector<uint8_t> joinMsg = { pID };
			broadcast(joinMsg, packet->systemAddress, PlayerJoined);
			numPlayers++;

			return;
		}
	}

	// If we make it here, we somehow got a connection to an unknown address
	CULogError("Unknown connection target; disconnecting");
	peer->CloseConnection(packet->systemAddress, true);

}

void cugl::NetworkConnection::cr1ClientReceivedInfo(ClientPeer& c, const std::vector<uint8_t>& msgConverted) {

	CULog("Reconnection Progress: Received data from host");

	

	bool success = msgConverted[3] == apiVer;
	if (!success) {
		CULogError("API version mismatch; currently %d but host was %d", apiVer,
			msgConverted[3]);
		status = NetStatus::ApiMismatch;
	} else if (status != NetStatus::Reconnecting) {
		CULogError("But we're not trying to reconnect. Failure.");
		success = false;
	} else if (playerID != msgConverted[2]) {
		CULogError("Invalid reconnection target; we are player ID %d but host thought we were %d",
			playerID.has_value() ? *playerID : -1, msgConverted[2]);
		status = NetStatus::Disconnected;
		success = false;
	} else {
		CULog("Reconnection Progress: Connection OK");
		numPlayers = msgConverted[0];
		maxPlayers = msgConverted[1];
		playerID = msgConverted[2];
		status = NetStatus::Connected;

		lastReconnAttempt.reset();
		disconnTime.reset();
	}
	peer->CloseConnection(*natPunchServerAddress, true);

	directSend({ 
		static_cast<uint8_t>(playerID.has_value() ? *playerID : 0),
		static_cast<uint8_t>(success ? 1 : 0)
	}, Reconnect, *c.addr);
}

void cugl::NetworkConnection::cr2HostGetClientResp(
	HostPeers& h, SLNet::Packet* packet, const std::vector<uint8_t>& msgConverted)
{
	CULog("Host processing reconnection response");
	cc7HostGetClientData(h, packet, msgConverted);
}

#pragma endregion

void NetworkConnection::broadcast(const std::vector<uint8_t>& msg, SLNet::SystemAddress& ignore,
	CustomDataPackets packetType) {
	SLNet::BitStream bs;
	bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM + packetType));
	bs.Write(static_cast<uint8_t>(msg.size()));
	bs.WriteAlignedBytes(msg.data(), static_cast<unsigned int>(msg.size()));
	peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, ignore, true);
}

void NetworkConnection::send(const std::vector<uint8_t>& msg) { send(msg, Standard); }

void cugl::NetworkConnection::sendOnlyToHost(const std::vector<uint8_t>& msg) {
	std::visit(make_visitor(
		[&](HostPeers& /*h*/) {},
		[&](ClientPeer& c) {
			send(msg, DirectToHost);
		}), remotePeer);
}

void NetworkConnection::send(const std::vector<uint8_t>& msg, CustomDataPackets packetType) {
	SLNet::BitStream bs;
	bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM + packetType));
	bs.Write(static_cast<uint8_t>(msg.size()));
	bs.WriteAlignedBytes(msg.data(), static_cast<unsigned int>(msg.size()));

	std::visit(make_visitor(
		[&](HostPeers& /*h*/) {
			peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, *natPunchServerAddress, true);
		},
		[&](ClientPeer& c) {
			if (c.addr == nullptr) {
				return;
			}
			peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, *c.addr, false);
		}), remotePeer);
}

void cugl::NetworkConnection::directSend(
	const std::vector<uint8_t>& msg, CustomDataPackets packetType, SLNet::SystemAddress dest
) {
	SLNet::BitStream bs;
	bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM + packetType));
	bs.Write(static_cast<uint8_t>(msg.size()));
	bs.WriteAlignedBytes(msg.data(), static_cast<unsigned int>(msg.size()));
	peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, dest, false);
}

void cugl::NetworkConnection::attemptReconnect() {
	CUAssertLog(disconnTime.has_value(), "No time for disconnect??");

	time_t now = time(nullptr);
	if (now - *disconnTime > RECONN_TIMEOUT) {
		CULog("Reconnection timed out; giving up");
		status = NetStatus::Disconnected;
		return;
	}

	if (lastReconnAttempt.has_value()) {
		if (now - *lastReconnAttempt < RECONN_GAP) {
			// Too soon after last attempt; abort
			return;
		}
	}

	CULog("Attempting reconnection");

	peer->Shutdown(0);
	SLNet::RakPeerInterface::DestroyInstance(peer.release());
	
	lastReconnAttempt = now;
	peer = nullptr;

	c0StartupConn();
	peer->SetMaximumIncomingConnections(1);
}


void NetworkConnection::receive(
	const std::function<void(const std::vector<uint8_t>&)>& dispatcher) {

	switch (status) {
	case NetStatus::Reconnecting:
		attemptReconnect();
		if (peer == nullptr) {
			CULog("Peer null");
			return;
		}
		break;
	case NetStatus::Disconnected:
	case NetStatus::GenericError:
	case NetStatus::ApiMismatch:
	case NetStatus::RoomNotFound:
		return;
	case NetStatus::Connected:
	case NetStatus::Pending:
		break;
	}


	SLNet::Packet* packet = nullptr;
	for (packet = peer->Receive(); packet != nullptr;
		peer->DeallocatePacket(packet), packet = peer->Receive()) {
		SLNet::BitStream bts(packet->data, packet->length, false);

		switch (packet->data[0]) {
		case ID_CONNECTION_REQUEST_ACCEPTED:
			// Connected to some remote server
			if (packet->systemAddress == *(this->natPunchServerAddress)) {
				// Punchthrough server
				std::visit(make_visitor(
					[&](HostPeers& h) { ch1HostConnServer(h); },
					[&](ClientPeer& c) { cc1ClientConnServer(c); }), remotePeer);
			}
			else {
				std::visit(make_visitor(
					[&](HostPeers& h) { cc5HostConfirmClient(h, packet); },
					[&](ClientPeer& /*c*/) {
						CULogError(
							"A connection request you sent was accepted despite being client?");
					}), remotePeer);
			}
			break;
		case ID_NEW_INCOMING_CONNECTION: // Someone connected to you
			CULog("A peer connected");
			std::visit(make_visitor(
				[&](HostPeers& /*h*/) { CULogError("How did that happen? You're the host"); },
				[&](ClientPeer& c) { cc4ClientReceiveHostConnection(c, packet); }), remotePeer);
			break;
		case ID_NAT_PUNCHTHROUGH_SUCCEEDED: // Punchthrough succeeded
			CULog("Punchthrough success");

			std::visit(make_visitor(
				[&](HostPeers& h) { cc3HostReceivedPunch(h, packet); },
				[&](ClientPeer& c) { cc2ClientPunchSuccess(c, packet); }), remotePeer);
			break;
		case ID_NAT_TARGET_NOT_CONNECTED:
			status = NetStatus::GenericError;
			break;
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
		case ID_REMOTE_CONNECTION_LOST:
		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
			CULog("Received disconnect notification");
			std::visit(make_visitor(
				[&](HostPeers& h) {
					for (uint8_t i = 0; i < h.peers.size(); i++) {
						if (h.peers.at(i) == nullptr) {
							continue;
						}
						if (*h.peers.at(i) == packet->systemAddress) {
							uint8_t pID = i + 1;
							CULog("Lost connection to player %d", pID);
							std::vector<uint8_t> disconnMsg{ pID };
							h.peers.at(i) = nullptr;
							if (connectedPlayers.test(pID)) {
								numPlayers--;
								connectedPlayers.reset(pID);
							}
							send(disconnMsg, PlayerLeft);

							if (peer->GetConnectionState(packet->systemAddress) == SLNet::IS_CONNECTED) {
								peer->CloseConnection(packet->systemAddress, true);
							}
							return;
						}
					}
				},
				[&](ClientPeer& c) {
					if (packet->systemAddress == *natPunchServerAddress) {
						CULog("Successfully disconnected from Punchthrough server");
					}
					if (packet->systemAddress == *c.addr) {
						CULog("Lost connection to host");
						connectedPlayers.reset(0);
						switch (status) {
						case NetStatus::Pending:
							status = NetStatus::GenericError;
							return;
						case NetStatus::Connected:
							status = NetStatus::Reconnecting;
							disconnTime = time(nullptr);
							return;
						case NetStatus::Reconnecting:
						case NetStatus::Disconnected:
						case NetStatus::RoomNotFound:
						case NetStatus::ApiMismatch:
						case NetStatus::GenericError:
							return;
						}
					}
				}), remotePeer);

			break;
		case ID_NAT_PUNCHTHROUGH_FAILED:
		case ID_CONNECTION_ATTEMPT_FAILED:
		case ID_NAT_TARGET_UNRESPONSIVE: {
			CULogError("Punchthrough failure %d", packet->data[0]);

			status = NetStatus::GenericError;
			bts.IgnoreBytes(sizeof(SLNet::MessageID));
			SLNet::RakNetGUID recipientGuid;
			bts.Read(recipientGuid);

			CULogError("Attempted punchthrough to GUID %s failed", recipientGuid.ToString());
			break;
		}
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			status = NetStatus::RoomNotFound;
			break;

		// Begin Non-SLikeNet Reported Codes
		case ID_USER_PACKET_ENUM + Standard: {
			auto msgConverted = readBs(bts);
			dispatcher(msgConverted);

			std::visit(make_visitor(
				[&](HostPeers& /*h*/) { broadcast(msgConverted, packet->systemAddress); },
				[&](ClientPeer& c) {}), remotePeer);

			break;
		}
		case ID_USER_PACKET_ENUM + DirectToHost: {
			auto msgConverted = readBs(bts);

			std::visit(make_visitor(
				[&](HostPeers& /*h*/) {
					dispatcher(msgConverted);
				},
				[&](ClientPeer& c) {
					CULogError("Received direct to host message as client");
				}), remotePeer);

			break;
		}
		case ID_USER_PACKET_ENUM + AssignedRoom: {

			std::visit(make_visitor(
				[&](HostPeers& h) { ch2HostGetRoomID(h, bts); },
				[&](ClientPeer& c) {CULog("Assigned room ID but ignoring"); }), remotePeer);

			break;
		}
		case ID_USER_PACKET_ENUM + JoinRoom: {
			auto msgConverted = readBs(bts);

			std::visit(make_visitor(
				[&](HostPeers& h) { cc7HostGetClientData(h, packet, msgConverted); },
				[&](ClientPeer& c) { cc6ClientAssignedID(c, msgConverted); }
			), remotePeer);
			break;
		}
		case ID_USER_PACKET_ENUM + JoinRoomFail: {
			CULog("Failed to join room");
			status = NetStatus::RoomNotFound;
			break;
		}
		case ID_USER_PACKET_ENUM + Reconnect: {
			auto msgConverted = readBs(bts);

			std::visit(make_visitor(
				[&](HostPeers& h) { cr2HostGetClientResp(h, packet, msgConverted); },
				[&](ClientPeer& c) { cr1ClientReceivedInfo(c, msgConverted); }), remotePeer);

			break;
		}
		case ID_USER_PACKET_ENUM + PlayerJoined: {
			auto msgConverted = readBs(bts);

			std::visit(make_visitor(
				[&](HostPeers& h) { CULogError("Received player joined message as host"); },
				[&](ClientPeer& c) {
					connectedPlayers.set(msgConverted[0]);
					numPlayers++;
					maxPlayers++;
				}), remotePeer);

			break;
		}
		case ID_USER_PACKET_ENUM + PlayerLeft: {
			auto msgConverted = readBs(bts);

			std::visit(make_visitor(
				[&](HostPeers& h) { CULogError("Received player left message as host"); },
				[&](ClientPeer& c) {
					connectedPlayers.reset(msgConverted[0]);
					numPlayers--;
				}), remotePeer);
			break;
		}
		case ID_USER_PACKET_ENUM + StartGame: {
			startGame();
			break;
		}
		default:
			CULog("Received unknown message: %d", packet->data[0]);
			break;
		}
	}
}

void NetworkConnection::startGame() {
	CULog("Starting Game");
	std::visit(make_visitor([&](HostPeers& h) {
		h.started = true;
		broadcast({}, const_cast<SLNet::SystemAddress&>(SLNet::UNASSIGNED_SYSTEM_ADDRESS), StartGame);
		}, [&](ClientPeer& c) {}), remotePeer);
	maxPlayers = numPlayers;
}

cugl::NetworkConnection::NetStatus cugl::NetworkConnection::getStatus() {
	return status;
}
