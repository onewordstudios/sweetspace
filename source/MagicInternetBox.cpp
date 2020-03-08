#include "MagicInternetBox.h"

using namespace cugl;

constexpr auto GAME_SERVER = "ws://sweetspace-server.azurewebsites.net/";

bool MagicInternetBox::initHost() {
	ws = easywsclient::WebSocket::from_url(GAME_SERVER);
	if (!ws) {
		CULog("FAILED TO CONNECT");
		return false;
	}
	ws->send("hello world");
	return true;
}

bool MagicInternetBox::initClient(std::string id) { return initHost(); }

void MagicInternetBox::leaveRoom() {}

std::string MagicInternetBox::getRoomID() { return std::string(); }

int MagicInternetBox::getPlayerID() { return 0; }

void MagicInternetBox::update() {
	if (ws == nullptr) {
		CULog("WS WAS NULLPTR");
		return;
	}
	ws->poll();
	ws->dispatch([](const std::string& message) { CULog(">>> %s\n", message.c_str()); });
}

void MagicInternetBox::createBreach(float angle, int player, int id) {}

void MagicInternetBox::resolveBreach(int id) {}

void MagicInternetBox::createDualTask(float angle, int player1, int player2, int id) {}

void MagicInternetBox::flagDualTask(int id) {}
