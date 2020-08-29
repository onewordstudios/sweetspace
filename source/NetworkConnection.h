#ifndef __NETWORK_CONNECTION_H__
#define __NETWORK_CONNECTION_H__

#include <string>
#include <thread>

#include "libraries/RakNet/BitStream.h"
#include "libraries/RakNet/MessageIdentifiers.h"
#include "libraries/RakNet/NatPunchthroughClient.h"
#include "libraries/RakNet/RakNetTypes.h"
#include "libraries/RakNet/RakPeerInterface.h"

class NetworkConnection {
   public:
	NetworkConnection();
	NetworkConnection(std::string roomID);
	~NetworkConnection();

   private:
	std::unique_ptr<RakNet::RakPeerInterface> peer;

	bool isListening;

	std::unique_ptr<RakNet::SystemAddress> natPunchServerAddress;
	RakNet::NatPunchthroughClient natPunchthroughClient;

	std::unique_ptr<std::thread> listenLoopThread;

	RakNet::SystemAddress remotePeer;
	bool sentNatPunchthroughRequest;

	void startupConn();

	void listenLoop();
};

#endif /* __NETWORK_CONNECTION_H__ */
