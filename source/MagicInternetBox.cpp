#include "MagicInternetBox.h"

#include <array>
#include <sstream>

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

#include "Globals.h"
#include "LevelConstants.h"
#include "NetworkConnection.h"
#include "NetworkDataType.h"
#include "StateReconciler.h"

/** The state synchronization frequency */
constexpr unsigned int STATE_SYNC_FREQ = globals::NETWORK_TICK * 5;

/** Minimum number of seconds to wait after a connection attempt before allowing retrys */
constexpr double MIN_WAIT_TIME = 0.5;

/** How many ticks without a server message before considering oneself disconnected */
constexpr unsigned int SERVER_TIMEOUT = 300;

class MagicInternetBox::Mimpl {
   private:
	/** The network connection */
	std::unique_ptr<NetworkConnection> conn;

	/** The current status */
	MatchmakingStatus status;

	/** The last major unacknowledged network event */
	NetworkEvents events;

	/** The current frame, modulo the network tick rate. */
	unsigned int currFrame;

	/** ID of the current player, or empty if unassigned */
	tl::optional<uint8_t> playerID;

	/** The ID of the current room, or "" if unassigned */
	std::string roomID;

	/** Current level number, or empty if unassigned */
	tl::optional<uint8_t> levelNum;
	/** Parity of current level (to sync state syncs) */
	bool levelParity;

	/** Whether to skip tutorial levels */
	bool skipTutorial;

	/** Start the given level */
	void startLevelInternal(uint8_t num, bool parity) {
		levelNum = num;
		levelParity = parity;
		stateReconciler.reset();
		if (num >= MAX_NUM_LEVELS || num < 0) {
			events = EndGame;
		} else {
			events = LoadLevel;
		}
	}

	/** Number of connected players */
	uint8_t numPlayers;

	/** Maximum number of players for this ship */
	uint8_t maxPlayers;

	/** Array representing active and inactive players */
	std::array<bool, globals::MAX_PLAYERS> activePlayers;

	/** Helper controller to reconcile states during state sync */
	StateReconciler stateReconciler;

	/** Number of frames since the last inbound server message */
	unsigned int lastConnection;

	/** Time at which the last connection was attempted */
	std::chrono::time_point<std::chrono::system_clock> lastAttemptConnectionTime;

	/**
	 * Initialize the network connection.
	 * Will establish a connection to the server.
	 *
	 * @returns Whether the connection was successfully established
	 */
	bool initConnection() {
		switch (status) {
			case Disconnected:
			case Uninitialized:
			case HostError:
			case ClientRoomInvalid:
			case ClientRoomFull:
			case ClientApiMismatch:
			case ClientError:
			case ReconnectError:
				break;
			default:
				CULog("ERROR: MIB already initialized");
				return false;
		}

		auto currTime = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = currTime - lastAttemptConnectionTime;
		if (diff.count() < MIN_WAIT_TIME) {
			CULog("Reconnect attempt too fast; aborting");
			return false;
		}
		lastAttemptConnectionTime = currTime;

		stateReconciler.reset();
		skipTutorial = false;
		return true;
	}

	/**
	 * Send data over the network as described in the architecture specification.
	 *
	 * Angle field is for the angle, if applicable.
	 * ID field is for the ID of the object being acted on, if applicable.
	 * Remaining data fields should be filled from first applicable data type back in the same order
	 * that arguments are passed to the calling method in this class.
	 *
	 * Any unused fields should be set to -1.
	 *
	 * For example, create dual task passes angle to angle, task id to id, the two players to data1
	 * and data2 respectively, and sets data3 to -1.
	 */
	void sendData(NetworkDataType type, float angle, uint8_t id, uint8_t data1, uint8_t data2,
				  float data3) {
		if (conn == nullptr) {
			CULogError("Attempted to send data to a null network connection; dropping");
			return;
		}
		/*

		DATA FORMAT

		[ TYPE (enum) | ANGLE (2 bytes) | ID (1 byte) | data1 (1 byte) | data2 (1 byte) | data3
		(3 bytes) ]

		Each 2-byte block is stored smaller first, then larger; ie 2^8 * byte1 + byte0 gives
		the original. All data is truncated to fit 16 bytes. Floats are multiplied by
		FLOAT_PRECISION and then cast to int before running through same algorithm. Only data3 can
		handle negative numbers. The first byte is 1 for positive and 0 for negative.

		*/

		std::vector<uint8_t> data;

		data.push_back(static_cast<uint8_t>(type));

		StateReconciler::encodeFloat(angle, data);

		data.push_back(id);
		data.push_back(data1);
		data.push_back(data2);

		uint8_t d3Positive = data3 >= 0 ? 1 : 0;
		data.push_back(d3Positive);
		StateReconciler::encodeFloat(abs(data3), data);

		conn->send(data);
	}

