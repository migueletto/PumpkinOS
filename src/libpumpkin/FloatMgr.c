#include <PalmOS.h>

#include "debug.h"

Err FlpBase10Info(FlpDouble a, UInt32 *mantissaP, Int16 *exponentP, Int16 *signP) {
  debug(DEBUG_ERROR, "PALMOS", "FlpBase10Info not implemented");
  return sysErrParamErr;
}

Err FlpFToA(FlpDouble a, Char *s) {
  Err err = sysErrParamErr;

  if (s) {
    StrPrintF(s, "%f", a);
    err = errNone;
  }

  return err;
}

FlpDouble FlpAToF(const Char *s) {
  return s ? sys_atof(s) : 0.0;
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
  if (result && s) *result = sys_atof(s);
}

UInt32 FlpVersion(void) {
  return flpVersion;
}

void FlpSelectorErrPrv(UInt16 flpSelector) {
  debug(DEBUG_ERROR, "PALMOS", "FlpSelectorErrPrv not implemented");
}

FlpDouble _d_add(FlpDouble a, FlpDouble b) {
  return a + b;
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
