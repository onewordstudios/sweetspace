#include "ButtonModel.h"

/**
 * Disposes all resources and assets of this door.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a door may not be used until it is initialized again.
 */
void ButtonModel::dispose() {}

int ButtonModel::getSection() {
    float mod = fmod(getAngle(), (float)45);
    int section = (int)(mod < 22.5 ? ceilf(getAngle() / 45) : floorf(getAngle() / 45));
    return section;

}