   public:
#pragma region Initialization
	Mimpl() : activePlayers() {
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

		conn = nullptr;
		status = Uninitialized;
		events = None;
		levelParity = true;
		currFrame = 0;
		skipTutorial = false;
		numPlayers = 0;
		maxPlayers = 0;
		lastConnection = 0;
		activePlayers.fill(false);
	}

	bool initHost() {
		if (!initConnection()) {
			status = HostError;
			return false;
		}

		conn = std::make_unique<NetworkConnection>();

		this->playerID = 0;
		this->numPlayers = 1;
		status = HostConnecting;

		return true;
	}

	bool initClient(const std::string& id) {
		if (!initConnection()) {
			status = ClientError;
			return false;
		}

		conn = std::make_unique<NetworkConnection>(id);

		roomID = id;
		status = ClientConnecting;

		return true;
	}

	bool reconnect() {
		if (!initConnection() || !playerID.has_value() || roomID.empty()) {
			status = ReconnectError;
			return false;
		}

		conn = std::make_unique<NetworkConnection>(roomID);
		status = Reconnecting;

		return true;
	}
#pragma endregion

#pragma region Getters
	MagicInternetBox::MatchmakingStatus matchStatus() { return status; }

	MagicInternetBox::NetworkEvents lastNetworkEvent() { return events; }

	void acknowledgeNetworkEvent() { events = MagicInternetBox::NetworkEvents::None; }

	std::string getRoomID() { return roomID; }

	tl::optional<uint8_t> getLevelNum() { return levelNum; }

	tl::optional<uint8_t> getPlayerID() { return playerID; }

	uint8_t getNumPlayers() const { return numPlayers; }

	uint8_t getMaxNumPlayers() const { return maxPlayers; }

	bool isPlayerActive(uint8_t playerID) { return activePlayers.at(playerID); }
#pragma endregion

	void setSkipTutorial(bool skip) { skipTutorial = skip; }

#pragma region Game Management
	void startGame(uint8_t levelNum) {
		switch (status) {
			case HostWaitingOnOthers:
			case ClientWaitingOnOthers:
				break;
			default:
				CULog("ERROR: Trying to start game during invalid state %d", status);
				return;
		}

		if (skipTutorial) {
			while (strcmp(LEVEL_NAMES.at(levelNum), "") == 0) {
				CULog("Level Num %d is a tutorial; skipping", levelNum);
				levelNum++;
			}
		}
		std::vector<uint8_t> data;
		data.push_back(uint8_t{StartGame});
		data.push_back(levelNum);
		this->levelNum = levelNum;
		conn->send(data);

		maxPlayers = numPlayers;
		status = GameStart;
		stateReconciler.reset();
		conn->startGame();
	}

	void restartGame() {
		if (status != GameStart) {
			CULog("ERROR: Trying to restart game during invalid state %d", status);
			return;
		}

		levelParity = !levelParity;

		std::vector<uint8_t> data;
		data.push_back(uint8_t{ChangeGame});
		data.push_back(0);
		data.push_back(levelParity ? 1 : 0);
		conn->send(data);

		startLevelInternal(levelNum.value(), levelParity);
	}

