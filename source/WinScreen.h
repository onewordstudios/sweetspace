#ifndef WIN_SCREEN_H
#define WIN_SCREEN_H
#include <cugl/cugl.h>

class WinScreen : public cugl::Node {
   private:
	uint8_t prevLevel;
	uint8_t currLevel;
	size_t currFrame;

	std::shared_ptr<cugl::Node> screen;

   public:
	WinScreen();
	~WinScreen();

	bool init(const std::shared_ptr<cugl::AssetManager> &assets);

	void dispose() override;

	void draw(const shared_ptr<cugl::SpriteBatch> &batch, const cugl::Mat4 &transform,
			  cugl::Color4 tint) override;

	void activate(uint8_t completedLevel);

	bool isActive();

	void update();
};

#endif // WIN_SCREEN_H
