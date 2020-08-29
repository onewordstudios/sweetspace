#ifndef __NETWORK_CONNECTION_H__
#define __NETWORK_CONNECTION_H__

#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "libraries/RakNet/BitStream.h"
#include "libraries/RakNet/MessageIdentifiers.h"
#include "libraries/RakNet/NatPunchthroughClient.h"
#include "libraries/RakNet/RakNetTypes.h"
#include "libraries/RakNet/RakPeerInterface.h"

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
	NetworkConnection(std::string roomID);

	/** Delete and cleanup this connection. */
	~NetworkConnection();

	void send(const std::vector<uint8_t>& msg);

	void receive(std::function<void(const std::vector<uint8_t>&)> dispatcher);

   private:
	/** Connection object */
	std::unique_ptr<RakNet::RakPeerInterface> peer;

	/** Whether this user is a host */
	bool isHost;
	/** Room ID, defined only if isHost == false */
	std::string room;

#pragma region Punchthrough
	/** Address of punchthrough server */
	std::unique_ptr<RakNet::SystemAddress> natPunchServerAddress;
	/** NAT Punchthrough Client */
	RakNet::NatPunchthroughClient natPunchthroughClient;
#pragma endregion

	RakNet::SystemAddress remotePeer;

	void startupConn();
};

#endif /* __NETWORK_CONNECTION_H__ */
