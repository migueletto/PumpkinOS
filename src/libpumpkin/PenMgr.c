#include <PalmOS.h>

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
  debug(DEBUG_ERROR, OSNAME, "PenGetRawPen not implemented");
  return sysErrParamErr;
}

Err PenCalibrate(PointType *digTopLeftP, PointType *digBotRightP, PointType *scrTopLeftP, PointType *scrBotRightP) {
  return errNone;
}

Err PenResetCalibration(void) {
  return errNone;
}

Err PenRawToScreen(PointType *penP) {
  debug(DEBUG_ERROR, OSNAME, "PenRawToScreen not implemented");
  return sysErrParamErr;
}

Err PenScreenToRaw(PointType *penP) {
  debug(DEBUG_ERROR, OSNAME, "PenScreenToRaw not implemented");
  return sysErrParamErr;
}
