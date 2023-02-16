#include <PalmOS.h>
#include <time.h>
#include <sys/time.h>

#include "pumpkin.h"
#include "debug.h"

Err PenOpen(void) {
  return errNone;
}

Err PenClose(void) {
  return errNone;
}

Err PenSleep(void) {
  return errNone;
}

Err PenWake(void) {
  return errNone;
}

Err PenGetRawPen(PointType *penP) {
  debug(DEBUG_ERROR, PUMPKINOS, "PenGetRawPen not implemented");
  return sysErrParamErr;
}

Err PenCalibrate(PointType *digTopLeftP, PointType *digBotRightP, PointType *scrTopLeftP, PointType *scrBotRightP) {
  return errNone;
}

Err PenResetCalibration(void) {
  return errNone;
}

Err PenRawToScreen(PointType *penP) {
  debug(DEBUG_ERROR, PUMPKINOS, "PenRawToScreen not implemented");
  return sysErrParamErr;
}

Err PenScreenToRaw(PointType *penP) {
  debug(DEBUG_ERROR, PUMPKINOS, "PenScreenToRaw not implemented");
  return sysErrParamErr;
}
