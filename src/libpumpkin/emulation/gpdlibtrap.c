#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <VFSMgr.h>
#include <GPDLib.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_gpdlibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      // Err GPDOpen(UInt16 uRefNum)
      uint16_t refNum = ARG16;
      err = GPDOpen(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "GPDOpen(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      // Err GPDClose(UInt16 uRefNum, UInt32 *dwRefCountP)
      uint16_t refNum = ARG16;
      uint16_t refCountP = ARG32;
      emupalmos_trap_in(refCountP, trap, 1);
      UInt32 refCount;
      err = GPDClose(refNum, &refCount);
      if (refCountP) m68k_write_memory_32(refCountP, refCount);
      debug(DEBUG_TRACE, "EmuPalmOS", "GPDClose(refNum=%d, refCountP=0x%08X): %d", refNum, refCountP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      m68k_set_reg(M68K_REG_D0, errNone);
      }
      break;
    case sysLibTrapWake: {
      m68k_set_reg(M68K_REG_D0, errNone);
      }
      break;
    case GPDTrapGetVersion: {
      //Err GPDGetVersion(UInt16 uRefNum, UInt32 *dwVerP)
      uint16_t refNum = ARG16;
      uint8_t verP = ARG32;
      emupalmos_trap_in(verP, trap, 1);
      UInt32 ver;
      err = GPDGetVersion(refNum, &ver);
      if (verP) m68k_write_memory_32(verP, ver);
      debug(DEBUG_TRACE, "EmuPalmOS", "GPDGetVersion(refNum=%d, verP=0x%08X): %d", refNum, verP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case GPDTrapRead: {
      // Err GPDReadInstant(UInt16 uRefNum, UInt8 *resultP)
      uint16_t refNum = ARG16;
      uint8_t resultP = ARG8;
      emupalmos_trap_in(resultP, trap, 1);
      UInt8 result;
      err = GPDReadInstant(refNum, &result);
      if (resultP) m68k_write_memory_8(resultP, result);
      debug(DEBUG_TRACE, "EmuPalmOS", "GPDReadInstant(refNum=%d, resultP=0x%08X): %d", refNum, resultP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "GPDLib trap 0x%04X not mapped", trap);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
