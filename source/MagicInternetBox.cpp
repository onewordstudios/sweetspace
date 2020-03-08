#include "MagicInternetBox.h"

using namespace cugl;

constexpr auto GAME_SERVER = "ws://sweetspace-server.azurewebsites.net/";
constexpr float FLOAT_PRECISION = 1000.0f;

bool MagicInternetBox::initHost() {
	using easywsclient::WebSocket;

	// I actually don't know what this stuff does but it won't run on Windows without it,
	// so ¯\_(ツ)_/¯
#ifdef _WIN32
	INT rc;
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		CULog("WSAStartup Failed");
		return 1;
	}
#endif

	ws = WebSocket::from_url(GAME_SERVER);
	if (!ws) {
		CULog("FAILED TO CONNECT");
		return false;
	}
	ws->send("hello world");
	return true;
}

void MagicInternetBox::sendData(NetworkDataType type, float angle, int id, int data1, int data2,
								float data3) {
	std::vector<uint8_t> data;

	data.push_back((uint8_t)type);
	// Approximate float down to 0.001 accuracy
	data.push_back((uint8_t)(FLOAT_PRECISION * angle));
	data.push_back(id);
	data.push_back(data1);
	data.push_back(data2);
	data.push_back((uint8_t)(FLOAT_PRECISION * data3));

	ws->sendBinary(data);
}

bool MagicInternetBox::initClient(std::string id) { return initHost(); }

void MagicInternetBox::leaveRoom() {}

std::string MagicInternetBox::getRoomID() { return std::string(); }

int MagicInternetBox::getPlayerID() { return 0; }

void MagicInternetBox::update() {
	ws->poll();
	ws->dispatchBinary([](const std::vector<uint8_t>& message) {
		// CULog(">>>\tReceived Websocket Message: %s", message.c_str());
		NetworkDataType type = static_cast<NetworkDataType>(message[0]);
		switch (type) {
			case PositionUpdate: {
				break;
			}
			case BreachCreate: {
				break;
			}
			case BreachResolve: {
				break;
			}
			case DualCreate: {
				break;
			}
			case DualResolve: {
				break;
			}
		}
	});
}

void MagicInternetBox::createBreach(float angle, int player, int id) {
	sendData(BreachCreate, angle, id, player, -1, -1.0f);
}

void MagicInternetBox::resolveBreach(int id) { sendData(BreachResolve, -1.0f, id, -1, -1, -1.0f); }

void MagicInternetBox::createDualTask(float angle, int player1, int player2, int id) {
	sendData(DualCreate, angle, id, player1, player2, -1.0f);
}

void MagicInternetBox::flagDualTask(int id) { sendData(DualResolve, -1.0f, id, -1, -1, -1.0f); }
