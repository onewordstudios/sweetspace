#include "MagicInternetBox.h"

using namespace cugl;

constexpr auto GAME_SERVER = "ws://sweetspace-server.azurewebsites.net/";
constexpr float FLOAT_PRECISION = 1000.0f;
constexpr unsigned int NETWORK_TICK = 12;

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
	currFrame = (currFrame + 1) % NETWORK_TICK;
	if (currFrame == 0) {
		// NETWORK TICK

		// TODO update numbers
		float angle = 0.0f;
		float velocity = 0.0f;
		sendData(PositionUpdate, angle, playerID, -1, -1, velocity);
	}
	ws->poll();
	ws->dispatchBinary([](const std::vector<uint8_t>& message) {
		NetworkDataType type = static_cast<NetworkDataType>(message[0]);
		switch (type) {
			case PositionUpdate: {
				float angle = (float)message[1] * FLOAT_PRECISION;
				float velocity = (float)message[4] * FLOAT_PRECISION;
				unsigned int id = message[2];
				break;
			}
			case BreachCreate: {
				float angle = (float)message[1] * FLOAT_PRECISION;
				unsigned int breachID = message[2];
				unsigned int playerID = message[3];
				break;
			}
			case BreachResolve: {
				unsigned int breachID = message[2];
				break;
			}
			case DualCreate: {
				float angle = (float)message[1] * FLOAT_PRECISION;
				unsigned int taskID = message[2];
				unsigned int player1 = message[3];
				unsigned int player2 = message[4];
				break;
			}
			case DualResolve: {
				unsigned int taskID = message[2];
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
