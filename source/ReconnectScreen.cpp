#include "ReconnectScreen.h"

#include "Globals.h"

/** Milliseconds before connection timeout */
constexpr int CONNECTION_TIMEOUT = 15000;

/** Milliseconds in a second */
constexpr int MILLISECONDS_IN_SECONDS = 1000;

/** Ratio of spin on reconnect donut */
constexpr float RECONNECT_SPIN_RATIO = 0.26f;

/** Animation cycle length of ellipses */
constexpr int MAX_ELLIPSES_FRAMES = 180;

ReconnectScreen::ReconnectScreen(const std::shared_ptr<cugl::AssetManager>& assets) : currFrame(0) {
	Node::init();
	setAnchor({1.f / 2, 1.f / 2});
	setPosition(0, 0);
	setVisible(false);

	auto screen = assets->get<Node>("reconnect");
	addChild(screen);

	reconnScreen = assets->get<Node>("reconnect_reconnect");
	disconnScreen = assets->get<Node>("reconnect_timeout");

	ellipsis2 = assets->get<Node>("reconnect_reconnect_ellipsis2");
	ellipsis3 = assets->get<Node>("reconnect_reconnect_ellipsis3");
	donut = assets->get<Node>("reconnect_reconnect_donut");

	countdown = dynamic_pointer_cast<cugl::Label>(assets->get<Node>("reconnect_timeout_countdown"));

	timeoutCurrent.mark();
	timeoutStart.mark();

	cugl::Size dimen = cugl::Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width;
	setContentSize(dimen);
	screen->setPosition(static_cast<float>(globals::SCENE_WIDTH) / 2, dimen.height / 2);
	doLayout();
}

ReconnectScreen::~ReconnectScreen() {
	Node::dispose();
	removeAllChildren();
}

void ReconnectScreen::activate() {
	if (_isVisible) {
		return;
	}

	setVisible(true);
	currFrame = 0;
	disconnScreen->setVisible(false);
	reconnScreen->setVisible(true);
}

void ReconnectScreen::deactivate() { setVisible(false); }

bool ReconnectScreen::update() {
	if (!_isVisible) {
		return false;
	}

	// COPIED IN
	if (timeoutCurrent.ellapsedNanos(timeoutStart) < 0) {
		timeoutStart.mark();
	}
	if (timeoutCurrent.ellapsedMillis(timeoutStart) <
		CONNECTION_TIMEOUT - 3 * MILLISECONDS_IN_SECONDS) {
		// Regular Reconnect Display
		timeoutCurrent.mark();
		donut->setAngle((donut->getAngle() - globals::PI_180 * RECONNECT_SPIN_RATIO));

		if (currFrame % MAX_ELLIPSES_FRAMES < MAX_ELLIPSES_FRAMES / 3) {
			ellipsis2->setVisible(false);
			ellipsis3->setVisible(false);
		} else if (currFrame % MAX_ELLIPSES_FRAMES < 2 * MAX_ELLIPSES_FRAMES / 3) {
			ellipsis2->setVisible(true);
		} else {
			ellipsis3->setVisible(true);
		}
	} else {
		// 3 Second Timeout Counter back to lobby
		reconnScreen->setVisible(false);
		disconnScreen->setVisible(true);
		if (timeoutCurrent.ellapsedMillis(timeoutStart) <
			CONNECTION_TIMEOUT - 2 * MILLISECONDS_IN_SECONDS) {
			countdown->setText("3");
		} else if (timeoutCurrent.ellapsedMillis(timeoutStart) <
				   CONNECTION_TIMEOUT - MILLISECONDS_IN_SECONDS) {
			countdown->setText("2");
		} else if (timeoutCurrent.ellapsedMillis(timeoutStart) < CONNECTION_TIMEOUT) {
			countdown->setText("1");
		} else {
			return true;
		}
		timeoutCurrent.mark();
	}

	currFrame++;
	return false;
}