	void nextLevel() {
		if (status != GameStart) {
			CULog("ERROR: Trying to move to next level during invalid state %d", status);
			return;
		}

		uint8_t level = levelNum.value() + 1;
		if (skipTutorial) {
			while (strcmp(LEVEL_NAMES.at(level), "") == 0) {
				CULog("Level Num %d is a tutorial; skipping", level);
				level++;
			}
		}
		levelParity = !levelParity;
		startLevelInternal(level, levelParity);

		std::vector<uint8_t> data;
		data.push_back(uint8_t{ChangeGame});
		data.push_back(1);
		data.push_back(level);
		data.push_back(levelParity ? 1 : 0);
		conn->send(data);
	}
#pragma endregion

#pragma region Inbound Networking
	void update() {
		switch (status) {
			case GameStart:
				CULog("ERROR: Matchmaking update called on MIB after game start; aborting");
			case Uninitialized:
			case ClientRoomInvalid:
			case ClientRoomFull:
			case ClientApiMismatch:
				return;
			default:
				break;
		}

		conn->receive([this](const std::vector<uint8_t>& message) {
			if (message.empty()) {
				return;
			}

			auto type = static_cast<NetworkDataType>(message[0]);

			switch (type) {
				case GenericError: {
					if (playerID == 0) {
						if (status != HostWaitingOnOthers) {
							status = HostError;
						} else {
							CULog("Error occured; swallowing in mib");
						}

					} else {
						status = ClientError;
					}
					return;
				}
				case ApiMismatch: {
					CULog("API Mismatch Occured; Aborting");
					if (playerID == 0) {
						status = HostApiMismatch;
					} else {
						status = ClientApiMismatch;
					}
					return;
				}
				case AssignedRoom: {
					if (playerID != 0) {
						break;
					}
					std::stringstream newRoomId;
					for (size_t i = 0; i < globals::ROOM_LENGTH; i++) {
						newRoomId << static_cast<char>(message[i + 1]);
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
							playerID = message[3];
							if (message[4] > globals::API_VER) {
								CULog("Error API out of date; current is %d but server is %d",
									  globals::API_VER, message[4]);
								status = ClientApiMismatch;
								return;
							}
							CULog("Join Room Success; player id %d out of %d players", *playerID,
								  numPlayers);
							for (unsigned int i = 0; i < numPlayers; i++) {
								activePlayers.at(i) = true;
							}
							status = ClientWaitingOnOthers;
							return;
						}
						case 1: {
							CULog("Room Does Not Exist");
							status = ClientRoomInvalid;
							return;
						}
						case 2: {
							CULog("Room Full");
							status = ClientRoomFull;
							return;
						}
						case 3:
						case 4: {
							// Reconnecting
							if (status != Reconnecting) {
								CULog(
									"ERROR: Received reconnecting response from server when not "
									"reconnecting");
								status = ClientRoomFull;
								return;
							}
							status = message[1] == 3 ? ReconnectPending : ReconnectError;
							return;
						}
					}
				}
				case PlayerJoined: {
					CULog("Player Joined");
					activePlayers.at(message[1]) = true;
					numPlayers++;
					return;
				}
				case PlayerDisconnect: {
					CULog("Player Left");
					activePlayers.at(message[1]) = false;
					numPlayers--;
					return;
				}
				case StartGame: {
					status = GameStart;
					maxPlayers = numPlayers;
					levelNum = message[1];
					stateReconciler.reset();
					return;
				}
				case StateSync: {
					if (status == ReconnectPending) {
						auto t = StateReconciler::decodeLevelNum(message[1]);
						if (t.first == levelNum) {
							CULog("Reconnect success");
							status = GameStart;
						} else {
							CULog("Game level %d, local level %d; abort reconnect", t.first,
								  *levelNum);
							status = ReconnectError;
						}
					} else {
						CULog("Received state sync during connection but not reconnecting");
					}
					return;
				}
				default:
					CULog("Received invalid gameplay message during connection; %d", message[0]);
					return;
			}
		});

