#include "WinScreen.h"

#include <algorithm>
#include <cmath>

#include "Globals.h"
#include "LevelConstants.h"
#include "MagicInternetBox.h"
#include "Tween.h"

constexpr size_t TRAVEL_TIME = 180;
constexpr size_t LOOP_TIME = 60;
constexpr size_t FADE_TIME = 30;

constexpr float PI = static_cast<float>(M_PI);
constexpr float CIRCLE_DIM = 50.f * 3;
constexpr float CIRCLE_STROKE = 2.f * 3;
constexpr unsigned int CIRCLE_SEG = 32;
constexpr float WIDTH = static_cast<float>(globals::SCENE_WIDTH);

constexpr float HEIGHT_SCALE = 0.3f;
constexpr float WIDTH_SCALE = 0.6f;
constexpr float WIDTH_OFFSET = 1.25f;

/** How much the scale of the node holding all the level markers is scaled */
constexpr float LVL_SCALE = 0.4f;

constexpr float START_SCALE = 1.1f;

uint8_t closestLevelBtn(uint8_t lvl) {
	// NOLINTNEXTLINE clang and MSVC disagree for some reason
	auto x = std::adjacent_find(LEVEL_ENTRY_POINTS.begin(), LEVEL_ENTRY_POINTS.end(),
								[=](uint8_t a, uint8_t b) { return a <= lvl && b > lvl; });
	return static_cast<uint8_t>(x - LEVEL_ENTRY_POINTS.begin());
}

constexpr float LVL_X(float x) { return (-(WIDTH / 2.f) * (1 - LVL_SCALE) + x) / LVL_SCALE; }

class WinScreen::IconManager {
   private:
	/** Pointers to the icons themselves */
	std::array<std::shared_ptr<cugl::Node>, NUM_LEVEL_BTNS> icons;
	/** Initial positions of each icon */
	std::array<cugl::Vec2, NUM_LEVEL_BTNS> initPos;

	/** Pointer to final (big) star icon */
	std::shared_ptr<cugl::Node> finalIcon;

	/** Index of icon intended for left dest */
	uint8_t destIcon;

	/** Destination x positions for each level marker */
	std::array<float, NUM_LEVEL_BTNS> xDestPos;
	/** Destination y positions for each level marker */
	float yDestPos;
	/** Destination x position for the final star icon, or <0 if unneeded */
	float xFinalPos;

   public:
	explicit IconManager(const std::shared_ptr<cugl::AssetManager>& assets)
		: destIcon(0), xDestPos(), yDestPos(-1), xFinalPos(-1) {
		for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
			icons.at(i) = assets->get<Node>("winscreen_levels_lvl" + std::to_string(i));
			initPos.at(i) = icons.at(i)->getPosition();
			xDestPos.at(i) = -1;
		}

		finalIcon = cugl::PolygonNode::allocWithFile("textures/wl_screens/destination.png");
		finalIcon->setAnchor({1.f / 2, 1.f / 2});
		assets->get<Node>("winscreen_levels")->addChild(finalIcon);
	}

	void activate(uint8_t lvl, float contentHeight) {
		finalIcon->setPosition(0, 0);
		finalIcon->setColor(Tween::fade(0));

		destIcon = closestLevelBtn(lvl);
		if (destIcon == NUM_LEVEL_BTNS) {
			destIcon--;
		}

		float left = (1 - WIDTH_SCALE) * WIDTH / 2;
		float diff = WIDTH * WIDTH_SCALE;

		for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
			icons.at(i)->setPosition(initPos.at(i));

			if (i <= destIcon) {
				// to the left
				xDestPos.at(i) = LVL_X(left - static_cast<float>(destIcon - i) * diff);
			} else {
				// to the right
				xDestPos.at(i) = LVL_X(left + static_cast<float>(i - destIcon) * diff);
			}
			CULog("Icon number %d going to %f", i, xDestPos.at(i));
		}
		CULog("Parent width %f", icons[0]->getParent()->getContentSize().width);
		yDestPos = WIDTH * HEIGHT_SCALE + contentHeight;

		if (destIcon == NUM_LEVEL_BTNS - 1) {
			xFinalPos = left + diff;
		} else {
			xFinalPos = -1;
		}
	}

	void step(size_t currFrame) {
		if (currFrame > FADE_TIME) {
			return;
		}
		for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
			float initX = initPos.at(i).x;
			float initY = initPos.at(i).y;

			float x = Tween::easeInOut(initX, xDestPos.at(i), currFrame, FADE_TIME);
			float y = Tween::easeInOut(initY, yDestPos, currFrame, FADE_TIME);

			icons.at(i)->setPosition(x, y);
		}
		if (xFinalPos != -1) {
			finalIcon->setColor(Tween::fade(Tween::easeOut(0, 1, currFrame, FADE_TIME)));

			float x = Tween::easeInOut(0, xFinalPos, currFrame, FADE_TIME);
			float y = Tween::easeInOut(0, yDestPos, currFrame, FADE_TIME);
			finalIcon->setPosition(x, y);
		}
	}
};

/**
 * Compute the maximum number of levels between two buttons on the level select screen
 *
 * @return The number of levels between two buttons. Subtract one from this number to get
 * the number of intermediate level icons to allocate.
 */
uint8_t computeMaxLevelInterval() {
	uint8_t max = 0;
	for (size_t i = 1; i < LEVEL_ENTRY_POINTS.size(); i++) {
		uint8_t diff = LEVEL_ENTRY_POINTS.at(i) - LEVEL_ENTRY_POINTS.at(i - 1);
		if (diff > max) {
			max = diff;
		}
	}

	uint8_t finalDiff = MAX_NUM_LEVELS - LEVEL_ENTRY_POINTS[LEVEL_ENTRY_POINTS.size() - 1];
	if (finalDiff > max) {
		max = finalDiff;
	}

	return max;
}

