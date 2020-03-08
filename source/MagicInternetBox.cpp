#include "MagicInternetBox.h"

using namespace cugl;

constexpr auto GAME_SERVER = "wss://sweetspace-server.azurewebsites.net";

bool MagicInternetBox::initHost() {
	ws = easywsclient::WebSocket::from_url(GAME_SERVER);
	if (!ws) {
		return false;
	}
	ws->send("hello world");
	return true;
}

bool MagicInternetBox::initClient(std::string id) { return false; }

void MagicInternetBox::leaveRoom() {}

std::string MagicInternetBox::getRoomID() { return std::string(); }

int MagicInternetBox::getPlayerID() { return 0; }

void MagicInternetBox::update() { ws->poll(); }

void MagicInternetBox::createBreach(float angle, int player, int id) {}

void MagicInternetBox::resolveBreach(int id) {}

void MagicInternetBox::createDualTask(float angle, int player1, int player2, int id) {}

void MagicInternetBox::flagDualTask(int id) {}
