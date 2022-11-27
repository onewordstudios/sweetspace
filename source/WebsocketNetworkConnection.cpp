#include "WebsocketNetworkConnection.h"

#include <cugl/cugl.h>

#include <sstream>

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
	: apiVer(config.apiVersion), maxPlayers(config.maxNumPlayers), ws(nullptr) {
	if (!initConnection(config)) {
		status = NetStatus::GenericError;
		return;
	}

	std::vector<uint8_t> data;
	data.push_back(AssignedRoom);
	data.push_back(config.apiVersion);
	ws->sendBinary(data);
	this->playerID = 0;
	this->numPlayers = 1;
	status = NetStatus::Pending;
}

WebsocketNetworkConnection::WebsocketNetworkConnection(ConnectionConfig config, std::string roomID)
	: apiVer(config.apiVersion), maxPlayers(config.maxNumPlayers), roomID(roomID), ws(nullptr) {
	if (!initConnection(config)) {
		status = NetStatus::GenericError;
		return;
	}

	std::vector<uint8_t> data;
	data.push_back(JoinRoom);
	for (char i : roomID) {
		data.push_back(static_cast<uint8_t>(i));
	}
	data.push_back(config.apiVersion);
	ws->sendBinary(data);
	this->numPlayers = 1;
	status = NetStatus::Pending;
}

WebsocketNetworkConnection::~WebsocketNetworkConnection() = default;

bool cugl::WebsocketNetworkConnection::initConnection(ConnectionConfig config) {
	switch (status) {
		case NetStatus::Disconnected:
		case NetStatus::RoomNotFound:
		case NetStatus::ApiMismatch:
		case NetStatus::GenericError:
			break;
		case NetStatus::Pending:
		case NetStatus::Connected:
		case NetStatus::Reconnecting:
			CULogError("ws connection already inited");
			return false;
	}

	using easywsclient::WebSocket;

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

	std::stringstream serverUrl;
	serverUrl << "ws://";
	serverUrl << config.punchthroughServerAddr;
	serverUrl << ':';
	serverUrl << config.fallbackServerPort;

	ws = WebSocket::from_url(serverUrl.str());
	if (!ws) { // NOLINT
		status = NetStatus::GenericError;
		return false;
	}

	return true;
}

void WebsocketNetworkConnection::send(const std::vector<uint8_t>& msg) {
	std::vector<uint8_t> fmsg;
	fmsg.push_back(GeneralMsg);
	fmsg.insert(fmsg.end(), msg.begin(), msg.end());
	ws->sendBinary(fmsg);
}

void WebsocketNetworkConnection::sendOnlyToHost(const std::vector<uint8_t>& msg) {
	std::vector<uint8_t> fmsg;
	fmsg.push_back(HostMsg);
	fmsg.insert(fmsg.end(), msg.begin(), msg.end());
	ws->sendBinary(fmsg);
}

void WebsocketNetworkConnection::manualDisconnect() {
	if (ws != nullptr) {
		ws->close();
		ws = nullptr;
	}
	status = NetStatus::Reconnecting;
}

void WebsocketNetworkConnection::startGame() {
	if (*playerID == 0) {
		std::vector<uint8_t> data;
		data.push_back(StartGame);
		ws->sendBinary(data);
	}
	maxPlayers = numPlayers;
}

NetworkConnection::NetStatus WebsocketNetworkConnection::getStatus() const { return status; }

void WebsocketNetworkConnection::receive(
	const std::function<void(const std::vector<uint8_t>&)>& dispatcher) {
	switch (status) {
		case NetStatus::Pending:
		case NetStatus::Connected:
		case NetStatus::Reconnecting:
			break;
		case NetStatus::Disconnected:
		case NetStatus::RoomNotFound:
		case NetStatus::ApiMismatch:
		case NetStatus::GenericError:
			return;
	}

	ws->poll();
	ws->dispatchBinary([this, &dispatcher](const std::vector<uint8_t>& message) {
		if (message.empty()) {
			return;
		}

		const auto type = static_cast<CustomDataPackets>(message[0]);
		// Uses some slow array copies, but who cares; this is just the fallback code we hopefully
		// never need
		switch (type) {
			case GeneralMsg:
				dispatcher(vector<uint8_t>(message.begin() + 1, message.end()));
				break;
			case HostMsg:
				if (*playerID == 0) {
					dispatcher(vector<uint8_t>(message.begin() + 1, message.end()));
				}
				break;
			case StartGame:
				startGame();
				break;
			case PlayerJoined: {
				const uint8_t newID = message[1];
				connectedPlayers.set(newID);
				numPlayers++;
				break;
			}
			case PlayerDisconnect: {
				const uint8_t newID = message[1];
				connectedPlayers.reset(newID);
				numPlayers--;
				break;
			}
			case AssignedRoom: {
				std::stringstream newRoomId;
				for (unsigned int i = 1; i < message.size(); i++) {
					newRoomId << static_cast<char>(message[i]);
				}
				connectedPlayers.set(0);
				roomID = newRoomId.str();
				CULog("Got room ID: %s", roomID.c_str());
				status = NetStatus::Connected;
				break;
			}
			case JoinRoom: {
				const uint8_t result = message[1];
				switch (result) {
					case 0: {
						numPlayers = message[2];
						playerID = message[3];
						for (uint8_t i = 0; i < numPlayers; i++) {
							connectedPlayers.set(i);
						}
						status = NetStatus::Connected;
						break;
					}
					case 1:
					case 2:
						status = NetStatus::RoomNotFound;
						break;
					case 4:
						status = NetStatus::Disconnected;
						break;
					default:
						CULogError("Invalid join room value %d", result);
						break;
				}
				break;
			}
			case ApiMismatch:
				status = NetStatus::ApiMismatch;
				break;
		}
	});
}
