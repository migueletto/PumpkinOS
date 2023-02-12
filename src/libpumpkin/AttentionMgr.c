#include <PalmOS.h>

#include "debug.h"

Err AttnGetAttention(UInt16 cardNo, LocalID dbID, UInt32 userData, AttnCallbackProc *callbackFnP, AttnLevelType level, AttnFlagsType flags, UInt16 nagRateInSeconds, UInt16 nagRepeatLimit) {
  debug(DEBUG_ERROR, "PALMOS", "AttnGetAttention not implemented");
  return sysErrParamErr;
}

Boolean AttnUpdate(UInt16 cardNo, LocalID dbID, UInt32 userData, AttnCallbackProc *callbackFnP, AttnFlagsType *flagsP, UInt16 *nagRateInSecondsP, UInt16 *nagRepeatLimitP) {
  debug(DEBUG_ERROR, "PALMOS", "AttnUpdate not implemented");
  return false;
}

Boolean AttnForgetIt(UInt16 cardNo, LocalID dbID, UInt32 userData) {
  debug(DEBUG_ERROR, "PALMOS", "AttnForgetIt not implemented");
  return false;
}

UInt16 AttnGetCounts(UInt16 cardNo, LocalID dbID, UInt16 *insistentCountP, UInt16 *subtleCountP) {
  debug(DEBUG_ERROR, "PALMOS", "AttnGetCounts not implemented");
  return 0;
}

void AttnListOpen(void) {
  debug(DEBUG_ERROR, "PALMOS", "AttnListOpen not implemented");
}

void AttnIterate(UInt16 cardNo, LocalID dbID, UInt32 iterationData) {
  debug(DEBUG_ERROR, "PALMOS", "AttnIterate not implemented");
}

Err AttnDoSpecialEffects(AttnFlagsType flags) {
  debug(DEBUG_ERROR, "PALMOS", "AttnDoSpecialEffects not implemented");
  return sysErrParamErr;
}

void AttnIndicatorEnable(Boolean enableIt) {
  debug(DEBUG_ERROR, "PALMOS", "AttnIndicatorEnable not implemented");
}

Boolean AttnIndicatorEnabled(void) {
  debug(DEBUG_ERROR, "PALMOS", "AttnIndicatorEnabled not implemented");
  return false;
}
