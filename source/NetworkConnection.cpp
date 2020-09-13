#include "NetworkConnection.h"

#include <cugl/cugl.h>

#include "Globals.h"
#include "NetworkDataType.h"

/** IP of the NAT punchthrough server */
constexpr auto SERVER_ADDRESS = "35.231.212.113";
/** Port of the NAT punchthrough server */
constexpr unsigned short SERVER_PORT = 61111;

NetworkConnection::NetworkConnection() {
	startupConn();
	remotePeer = HostPeers();
}

NetworkConnection::NetworkConnection(std::string roomID) {
	startupConn();
	remotePeer = ClientPeer(roomID);
}

NetworkConnection::~NetworkConnection() {
	peer->Shutdown(0);
	RakNet::RakPeerInterface::DestroyInstance(peer.get());
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
				  this->natPunchServerAddress->GetPort(), 0, 0);
}

void NetworkConnection::send(const std::vector<uint8_t>& msg) {
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_USER_PACKET_ENUM);
	bs.Write((uint8_t)msg.size());
	bs.WriteAlignedBytes(msg.data(), msg.size());

	remotePeer.match(
		[&](HostPeers& h) {
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
						[&](HostPeers& h) {
							CULog("Accepting connections now");
							peer->SetMaximumIncomingConnections(globals::MAX_PLAYERS);
							dispatcher({NetworkDataType::AssignedRoom, 0, 0, 0, 0, 0});
						},
						[&](ClientPeer& c) {
							CULog("Trying to connect to %s", c.room.c_str());
							RakNet::RakNetGUID remote;
							remote.FromString(c.room.c_str());
							this->natPunchthroughClient.OpenNAT(remote,
																*(this->natPunchServerAddress));
						});
				} else {
					// packet->systemAddress
					CULog("Connected to a peer");
				}
				break;
			case ID_NEW_INCOMING_CONNECTION: // Someone connected to you
				CULog("A peer connected");
				break;
			case ID_NAT_PUNCHTHROUGH_SUCCEEDED: // Punchthrough succeeded
				CULog("Punchthrough success");

				remotePeer.match(
					[&](HostPeers& h) {
						// CULog("Connecting to peer");

						auto p = packet->systemAddress;

						bool hasRoom = false;
						for (size_t i = 0; i < h.size(); i++) {
							if (h[i] == nullptr) {
								hasRoom = true;
								h[i] = std::make_unique<RakNet::SystemAddress>(p);
							}
						}

						if (hasRoom) {
							CULog("Connecting to client now");
							peer->Connect(p.ToString(false), p.GetPort(), 0, 0);
						} else {
							CULog("Client attempted to join but room was full");
						}
					},
					[&](ClientPeer& c) {
						c.addr = std::make_unique<RakNet::SystemAddress>(packet->systemAddress);
						peer->SetMaximumIncomingConnections(1);
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
					[&](HostPeers& h) {
						RakNet::BitStream bs;
						bs.Write((uint8_t)ID_USER_PACKET_ENUM);
						bs.Write((uint8_t)msgConverted.size());
						bs.WriteAlignedBytes(msgConverted.data(), msgConverted.size());
						peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE, 1, packet->systemAddress, true);
					},
					[&](ClientPeer& c) {

					});

				delete[] message; // NOLINT
				break;
			}
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			case ID_REMOTE_CONNECTION_LOST:
			case ID_DISCONNECTION_NOTIFICATION:
			case ID_CONNECTION_LOST:
				CULog("A disconnect occured");
				break;
			case ID_NAT_PUNCHTHROUGH_FAILED:
				CULog("Punchthrough failure");
				break;
			default:
				CULog("Received unknown message: %d", packet->data[0]); // NOLINT
				break;
		}
	}
}
