#ifndef __SWEETSPACE_H__
#define __SWEETSPACE_H__
#include <cugl/cugl.h>

#include "GameMode.h"
#include "LoadingMode.h"
#include "MagicInternetBox.h"
#include "MatchmakingMode.h"

/**
 * The application root.
 */
class Sweetspace : public cugl::Application {
   private:
	/** The global sprite batch for drawing (only want one of these) */
	std::shared_ptr<cugl::SpriteBatch> batch;
	/** The global asset manager */
	std::shared_ptr<cugl::AssetManager> assets;

	/**
	 * An enumeration representing the current mode of the game.
	 *
	 * Implementation note: each main mode is a multiple of 100. Transitions LEAVING a mode
	 * increment within the same block of 100. Their ones digit corresponds to the hundreds digit of
	 * the outbound mode. 0 is not a valid mode.
	 */
	enum GameStatus {
		/** Loading screen */
		Loading = 100,
		LoadToMain = 102,
		/** Main menu screen */
		MainMenu = 200,
		MainToGame = 203,
		/** Primary game screen */
		Game = 300
	};

	/** The current status of the game */
	GameStatus status;

	// Player modes
	/** The primary controller for the game world */
	GameMode gameplay;
	/** The controller for the loading screen */
	LoadingMode loading;
	/** The controller for matchmaking */
	MatchmakingMode matchmaking;

	/** Whether or not we have finished loading all assets */
	bool loaded;
	/** Whether or not we have finished matchmaking */
	bool matched;
	/** Whether or not the game has started */
	bool gameStarted;

   public:
	/**
	 * Creates, but does not initialized a new application.
	 *
	 * This constructor is called by main.cpp.  You will notice that, like
	 * most of the classes in CUGL, we do not do any initialization in the
	 * constructor.  That is the purpose of the init() method.  Separation
	 * of initialization from the constructor allows main.cpp to perform
	 * advanced configuration of the application before it starts.
	 */
	Sweetspace()
		: cugl::Application(),
		  status(Loading),
		  gameplay(),
		  loading(),
		  matchmaking(),
		  loaded(false),
		  matched(false),
		  gameStarted(false) {}

	/**
	 * Disposes of this application, releasing all resources.
	 *
	 * This destructor is called by main.cpp when the application quits.
	 * It simply calls the dispose() method in Application.  There is nothing
	 * special to do here.
	 */
	~Sweetspace() {}

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
	void onStartup() override;

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
	void onShutdown() override;

	/**
	 * The method called to update the application data.
	 *
	 * This is your core loop and should be replaced with your custom implementation.
	 * This method should contain any code that is not an OpenGL call.
	 *
	 * When overriding this method, you do not need to call the parent method
	 * at all. The default implmentation does nothing.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep) override;

	/**
	 * The method called to draw the application to the screen.
	 *
	 * This is your core loop and should be replaced with your custom implementation.
	 * This method should OpenGL and related drawing calls.
	 *
	 * When overriding this method, you do not need to call the parent method
	 * at all. The default implmentation does nothing.
	 */
	void draw() override;
};

#endif /* __SWEETSPACE_H__ */
