#include "AdUtils.h"

#include "../firebase/include/firebase/admob/types.h"
#if defined(__ANDROID__)
// Android ad unit IDs
const char* const kBannerAdUnit = "ca-app-pub-3940256099942544/6300978111";
const char* const kInterstitialAdUnit = "ca-app-pub-3940256099942544/1033173712";
#else
// iOS ad unit IDs
const char* const kBannerAdUnit = "ca-app-pub-3940256099942544/2934735716";
const char* const kInterstitialAdUnit = "ca-app-pub-3940256099942544/4411468910";
#endif

#if defined(__ANDROID__) || defined(__IPHONEOS__)
firebase::admob::BannerView* AdUtils::bannerView;
firebase::admob::InterstitialAd* AdUtils::interstitial_ad;
firebase::admob::AdRequest AdUtils::request = {};
#endif
