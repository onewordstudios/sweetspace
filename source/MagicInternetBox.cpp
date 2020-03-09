#include "MagicInternetBox.h"

using namespace cugl;

constexpr auto GAME_SERVER = "ws://sweetspace-server.azurewebsites.net/";
constexpr float FLOAT_PRECISION = 180.0f;
constexpr unsigned int NETWORK_TICK = 12;
constexpr unsigned int ONE_BYTE = 256;

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

/*

DATA FORMAT

[ TYPE (enum) | ANGLE (2 bytes) | ID (2 bytes) | data1 (2 bytes) | data2 (2 bytes) | data3 (2 bytes)

Each 2-byte block is stored smaller first, then larger; ie 2^8 * byte1 + byte0 gives the original.
All data is truncated to fit 16 bytes.
Floats are multiplied by FLOAT_PRECISION and then cast to int before running through same algorithm

*/

void MagicInternetBox::sendData(NetworkDataType type, float angle, int id, int data1, int data2,
								float data3) {
	std::vector<uint8_t> data;

	data.push_back((uint8_t)type);

	int angleConv = (int)(FLOAT_PRECISION * angle);

	data.push_back((uint8_t)(angleConv % ONE_BYTE));
	data.push_back((uint8_t)(angleConv / ONE_BYTE));

	data.push_back((uint8_t)(id % ONE_BYTE));
	data.push_back((uint8_t)(id / ONE_BYTE));

	data.push_back((uint8_t)(data1 % ONE_BYTE));
	data.push_back((uint8_t)(data1 / ONE_BYTE));

	data.push_back((uint8_t)(data2 % ONE_BYTE));
	data.push_back((uint8_t)(data2 / ONE_BYTE));

	int d3 = (int)(FLOAT_PRECISION * data3);

	data.push_back((uint8_t)(d3 % ONE_BYTE));
	data.push_back((uint8_t)(d3 / ONE_BYTE));

	ws->sendBinary(data);
}

bool MagicInternetBox::initClient(std::string id) { return initHost(); }

void MagicInternetBox::leaveRoom() {}

std::string MagicInternetBox::getRoomID() { return std::string(); }

int MagicInternetBox::getPlayerID() { return playerID; }

void MagicInternetBox::update(std::shared_ptr<ShipModel> state) {
	// NETWORK TICK
	currFrame = (currFrame + 1) % NETWORK_TICK;
	if (currFrame == 0) {
		if (playerID != -1) {
			std::shared_ptr<DonutModel> player = state->getDonuts()[playerID];
			float angle = player->getAngle();
			float velocity = player->getVelocity();
			sendData(PositionUpdate, angle, playerID, -1, -1, velocity);
		}
	}

	ws->poll();
	ws->dispatchBinary([&state, this](const std::vector<uint8_t>& message) {
		if (message.size() == 0) {
			return;
		}
		if (playerID == -1 && message[0] == 0) {
			CULog("Got message %d, %d", message[0], message[1]);
			this->playerID = message[1];
			CULog("Got player id %d", this->getPlayerID());
			resolveBreach(0);
			resolveBreach(1);
			resolveBreach(2);
			return;
		}

		if (playerID == -1) {
			return;
		}

		NetworkDataType type = static_cast<NetworkDataType>(message[0]);

		float angle = (float)(message[1] + ONE_BYTE * message[2]) / FLOAT_PRECISION;
		int id = (int)(message[3] + ONE_BYTE * message[4]);
		// Networking code is finnicky and having these magic numbers is the easiest solution
		// NOLINTNEXTLINE Simple counting numbers
		int data1 = (int)(message[5] + ONE_BYTE * message[6]);
		// NOLINTNEXTLINE Ditto
		int data2 = (int)(message[7] + ONE_BYTE * message[8]);
		// NOLINTNEXTLINE Ditto
		float data3 = (float)(message[9] + ONE_BYTE * message[10]) / FLOAT_PRECISION;

		switch (type) {
			case PositionUpdate: {
				std::shared_ptr<DonutModel> donut = state->getDonuts()[id];
				donut->setAngle(angle);
				donut->setVelocity(data3);
				break;
			}
			case BreachCreate: {
				state->createBreach(angle, 3, data1, id);
				CULog("Creating breach %d at angle %f with user %d", id, angle, data1);
				break;
			}
			case BreachResolve: {
				state->resolveBreach(id);
				CULog("Resolve breach %d", id);
				break;
			}
			case DualCreate: {
				unsigned int taskID = id;
				unsigned int player1 = data1;
				unsigned int player2 = data2;
				// TODO
				break;
			}
			case DualResolve: {
				unsigned int taskID = id;
				// TODO
				break;
			}
		}
	});
}

void MagicInternetBox::createBreach(float angle, int player, int id) {
	sendData(BreachCreate, angle, id, player, -1, -1.0f);
	CULog("Creating breach id %d player %d angle %f", id, player, angle);
}

void MagicInternetBox::resolveBreach(int id) {
	sendData(BreachResolve, -1.0f, id, -1, -1, -1.0f);
	CULog("Sending resolve id %d", id);
}

void MagicInternetBox::createDualTask(float angle, int player1, int player2, int id) {
	sendData(DualCreate, angle, id, player1, player2, -1.0f);
}

void MagicInternetBox::flagDualTask(int id) { sendData(DualResolve, -1.0f, id, -1, -1, -1.0f); }
