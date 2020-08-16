#include "CustomServer.h"

#include <algorithm>
#include <functional>
#include <iostream>

using namespace std;

CustomServer::CustomServer() {
	// Wire up our event handlers
	this->endpoint.set_open_handler(std::bind(&CustomServer::onOpen, this, std::placeholders::_1));
	this->endpoint.set_close_handler(
		std::bind(&CustomServer::onClose, this, std::placeholders::_1));
	this->endpoint.set_message_handler(
		std::bind(&CustomServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));

	// Initialise the Asio library, using our own event loop object
	this->endpoint.init_asio(&(this->eventLoop));
}

void CustomServer::run(int port) {
	// Listen on the specified port number and start accepting connections
	this->endpoint.listen(port);
	this->endpoint.start_accept();

	// Start the Asio event loop
	this->endpoint.run();
}

size_t CustomServer::numConnections() {
	// Prevent concurrent access to the list of open connections from multiple threads
	std::lock_guard<std::mutex> lock(this->connectionListMutex);

	return this->openConnections.size();
}

void CustomServer::sendMessage(ClientConnection conn, const string& messageType) {
	// Send the JSON data to the client (will happen on the networking thread's event loop)
	this->endpoint.send(conn, messageType, websocketpp::frame::opcode::text);
}

void CustomServer::broadcastMessage(const string& messageType) {
	// Prevent concurrent access to the list of open connections from multiple threads
	std::lock_guard<std::mutex> lock(this->connectionListMutex);

	for (auto conn : this->openConnections) {
		this->sendMessage(conn, messageType);
	}
}

void CustomServer::onOpen(ClientConnection conn) {
	{
		// Prevent concurrent access to the list of open connections from multiple threads
		std::lock_guard<std::mutex> lock(this->connectionListMutex);

		// Add the connection handle to our list of open connections
		this->openConnections.push_back(conn);
	}

	// Invoke any registered handlers
	for (auto handler : this->connectHandlers) {
		handler(conn);
	}
}

void CustomServer::onClose(ClientConnection conn) {
	{
		// Prevent concurrent access to the list of open connections from multiple threads
		std::lock_guard<std::mutex> lock(this->connectionListMutex);

		// Remove the connection handle from our list of open connections
		auto connVal = conn.lock();
		auto newEnd = std::remove_if(this->openConnections.begin(), this->openConnections.end(),
									 [&connVal](ClientConnection elem) {
										 // If the pointer has expired, remove it from the vector
										 if (elem.expired() == true) {
											 return true;
										 }

										 // If the pointer is still valid, compare it to the handle
										 // for the closed connection
										 auto elemVal = elem.lock();
										 if (elemVal.get() == connVal.get()) {
											 return true;
										 }

										 return false;
									 });

		// Truncate the connections vector to erase the removed elements
		this->openConnections.resize(std::distance(openConnections.begin(), newEnd));
	}

	// Invoke any registered handlers
	for (auto handler : this->disconnectHandlers) {
		handler(conn);
	}
}

void CustomServer::onMessage(ClientConnection conn, WebsocketEndpoint::message_ptr msg) {
	broadcastMessage(msg->get_payload());
}
