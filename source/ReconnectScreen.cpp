#include "ReconnectScreen.h"

#include "Globals.h"
#include "Tween.h"

/** Starting zoom of node */
constexpr float ZOOM = 1.5f;

/** Ratio of spin on reconnect donut */
constexpr float RECONNECT_SPIN_RATIO = 0.26f;

/** Units which animations below are based on (# of frames) */
constexpr size_t FRAME_UNIT = 60;

/** Animation cycle length of ellipses */
constexpr size_t MAX_ELLIPSES_FRAMES = 3 * FRAME_UNIT;

/** Number of frames before timing out the connection attempt */
constexpr size_t CONN_TIMEOUT = 15 * FRAME_UNIT;

/** Time to animate in and out */
constexpr size_t ANIM_TIME = FRAME_UNIT / 2;

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

void ReconnectScreen::deactivateStep() {
	if (!_isVisible) {
		return;
	}
	if (currFrame > ANIM_TIME) {
		currFrame = ANIM_TIME;
	}

	// We're decrementing currFrame here, so all animations are coded backwards.
	// This allows the animation to seemlessly reverse if connection is re-established
	// before this node finishes animating on screen.
	setScale(Tween::easeOut(ZOOM, 1, currFrame, ANIM_TIME));
	setColor(Tween::fade(Tween::linear(0, 1, currFrame, ANIM_TIME)));

	if (currFrame == 0) {
		setVisible(false);
		return;
	}
	currFrame--;
}

bool ReconnectScreen::activeStep() {
	if (!_isVisible) {
		setVisible(true);
		currFrame = 0;
		disconnScreen->setVisible(false);
		reconnScreen->setVisible(true);
	}

	if (currFrame <= ANIM_TIME) {
		setScale(Tween::easeOut(ZOOM, 1, currFrame, ANIM_TIME));
		setColor(Tween::fade(Tween::linear(0, 1, currFrame, ANIM_TIME)));
	}

	if (currFrame < CONN_TIMEOUT - 3 * FRAME_UNIT) {
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

		if (currFrame < CONN_TIMEOUT - 2 * FRAME_UNIT) {
			countdown->setText("3");
		} else if (currFrame < CONN_TIMEOUT - FRAME_UNIT) {
			countdown->setText("2");
		} else if (currFrame < CONN_TIMEOUT) {
			countdown->setText("1");
		} else {
			return true;
		}
	}

	currFrame++;
	return false;
}
