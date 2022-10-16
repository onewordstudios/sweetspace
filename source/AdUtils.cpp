#include "AdUtils.h"

#include "../firebase/include/firebase/gma/types.h"
#if defined(__ANDROID__)
// Android ad unit IDs
const char* const K_BANNER_AD_UNIT = "ca-app-pub-3940256099942544/6300978111";
const char* const K_INTERSTITIAL_AD_UNIT = "ca-app-pub-3940256099942544/1033173712";
#else
// iOS ad unit IDs
const char* const K_BANNER_AD_UNIT = "ca-app-pub-9909379902934039/5955918815";
const char* const K_INTERSTITIAL_AD_UNIT = "ca-app-pub-9909379902934039/2995531792";
#endif

#if defined(__ANDROID__) || defined(__IPHONEOS__)
firebase::gma::AdView* AdUtils::bannerView;
firebase::gma::InterstitialAd* AdUtils::interstitial_ad;
firebase::gma::AdRequest AdUtils::request = {};
#endif
