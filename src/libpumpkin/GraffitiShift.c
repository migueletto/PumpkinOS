#include <PalmOS.h>

#include "debug.h"

void GsiInitialize(void) {
  debug(DEBUG_ERROR, "PALMOS", "GsiInitialize not implemented");
}

void GsiSetLocation(const Int16 x, const Int16 y) {
  debug(DEBUG_ERROR, "PALMOS", "GsiSetLocation not implemented");
}

void GsiEnable(const Boolean enableIt) {
  debug(DEBUG_ERROR, "PALMOS", "GsiEnable not implemented");
}

Boolean GsiEnabled(void) {
  debug(DEBUG_ERROR, "PALMOS", "GsiEnabled not implemented");
  return false;
}

void GsiSetShiftState(const UInt16 lockFlags, const UInt16 tempShift) {
  debug(DEBUG_ERROR, "PALMOS", "GsiSetShiftState not implemented");
}
