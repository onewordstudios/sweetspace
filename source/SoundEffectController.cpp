#include "SoundEffectController.h"

using namespace cugl;

// Apparently this is necessary...
std::shared_ptr<SoundEffectController> SoundEffectController::instance; // NOLINT (clang-tidy bug)
