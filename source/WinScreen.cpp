#include "WinScreen.h"

#include <algorithm>
#include <cmath>

#include "Globals.h"
#include "LevelConstants.h"
#include "MagicInternetBox.h"
#include "Tween.h"

/** Time for screen to fade in; overlaps with POS_TIME */
constexpr size_t FADE_TIME = 30;
/** Time for icons to shift into place; overlaps with FADE_TIME */
constexpr size_t POS_TIME = 90;
/** Time for ship to travel */
constexpr size_t TRAVEL_TIME = 180;

/** Cycle time for ship's pulsing dot */
constexpr size_t LOOP_TIME = 60;

constexpr float CIRCLE_DIM = 50.f * 3;
constexpr float CIRCLE_STROKE = 2.f * 3;
constexpr unsigned int CIRCLE_SEG = 32;

constexpr float WIDTH = static_cast<float>(globals::SCENE_WIDTH);

constexpr float HEIGHT_SCALE = 0.3f;
constexpr float WIDTH_SCALE = 0.6f;

/** How much the scale of the node holding all the level markers is scaled */
constexpr float LVL_SCALE = 0.4f;

constexpr float START_SCALE = 1.3f;

uint8_t closestLevelBtn(uint8_t lvl) {
	// NOLINTNEXTLINE clang and MSVC disagree for some reason
	auto x = std::adjacent_find(LEVEL_ENTRY_POINTS.begin(), LEVEL_ENTRY_POINTS.end(),
								[=](uint8_t a, uint8_t b) { return a <= lvl && b > lvl; });
	return static_cast<uint8_t>(x - LEVEL_ENTRY_POINTS.begin());
}

/**
 * Convert an x coordinate to a coordinate on the level button wrapper (which is transformed)
 */
constexpr float LVL_X(float x) { return (-(WIDTH / 2) * (1 - LVL_SCALE) + x) / LVL_SCALE; }

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

	/** Whether to slide the whole screen over one afterwards */
	bool mustShift;

   public:
	explicit IconManager(const std::shared_ptr<cugl::AssetManager>& assets)
		: destIcon(0), xDestPos(), yDestPos(-1), xFinalPos(-1), mustShift(false) {
		for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
			icons.at(i) = assets->get<Node>("winscreen_levels_lvl" + std::to_string(i));
			initPos.at(i) = icons.at(i)->getPosition();
			xDestPos.at(i) = -1;
		}

		finalIcon = cugl::PolygonNode::allocWithFile("textures/wl_screens/destination.png");
		finalIcon->setAnchor({1.f / 2, 1.f / 2});
		finalIcon->setScale(1 / LVL_SCALE);
		assets->get<Node>("winscreen_levels")->addChild(finalIcon);
	}

	~IconManager() {
		for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
			icons.at(i)->setPosition(initPos.at(i));
		}
		finalIcon->getParent()->removeChild(finalIcon);
	}

	void activate(uint8_t lvl, float contentHeight, bool shift) {
		mustShift = shift;

		destIcon = closestLevelBtn(lvl);
		if (destIcon == NUM_LEVEL_BTNS) {
			destIcon--;
		}

		const float left = (1 - WIDTH_SCALE) * WIDTH / 2;
		const float diff = WIDTH * WIDTH_SCALE;

		for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
			icons.at(i)->setPosition(initPos.at(i));

			if (i <= destIcon) {
				// to the left
				xDestPos.at(i) = LVL_X(left - static_cast<float>(destIcon - i) * diff);
			} else {
				// to the right
				xDestPos.at(i) = LVL_X(left + static_cast<float>(i - destIcon) * diff);
			}
		}
		yDestPos = WIDTH * HEIGHT_SCALE + contentHeight;

		finalIcon->setColor(Tween::fade(0));
		finalIcon->setPosition(LVL_X(left + diff + diff), yDestPos);
		if (destIcon == NUM_LEVEL_BTNS - 1) {
			xFinalPos = LVL_X(left + diff);
		} else {
			xFinalPos = -1;
		}
	}

	void step(size_t currFrame) {
		if (currFrame <= POS_TIME) {
			for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
				const float initX = initPos.at(i).x;
				const float initY = initPos.at(i).y;

				const float x = Tween::easeInOut(initX, xDestPos.at(i), currFrame, POS_TIME);
				const float y = Tween::easeInOut(initY, yDestPos, currFrame, POS_TIME);

				icons.at(i)->setPosition(x, y);
			}
			if (xFinalPos != -1) {
				finalIcon->setColor(Tween::fade(Tween::easeOut(0, 1, currFrame, POS_TIME)));

				const float x = Tween::easeInOut(xFinalPos + xDestPos[1] - xDestPos[0], xFinalPos,
												 currFrame, POS_TIME);
				finalIcon->setPositionX(x);
			}
		} else if (mustShift) {
			if (currFrame > POS_TIME + TRAVEL_TIME + POS_TIME) {
				return;
			}
			if (currFrame < POS_TIME + TRAVEL_TIME) {
				return;
			}

			const size_t cf = currFrame - POS_TIME - TRAVEL_TIME;
			const float diff = xDestPos[1] - xDestPos[0];

			for (size_t i = 0; i < NUM_LEVEL_BTNS; i++) {
				const float x =
					Tween::easeInOut(xDestPos.at(i), xDestPos.at(i) - diff, cf, POS_TIME);
				icons.at(i)->setPositionX(x);
			}

			if (xFinalPos == -1) {
				xFinalPos = xDestPos[NUM_LEVEL_BTNS - 1] + diff;
			}

			finalIcon->setPositionX(Tween::easeInOut(xFinalPos, xFinalPos - diff, cf, POS_TIME));
			if (cf == 0) {
				finalIcon->setVisible(true);
				finalIcon->setColor(cugl::Color4::WHITE);
			}
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
		const uint8_t diff = LEVEL_ENTRY_POINTS.at(i) - LEVEL_ENTRY_POINTS.at(i - 1);
		if (diff > max) {
			max = diff;
		}
	}

	const uint8_t finalDiff = MAX_NUM_LEVELS - LEVEL_ENTRY_POINTS[LEVEL_ENTRY_POINTS.size() - 1];
	if (finalDiff > max) {
		max = finalDiff;
	}

	return max;
}

