#include "NetworkConnection.h"

#include <cugl/cugl.h>

#include "Globals.h"

constexpr auto SERVER_ADDRESS = "35.231.212.113";
constexpr unsigned short SERVER_PORT = 61111;

NetworkConnection::NetworkConnection()
	: isListening(false), listenLoopThread(nullptr), sentNatPunchthroughRequest(false) {
	startupConn();

	while (!isListening) {
	}
	CULog("Listening for connections now");
	peer->SetMaximumIncomingConnections(globals::MAX_PLAYERS);
}

NetworkConnection::NetworkConnection(std::string roomID)
	: isListening(false), listenLoopThread(nullptr), sentNatPunchthroughRequest(false) {
	startupConn();
	while (!isListening) {
	}
	CULog("Trying to connect to %s", roomID.c_str());
	this->sentNatPunchthroughRequest = true;
	RakNet::RakNetGUID remote;
	remote.FromString(roomID.c_str());
	this->natPunchthroughClient.OpenNAT(remote, *(this->natPunchServerAddress));
}

NetworkConnection::~NetworkConnection() {
	peer->Shutdown(0);

	isListening = false;
	listenLoopThread->join();

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
	listenLoopThread = std::make_unique<std::thread>(&NetworkConnection::listenLoop, this);

	// Connect to the NAT Punchthrough server
	CULog("Connecting to punchthrough server");
	peer->Connect(this->natPunchServerAddress->ToString(false),
				  this->natPunchServerAddress->GetPort(), 0, 0);
}

void NetworkConnection::listenLoop() {
	// Allocate the buffer for the incoming message string
	char* message = new char[100];
	CULog("Listening now");

	while (true) {
		RakNet::Packet* packet = nullptr;
		for (packet = peer->Receive(); packet != nullptr;
			 peer->DeallocatePacket(packet), packet = peer->Receive()) {
			RakNet::BitStream bts(packet->data, packet->length, false);

			// Check the packet identifier
			switch (packet->data[0]) {
				case ID_CONNECTION_REQUEST_ACCEPTED:
					if (packet->systemAddress == *(this->natPunchServerAddress)) {
						CULog("Connected to punchthrough server");
						this->isListening = true;
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

					if (this->sentNatPunchthroughRequest) {
						this->sentNatPunchthroughRequest = false;
						CULog("Connecting to peer");
						peer->Connect(this->remotePeer.ToString(false), this->remotePeer.GetPort(),
									  0, 0);
					}
					break;
				case ID_USER_PACKET_ENUM:
					unsigned char rcv_id;
					bts.Read(rcv_id);
					unsigned int length;
					bts.Read(length);
					bts.Read(message, length);
					std::cout << "* Message received:" << std::endl << message << std::endl;
					break;
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					std::cout << "* Remote peer has disconnected." << std::endl;
					break;
				case ID_REMOTE_CONNECTION_LOST:
					std::cout << "* Remote peer lost connection." << std::endl;
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					std::cout << "* A peer has disconnected." << std::endl;
					break;
				case ID_CONNECTION_LOST:
					std::cout << "* A connection was lost." << std::endl;
					break;
				default:
					CULog("Received unknown message");
					break;
			} // check package identifier
		}	  // package receive loop
	}		  // listening loop

	delete[] message;
}
