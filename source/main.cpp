//  CUGL zlib License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
#include "Globals.h"
#include "Sweetspace.h"
#include "firebase/admob.h"
#include "firebase/admob/types.h"
#include "firebase/app.h"
#include "firebase/future.h"

// This keeps us from having to write cugl:: all the time
using namespace cugl;

// These really only apply if the game is windowed (not on mobile device)
constexpr unsigned int GAME_HEIGHT = 576;

constexpr float FRAMERATE = 60.0f;

/**
 * The main entry point of any CUGL application.
 *
 * This class creates the application and runs it until done.  You may
 * need to tailor this to your application, particularly the application
 * settings.  However, never modify anything below the line marked.
 *
 * @return the exit status of the application
 */
int main(int /*argc*/, char* /*argv*/[]) {
	// Change this to your application class
	Sweetspace app;

	/// SET YOUR APPLICATION PROPERTIES

	// The unique application name
	app.setName("SweetSpace");

	// The name of your studio (for organizing save files)
	app.setOrganization("onewordstudios");

	// Set the window properties (Only applies to OS X/Windows Desktop)
	app.setSize(globals::SCENE_WIDTH, GAME_HEIGHT);
	app.setFPS(FRAMERATE);
	app.setHighDPI(true);

#if defined(__ANDROID__)
	// Create the Firebase app.
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	firebase::App* fbapp =
			firebase::App::Create(firebase::AppOptions(),
								  env,
								  activity);

	// Your Android AdMob app ID.
	const char* kAdMobAppID = "ca-app-pub-9909379902934039~2417251914";
#else
	// Create the Firebase app.
	firebase::App* fbapp =
		firebase::App::Create(firebase::AppOptions());

	// Your iOS AdMob app ID.
	const char* kAdMobAppID = "ca-app-pub-3940256099942544~3347511713";
#endif  // __ANDROID__

	// Initialize the AdMob library with your AdMob app ID.
	firebase::admob::Initialize(*fbapp, kAdMobAppID);


	/// DO NOT MODIFY ANYTHING BELOW THIS LINE
	if (!app.init()) {
		return 1;
	}

	// Run the application until completion
	app.onStartup();
	while (app.step()) {
	}
	app.onShutdown();

	// NOLINTNEXTLINE Don't care that this isn't thread safe
	exit(0);  // Necessary to quit on mobile devices
	return 0; // This line is never reached
}
