#include "MagicInternetBox.h"

using namespace cugl;

bool MagicInternetBox::initHost() { return false; }

bool MagicInternetBox::initClient(std::string id) { return false; }

void MagicInternetBox::leaveRoom() {}

std::string MagicInternetBox::getRoomID() { return std::string(); }

int MagicInternetBox::getPlayerID() { return 0; }

void MagicInternetBox::update() {}

void MagicInternetBox::createBreach(float angle, int player, int id) {}

void MagicInternetBox::resolveBreach(int id) {}

void MagicInternetBox::createDualTask(float angle, int player1, int player2, int id) {}

void MagicInternetBox::flagDualTask(int id) {}
