#ifndef WIN_SCREEN_H
#define WIN_SCREEN_H
#include <cugl/cugl.h>

#include "ButtonManager.h"

class WinScreen : public cugl::Node {
   private:
	size_t currFrame;

	float startPer;
	float endPer;

	bool isHost;

	std::shared_ptr<cugl::Node> screen;
	std::shared_ptr<cugl::PathNode> circle;

	std::shared_ptr<cugl::Button> btn;
	std::shared_ptr<cugl::Node> waitText;

	ButtonManager btns;

   public:
	WinScreen();
	~WinScreen();

	bool init(const std::shared_ptr<cugl::AssetManager> &assets);

	void dispose() override;

	void activate(uint8_t completedLevel);

	bool isActive();

	bool tappedNext(std::tuple<cugl::Vec2, cugl::Vec2> tapData);

	void update();
};

#endif // WIN_SCREEN_H