		switch (status) {
			case ClientApiMismatch:
			case ClientRoomInvalid:
			case ClientRoomFull:
				conn = nullptr;
				break;
			default:
				break;
		}
	}

	void update(std::shared_ptr<ShipModel> state) {
		if (status != GameStart) {
			CULog("ERROR: Gameplay update called on MIB before game start; aborting");
			return;
		}

		lastConnection++;
		uint8_t pID = playerID.value();

		// NETWORK TICK
		currFrame = (currFrame + 1) % STATE_SYNC_FREQ;
		if (currFrame % globals::NETWORK_TICK == 0) {
			std::shared_ptr<DonutModel> player = state->getDonuts()[pID];
			float angle = player->getAngle();
			float velocity = player->getVelocity();
			sendData(PositionUpdate, angle, pID, -1, -1, velocity);

			// STATE SYNC (and check for server connection)
			if (currFrame == 0) {
				if (pID == 0) {
					if (!state->isLevelOver()) {
						std::vector<uint8_t> data;
						data.push_back(StateSync);
						StateReconciler::encode(state, data, levelNum.value(), levelParity);
						conn->send(data);
					}
				}
				if (lastConnection > SERVER_TIMEOUT) {
					CULog(
						"HAS NOT RECEIVED SERVER MESSAGE IN TIMEOUT FRAMES; assuming disconnected");
					forceDisconnect();
					return;
				}
			}
		}

		conn->receive([&state, this](const std::vector<uint8_t>& message) {
			if (message.empty()) {
				return;
			}

			auto type = static_cast<NetworkDataType>(message[0]);

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
						if (!stateReconciler.reconcile(state, message, levelNum.value(),
													   levelParity)) {
							CULog("Wrong level state sync; ignoring");
						}
					}
					return;
				}
				case ChangeGame: {
					if (message[1] == 0) {
						startLevelInternal(levelNum.value(), message[2] != 0);
					} else {
						startLevelInternal(message[2], message[3] != 0);
					}
					return;
				}
				default:
					break;
			}

			if (state->isLevelOver()) {
				return;
			}

			float angle = StateReconciler::decodeFloat(message[1], message[2]);
			uint8_t id = message[3];
			uint8_t data1 = message[4];
			// Networking code is finnicky and having these magic numbers is the easiest solution
			uint8_t data2 = message[5];				   // NOLINT Simple counting numbers
			float data3 = (message[6] == 1 ? 1 : -1) * // NOLINT
						  StateReconciler::decodeFloat(message[7], message[8]); // NOLINT

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
					break;
				}
				case AllFail: {
					state->failAllTask();
					break;
				}
				case AllSucceed: {
					state->stabilizerTutorial = true;
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
		switch (status) {
			case ReconnectError:
				conn = nullptr;
			default:
				break;
		}
	}
#pragma endregion

#pragma region Outbound Networking
	void createBreach(float angle, uint8_t player, uint8_t id) {
		sendData(BreachCreate, angle, id, player, -1, -1.0f);
		CULog("Creating breach id %d player %d angle %f", id, player, angle);
	}

	void resolveBreach(uint8_t id) {
		sendData(BreachShrink, -1.0f, id, -1, -1, -1.0f);
		CULog("Sending resolve id %d", id);
	}

	void createDualTask(float angle, uint8_t id) { sendData(DualCreate, angle, id, -1, -1, -1.0f); }

	void flagDualTask(uint8_t id, uint8_t player, uint8_t flag) {
		sendData(DualResolve, -1.0f, id, player, flag, -1.0f);
	}

	void createButtonTask(float angle1, uint8_t id1, float angle2, uint8_t id2) {
		sendData(ButtonCreate, angle1, id1, id2, -1, angle2);
	}

	void flagButton(uint8_t id) { sendData(ButtonFlag, -1, id, -1, -1, -1.0f); }

	void resolveButton(uint8_t id) { sendData(ButtonResolve, -1, id, -1, -1, -1.0f); }

	void createAllTask(uint8_t player) { sendData(AllCreate, -1.0f, player, -1, -1, -1.0f); }

	void failAllTask() { sendData(AllFail, -1.0f, -1, -1, -1, -1.0f); }

	void succeedAllTask() { sendData(AllSucceed, -1.0f, -1, -1, -1, -1.0f); }

	void forceWinLevel() { sendData(ForceWin, -1.0f, -1, -1, -1, -1.0f); }

	void jump(uint8_t player) { sendData(Jump, -1.0f, player, -1, -1, -1.0f); }
