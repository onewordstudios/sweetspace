#include "Sweetspace.h"

using namespace cugl;

#pragma mark -
#pragma mark Gameplay Control

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

	// Start-up basic input
#ifdef CU_TOUCH_SCREEN
	Input::activate<Touchscreen>();
#else
	Input::activate<Mouse>();
#endif

	assets->attach<Font>(FontLoader::alloc()->getHook());
	assets->attach<Texture>(TextureLoader::alloc()->getHook());
	assets->attach<Sound>(SoundLoader::alloc()->getHook());
	assets->attach<Node>(SceneLoader::alloc()->getHook());

	// Create a "loading" screen
	loaded = false;
	loading.init(assets);

	// Queue up the other assets NOLINTNEXTLINE
	AudioChannels::start(24);
	assets->loadDirectoryAsync("json/assets.json", nullptr);

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
	matchmaking.dispose();
	assets = nullptr;
	batch = nullptr;

	// Shutdown input
#ifdef CU_TOUCH_SCREEN
	Input::deactivate<Touchscreen>();
#else
	Input::deactivate<Mouse>();
#endif

	Application::onShutdown(); // YOU MUST END with call to parent
}

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
			matchmaking.init(assets);
			status = MainMenu;
			return;
		}
		case MainMenu: {
			matchmaking.update(timestep);
			if (matchmaking.isGameReady()) {
				status = MainToGame;
			}
			return;
		}
		case MainToGame: {
			matchmaking.dispose();
			gameplay.init(assets);
			status = Game;
			return;
		}
		case Game: {
			gameplay.update(timestep);
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
	switch (status / 100) {
		case Loading / 100: {
			loading.render(batch);
			return;
		}
		case MainMenu / 100: {
			matchmaking.draw(batch);
			return;
		}
		case Game / 100: {
			gameplay.draw(batch);
			return;
		}
	}
}
