#include <PalmOS.h>

#include "debug.h"

Err FlpBase10Info(FlpDouble a, UInt32 *mantissaP, Int16 *exponentP, Int16 *signP) {
  debug(DEBUG_ERROR, "PALMOS", "FlpBase10Info not implemented");
  return sysErrParamErr;
}

Err FlpFToA(FlpDouble a, Char *s) {
  debug(DEBUG_ERROR, "PALMOS", "FlpFToA not implemented");
  return sysErrParamErr;
}

FlpDouble FlpAToF(const Char *s) {
  FlpDouble f;
  debug(DEBUG_ERROR, "PALMOS", "FlpAToF not implemented");
  MemSet(&f, sizeof(f), 0);
  return f;
}

FlpDouble FlpCorrectedAdd(FlpDouble firstOperand, FlpDouble secondOperand, Int16 howAccurate) {
  FlpDouble f;
  debug(DEBUG_ERROR, "PALMOS", "FlpCorrectedAdd not implemented");
  MemSet(&f, sizeof(f), 0);
  return f;
}

FlpDouble FlpCorrectedSub(FlpDouble firstOperand, FlpDouble secondOperand, Int16 howAccurate) {
  FlpDouble f;
  debug(DEBUG_ERROR, "PALMOS", "FlpCorrectedSub not implemented");
  MemSet(&f, sizeof(f), 0);
  return f;
}

void FlpBufferAToF(FlpDouble *result, const Char *s) {
  debug(DEBUG_ERROR, "PALMOS", "FlpBufferAToF not implemented");
}

UInt32 FlpVersion(void) {
  debug(DEBUG_ERROR, "PALMOS", "FlpVersion not implemented");
  return 0;
}

void FlpSelectorErrPrv(UInt16 flpSelector) {
  debug(DEBUG_ERROR, "PALMOS", "FlpSelectorErrPrv not implemented");
}

/*
FlpFloat _f_utof(UInt32 u) {
}

FlpFloat _f_itof(Int32 i) {
}

UInt32 _d_dtou(FlpDouble d) {
}

Int32 _d_dtoi(FlpDouble d) {
}
*/
