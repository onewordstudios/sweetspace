#ifndef AD_UTILS_H
#define AD_UTILS_H

#include <cugl/cugl.h>
#include "firebase/admob.h"
#include "firebase/admob/types.h"
#include "firebase/app.h"
#include "firebase/future.h"
#include "firebase/admob/banner_view.h"
#include "firebase/admob/interstitial_ad.h"

#if defined(__ANDROID__)
// Android ad unit IDs
extern const char* kBannerAdUnit;
extern const char* kInterstitialAdUnit;
#else
// iOS ad unit IDs
extern const char* kBannerAdUnit;
extern const char* kInterstitialAdUnit;
#endif

/**
 * This is a helper class whose job it is to display ads
 */
class AdUtils {

private:
	static const int BANNER_WIDTH = 320;
	static const int BANNER_HEIGHT = 50;
	static firebase::admob::AdRequest request;
	static firebase::admob::BannerView* bannerView;
	static firebase::admob::InterstitialAd* interstitial_ad;

public:

	/**
	 * Initializes firebase admob
	 */
	static void initialize() {
		// Create the Firebase app.
		JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
		jobject activity = (jobject)SDL_AndroidGetActivity();
		firebase::App* fbapp = firebase::App::Create(firebase::AppOptions(), env, activity);

		env->DeleteLocalRef(activity);
		// Your Android AdMob app ID.
		const char* kAdMobAppID = "ca-app-pub-9909379902934039~2417251914";
		// Initialize the AdMob library with your AdMob app ID.
		firebase::admob::Initialize(*fbapp, kAdMobAppID);
		bannerView = new firebase::admob::BannerView();
		interstitial_ad = new firebase::admob::InterstitialAd();
	};

	/**
	 * displays a banner ad
	 */
	static void displayBanner() {
		CULog("Show banner");
		if(bannerView->InitializeLastResult().status() == firebase::kFutureStatusInvalid) {
			JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
			jobject activity = (jobject)SDL_AndroidGetActivity();
			firebase::admob::AdSize ad_size;
			ad_size.ad_size_type = firebase::admob::kAdSizeStandard;
			ad_size.width = BANNER_WIDTH;
			ad_size.height = BANNER_HEIGHT;

			request.gender = firebase::admob::kGenderUnknown;

			firebase::Future<void> future = bannerView->Initialize(activity, kBannerAdUnit, ad_size);
			future.OnCompletion(LoadBannerCallback, bannerView);
			env->DeleteLocalRef(activity);
		} else {
			firebase::Future<void> loadFuture = bannerView->LoadAd(request);
			loadFuture.OnCompletion(ShowBannerCallback, bannerView);
		}

	};

	/**
	 * hides a banner ad
	 */
	static void hideBanner() {
		CULog("Hide banner");
		bannerView->Hide();
	};

	/**
	 * Initializes firebase admob
	 */
	static void displayInterstitial() {
		if(interstitial_ad->InitializeLastResult().status() == firebase::kFutureStatusInvalid) {
			JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
			jobject activity = (jobject) SDL_AndroidGetActivity();
			request.gender = firebase::admob::kGenderUnknown;

			firebase::Future<void> future = interstitial_ad->Initialize(activity,
																		kInterstitialAdUnit);
			future.OnCompletion(LoadInterstitialCallback, interstitial_ad);

			env->DeleteLocalRef(activity);
		} else {
			firebase::Future<void> loadFuture = interstitial_ad->LoadAd(request);
			loadFuture.OnCompletion(ShowInterstitialCallback, interstitial_ad);
		}
	};

	static void LoadInterstitialCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::InterstitialAd *interstitial_ad = static_cast<firebase::admob::InterstitialAd*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			firebase::Future<void> loadFuture = interstitial_ad->LoadAd(request);
			loadFuture.OnCompletion(ShowInterstitialCallback, interstitial_ad);
		}
	}
	static void ShowInterstitialCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::InterstitialAd *interstitial_ad = static_cast<firebase::admob::InterstitialAd*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			interstitial_ad->Show();
		}
	}
	static void LoadBannerCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::BannerView *bannerView = static_cast<firebase::admob::BannerView*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			firebase::Future<void> loadFuture = bannerView->LoadAd(request);
			loadFuture.OnCompletion(ShowBannerCallback, bannerView);
		}
	}
	static void ShowBannerCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::BannerView *bannerView = static_cast<firebase::admob::BannerView*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			bannerView->Show();
			bannerView->MoveTo(firebase::admob::BannerView::kPositionTop);
		}
	}
};
#endif /* AD_UTILS_H */
