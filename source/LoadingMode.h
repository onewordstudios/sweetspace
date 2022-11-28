#ifndef LOADING_MODE_H
#define LOADING_MODE_H
#include <cugl/cugl.h>

/**
 * This class is a simple loading screen for asychronous asset loading.
 *
 * The screen will display a very minimal progress bar that displays the
 * status of the asset manager.  Make sure that all asychronous load requests
 * are issued BEFORE calling update for the first time, or else this screen
 * will think that asset loading is complete.
 *
 * Once asset loading is completed, it will display a play button.  Clicking
 * this button will inform the application root to switch to the gameplay mode.
 */
class LoadingMode : public cugl::Scene {
   protected:
	/** The asset manager for loading. */
	std::shared_ptr<cugl::AssetManager> assets;

	// NO CONTROLLER (ALL IN SEPARATE THREAD)

	// VIEW
	/** The animated progress bar */
	std::shared_ptr<cugl::ProgressBar> bar;

	// MODEL
	/** The progress displayed on the screen */
	float progress;

	/** Whether we're ready to move on */
	bool ready;

	/** Current transition frame */
	int transition;

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new loading mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	LoadingMode() : progress(0.0f), ready(false), transition(0) {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 *
	 * This method is different from dispose() in that it ALSO shuts off any
	 * static resources, like the input controller.
	 */
	virtual ~LoadingMode() { dispose(); }

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	void dispose() override;

	/**
	 * Initializes the controller contents, making it ready for loading
	 *
	 * The constructor does not allocate any objects or memory.  This allows
	 * us to have a non-pointer reference to this controller, reducing our
	 * memory allocation.  Instead, allocation happens in this method.
	 *
	 * @param assets    The (loaded) assets for this game mode
	 *
	 * @return true if the controller is initialized properly, false otherwise.
	 */
	bool init(const std::shared_ptr<cugl::AssetManager>& assets);

#pragma mark -
#pragma mark Progress Monitoring
	/**
	 * The method called to update the game mode.
	 *
	 * This method updates the progress bar amount.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep) override;

	/**
	 * Returns true if loading is complete
	 *
	 * @return true if loading is complete
	 */
	bool isLoaded() const;
};

#endif /* __LOADING_MODE_H__ */
