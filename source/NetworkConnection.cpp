#include "NetworkConnection.h"

#include <cugl/cugl.h>

#include <utility>

#include "Globals.h"
#include "NetworkDataType.h"

/** IP of the NAT punchthrough server */
constexpr auto SERVER_ADDRESS = "35.231.212.113";
/** Port of the NAT punchthrough server */
constexpr uint16_t SERVER_PORT = 61111;
/** How long to block on shutdown */
constexpr unsigned int SHUTDOWN_BLOCK = 10;

NetworkConnection::NetworkConnection() {
	startupConn();
	remotePeer = HostPeers();
}

NetworkConnection::NetworkConnection(std::string roomID) {
	startupConn();
	remotePeer = ClientPeer(std::move(roomID));
	peer->SetMaximumIncomingConnections(1);
}

NetworkConnection::~NetworkConnection() {
	peer->Shutdown(SHUTDOWN_BLOCK);
	RakNet::RakPeerInterface::DestroyInstance(peer.release());
}

void NetworkConnection::startupConn() {
	peer = std::unique_ptr<RakNet::RakPeerInterface>(RakNet::RakPeerInterface::GetInstance());

	peer->AttachPlugin(&(natPunchthroughClient));
	natPunchServerAddress =
		std::make_unique<RakNet::SystemAddress>(RakNet::SystemAddress(SERVER_ADDRESS, SERVER_PORT));

	// Use the default socket descriptor
	// This will make the OS assign us a random port.
	RakNet::SocketDescriptor socketDescriptor;
	// Allow connections for each player and one for the NAT server.
	peer->Startup(globals::MAX_PLAYERS, &socketDescriptor, 1);

	CULog("Your GUID is: %s",
		  peer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());

	// Connect to the NAT Punchthrough server
	CULog("Connecting to punchthrough server");
	peer->Connect(this->natPunchServerAddress->ToString(false),
				  this->natPunchServerAddress->GetPort(), nullptr, 0);
}

void NetworkConnection::broadcast(const std::vector<uint8_t>& msg, RakNet::SystemAddress& ignore) {
	RakNet::BitStream bs;
	bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	bs.Write(static_cast<uint8_t>(msg.size()));
	bs.WriteAlignedBytes(msg.data(), static_cast<unsigned int>(msg.size()));
	peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, ignore, true);
}

void NetworkConnection::send(const std::vector<uint8_t>& msg) {
	RakNet::BitStream bs;
	bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	bs.Write(static_cast<uint8_t>(msg.size()));
	bs.WriteAlignedBytes(msg.data(), static_cast<unsigned int>(msg.size()));

	remotePeer.match(
		[&](HostPeers& /*h*/) {
			peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, *natPunchServerAddress, true);
		},
		[&](ClientPeer& c) { peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, *c.addr, false); });
}

