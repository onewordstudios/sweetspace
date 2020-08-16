#ifndef __CUSTOM_SERVER_H__
#define __CUSTOM_SERVER_H__

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#include <asio/io_service.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#ifdef __CUSTOM_SERVER_H__
// This is literally just a hack to get <functional> under the other two includes
// because clang-format will stick it up there otherwise
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#endif

typedef websocketpp::server<websocketpp::config::asio> WebsocketEndpoint;
typedef websocketpp::connection_hdl ClientConnection;

class CustomServer {
   public:
	CustomServer();
	void run(int port);

	// Returns the number of currently connected clients
	size_t numConnections();

	// Registers a callback for when a client connects
	template <typename CallbackTy>
	void connect(CallbackTy handler) {
		// Make sure we only access the handlers list from the networking thread
		this->eventLoop.post([this, handler]() { this->connectHandlers.push_back(handler); });
	}

	// Registers a callback for when a client disconnects
	template <typename CallbackTy>
	void disconnect(CallbackTy handler) {
		// Make sure we only access the handlers list from the networking thread
		this->eventLoop.post([this, handler]() { this->disconnectHandlers.push_back(handler); });
	}

	// Registers a callback for when a particular type of message is received
	template <typename CallbackTy>
	void message(const std::string& messageType, CallbackTy handler) {
		// Make sure we only access the handlers list from the networking thread
		this->eventLoop.post([this, messageType, handler]() {
			this->messageHandlers[messageType].push_back(handler);
		});
	}

	// Sends a message to an individual client
	//(Note: the data transmission will take place on the thread that called WebsocketServer::run())
	void sendMessage(ClientConnection conn, const std::string& messageType);

	// Sends a message to all connected clients
	//(Note: the data transmission will take place on the thread that called WebsocketServer::run())
	void broadcastMessage(const std::string& messageType);

   protected:
	void onOpen(ClientConnection conn);
	void onClose(ClientConnection conn);
	void onMessage(ClientConnection conn, WebsocketEndpoint::message_ptr msg);

	asio::io_service eventLoop;
	WebsocketEndpoint endpoint;
	std::vector<ClientConnection> openConnections;
	std::mutex connectionListMutex;

	std::vector<std::function<void(ClientConnection)>> connectHandlers;
	std::vector<std::function<void(ClientConnection)>> disconnectHandlers;
	std::map<std::string, std::vector<std::function<void(ClientConnection)>>> messageHandlers;
};
#endif /* __CUSTOM_SERVER_H__ */
