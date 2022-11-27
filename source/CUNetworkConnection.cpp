#include "CUNetworkConnection.h"

#include <cugl/cugl.h>

#include <sstream>
#include <utility>

#include "AdHocNetworkConnection.h"
#include "WebsocketNetworkConnection.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <fcntl.h>
#pragma comment(lib, "ws2_32")
#include <io.h>
#include <stdio.h>	// NOLINT
#include <stdlib.h> // NOLINT
#include <string.h> // NOLINT
#include <sys/types.h>
#ifndef _SSIZE_T_DEFINED
typedef int ssize_t; // NOLINT
// NOLINTNEXTLINE
#define _SSIZE_T_DEFINED
#endif
#ifndef _SOCKET_T_DEFINED
typedef SOCKET socket_t; // NOLINT
// NOLINTNEXTLINE
#define _SOCKET_T_DEFINED
#endif
#endif

using namespace cugl;

class HostWrapperNetworkConnection : public NetworkConnection {
   public:
	explicit HostWrapperNetworkConnection(ConnectionConfig config)
		: hasConn(false), isAdHoc(true), config(config) {
		conn = std::make_unique<AdHocNetworkConnection>(config);
	}

	void receive(const std::function<void(const std::vector<uint8_t>&)>& dispatcher) override {
		conn->receive(dispatcher);

		if (!hasConn) {
			switch (conn->getStatus()) {
				case NetStatus::GenericError:
				case NetStatus::Disconnected: {
					if (isAdHoc) {
						CULog("Failed to connect as host to adhoc; trying websocket");
						isAdHoc = false;
						conn = std::make_unique<WebsocketNetworkConnection>(config);
					}
					break;
				}
				case NetStatus::Connected: {
					hasConn = true;
					break;
				}
				default:
					break;
			}
		}
	}

	void send(const std::vector<uint8_t>& msg) override { return conn->send(msg); }

	void sendOnlyToHost(const std::vector<uint8_t>& msg) override {
		return conn->sendOnlyToHost(msg);
	}

	void manualDisconnect() override { return conn->manualDisconnect(); }

	void startGame() override { return conn->startGame(); }

	NetStatus getStatus() const override { return conn->getStatus(); }

	tl::optional<uint8_t> getPlayerID() const override { return conn->getPlayerID(); }

	std::string getRoomID() const override { return conn->getRoomID(); }

	bool isPlayerActive(uint8_t playerID) const override { return conn->isPlayerActive(playerID); }

	uint8_t getNumPlayers() const override { return conn->getNumPlayers(); }

	uint8_t getTotalPlayers() const override { return conn->getTotalPlayers(); }

   private:
	bool hasConn;
	bool isAdHoc;
	ConnectionConfig config;
	std::unique_ptr<NetworkConnection> conn;
};

std::unique_ptr<NetworkConnection> cugl::NetworkConnection::newHostConnection(
	ConnectionConfig config) {
	return std::make_unique<HostWrapperNetworkConnection>(config);
}

std::unique_ptr<NetworkConnection> cugl::NetworkConnection::newClientConnection(
	ConnectionConfig config, std::string roomID) {
	if (roomID.at(0) == '0') {
		// Fallback websocket config
		return std::make_unique<WebsocketNetworkConnection>(config, roomID);
	}

	return std::make_unique<AdHocNetworkConnection>(config, roomID);
}
