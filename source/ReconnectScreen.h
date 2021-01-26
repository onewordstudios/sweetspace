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

	/** Connection Timeout Start */
	cugl::Timestamp timeoutStart;
	/** Connection Timeout Counter */
	cugl::Timestamp timeoutCurrent;

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
	 * Activate the reconnect screen; call when disconnected
	 */
	void activate();

	/**
	 * Disable the reconnect screen; call if connection is re-established successfully
	 */
	void deactivate();

	/**
	 * Update the animation for this node. Should be called once every frame.
	 * Will return true if the full reconnect timeout animation has passed.
	 */
	bool update();
};

#endif // RECONNECT_SCREEN_H
