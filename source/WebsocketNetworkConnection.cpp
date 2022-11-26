#include "WebsocketNetworkConnection.h"

#include <cugl/cugl.h>

using namespace cugl;

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
typedef int ssize_t;	 // NOLINT
#define _SSIZE_T_DEFINED // NOLINT
#endif
#ifndef _SOCKET_T_DEFINED
typedef SOCKET socket_t;  // NOLINT
#define _SOCKET_T_DEFINED // NOLINT
#endif
#endif

WebsocketNetworkConnection::WebsocketNetworkConnection(ConnectionConfig config)
	: apiVer(config.apiVersion) {
#ifdef _WIN32
	INT rc; // NOLINT
	WSADATA wsaData;

	// NOLINTNEXTLINE
	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) { // NOLINT
		CULogError("WSAStartup Failed");
		// NOLINTNEXTLINE
		throw "WSA Startup Failed";
	}
#endif
}

WebsocketNetworkConnection::WebsocketNetworkConnection(ConnectionConfig config, std::string roomID)
	: apiVer(config.apiVersion) {
#ifdef _WIN32
	INT rc; // NOLINT
	WSADATA wsaData;

	// NOLINTNEXTLINE
	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) { // NOLINT
		CULogError("WSAStartup Failed");
		// NOLINTNEXTLINE
		throw "WSA Startup Failed";
	}
#endif
}

WebsocketNetworkConnection::~WebsocketNetworkConnection() {}

void WebsocketNetworkConnection::send(const std::vector<uint8_t>& msg) {}

void WebsocketNetworkConnection::sendOnlyToHost(const std::vector<uint8_t>& msg) {}

void WebsocketNetworkConnection::receive(
	const std::function<void(const std::vector<uint8_t>&)>& dispatcher) {}

void WebsocketNetworkConnection::manualDisconnect() {}

void WebsocketNetworkConnection::startGame() {}

NetworkConnection::NetStatus WebsocketNetworkConnection::getStatus() const { return NetStatus(); }
