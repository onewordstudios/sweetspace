#include "NetworkConnection.h"

#include <cugl/cugl.h>

#include "Globals.h"

constexpr unsigned short SERVER_PORT = 60001;

NetworkConnection::NetworkConnection() : isListening(false), listenLoopThread(nullptr) {
	startupConn();
}

NetworkConnection::NetworkConnection(std::string roomID)
	: isListening(false), listenLoopThread(nullptr) {
	startupConn();
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

	RakNet::SocketDescriptor sd(SERVER_PORT, 0);
	peer->Startup(globals::MAX_PLAYERS, &sd, 1);
	peer->SetMaximumIncomingConnections(globals::MAX_PLAYERS);

	peer->AttachPlugin(&(natPunchthroughClient));
	natPunchServerAddress = std::make_unique<RakNet::SystemAddress>(
		RakNet::SystemAddress("natpunch.jenkinssoftware.com", 61111));

	// Use the default socket descriptor; this will make the OS assign us a
	// random port.
	RakNet::SocketDescriptor socketDescriptor;
	// Allow 2 connections: one for the other peer, one for the NAT server.
	peer->Startup(2, &socketDescriptor, 1);

	CULog("Your GUID is: %s",
		  peer->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());

	// Start the thread for packet receiving
	this->isListening = true;
	listenLoopThread = std::make_unique<std::thread>(&NetworkConnection::listenLoop, this);

	// Connect to the NAT Punchthrough server
	CULog("Connecting to punchthrough server");
	peer->Connect(this->natPunchServerAddress->ToString(false),
				  this->natPunchServerAddress->GetPort(), 0, 0);
}

void NetworkConnection::listenLoop() {}