std::pair<uint8_t, uint8_t> WinScreen::layoutLevelMarkers(uint8_t completedLevel) {
	uint8_t leftLevel = closestLevelBtn(completedLevel);
	uint8_t rightLevel; // NOLINT
	if (leftLevel == NUM_LEVEL_BTNS) {
		leftLevel = LEVEL_ENTRY_POINTS.at(NUM_LEVEL_BTNS - 1);
		rightLevel = MAX_NUM_LEVELS - 1;
	} else {
		rightLevel = LEVEL_ENTRY_POINTS.at(leftLevel + 1);
		leftLevel = LEVEL_ENTRY_POINTS.at(leftLevel);
	}
	const uint8_t numLevels = rightLevel - leftLevel;
	const float spacing = WIDTH * WIDTH_SCALE / static_cast<float>(numLevels);
	const float left = (1 - WIDTH_SCALE) * WIDTH / 2;

	for (size_t i = 0; i < levelMarkers.size(); i++) {
		if (i < numLevels - 1) {
			levelMarkers.at(i)->setPosition(left + static_cast<float>(i + 1) * spacing,
											(WIDTH * HEIGHT_SCALE + _contentSize.height) / 2);
			levelMarkers.at(i)->setVisible(true);
			levelMarkers.at(i)->setColor(cugl::Color4::CLEAR);
		} else {
			levelMarkers.at(i)->setVisible(false);
		}
	}

	return {numLevels, leftLevel};
}

