#include <stdlib.h>
#include <stdarg.h>

#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_flptrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    //    case sysFloatBase10Info: {
    //      // Err FlpBase10Info(FlpDouble a, UInt32 *mantissaP, Int16 *exponentP, Int16 *signP)
    //      ARG_DOUBLE(a);
    //      uint32_t mantissaP = ARG32;
    //      uint32_t exponentP = ARG32;
    //      uint32_t signP = ARG32;
    //      }
    //      break;
    //    case sysFloatFToA:
    //      // Err FlpFToA(FlpDouble a, Char *s)
    //      break;
    case sysFloatAToF: {
      // FlpDouble FlpAToF(const Char *s)
      // void FlpBufferAToF(FlpDouble *result, const Char *s)
      uint32_t resP = ARG32;
      uint32_t sP = ARG32;
      char *s = emupalmos_trap_sel_in(sP, sysTrapFlpDispatch, sel, 1);
      flp_double_t d;
      d.d = atof(s);
      RES_DOUBLE(d, resP);
      debug(DEBUG_TRACE, "EmuPalmOS", "FlpAToF(\"%s\"): %f", s ? s : "", d.d);
    }
      break;
    //case sysFloatCorrectedAdd:
      // FlpDouble FlpCorrectedAdd(FlpDouble firstOperand, FlpDouble secondOperand, Int16 howAccurate)
      // void FlpBufferCorrectedAdd(FlpDouble *result, FlpDouble firstOperand, FlpDouble secondOperand, Int16 howAccurate)
    //  break;
    //case sysFloatCorrectedSub:
      // FlpDouble FlpCorrectedSub(FlpDouble firstOperand, FlpDouble secondOperand, Int16 howAccurate)
      // void FlpBufferCorrectedSub(FlpDouble *result, FlpDouble firstOperand, FlpDouble secondOperand, Int16 howAccurate)
    //  break;
    default:
      snprintf(buf, sizeof(buf)-1, "FlpDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
