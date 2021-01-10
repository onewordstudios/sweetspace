#include "Sweetspace.h"

using namespace cugl;

/** The round number each mode in the enum steps up by */
constexpr unsigned int MODE_ENUM_STEP = 100;

#pragma region Setup

/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void Sweetspace::onStartup() {
	assets = AssetManager::alloc();
	batch = SpriteBatch::alloc();

	// Start up input controller
	InputController::getInstance();

	assets->attach<Font>(FontLoader::alloc()->getHook());
	assets->attach<Texture>(TextureLoader::alloc()->getHook());
	assets->attach<Sound>(SoundLoader::alloc()->getHook());
	assets->attach<Node>(SceneLoader::alloc()->getHook());
	assets->attach<LevelModel>(GenericLoader<LevelModel>::alloc()->getHook());

	// Create a "loading" screen
	loaded = false;
	loading.init(assets);

	// Queue up the other assets NOLINTNEXTLINE
	AudioChannels::start(24);
	assets->loadDirectoryAsync("json/assets.json", nullptr);
	for (const auto *level : LEVEL_NAMES) {
		if (strcmp(level, "") == 0) {
			continue;
		}
		assets->loadAsync<LevelModel>(level, level, nullptr);
	}

	Application::onStartup(); // YOU MUST END with call to parent
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void Sweetspace::onShutdown() {
	loading.dispose();
	gameplay.dispose();
	mainmenu.dispose();
	InputController::cleanup();
	assets = nullptr;
	batch = nullptr;

	Application::onShutdown(); // YOU MUST END with call to parent
}

#pragma endregion

/**
 * Update the game mode. Should be called each frame.
 *
 * Part 1 of 2 within the lifecycle of a frame. Computes all game computations and state updates in
 * preparation for the draw phase. This method contains basically all gameplay code that is not an
 * OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void Sweetspace::update(float timestep) {
	switch (status) {
		case Loading: {
			loading.update(0.01f); // NOLINT
			if (loading.isLoaded()) {
				status = LoadToMain;
			}
			return;
		}
		case LoadToMain: {
			loading.dispose(); // Disables the input listeners in this mode
			// Prepare sound effects for the main menu
			SoundEffectController::getInstance()->init(assets);
			mainmenu.init(assets);
			status = MainMenu;
			return;
		}
		case MainMenu: {
			mainmenu.update(timestep);
			if (mainmenu.isGameReady()) {
				status = MainToGame;
			}
			return;
		}
		case MainToGame: {
			mainmenu.dispose();
			gameplay.init(assets);
			status = Game;
			return;
		}
		case Game: {
			gameplay.update(timestep);
			auto mib = MagicInternetBox::getInstance();
			if (gameplay.getIsBackToMainMenu()) {
				gameplay.dispose();
				mainmenu.init(assets);
				mib->reset();
				CULog("Ending");
				status = MainMenu;
			} else if (mib->lastNetworkEvent() != MagicInternetBox::NetworkEvents::None) {
				auto lastEvent = mib->lastNetworkEvent();
				mib->acknowledgeNetworkEvent();
				gameplay.dispose();
				if (lastEvent == MagicInternetBox::NetworkEvents::EndGame) {
					CULog("Winner");
					mainmenu.init(assets, true);
					mib->reset();
					status = MainMenu;
				} else {
					CULog("Restarting Level");
					gameplay.init(assets);
				}
				return;
			}
			return;
		}
	}
}

/**
 * Draws the game. Should be called each frame.
 *
 * Part 2 of 2 within the lifecycle of a frame. Renders the game state to the screen after
 * computations are complete from the update phase. This method contains all OpenGL and related
 * drawing code.
 */
void Sweetspace::draw() {
	switch (status / MODE_ENUM_STEP) {
		case Loading / MODE_ENUM_STEP: {
			loading.render(batch);
			return;
		}
		case MainMenu / MODE_ENUM_STEP: {
			mainmenu.draw(batch);
			return;
		}
		case Game / MODE_ENUM_STEP: {
			gameplay.draw(batch);
			return;
		}
	}
}
