#ifndef RECONNECT_SCREEN_H
#define RECONNECT_SCREEN_H

#include <cugl/cugl.h>

class ReconnectScreen : public cugl::Node {
   private:
	std::shared_ptr<cugl::Node> reconnScreen;
	std::shared_ptr<cugl::Node> ellipsis2;
	std::shared_ptr<cugl::Node> ellipsis3;
	std::shared_ptr<cugl::Node> donut;

	std::shared_ptr<cugl::Node> disconnScreen;
	std::shared_ptr<cugl::Label> countdown;

	size_t currFrame;

   public:
	/**
	 * Construct this reconnect screen with assets from the pointed asset manager.
	 * The screen will remain invisible until {@link activate()} is called
	 *
	 * @param assets Asset manager to load reconnect screen assets from
	 */
	explicit ReconnectScreen(const std::shared_ptr<cugl::AssetManager> &assets);
	/** Cleanup and delete this screen */
	~ReconnectScreen();

	/**
	 * Step the animation for this node when it should be hidden. Call this every frame if
	 * connected, because if this node was previously visible, the outgoing animation is played
	 * here.
	 */
	void deactivateStep();

	/**
	 * Activate and update the animation for this node. Should be called once every frame when
	 * disconnected and should NOT be called otherwise. Will return true if the full reconnect
	 * timeout animation has passed, but will not auto-hide this node when that occurs.
	 *
	 * @return True iff timeout animation has completed
	 */
	bool activeStep();
};

#endif // RECONNECT_SCREEN_H
