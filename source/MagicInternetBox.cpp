#include "MagicInternetBox.h"

#include <sstream>

#include "Globals.h"
#include "LevelConstants.h"

using namespace cugl;

#pragma region API CONSTANTS

/** The networking server */
constexpr auto GAME_SERVER = "ws://sweetspace-server.herokuapp.com/";

/** API version number. Bump this everytime a backwards incompatible API change happens. */
constexpr uint8_t API_VER = 0;

#pragma endregion

/** The precision to multiply floating point numbers by */
constexpr float FLOAT_PRECISION = 10.0f;

/** The state synchronization frequency */
constexpr unsigned int STATE_SYNC_FREQ = globals::NETWORK_TICK * 5;

/** One byte */
constexpr unsigned int ONE_BYTE = 256;

/** How close to consider floating point numbers identical */
constexpr float FLOAT_EPSILON = 0.1f;

/** How many ticks without a server message before considering oneself disconnected */
constexpr unsigned int SERVER_TIMEOUT = 300;

std::shared_ptr<MagicInternetBox> MagicInternetBox::instance;

#pragma region Initialization

MagicInternetBox::MagicInternetBox()
	: activePlayers(), stateReconciler(ONE_BYTE, FLOAT_PRECISION, FLOAT_EPSILON) {
	ws = nullptr;
	status = Uninitialized;
	events = None;
	levelNum = -1;
	currFrame = 0;
	playerID = -1;
	numPlayers = 0;
	maxPlayers = 0;
	lastConnection = 0;
	activePlayers.fill(false);
}

bool MagicInternetBox::initConnection() {
	switch (status) {
		case Disconnected:
		case Uninitialized:
		case HostError:
		case ClientRoomInvalid:
		case ClientRoomFull:
		case ClientError:
		case ReconnectError:
			break;
		default:
			CULog("ERROR: MIB already initialized");
			return false;
	}
	using easywsclient::WebSocket;

	// I actually don't know what this stuff does but it won't run on Windows without it,
	// so ¯\_(ツ)_/¯
#ifdef _WIN32
	INT rc; // NOLINT
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		CULog("WSAStartup Failed");
		return false;
	}
#endif

	ws = WebSocket::from_url(GAME_SERVER);
	if (!ws) {
		CULog("FAILED TO CONNECT");
		return false;
	}

	stateReconciler.reset();
	return true;
}

bool MagicInternetBox::initHost() {
	if (!initConnection()) {
		status = HostError;
		return false;
	}

	std::vector<uint8_t> data;
	data.push_back((uint8_t)NetworkDataType::AssignedRoom);
	data.push_back(API_VER);
	ws->sendBinary(data);
	this->playerID = 0;
	this->numPlayers = 1;
	status = HostConnecting;

	return true;
}

bool MagicInternetBox::initClient(std::string id) {
	if (!initConnection()) {
		status = ClientError;
		return false;
	}

	std::vector<uint8_t> data;
	data.push_back((uint8_t)NetworkDataType::JoinRoom);
	for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
		data.push_back((uint8_t)id.at(i));
	}
	data.push_back(API_VER);
	ws->sendBinary(data);
	roomID = id;
	status = ClientConnecting;

	return true;
}

bool MagicInternetBox::reconnect() {
	if (!initConnection() || playerID < 0 || roomID == "") {
		status = ReconnectError;
		return false;
	}

	std::vector<uint8_t> data;
	data.push_back((uint8_t)NetworkDataType::JoinRoom);
	for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
		data.push_back((uint8_t)roomID.at(i));
	}
	data.push_back(playerID);
	ws->sendBinary(data);
	status = Reconnecting;

	return true;
}

#pragma endregion

