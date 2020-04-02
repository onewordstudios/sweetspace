#include "LoadingMode.h"

#include "Globals.h"

using namespace cugl;

constexpr float CLEAR_COLOR_R = 13.0f;
constexpr float CLEAR_COLOR_G = 21.0f;
constexpr float CLEAR_COLOR_B = 51.0f;

#pragma mark -
#pragma mark Constructors

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
bool LoadingMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	} else if (!Scene::init(dimen)) {
		return false;
	}

	// IMMEDIATELY load the splash screen assets
	this->assets = assets;
	assets->loadDirectory("json/loading.json");
	auto layer = assets->get<Node>("load");
	layer->setContentSize(dimen);
	layer->doLayout(); // This rearranges the children to fit the screen

	bar = std::dynamic_pointer_cast<ProgressBar>(assets->get<Node>("load_bar"));
	button = std::dynamic_pointer_cast<Button>(assets->get<Node>("load_claw_play"));
	button->setListener([=](const std::string& name, bool down) { this->_active = down; });

	Application::get()->setClearColor(Color4(CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B));
	addChild(layer);
	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LoadingMode::dispose() {
	// Deactivate the button (platform dependent)
	if (button != nullptr && button->isVisible()) {
		button->deactivate();
	}
	button = nullptr;
	bar = nullptr;
	assets = nullptr;
	progress = 0.0f;
}

#pragma mark -
#pragma mark Progress Monitoring
/**
 * The method called to update the game mode.
 *
 * This method updates the progress bar amount.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LoadingMode::update(float timestep) {
	if (progress < 1) {
		progress = assets->progress();
		if (progress >= 1) {
			progress = 1.0f;
			button->setVisible(true);
			button->activate(1);
		}
		bar->setProgress(progress);
	}
}

/**
 * Returns true if loading is complete, but the player has not pressed play
 *
 * @return true if loading is complete, but the player has not pressed play
 */
bool LoadingMode::isLoaded() const { return progress == 1.0f; }
