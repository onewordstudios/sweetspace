#ifndef AD_UTILS_H
#define AD_UTILS_H

#include <cugl/cugl.h>
#if defined(__ANDROID__) || defined(__IPHONEOS__)
#include "firebase/gma.h"
#include "firebase/gma/ad_view.h"
#include "firebase/gma/interstitial_ad.h"
#include "firebase/gma/types.h"
#include "firebase/app.h"
#include "firebase/future.h"

extern const char* const K_BANNER_AD_UNIT;
extern const char* const K_INTERSTITIAL_AD_UNIT;
#endif

#if defined(__IPHONEOS__)
firebase::gma::AdParent getWindow();
#endif

/**
 * This is a helper class whose job it is to display ads
 */
class AdUtils {
   private:
	static const int BANNER_WIDTH = 320;
	static const int BANNER_HEIGHT = 50;
#if defined(__ANDROID__) || defined(__IPHONEOS__)
	static firebase::gma::AdRequest request;
	static firebase::gma::AdView* bannerView;
	static firebase::gma::InterstitialAd* interstitial_ad;
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

		firebase::InitResult result;
		firebase::Future<firebase::gma::AdapterInitializationStatus> future =
				firebase::gma::Initialize(*fbapp, &result);

		if (result != firebase::kInitResultSuccess) {
			// Initialization immediately failed, most likely due to a missing dependency.
			// Check the device logs for more information.
			return;
		}

		bannerView = new firebase::gma::AdView();
		interstitial_ad = new firebase::gma::InterstitialAd();
#endif
#if defined(__IPHONEOS__)
		// Create the Firebase app.
		firebase::App* fbapp = firebase::App::Create();

		firebase::InitResult result;
		firebase::Future<firebase::gma::AdapterInitializationStatus> future =
						firebase::gma::Initialize(*fbapp, &result);
		bannerView = new firebase::gma::AdView();
		interstitial_ad = new firebase::gma::InterstitialAd();
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

			firebase::Future<void> future =
				bannerView->Initialize(activity, K_BANNER_AD_UNIT, firebase::gma::AdSize::kBanner);
			future.OnCompletion(&LoadBannerCallback, bannerView);
			env->DeleteLocalRef(activity);
		} else {
			firebase::Future<firebase::gma::AdResult> loadFuture = bannerView->LoadAd(request);
			loadFuture.OnCompletion(&ShowBannerCallback, bannerView);
		}
#endif
#if defined(__IPHONEOS__)
		if (bannerView->InitializeLastResult().status() == firebase::kFutureStatusInvalid) {

			firebase::Future<void> future =
				bannerView->Initialize(getWindow(), K_BANNER_AD_UNIT, firebase::gma::AdSize::kBanner);
			future.OnCompletion(LoadBannerCallback, bannerView);
		} else {
			firebase::Future<firebase::gma::AdResult> loadFuture = bannerView->LoadAd(request);
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

			firebase::Future<void> future =
				interstitial_ad->Initialize(activity);
			future.OnCompletion(LoadInterstitialCallback, interstitial_ad);

			env->DeleteLocalRef(activity);
		} else {
			firebase::Future<firebase::gma::AdResult> loadFuture = interstitial_ad->LoadAd(K_INTERSTITIAL_AD_UNIT, request);
			loadFuture.OnCompletion(ShowInterstitialCallback, interstitial_ad);
		}
#endif
#if defined(__IPHONEOS__)
		if (interstitial_ad->InitializeLastResult().status() == firebase::kFutureStatusInvalid) {


			firebase::Future<void> future =
				interstitial_ad->Initialize(getWindow());
			future.OnCompletion(LoadInterstitialCallback, interstitial_ad);
		} else {
			firebase::Future<firebase::gma::AdResult> loadFuture = interstitial_ad->LoadAd(K_INTERSTITIAL_AD_UNIT, request);
			loadFuture.OnCompletion(ShowInterstitialCallback, interstitial_ad);
		}
#endif
	};

#if defined(__ANDROID__) || defined(__IPHONEOS__)
	static void LoadInterstitialCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::gma::InterstitialAd* interstitial_ad =
			static_cast<firebase::gma::InterstitialAd*>(user_data);
		if (future.error() == firebase::gma::kAdErrorCodeNone) {
			firebase::Future<firebase::gma::AdResult> loadFuture = interstitial_ad->LoadAd(K_INTERSTITIAL_AD_UNIT, request);
			loadFuture.OnCompletion(ShowInterstitialCallback, interstitial_ad);
		}
	}
	static void ShowInterstitialCallback(const firebase::Future<firebase::gma::AdResult>& future, void* user_data) {
		firebase::gma::InterstitialAd* interstitial_ad =
			static_cast<firebase::gma::InterstitialAd*>(user_data);
		if (future.error() == firebase::gma::kAdErrorCodeNone) {
			interstitial_ad->Show();
		}
	}
	static void LoadBannerCallback(const firebase::Future<void>& future, void* user_data) {
		firebase::gma::AdView* bannerView =
			static_cast<firebase::gma::AdView*>(user_data);
		if (future.error() == firebase::gma::kAdErrorCodeNone) {
			firebase::Future<firebase::gma::AdResult> loadFuture = bannerView->LoadAd(request);
			loadFuture.OnCompletion(ShowBannerCallback, bannerView);
		}
	}
	static void ShowBannerCallback(const firebase::Future<firebase::gma::AdResult>& future, void* user_data) {
		firebase::gma::AdView* bannerView =
			static_cast<firebase::gma::AdView*>(user_data);
		if (future.error() == firebase::gma::kAdErrorCodeNone) {
			bannerView->Show();
			bannerView->SetPosition(firebase::gma::AdView::kPositionTop);
		}
	}
#endif
};
#endif /* AD_UTILS_H */