/*

DATA FORMAT

[ TYPE (enum) | ANGLE (2 bytes) | ID (2 bytes) | data1 (2 bytes) | data2 (2 bytes) | data3 (3 bytes)

Each 2-byte block is stored smaller first, then larger; ie 2^8 * byte1 + byte0 gives the original.
All data is truncated to fit 16 bytes.
Floats are multiplied by FLOAT_PRECISION and then cast to int before running through same algorithm
Only data3 can handle negative numbers. The first byte is 1 for positive and 0 for negative.

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

	int d3Positive = data3 >= 0 ? 1 : 0;
	int d3 = (int)(FLOAT_PRECISION * abs(data3));

	data.push_back((uint8_t)d3Positive);
	data.push_back((uint8_t)(d3 % ONE_BYTE));
	data.push_back((uint8_t)(d3 / ONE_BYTE));

	ws->sendBinary(data);
}

MagicInternetBox::MatchmakingStatus MagicInternetBox::matchStatus() { return status; }

void MagicInternetBox::leaveRoom() {}

#pragma region Getters

std::string MagicInternetBox::getRoomID() { return roomID; }

int MagicInternetBox::getPlayerID() { return playerID; }

unsigned int MagicInternetBox::getNumPlayers() { return numPlayers; }

bool MagicInternetBox::isPlayerActive(unsigned int playerID) { return activePlayers.at(playerID); }

#pragma endregion

void MagicInternetBox::startGame(int levelNum) {
	switch (status) {
		case HostWaitingOnOthers:
		case ClientWaitingOnOthers:
			break;
		default:
			CULog("ERROR: Trying to start game during invalid state %d", status);
			return;
	}

	std::vector<uint8_t> data;
	data.push_back((uint8_t)StartGame);
	data.push_back((uint8_t)levelNum);
	this->levelNum = levelNum;
	ws->sendBinary(data);

	maxPlayers = numPlayers;
	status = GameStart;
	stateReconciler.reset();
}

void MagicInternetBox::restartGame() {
	if (status != GameStart) {
		CULog("ERROR: Trying to restart game during invalid state %d", status);
		return;
	}
	std::vector<uint8_t> data;
	data.push_back((uint8_t)ChangeGame);
	data.push_back((uint8_t)0);
	ws->sendBinary(data);
	events = RestartLevel;
	stateReconciler.reset();
}

void MagicInternetBox::nextLevel() {
	if (status != GameStart) {
		CULog("ERROR: Trying to move to next level during invalid state %d", status);
		return;
	}
	std::vector<uint8_t> data;
	data.push_back((uint8_t)ChangeGame);
	data.push_back((uint8_t)1);
	ws->sendBinary(data);
	events = NextLevel;
	levelNum++;
	stateReconciler.reset();
	if (levelNum >= MAX_NUM_LEVELS) {
		events = EndGame;
	}
}

void MagicInternetBox::update() {
	switch (status) {
		case Uninitialized:
			CULog("ERROR: Update called on MIB before initialization; aborting");
			return;
		case GameStart:
			CULog("ERROR: Matchmaking update called on MIB after game start; aborting");
			return;
		case ClientRoomInvalid:
		case ClientRoomFull:
			CULog("Tried to call update() after failed client connection; aborting");
			return;
		default:
			break;
	}

	ws->poll();
	ws->dispatchBinary([this](const std::vector<uint8_t>& message) {
		if (message.size() == 0) {
			return;
		}

		NetworkDataType type = static_cast<NetworkDataType>(message[0]);

		switch (type) {
			case ApiMismatch: {
				CULog("API Mismatch Occured; Aborting");
				if (playerID == 0) {
					status = HostApiMismatch;
				} else {
					status = ClientError;
				}
				return;
			}
			case AssignedRoom: {
				std::stringstream newRoomId;
				for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
					newRoomId << (char)message[i + 1];
				}
				activePlayers[0] = true;
				roomID = newRoomId.str();
				CULog("Got room ID: %s", roomID.c_str());
				status = HostWaitingOnOthers;
				return;
			}
			case JoinRoom: {
				switch (message[1]) {
					case 0: {
						numPlayers = message[2];
						playerID = (int)numPlayers - 1;
						CULog("Join Room Success; player id %d", playerID);
						for (unsigned int i = 0; i < numPlayers; i++) {
							activePlayers.at(i) = true;
						}
						status = ClientWaitingOnOthers;
						return;
					}
					case 1: {
						CULog("Room Does Not Exist");
						status = ClientRoomInvalid;
						ws->close();
						return;
					}
					case 2: {
						CULog("Room Full");
						status = ClientRoomFull;
						ws->close();
						return;
					}
					case 3:
					case 4: {
						// Reconnecting
						if (status != Reconnecting) {
							CULog(
								"ERROR: Received reconnecting response from server when not "
								"reconnecting");
							return;
						}
						status = message[1] == 3 ? GameStart : ReconnectError;
						return;
					}
				}
			}
			case PlayerJoined: {
				CULog("Player Joined");
				activePlayers.at(numPlayers) = true;
				numPlayers++;
				return;
			}
			case StartGame: {
				status = GameStart;
				maxPlayers = numPlayers;
				levelNum = message[1];
				stateReconciler.reset();
				return;
			}
			default:
				CULog("Received invalid gameplay message during connection; %d", message[0]);
				return;
		}
	});
}

void MagicInternetBox::update(std::shared_ptr<ShipModel> state) {
	if (status != GameStart) {
		CULog("ERROR: Gameplay update called on MIB before game start; aborting");
		return;
	}

	lastConnection++;

	// NETWORK TICK
	currFrame = (currFrame + 1) % STATE_SYNC_FREQ;
	if (currFrame % globals::NETWORK_TICK == 0) {
		std::shared_ptr<DonutModel> player = state->getDonuts()[playerID];
		float angle = player->getAngle();
		float velocity = player->getVelocity();
		sendData(PositionUpdate, angle, playerID, -1, -1, velocity);

		// STATE SYNC (and check for server connection)
		if (currFrame == 0) {
			if (playerID == 0) {
				if (!state->isLevelOver()) {
					std::vector<uint8_t> data;
					data.push_back(StateSync);
					stateReconciler.encode(state, data);
					ws->sendBinary(data);
				}
			}
			if (lastConnection > SERVER_TIMEOUT) {
				CULog("HAS NOT RECEIVED SERVER MESSAGE IN TIMEOUT FRAMES; assuming disconnected");
				forceDisconnect();
				return;
			}
		}
	}

	ws->poll();
	ws->dispatchBinary([&state, this](const std::vector<uint8_t>& message) {
		if (message.size() == 0) {
			return;
		}

		NetworkDataType type = static_cast<NetworkDataType>(message[0]);

		if (type > AssignedRoom) {
			CULog("Received invalid connection message during gameplay; %d", message[0]);
			return;
		}

		lastConnection = 0;

		switch (type) {
			case PlayerJoined: {
				numPlayers++;
				unsigned int playerID = message[1];
				CULog("Player has reconnected, %d", playerID);
				state->getDonuts()[playerID]->setIsActive(true);
				activePlayers.at(playerID) = true;
				return;
			}
			case PlayerDisconnect: {
				numPlayers--;
				unsigned int playerID = message[1];
				CULog("Player has disconnected, %d", playerID);
				state->getDonuts()[playerID]->setIsActive(false);
				activePlayers.at(playerID) = false;
				return;
			}
			case StateSync: {
				if (!state->isLevelOver()) {
					if (!stateReconciler.reconcile(state, message)) {
						CULog("Should abort level");
						// TODO abort level properly
					}
				}
				return;
			}
			case ChangeGame: {
				if (message[1] == 0) {
					events = RestartLevel;
				} else {
					events = NextLevel;
					levelNum++;
					if (levelNum >= MAX_NUM_LEVELS) {
						events = EndGame;
					}
				}
				stateReconciler.reset();
				return;
			}
			default:
				break;
		}

		if (state->isLevelOver()) {
			return;
		}

		float angle = (float)(message[1] + ONE_BYTE * message[2]) / FLOAT_PRECISION;
		int id = (int)(message[3] + ONE_BYTE * message[4]);
		// Networking code is finnicky and having these magic numbers is the easiest solution
		// NOLINTNEXTLINE Simple counting numbers
		int data1 = (int)(message[5] + ONE_BYTE * message[6]);
		// NOLINTNEXTLINE Ditto
		int data2 = (int)(message[7] + ONE_BYTE * message[8]);
		// NOLINTNEXTLINE Ditto
		float data3 = (message[9] == 1 ? 1 : -1) * (float)(message[10] + ONE_BYTE * message[11]) /
					  FLOAT_PRECISION;

		switch (type) {
			case PositionUpdate: {
				std::shared_ptr<DonutModel> donut = state->getDonuts()[id];
				donut->setAngle(angle);
				donut->setVelocity(data3);
				break;
			}
			case Jump: {
				state->getDonuts()[id]->startJump();
				break;
			}
			case BreachCreate: {
				state->createBreach(angle, data1, id);
				CULog("Creating breach %d at angle %f with user %d", id, angle, data1);
				break;
			}
			case BreachShrink: {
				state->resolveBreach(id);
				CULog("Resolve breach %d", id);
				break;
			}
			case DualCreate: {
				int taskID = id;
				state->createDoor(angle, taskID);
				break;
			}
			case DualResolve: {
				int taskID = id;
				int player = data1;
				int flag = data2;
				state->flagDoor(taskID, player, flag);
				break;
			}
			case ButtonCreate: {
				float angle1 = angle;
				float angle2 = data3;
				int id1 = id;
				int id2 = data1;
				state->createButton(angle1, id1, angle2, id2);
				break;
			}
			case ButtonFlag: {
				state->flagButton(id);
				break;
			}
			case ButtonResolve: {
				state->resolveButton(id);
				CULog("Resolve button %d", id);
				break;
			}
			case AllCreate: {
				if (playerID == id) {
					state->createAllTask();
				}
				state->setStabilizerStatus(ShipModel::ACTIVE);
				break;
			}
			case AllFail: {
				state->failAllTask();
				state->setStabilizerStatus(ShipModel::FAILURE);
				break;
			}
			case AllSucceed: {
				state->setStabilizerStatus(ShipModel::SUCCESS);
				break;
			}
			case ForceWin: {
				state->setTimeless(false);
				state->initTimer(0);
				break;
			}
			default:
				break;
		}
	});
}

void MagicInternetBox::createBreach(float angle, int player, int id) {
	sendData(BreachCreate, angle, id, player, -1, -1.0f);
	CULog("Creating breach id %d player %d angle %f", id, player, angle);
}

void MagicInternetBox::resolveBreach(int id) {
	sendData(BreachShrink, -1.0f, id, -1, -1, -1.0f);
	CULog("Sending resolve id %d", id);
}

void MagicInternetBox::createDualTask(float angle, int id) {
	sendData(DualCreate, angle, id, -1, -1, -1.0f);
}

void MagicInternetBox::flagDualTask(int id, int player, int flag) {
	sendData(DualResolve, -1.0f, id, player, flag, -1.0f);
}

void MagicInternetBox::createAllTask(int player) {
	sendData(AllCreate, -1.0f, player, -1, -1, -1.0f);
}

void MagicInternetBox::createButtonTask(float angle1, int id1, float angle2, int id2) {
	sendData(ButtonCreate, angle1, id1, id2, -1, angle2);
}

void MagicInternetBox::flagButton(int id) { sendData(ButtonFlag, -1, id, -1, -1, -1.0f); }

void MagicInternetBox::resolveButton(int id) { sendData(ButtonResolve, -1, id, -1, -1, -1.0f); }

void MagicInternetBox::failAllTask() { sendData(AllFail, -1.0f, -1, -1, -1, -1.0f); }

void MagicInternetBox::succeedAllTask() { sendData(AllSucceed, -1.0f, -1, -1, -1, -1.0f); }

void MagicInternetBox::forceWinLevel() { sendData(ForceWin, -1.0f, -1, -1, -1, -1.0f); }

void MagicInternetBox::jump(int player) { sendData(Jump, -1.0f, player, -1, -1, -1.0f); }

void MagicInternetBox::forceDisconnect() {
	CULog("Force disconnecting");
	if (ws == nullptr) {
		return;
	}

	std::vector<uint8_t> data;
	data.push_back((uint8_t)PlayerDisconnect);
	ws->sendBinary(data);
	ws->poll();

	ws->close();
	status = Disconnected;
	lastConnection = 0;
}

void MagicInternetBox::reset() {
	forceDisconnect();
	delete ws;
	ws = nullptr;
	status = Uninitialized;
	activePlayers.fill(false);
	stateReconciler.reset();
}