WinScreen::WinScreen(const std::shared_ptr<cugl::AssetManager>& assets)
	: currFrame(0), startPer(0), endPer(0), isHost(false), levelMarkers(computeMaxLevelInterval()) {
	init(assets);
}

WinScreen::~WinScreen() { dispose(); }

bool WinScreen::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	Node::init();
	setAnchor({1.f / 2, 1.f / 2});
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

	ship = cugl::PolygonNode::allocWithFile("textures/wl_screens/small_ship.png");
	ship->setAnchor({1.f / 2, 1.f / 2});
	CULog("Ship size %f", ship->getContentWidth());
	circle = cugl::PathNode::allocWithEllipse( // Forcing a line break
		{0, 0}, {CIRCLE_DIM, CIRCLE_DIM}, CIRCLE_STROKE, CIRCLE_SEG, cugl::PathJoint::ROUND);
	ship->addChild(circle);
	ship->setScale(1.f / 3);
	circle->setPosition(ship->getContentWidth() / 2, ship->getContentHeight() / 2);
	addChild(ship);

	const size_t maxLevels = computeMaxLevelInterval();
	std::shared_ptr<cugl::Texture> starTexture =
		cugl::Texture::allocWithFile("textures/wl_screens/destination.png");
	for (size_t i = 0; i < maxLevels; i++) {
		levelMarkers[i] = cugl::PolygonNode::allocWithTexture(starTexture);
	}

	icons = std::make_unique<IconManager>(assets);

	cugl::Size dimen = cugl::Application::get()->getDisplaySize();
	dimen *= WIDTH / dimen.width;
	setContentSize(dimen);
	screen->setPosition(WIDTH / 2, dimen.height / 2);
	doLayout();

	return true;
}

void WinScreen::dispose() {
	Node::dispose();
	circle = nullptr;
	btn = nullptr;
	waitText = nullptr;
	btns.clear();
	removeAllChildren();
}

/**
 * Convert a level number to its angular position in radians
 *
 * @param lvl The level number
 *
 * @return The angle, in radians, that the level should be on the level map. Note that level 0 is at
 * angle 0, which is on the left side of the map, not the traditional right. Angle increases
 * clockwise from (-1, 0).
 */
float levelToPos(uint8_t lvl) {
	// NOLINTNEXTLINE clang and MSVC disagree for some reason
	auto x = std::adjacent_find(LEVEL_ENTRY_POINTS.begin(), LEVEL_ENTRY_POINTS.end(),
								[=](uint8_t a, uint8_t b) { return a <= lvl && b > lvl; });
	if (x == LEVEL_ENTRY_POINTS.end()) {
		// Past the end

		// NOLINTNEXTLINE clang and MSVC disagree for some reason
		auto endIt = LEVEL_ENTRY_POINTS.end();
		uint8_t lastEntry = *(--endIt);
		float t =
			static_cast<float>(lvl - lastEntry) / static_cast<float>(MAX_NUM_LEVELS - lastEntry);
		float slice = 1.f / (LEVEL_ENTRY_POINTS.size() - 1);
		return (1.f + t * slice) * PI;
	}

	// Not past the end
	uint8_t pos = static_cast<uint8_t>(x - LEVEL_ENTRY_POINTS.begin());
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
	setColor(cugl::Color4::CLEAR);

	// Find level range

	startPer = levelToPos(completedLevel);
	endPer = levelToPos(completedLevel + 1);

	currFrame = 0;

	icons->activate(completedLevel, _contentSize.height);
}

bool WinScreen::tappedNext(const std::tuple<cugl::Vec2, cugl::Vec2>& tapData) const {
	return isHost && currFrame > TRAVEL_TIME + FADE_TIME &&
		   ButtonManager::tappedButton(btn, tapData);
}

void WinScreen::update() {
	if (!_isVisible) {
		return;
	}

	btns.process();
	circle->setScale(static_cast<float>(currFrame % LOOP_TIME) / LOOP_TIME);
	circle->setColor(Tween::fade(Tween::loop(currFrame, LOOP_TIME)));

	icons->step(currFrame);

	if (currFrame <= FADE_TIME) {
		setScale(Tween::easeOut(START_SCALE, 1.f, currFrame, FADE_TIME));
		setColor(Tween::fade(Tween::easeOut(0, 1, currFrame, FADE_TIME)));
	}

	if (currFrame <= TRAVEL_TIME) {
		float percent = Tween::easeInOut(startPer, endPer, currFrame, TRAVEL_TIME);
		// Uncomment the following line to test the dot circling around
		// percent = static_cast<float>(currFrame) / TRAVEL_TIME;
		float x = (WIDTH_OFFSET - cosf(percent)) * globals::SCENE_WIDTH * WIDTH_SCALE / 2;
		float y = (sinf(percent) * globals::SCENE_WIDTH * HEIGHT_SCALE + _contentSize.height) / 2;
		ship->setPositionX(x);
		ship->setPositionY(y);
	} else if (currFrame <= TRAVEL_TIME + FADE_TIME) {
		auto fadeColor = Tween::fade(static_cast<float>(currFrame - TRAVEL_TIME) / FADE_TIME);
		btn->setColor(fadeColor);
		waitText->setColor(fadeColor);
	}

	currFrame++;
}
