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

Int32 _fp_round(Int32 a) {
  return flpToNearest;
}

Int32 _fp_get_fpscr(void) {
  return flpOverflow;
}

void _fp_set_fpscr(Int32) {
}

FlpFloat _f_utof(UInt32 a) {
  return a;
}

FlpFloat _f_itof(Int32 a) {
  return a;
}

/*
FlpFloat _f_ulltof(UInt64 a) {
  return a;
}

FlpFloat _f_lltof(Int64 a) {
  return a;
}
*/

FlpDouble _d_utod(UInt32 a) {
  return a;
}

FlpDouble _d_itod(Int32 a) {
  return a;
}

/*
FlpDouble _d_ulltod(UInt64 a) {
  return a;
}

FlpDouble _d_lltod(Int64 a) {
  return a;
}
*/

FlpDouble _f_ftod(FlpFloat a) {
  return a;
}

FlpFloat _d_dtof(FlpDouble a) {
  return a;
}

UInt32 _f_ftou(FlpFloat a) {
  return a;
}

Int32 _f_ftoi(FlpFloat a) {
  return a;
}

/*
UInt64 _f_ftoull(FlpFloat a) {
  return a;
}

Int64 _f_ftoll(FlpFloat a) {
  return a;
}
*/

UInt32 _d_dtou(FlpDouble a) {
  return a;
}

Int32 _d_dtoi(FlpDouble a) {
  return a;
}

/*
UInt64 _d_dtoull(FlpDouble a) {
  return a;
}

Int64 _d_dtoll(FlpDouble a) {
  return a;
}
*/

/*
Int32 _f_cmp(FlpFloat a, FlpFloat b) {
}

Int32 _f_cmpe(FlpFloat a, FlpFloat b) {
}
*/

Int32 _f_feq(FlpFloat a, FlpFloat b) {
  return a == b;
}

Int32 _f_fne(FlpFloat a, FlpFloat b) {
  return a != b;
}

Int32 _f_flt(FlpFloat a, FlpFloat b) {
  return a < b;
}

Int32 _f_fle(FlpFloat a, FlpFloat b) {
  return a <= b;
}

Int32 _f_fgt(FlpFloat a, FlpFloat b) {
  return a > b;
}

Int32 _f_fge(FlpFloat a, FlpFloat b) {
  return a >= b;
}

/*
Int32 _f_fun(FlpFloat a, FlpFloat b) {
}

Int32 _f_for(FlpFloat a, FlpFloat b) {
}

Int32 _d_cmp(FlpDouble a, FlpDouble b) {
}

Int32 _d_cmpe(FlpDouble a, FlpDouble b) {
}
*/

Int32 _d_feq(FlpDouble a, FlpDouble b) {
  return a == b;
}

Int32 _d_fne(FlpDouble a, FlpDouble b) {
  return a != b;
}

Int32 _d_flt(FlpDouble a, FlpDouble b) {
  return a < b;
}

Int32 _d_fle(FlpDouble a, FlpDouble b) {
  return a <= b;
}

Int32 _d_fgt(FlpDouble a, FlpDouble b) {
  return a > b;
}

Int32 _d_fge(FlpDouble a, FlpDouble b) {
  return a >= b;
}

/*
Int32 _d_fun(FlpDouble a, FlpDouble b) {
}

Int32 _d_for(FlpDouble a, FlpDouble b) {
}
*/

FlpFloat _f_neg(FlpFloat a) {
  return -a;
}

FlpFloat _f_add(FlpFloat a, FlpFloat b) {
  return a + b;
}

FlpFloat _f_mul(FlpFloat a, FlpFloat b) {
  return a * b;
}

FlpFloat _f_sub(FlpFloat a, FlpFloat b) {
  return a - b;
}

FlpFloat _f_div(FlpFloat a, FlpFloat b) {
  return a / b;
}

FlpDouble _d_neg(FlpDouble a) {
  return -a;
}

FlpDouble _d_add(FlpDouble a, FlpDouble b) {
  return a + b;
}

FlpDouble _d_mul(FlpDouble a, FlpDouble b) {
  return a * b;
}

FlpDouble _d_sub(FlpDouble a, FlpDouble b) {
  return a - b;
}

FlpDouble _d_div(FlpDouble a, FlpDouble b) {
  return a / b;
}
