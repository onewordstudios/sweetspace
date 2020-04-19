#include "ButtonModel.h"

#include "Globals.h"

/**
 * Disposes all resources and assets of this door.
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a door may not be used until it is initialized again.
 */
void ButtonModel::dispose() {}

int ButtonModel::getSection() {
	float mod = fmod(getAngle(), (float)globals::SEG_DEG);
	int section =
		(int)(mod < ((float)globals::SEG_DEG / 2) ? ceilf(getAngle() / globals::SEG_DEG)
												  : floorf(getAngle() / globals::SEG_DEG));
	return section;
}