#pragma endregion

	void forceDisconnect() {
		CULog("Force disconnecting");

		status = Disconnected;
		lastConnection = 0;
		conn = nullptr;
	}

	/**
	 * Reset the controller entirely; useful when leaving a game.
	 */
	void reset() {
		forceDisconnect();
		status = Uninitialized;
		activePlayers.fill(false);
		stateReconciler.reset();
		roomID = "";
		playerID = tl::nullopt;
		levelNum = tl::nullopt;
	}
};

#pragma region MIB Passthrough

MagicInternetBox::MagicInternetBox() : impl(new Mimpl) {}
MagicInternetBox::~MagicInternetBox() = default;
bool MagicInternetBox::initHost() { return impl->initHost(); }
bool MagicInternetBox::initClient(const std::string& id) { return impl->initClient(id); }
bool MagicInternetBox::reconnect() { return impl->reconnect(); }
MagicInternetBox::MatchmakingStatus MagicInternetBox::matchStatus() { return impl->matchStatus(); }
MagicInternetBox::NetworkEvents MagicInternetBox::lastNetworkEvent() {
	return impl->lastNetworkEvent();
}
void MagicInternetBox::acknowledgeNetworkEvent() { impl->acknowledgeNetworkEvent(); }
std::string MagicInternetBox::getRoomID() { return impl->getRoomID(); }
tl::optional<uint8_t> MagicInternetBox::getLevelNum() { return impl->getLevelNum(); }
tl::optional<uint8_t> MagicInternetBox::getPlayerID() { return impl->getPlayerID(); }
uint8_t MagicInternetBox::getNumPlayers() const { return impl->getNumPlayers(); }
uint8_t MagicInternetBox::getMaxNumPlayers() const { return impl->getMaxNumPlayers(); }
bool MagicInternetBox::isPlayerActive(uint8_t playerID) { return impl->isPlayerActive(playerID); }
void MagicInternetBox::setSkipTutorial(bool skip) { impl->setSkipTutorial(skip); }
void MagicInternetBox::startGame(uint8_t levelNum) { impl->startGame(levelNum); }
void MagicInternetBox::restartGame() { impl->restartGame(); }
void MagicInternetBox::nextLevel() { impl->nextLevel(); }
void MagicInternetBox::update() { impl->update(); }
void MagicInternetBox::update(std::shared_ptr<ShipModel> state) { impl->update(std::move(state)); }
void MagicInternetBox::createBreach(float angle, uint8_t player, uint8_t id) {
	impl->createBreach(angle, player, id);
}
void MagicInternetBox::resolveBreach(uint8_t id) { impl->resolveBreach(id); }
void MagicInternetBox::createDualTask(float angle, uint8_t id) { impl->createDualTask(angle, id); }
void MagicInternetBox::flagDualTask(uint8_t id, uint8_t player, uint8_t flag) {
	impl->flagDualTask(id, player, flag);
}
void MagicInternetBox::createAllTask(uint8_t player) { impl->createAllTask(player); }
void MagicInternetBox::createButtonTask(float angle1, uint8_t id1, float angle2, uint8_t id2) {
	impl->createButtonTask(angle1, id1, angle2, id2);
}
void MagicInternetBox::flagButton(uint8_t id) { impl->flagButton(id); }
void MagicInternetBox::resolveButton(uint8_t id) { impl->resolveButton(id); }
void MagicInternetBox::failAllTask() { impl->failAllTask(); }
void MagicInternetBox::succeedAllTask() { impl->succeedAllTask(); }
void MagicInternetBox::forceWinLevel() { impl->forceWinLevel(); }
void MagicInternetBox::jump(uint8_t player) { impl->jump(player); }
void MagicInternetBox::forceDisconnect() { impl->forceDisconnect(); }
void MagicInternetBox::reset() { impl->reset(); }

#pragma endregion
