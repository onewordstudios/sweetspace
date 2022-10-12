//
//  iOSHelperThatIHate.mm
//  Sweetspace
//
//  Created by Sam Sorenson on 10/30/21.
//  Copyright © 2021 onewordstudios. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <SDL/SDL.h>

#include "AdUtils.h"
#include <SDL/SDL_syswm.h>


firebase::admob::AdParent getWindow(){
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	cugl::Display* display = cugl::Display::get();
    SDL_GetWindowWMInfo(display->_window,&wmInfo);

    UIWindow* uiWindow = wmInfo.info.uikit.window;
    UIViewController* rootViewController = uiWindow.rootViewController;
    firebase::admob::AdParent uiView = rootViewController.view;
	return uiView;
}
