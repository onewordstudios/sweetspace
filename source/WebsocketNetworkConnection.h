#ifndef WEBSOCKET_NETWORK_CONNECTION_H
#define WEBSOCKET_NETWORK_CONNECTION_H

#include "CUNetworkConnection.h"
#include "libraries/easywsclient.hpp"

namespace cugl {
/**
 * Network connection to other players with a client-server implementation.
 */
class WebsocketNetworkConnection : public NetworkConnection {
   public:
#pragma region Setup
	explicit WebsocketNetworkConnection(ConnectionConfig config);

	WebsocketNetworkConnection(ConnectionConfig config, std::string roomID);

	virtual ~WebsocketNetworkConnection();
#pragma endregion

#pragma region Main Networking Methods
	void send(const std::vector<uint8_t>& msg) override;

	void sendOnlyToHost(const std::vector<uint8_t>& msg) override;

	void receive(const std::function<void(const std::vector<uint8_t>&)>& dispatcher) override;

	void manualDisconnect() override;
#pragma endregion

#pragma region State Management
	void startGame() override;
#pragma endregion

#pragma region Getters
	NetStatus getStatus() const override;

	tl::optional<uint8_t> getPlayerID() const override { return playerID; }

	std::string getRoomID() const override { return roomID; }

	bool isPlayerActive(uint8_t playerID) const override { return connectedPlayers.test(playerID); }

	uint8_t getNumPlayers() const override { return numPlayers; }

	uint8_t getTotalPlayers() const override { return maxPlayers; }
#pragma endregion

   private:
	static constexpr size_t ONE_BYTE = 256;

	/** Current status */
	NetStatus status;
	/** API version number */
	const uint8_t apiVer; // NOLINT
	/** Number of players currently connected */
	uint8_t numPlayers;
	/** Number of players connected when the game started */
	uint8_t maxPlayers;
	/** Current player ID */
	tl::optional<uint8_t> playerID;
	/** Connected room ID */
	std::string roomID;
	/** Which players are active */
	std::bitset<ONE_BYTE> connectedPlayers;

	/** The actual websocket connection */
	easywsclient::WebSocket::pointer ws;

	/**
	 * Initialize the network connection.
	 * Will establish a connection to the server.
	 *
	 * @returns Whether the connection was successfully established
	 */
	bool initConnection(ConnectionConfig config);

	enum CustomDataPackets : uint8_t {
		GeneralMsg = 0,
		HostMsg,

		PlayerJoined = 50,
		PlayerDisconnect,
		StartGame,

		AssignedRoom = 100,
		JoinRoom,
		ApiMismatch
	};
};
}; // namespace cugl

#endif // WEBSOCKET_NETWORK_CONNECTION_H