WinScreen::WinScreen(const std::shared_ptr<cugl::AssetManager>& assets)
	: currFrame(0),
	  startPos(0),
	  endPos(0),
	  mustShift(false),
	  completedLevel(0),
	  isHost(false),
	  levelMarkers(computeMaxLevelInterval()) {
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

	const size_t maxLevels = computeMaxLevelInterval();
	const std::shared_ptr<cugl::Texture> starTexture =
		cugl::Texture::allocWithFile("textures/wl_screens/destination.png");
	for (size_t i = 0; i < maxLevels; i++) {
		levelMarkers[i] = cugl::PolygonNode::allocWithTexture(starTexture);
		levelMarkers[i]->setVisible(false);
		levelMarkers[i]->setScale(1.f / 2);
		addChild(levelMarkers[i]);
	}

	ship = cugl::PolygonNode::allocWithFile("textures/wl_screens/small_ship.png");
	ship->setAnchor({1.f / 2, 1.f / 2});
	circle = cugl::PathNode::allocWithEllipse( // Forcing a line break
		{0, 0}, {CIRCLE_DIM, CIRCLE_DIM}, CIRCLE_STROKE, CIRCLE_SEG, cugl::PathJoint::ROUND);
	ship->addChild(circle);
	ship->setScale(1.f / 2);
	circle->setPosition(ship->getContentWidth() / 2, ship->getContentHeight() / 2);
	addChild(ship);

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

void WinScreen::activate(uint8_t completedLevel) {
	if (_isVisible) {
		return;
	}

	this->completedLevel = completedLevel;
	setVisible(true);
	btn->setColor(cugl::Color4::CLEAR);
	waitText->setColor(cugl::Color4::CLEAR);
	setColor(cugl::Color4::CLEAR);

	currFrame = 0;

	mustShift = std::find(LEVEL_ENTRY_POINTS.begin(), LEVEL_ENTRY_POINTS.end(),
						  completedLevel + 1) != LEVEL_ENTRY_POINTS.end();
	icons->activate(completedLevel, _contentSize.height, mustShift);

	// Figure out number of levels to show stars for
	auto layoutRes = layoutLevelMarkers(completedLevel);
	const uint8_t numLevels = layoutRes.first;
	const uint8_t leftLevel = layoutRes.second;
	ship->setPositionY((WIDTH * HEIGHT_SCALE + _contentSize.height) / 2);

	const float spacing = WIDTH * WIDTH_SCALE / static_cast<float>(numLevels);
	const float left = (1 - WIDTH_SCALE) * WIDTH / 2;

	const uint8_t lvlOffset = completedLevel - leftLevel;
	startPos = left + static_cast<float>(lvlOffset) * spacing;
	endPos = left + static_cast<float>(lvlOffset + 1) * spacing;
	ship->setPositionX(startPos);
	ship->setColor(cugl::Color4::CLEAR);
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

	if (currFrame > POS_TIME) {
		const size_t cf = currFrame - POS_TIME;
		if (cf <= TRAVEL_TIME) {
			const float pos = Tween::easeInOut(startPos, endPos, cf, TRAVEL_TIME);
			ship->setPositionX(pos);
		} else if (cf <= TRAVEL_TIME + FADE_TIME) {
			auto fadeColor = Tween::fade(static_cast<float>(cf - TRAVEL_TIME) / FADE_TIME);
			btn->setColor(fadeColor);
			waitText->setColor(fadeColor);
		}
	} else if (currFrame > POS_TIME - FADE_TIME) {
		auto fade =
			Tween::fade(Tween::easeOut(0.f, 1.f, currFrame - POS_TIME + FADE_TIME, FADE_TIME));
		ship->setColor(fade);
		for (auto& m : levelMarkers) {
			m->setColor(fade);
		}
	}

	if (mustShift && currFrame > POS_TIME + TRAVEL_TIME &&
		currFrame <= POS_TIME + TRAVEL_TIME + POS_TIME) {
		// TODO change icons, etc.
		const size_t cf = currFrame - POS_TIME - TRAVEL_TIME;

		const float start = (1 + WIDTH_SCALE) * WIDTH / 2;
		const float dest = (1 - WIDTH_SCALE) * WIDTH / 2;
		ship->setPositionX(Tween::easeInOut(start, dest, cf, POS_TIME));

		if (cf <= FADE_TIME) {
			auto fade = Tween::fade(Tween::easeOut(1.f, 0.f, cf, FADE_TIME));
			for (auto& m : levelMarkers) {
				m->setColor(fade);
			}
		}
		if (cf == FADE_TIME + 1) {
			layoutLevelMarkers(completedLevel + 1);
		}
		if (cf > POS_TIME - FADE_TIME) {
			auto fade = Tween::fade(Tween::easeOut(0.f, 1.f, cf - POS_TIME + FADE_TIME, FADE_TIME));
			for (auto& m : levelMarkers) {
				m->setColor(fade);
			}
		}
	}

	currFrame++;
}
