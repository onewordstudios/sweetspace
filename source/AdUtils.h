#ifndef AD_UTILS_H
#define AD_UTILS_H

#include <cugl/cugl.h>
#if defined(__ANDROID__) || defined(__IPHONEOS__)
#include "firebase/admob.h"
#include "firebase/admob/banner_view.h"
#include "firebase/admob/interstitial_ad.h"
#include "firebase/admob/types.h"
#include "firebase/app.h"
#include "firebase/future.h"


extern const char* kBannerAdUnit;
extern const char* kInterstitialAdUnit;
#endif


#if defined(__IPHONEOS__)
firebase::admob::AdParent getWindow();
#endif

/**
 * This is a helper class whose job it is to display ads
 */
class AdUtils {
   private:
	static const int BANNER_WIDTH = 320;
	static const int BANNER_HEIGHT = 50;
#if defined(__ANDROID__) || defined(__IPHONEOS__)
	static firebase::admob::AdRequest request;
	static firebase::admob::BannerView* bannerView;
	static firebase::admob::InterstitialAd* interstitial_ad;
#endif

   public:
	/**
	 * Initializes firebase admob
	 */
	static void initialize() {
#if defined(__ANDROID__)
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
#endif
#if defined(__IPHONEOS__)
		// Create the Firebase app.
		firebase::App* fbapp = firebase::App::Create();

		// Your Android AdMob app ID.
		const char* kAdMobAppID = "ca-app-pub-9909379902934039~8465986645";
		// Initialize the AdMob library with your AdMob app ID.
		firebase::admob::Initialize(*fbapp, kAdMobAppID);
		bannerView = new firebase::admob::BannerView();
		interstitial_ad = new firebase::admob::InterstitialAd();
#endif
	};
	/**
	 * displays a banner ad
	 */
	static void displayBanner() {
#if defined(__ANDROID__)
		if (bannerView->InitializeLastResult().status() == firebase::kFutureStatusInvalid) {
			JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
			jobject activity = (jobject)SDL_AndroidGetActivity();
			firebase::admob::AdSize ad_size;
			ad_size.ad_size_type = firebase::admob::kAdSizeStandard;
			ad_size.width = BANNER_WIDTH;
			ad_size.height = BANNER_HEIGHT;

			request.gender = firebase::admob::kGenderUnknown;

			firebase::Future<void> future =
				bannerView->Initialize(activity, kBannerAdUnit, ad_size);
			future.OnCompletion(LoadBannerCallback, bannerView);
			env->DeleteLocalRef(activity);
		} else {
			firebase::Future<void> loadFuture = bannerView->LoadAd(request);
			loadFuture.OnCompletion(ShowBannerCallback, bannerView);
		}
#endif
#if defined(__IPHONEOS__)
		if (bannerView->InitializeLastResult().status() == firebase::kFutureStatusInvalid) {
			firebase::admob::AdSize ad_size;
			ad_size.ad_size_type = firebase::admob::kAdSizeStandard;
			ad_size.width = BANNER_WIDTH;
			ad_size.height = BANNER_HEIGHT;

			request.gender = firebase::admob::kGenderUnknown;

			firebase::Future<void> future =
				bannerView->Initialize(getWindow(), kBannerAdUnit, ad_size);
			future.OnCompletion(LoadBannerCallback, bannerView);
		} else {
			firebase::Future<void> loadFuture = bannerView->LoadAd(request);
			loadFuture.OnCompletion(ShowBannerCallback, bannerView);
		}
#endif
	};
	

	/**
	 * hides a banner ad
	 */
	static void hideBanner() {
#if defined(__ANDROID__) || defined(__IPHONEOS__)
		bannerView->Hide();
#endif
	};

	/**
	 * Initializes firebase admob
	 */
	static void displayInterstitial() {
#if defined(__ANDROID__)
		if (interstitial_ad->InitializeLastResult().status() == firebase::kFutureStatusInvalid) {
			JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
			jobject activity = (jobject)SDL_AndroidGetActivity();
			request.gender = firebase::admob::kGenderUnknown;

			firebase::Future<void> future =
				interstitial_ad->Initialize(activity, kInterstitialAdUnit);
			future.OnCompletion(LoadInterstitialCallback, interstitial_ad);

			env->DeleteLocalRef(activity);
		} else {
			firebase::Future<void> loadFuture = interstitial_ad->LoadAd(request);
			loadFuture.OnCompletion(ShowInterstitialCallback, interstitial_ad);
		}
#endif
	};

#if defined(__ANDROID__) || defined(__IPHONEOS__)
	static void LoadInterstitialCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::InterstitialAd* interstitial_ad =
			static_cast<firebase::admob::InterstitialAd*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			firebase::Future<void> loadFuture = interstitial_ad->LoadAd(request);
			loadFuture.OnCompletion(ShowInterstitialCallback, interstitial_ad);
		}
	}
	static void ShowInterstitialCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::InterstitialAd* interstitial_ad =
			static_cast<firebase::admob::InterstitialAd*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			interstitial_ad->Show();
		}
	}
	static void LoadBannerCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::BannerView* bannerView =
			static_cast<firebase::admob::BannerView*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			firebase::Future<void> loadFuture = bannerView->LoadAd(request);
			loadFuture.OnCompletion(ShowBannerCallback, bannerView);
		}
	}
	static void ShowBannerCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::admob::BannerView* bannerView =
			static_cast<firebase::admob::BannerView*>(user_data);
		if (future.error() == firebase::admob::kAdMobErrorNone) {
			bannerView->Show();
			bannerView->MoveTo(firebase::admob::BannerView::kPositionTop);
		}
	}
#endif
};
#endif /* AD_UTILS_H */
