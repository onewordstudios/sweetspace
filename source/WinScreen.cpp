#include "WinScreen.h"

#include <algorithm>
#include <cmath>

#include "Globals.h"
#include "LevelConstants.h"
#include "MagicInternetBox.h"
#include "Tween.h"

constexpr size_t TRAVEL_TIME = 180;
constexpr size_t LOOP_TIME = 60;

constexpr float PI = static_cast<float>(M_PI);
constexpr float CIRCLE_DIM = 50.f;
constexpr float CIRCLE_STROKE = 2.f;
constexpr unsigned int CIRCLE_SEG = 32;

constexpr float HEIGHT_SCALE = 0.3f;
constexpr float WIDTH_SCALE = 0.8f;
constexpr float WIDTH_OFFSET = 1.25f;

WinScreen::WinScreen(const std::shared_ptr<cugl::AssetManager>& assets)
	: currFrame(0), startPer(0), endPer(0), isHost(false) {
	init(assets);
}

WinScreen::~WinScreen() { dispose(); }

bool WinScreen::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	Node::init();
	setAnchor({0, 0});
	setPosition(0, 0);
	setVisible(false);

	isHost = *(MagicInternetBox::getInstance().getPlayerID()) == 0;

	auto screen = assets->get<Node>("winscreen");
	btn = dynamic_pointer_cast<cugl::Button>(assets->get<Node>("winscreen_nextBtn"));
	waitText = assets->get<Node>("winscreen_waitText");
	if (isHost) {
		waitText->setVisible(false);
		btn->setVisible(true);
	} else {
		btn->setVisible(false);
		waitText->setVisible(true);
	}

	addChild(screen);
	btns.registerButton(btn);

	circle = cugl::PathNode::allocWithEllipse( // Forcing a line break
		{CIRCLE_DIM / 2, CIRCLE_DIM / 2}, {CIRCLE_DIM, CIRCLE_DIM}, CIRCLE_STROKE, CIRCLE_SEG,
		cugl::PathJoint::ROUND);
	addChild(circle);

	cugl::Size dimen = cugl::Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width;
	setContentSize(dimen);
	screen->setPosition(static_cast<float>(globals::SCENE_WIDTH) / 2, dimen.height / 2);
	doLayout();

	return true;
}

void WinScreen::dispose() {
	Node::dispose();
	circle = nullptr;
	btn = nullptr;
	waitText = nullptr;
	btns.clear();
}

float levelToPos(uint8_t lvl) {
	auto x = std::adjacent_find(LEVEL_ENTRY_POINTS.begin(), LEVEL_ENTRY_POINTS.end(),
								[=](uint8_t a, uint8_t b) { return a <= lvl && b > lvl; });
	if (x == LEVEL_ENTRY_POINTS.end()) {
		// Past the end

		uint8_t lastEntry = *(--LEVEL_ENTRY_POINTS.end());
		float t =
			static_cast<float>(lvl - lastEntry) / static_cast<float>(MAX_NUM_LEVELS - lastEntry);
		float slice = 1.f / (LEVEL_ENTRY_POINTS.size() - 1);
		return (1.f + t * slice) * PI;
	}

	// Not past the end
	uint8_t pos = x - LEVEL_ENTRY_POINTS.begin();
	float base = static_cast<float>(pos) / (LEVEL_ENTRY_POINTS.size() - 1);
	float slice = 1.f / (LEVEL_ENTRY_POINTS.size() - 1);
	float sliceChunk =
		static_cast<float>(lvl - *x) / static_cast<float>(LEVEL_ENTRY_POINTS.at(pos + 1) - *x);
	return (base + slice * sliceChunk) * PI;
}

void WinScreen::activate(uint8_t completedLevel) {
	if (_isVisible) {
		return;
	}

	setVisible(true);
	btn->setColor(cugl::Color4::CLEAR);
	waitText->setColor(cugl::Color4::CLEAR);

	// Find level range

	startPer = levelToPos(completedLevel);
	endPer = levelToPos(completedLevel + 1);

	currFrame = 0;
}

bool WinScreen::isActive() { return _isVisible; }

bool WinScreen::tappedNext(std::tuple<cugl::Vec2, cugl::Vec2> tapData) {
	return isHost && currFrame > TRAVEL_TIME &&
		   ButtonManager::tappedButton(btn, std::move(tapData));
}

void WinScreen::update() {
	if (!_isVisible) {
		return;
	}

	btns.process();
	circle->setScale(static_cast<float>(currFrame % LOOP_TIME) / LOOP_TIME);
	circle->setColor(Tween::fade(Tween::loop(currFrame, LOOP_TIME)));

	if (currFrame <= TRAVEL_TIME) {
		float percent = Tween::easeInOut(startPer, endPer, currFrame, TRAVEL_TIME);
		// Uncomment the following line to test the dot circling around
		// percent = static_cast<float>(currFrame) / TRAVEL_TIME;
		float x = (WIDTH_OFFSET - cosf(percent)) * globals::SCENE_WIDTH * WIDTH_SCALE / 2;
		float y = (sinf(percent) * globals::SCENE_WIDTH * HEIGHT_SCALE + _contentSize.height) / 2;
		circle->setPositionX(x);
		circle->setPositionY(y);
	} else if (currFrame <= TRAVEL_TIME + LOOP_TIME) {
		auto fadeColor = Tween::fade(static_cast<float>(currFrame - TRAVEL_TIME) / LOOP_TIME);
		btn->setColor(fadeColor);
		waitText->setColor(fadeColor);
	}

	currFrame++;
}
