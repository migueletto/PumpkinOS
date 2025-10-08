#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_pmuilibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      uint16_t refNum = ARG16;
      //err = PmUIUtilLibOpen(refNum);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "PmUIUtilLibOpen(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      uint16_t refNum = ARG16;
      //err = PmUIUtilLibClose(refNum);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "PmUIUtilLibClose(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      uint16_t refNum = ARG16;
      //err = PmUIUtilLibSleep(refNum);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "PmUIUtilLibSleep(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapWake: {
      uint16_t refNum = ARG16;
      //err = PmUIUtilLibWake(refNum);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "PmUIUtilLibWake(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "PmUIUtilLib trap 0x%04X not mapped %04X", trap, exgLibTrapGet);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
