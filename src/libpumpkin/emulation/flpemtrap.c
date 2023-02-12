#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <PalmOS.h>
#include <VFSMgr.h>

#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_flpemtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    case sysFloatEm_fp_round: {
      // Int32 _fp_round(Int32)
      // Constants passed to and received from _fp_round:
      //   flpToNearest, flpTowardZero, flpUpward, or flpDownward
      uint32_t u = ARG32;
      debug(DEBUG_TRACE, "FlpEm", "_fp_round(0x%08X): 0x%08X", u, u);
      m68k_set_reg(M68K_REG_D0, flpToNearest); // XXX argument ignored
    }
    break;
    case sysFloatEm_fp_get_fpscr: {
      // Int32 _fp_get_fpscr(void)
      // Constants passed to _fp_set_fpscr and received from _fp_get_fpscr:
      //   flpInvalid, flpOverflow, flpUnderflow, flpDivByZero, or flpInexact
      debug(DEBUG_TRACE, "FlpEm", "_fp_get_fpscr(): 0x%08X", flpOverflow);
      m68k_set_reg(M68K_REG_D0, flpOverflow);
    }
    break;
    case sysFloatEm_fp_set_fpscr: {
      // void _fp_set_fpscr(Int32)
      uint32_t u = ARG32;
      debug(DEBUG_TRACE, "FlpEm", "_fp_set_fpscr(0x%08X)", u);
    }
    break;
    case sysFloatEm_f_utof: {
      // FlpFloat _f_utof(UInt32)
      uint32_t u = ARG32;
      flp_float_t f;
      f.f = (float)u;
      debug(DEBUG_TRACE, "FlpEm", "_f_utof(%u): %f", u, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
    break;
    case sysFloatEm_f_itof: {
      // FlpFloat _f_itof(Int32)
      int32_t i = ARG32;
      flp_float_t f;
      f.f = (float)i;
      debug(DEBUG_TRACE, "FlpEm", "_f_itof(%d): %f", i, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_d_itod: {
      // FlpDouble _d_itod(Int32)
      uint32_t resP = ARG32;
      int32_t i = ARG32;
      flp_double_t d;
      d.d = (double)i;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_f_itod(%d): %f", i, d.d);
    }
      break;
    case sysFloatEm_d_dtof: {
      ARG_DOUBLE(a);
      flp_float_t f;
      f.f = (float)a.d;
      debug(DEBUG_TRACE, "FlpEm", "_f_dtof(%f): %f", a.d, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_ftou: {
      flp_float_t f;
      f.i = ARG32;
      UInt32 u = (UInt32)f.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_ftou(%f): %u", f.f, u);
      m68k_set_reg(M68K_REG_D0, u);
    }
      break;
    case sysFloatEm_f_ftoi: {
      flp_float_t f;
      f.i = ARG32;
      Int32 i = (Int32)f.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_ftoi(%f): %d", f.f, i);
      m68k_set_reg(M68K_REG_D0, i);
    }
      break;
    case sysFloatEm_f_ftod: {
      // FlpDouble _f_ftod(FlpFloat)
      uint32_t resP = ARG32;
      flp_float_t f;
      f.i = ARG32;
      flp_double_t d;
      d.d = f.f;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_f_ftod(%f): %f", f.f, d.d);
    }
      break;
    case sysFloatEm_d_dtou: {
      // UInt32 _d_dtou(FlpDouble)
      ARG_DOUBLE(a);
      UInt32 u = (UInt32)a.d;
      debug(DEBUG_TRACE, "FlpEm", "_d_dtou(%f): %u", a.d, u);
      m68k_set_reg(M68K_REG_D0, u);
    }
      break;
    case sysFloatEm_d_dtoi: {
      // Int32 _d_dtoi(FlpDouble)
      ARG_DOUBLE(a);
      Int32 i = (Int32)a.d;
      debug(DEBUG_TRACE, "FlpEm", "_d_dtoi(%f): %d", a.d, i);
      m68k_set_reg(M68K_REG_D0, i);
    }
      break;
    case sysFloatEm_d_neg: {
      uint32_t resP = ARG32;
      ARG_DOUBLE(a);
      flp_double_t d;
      d.d = -a.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_neg(%f): %f", a.d, d.d);
    }
      break;
    case sysFloatEm_d_cmp: {
      // Int32 _d_cmp(FlpDouble, FlpDouble)
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      Int32 res;
      if (a.d == b.d) res = 0;
      else if (b.d < a.d) res = -1;
      else res = 1;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_cmp(%f, %f): %d", a.d, b.d, res);
    }
      break;
    case sysFloatEm_d_add: {
      // FlpDouble _d_add(FlpDouble, FlpDouble)
      uint32_t resP = ARG32;
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      //debug(DEBUG_TRACE, "FlpEm", "_d_add a=%016llx b=%016llx resP=%08x", a.i, b.i, resP);
      flp_double_t d;
      d.d = a.d + b.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_add(%f, %f): %f", a.d, b.d, d.d);
    }
      break;
    case sysFloatEm_d_mul: {
      // FlpDouble _d_mul(FlpDouble, FlpDouble)
      uint32_t resP = ARG32;
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      //debug(DEBUG_TRACE, "FlpEm", "_d_mul a=%016llx b=%016llx", a.i, b.i);
      flp_double_t d;
      d.d = a.d * b.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_mul(%f, %f): %f", a.d, b.d, d.d);
    }
      break;
    case sysFloatEm_d_sub: {
      // FlpDouble _d_sub(FlpDouble, FlpDouble)
      uint32_t resP = ARG32;
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      flp_double_t d;
      d.d = a.d - b.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_sub(%f, %f): %f", a.d, b.d, d.d);
    }
      break;
    case sysFloatEm_d_div: {
      // FlpDouble _d_div(FlpDouble, FlpDouble)
      uint32_t resP = ARG32;
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      flp_double_t d;
      d.d = a.d / b.d;
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "FlpEm", "_d_div(%f, %f): %f", a.d, b.d, d.d);
    }
      break;
    case sysFloatEm_d_feq: {
      // Int32 _d_feq(FlpDouble, FlpDouble)
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      Int32 res;
      res = (a.d == b.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_feq(%f, %f): %d", a.d, b.d, res);
    }
      break;
    case sysFloatEm_d_fne: {
      // Int32 _d_fne(FlpDouble, FlpDouble)
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      Int32 res;
      res = (a.d != b.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fne(%f, %f): %d", a.d, b.d, res);
    }
      break;
    case sysFloatEm_d_flt: {
      // Int32 _d_flt(FlpDouble, FlpDouble)
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      Int32 res;
      res = (a.d < b.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_flt(%f, %f): %d", a.d, b.d, res);
    }
      break;
    case sysFloatEm_d_fle: {
      // Int32 _d_fle(FlpDouble, FlpDouble)
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      Int32 res;
      res = (a.d <= b.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fle(%f, %f): %d", a.d, b.d, res);
    }
      break;
    case sysFloatEm_d_fgt: {
      // Int32 _d_fgt(FlpDouble, FlpDouble)
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      Int32 res;
      res = (a.d > b.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fgt(%f, %f): %d", a.d, b.d, res);
    }
      break;
    case sysFloatEm_d_fge: {
      // Int32 _d_fge(FlpDouble, FlpDouble)
      ARG_DOUBLE(a);
      ARG_DOUBLE(b);
      Int32 res;
      res = (a.d >= b.d);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fge(%f, %f): %d", a.d, b.d, res);
    }
      break;
    case sysFloatEm_f_feq: {
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      Int32 res;
      res = (a.f == b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_f_feq(%f, %f): %d", a.f, b.f, res);
    }
      break;
    case sysFloatEm_f_fne: {
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      Int32 res;
      res = (a.f != b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fne(%f, %f): %d", a.f, b.f, res);
    }
      break;
    case sysFloatEm_f_add: {
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      flp_float_t f;
      f.f = a.f + b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_add(%f, %f): %f", (double)a.f, (double)b.f, (double)f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_sub: {
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      flp_float_t f;
      f.f = a.f - b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_sub(%f, %f): %f", a.f, b.f, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_mul: {
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      flp_float_t f;
      f.f = a.f * b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_mul(%f, %f): %f", a.f, b.f, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_div: {
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      flp_float_t f;
      f.f = a.f / b.f;
      debug(DEBUG_TRACE, "FlpEm", "_f_div(%f, %f): %f", a.f, b.f, f.f);
      m68k_set_reg(M68K_REG_D0, f.i);
    }
      break;
    case sysFloatEm_f_flt: {
      // Int32 _f_flt(FlpFloat, FlpFloat)
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      Int32 res;
      res = (a.f < b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_flt(%f, %f): %d", a.f, b.f, res);
    }
      break;
    case sysFloatEm_f_fgt: {
      // Int32 _f_fgt(FlpFloat, FlpFloat)
      flp_float_t a;
      a.i = ARG32;
      flp_float_t b;
      b.i = ARG32;
      Int32 res;
      res = (a.f > b.f);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "FlpEm", "_d_fgt(%f, %f): %d", a.f, b.f, res);
    }
      break;
    default:
      snprintf(buf, sizeof(buf)-1, "FlpEmDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