void NetworkConnection::receive(
	const std::function<void(const std::vector<uint8_t>&)>& dispatcher) {
	RakNet::Packet* packet = nullptr;
	for (packet = peer->Receive(); packet != nullptr;
		 peer->DeallocatePacket(packet), packet = peer->Receive()) {
		RakNet::BitStream bts(packet->data, packet->length, false);

		// NOLINTNEXTLINE Dealing with an old 2000s era C++ library here
		switch (packet->data[0]) {
			case ID_CONNECTION_REQUEST_ACCEPTED: // Connected to some remote server
				if (packet->systemAddress == *(this->natPunchServerAddress)) {
					CULog("Connected to punchthrough server");

					remotePeer.match(
						[&](HostPeers& /*h*/) {
							CULog("Accepting connections now");
							peer->SetMaximumIncomingConnections(globals::MAX_PLAYERS - 1);
						},
						[&](ClientPeer& c) {
							CULog("Trying to connect to %s", c.room.c_str());
							RakNet::RakNetGUID remote;
							remote.FromString(c.room.c_str());
							this->natPunchthroughClient.OpenNAT(remote,
																*(this->natPunchServerAddress));
						});
				} else {
					remotePeer.match(
						[&](HostPeers& h) {
							for (uint8_t i = 0; i < h.peers.size(); i++) {
								if (*h.peers.at(i) == packet->systemAddress) {
									uint8_t pID = i + 1;
									CULog("Player %d accepted connection request", pID);
									std::vector<uint8_t> joinMsg = {NetworkDataType::PlayerJoined,
																	pID};
									dispatcher(joinMsg);
									broadcast(joinMsg, packet->systemAddress);

									RakNet::BitStream bs;
									std::vector<uint8_t> connMsg = {NetworkDataType::JoinRoom, 0,
																	h.numPlayers, pID,
																	globals::API_VER};
									bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
									bs.Write(static_cast<uint8_t>(connMsg.size()));
									bs.WriteAlignedBytes(connMsg.data(),
														 static_cast<unsigned int>(connMsg.size()));
									peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1,
											   packet->systemAddress, false);
									break;
								}
							}
						},
						[&](ClientPeer& /*c*/) {
							CULogError(
								"A connection request you sent was accepted despite being client?");
						});
				}
				break;
			case ID_NEW_INCOMING_CONNECTION: // Someone connected to you
				CULog("A peer connected");
				remotePeer.match(
					[&](HostPeers& /*h*/) { CULogError("How did that happen? You're the host"); },
					[&](ClientPeer& c) {
						if (packet->systemAddress == *c.addr) {
							CULog("Connected to host :D");
						}
					});
				break;
			case ID_NAT_PUNCHTHROUGH_SUCCEEDED: // Punchthrough succeeded
				CULog("Punchthrough success");

				remotePeer.match(
					[&](HostPeers& h) {
						auto p = packet->systemAddress;

						bool hasRoom = false;
						for (size_t i = 0; i < h.peers.size(); i++) {
							if (h.peers.at(i) == nullptr) {
								hasRoom = true;
								h.peers.at(i) = std::make_unique<RakNet::SystemAddress>(p);
								h.numPlayers++;
								break;
							}
						}

						if (hasRoom) {
							CULog("Connecting to client now");
							peer->Connect(p.ToString(false), p.GetPort(), nullptr, 0);
						} else {
							CULogError(
								"Client attempted to join but room was full - if you're seeing "
								"this error, that means somehow there are ghost clients not "
								"actually connected even though mib thinks they are");
						}
					},
					[&](ClientPeer& c) {
						c.addr = std::make_unique<RakNet::SystemAddress>(packet->systemAddress);
					});
				break;
			case ID_USER_PACKET_ENUM: {
				// More Old C++ Library Shenanigans
				uint8_t ignored; // NOLINT
				bts.Read(ignored);
				uint8_t length; // NOLINT
				bts.Read(length);
				uint8_t* message = new uint8_t[length]; // NOLINT
				bts.ReadAlignedBytes(message, length);
				// NOLINTNEXTLINE
				std::vector<uint8_t> msgConverted(&message[0], &message[length]);
				dispatcher(msgConverted);

				remotePeer.match(
					[&](HostPeers& /*h*/) { broadcast(msgConverted, packet->systemAddress); },
					[&](ClientPeer& c) {});

				delete[] message; // NOLINT
				break;
			}
			case ID_NAT_TARGET_NOT_CONNECTED:
				dispatcher({NetworkDataType::JoinRoom, 1});
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			case ID_REMOTE_CONNECTION_LOST:
			case ID_DISCONNECTION_NOTIFICATION:
			case ID_CONNECTION_LOST:
				CULog("A disconnect occured");
				remotePeer.match(
					[&](HostPeers& h) {
						for (uint8_t i = 0; i < h.peers.size(); i++) {
							if (*h.peers.at(i) == packet->systemAddress) {
								uint8_t pID = i + 1;
								CULog("Lost connection to player %d", pID);
								std::vector<uint8_t> disconnMsg{NetworkDataType::PlayerDisconnect,
																pID};
								h.peers.at(i) = nullptr;
								h.numPlayers--;
								dispatcher(disconnMsg);
								send(disconnMsg);
								return;
							}
						}
					},
					[&](ClientPeer& c) {
						if (packet->systemAddress == *c.addr) {
							CULog("Lost connection to host");
							dispatcher({NetworkDataType::PlayerDisconnect, 0});
						}
					});

				break;
			case ID_NAT_PUNCHTHROUGH_FAILED:
			case ID_CONNECTION_ATTEMPT_FAILED:
			case ID_NAT_TARGET_UNRESPONSIVE:
				CULog("Punchthrough failure");
				dispatcher({NetworkDataType::GenericError});
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				CULog("Room full");
				dispatcher({NetworkDataType::JoinRoom, 2});
				break;
			default:
				CULog("Received unknown message: %d", packet->data[0]); // NOLINT
				break;
		}
	}
}
