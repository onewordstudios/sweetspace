#include "NetworkConnection.h"

#include <cugl/cugl.h>

#include "Globals.h"

/** IP of the NAT punchthrough server */
constexpr auto SERVER_ADDRESS = "35.231.212.113";
/** Port of the NAT punchthrough server */
constexpr unsigned short SERVER_PORT = 61111;
/** Largest message to send */
// constexpr unsigned int MAX_MESSAGE_LENGTH = 512;

NetworkConnection::NetworkConnection() : isHost(true), room() { startupConn(); }

NetworkConnection::NetworkConnection(std::string roomID) : isHost(false), room(roomID) {
	startupConn();
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

	// Use the default socket descriptor; this will make the OS assign us a
	// random port.
	RakNet::SocketDescriptor socketDescriptor;
	// Allow 2 connections: one for the other peer, one for the NAT server.
	peer->Startup(globals::MAX_PLAYERS, &socketDescriptor, 1);

	CULog("Your GUID is: %s",
		  peer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());

	// Start the thread for packet receiving
	// listenLoopThread = std::make_unique<std::thread>(&NetworkConnection::listenLoop, this);

	// Connect to the NAT Punchthrough server
	CULog("Connecting to punchthrough server");
	peer->Connect(this->natPunchServerAddress->ToString(false),
				  this->natPunchServerAddress->GetPort(), 0, 0);
}

void NetworkConnection::send(const std::vector<uint8_t>& msg) {
	// CUAssert(msg.size() < MAX_MESSAGE_LENGTH);
	RakNet::BitStream bs;
	bs.Write((uint8_t)ID_USER_PACKET_ENUM);
	bs.Write((uint8_t)msg.size());
	bs.WriteAlignedBytes(msg.data(), msg.size());
	// bs.WriteAlignedBytesSafe((const char*)msg.data(), msg.size(), MAX_MESSAGE_LENGTH);
}

void NetworkConnection::receive(std::function<void(const std::vector<uint8_t>&)> dispatcher) {
	RakNet::Packet* packet = nullptr;
	for (packet = peer->Receive(); packet != nullptr;
		 peer->DeallocatePacket(packet), packet = peer->Receive()) {
		RakNet::BitStream bts(packet->data, packet->length, false);

		// NOLINTNEXTLINE Dealing with an old 2000s era C++ library here
		switch (packet->data[0]) {
			case ID_CONNECTION_REQUEST_ACCEPTED:
				if (packet->systemAddress == *(this->natPunchServerAddress)) {
					CULog("Connected to punchthrough server");
					if (isHost) {
						CULog("Accepting connections now");
						peer->SetMaximumIncomingConnections(globals::MAX_PLAYERS);
					} else {
						CULog("Trying to connect to %s", room.c_str());
						RakNet::RakNetGUID remote;
						remote.FromString(room.c_str());
						this->natPunchthroughClient.OpenNAT(remote, *(this->natPunchServerAddress));
					}
				} else if (packet->systemAddress == this->remotePeer) {
					CULog("Connected to peer");
				}
				break;
			case ID_NEW_INCOMING_CONNECTION:
				CULog("A peer connected");
				break;
			case ID_NAT_PUNCHTHROUGH_SUCCEEDED:
				CULog("Punchthrough success");
				this->remotePeer = packet->systemAddress;

				if (isHost) {
					CULog("Connecting to peer");
					peer->Connect(this->remotePeer.ToString(false), this->remotePeer.GetPort(), 0,
								  0);
				}
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
				CULog("Received unknown message");
				break;
		}
	}
}
