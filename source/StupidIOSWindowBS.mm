#import <UIKit/UIKit.h>

#include <stdarg.h>

#include "AdUtils.h"

extern "C" int common_main(int argc, const char* argv[]);

@interface AppDelegate : UIResponder<UIApplicationDelegate>

@property(nonatomic, strong) UIWindow *window;

@end

@interface FTAViewController : UIViewController

@end

static int g_exit_status = 0;
static bool g_shutdown = false;
static NSCondition *g_shutdown_complete;
static NSCondition *g_shutdown_signal;
static UITextView *g_text_view;
static UIView *g_parent_view;

@implementation FTAViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  g_parent_view = self.view;
}

@end


WindowContext GetWindowContext() {
  return g_parent_view;
}

