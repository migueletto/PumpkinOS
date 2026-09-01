#include <PalmOS.h>
#include <VFSMgr.h>
#include <INetMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_inetlibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      // Err INetLibOpen(UInt16 libRefnum, UInt16 config, UInt32 flags, DmOpenRef cacheRef, UInt32 cacheSize, MemHandle *inetHP)
      uint16_t refNum = ARG16;
      uint16_t config = ARG16;
      uint16_t flags = ARG16;
      uint32_t cacheRefP = ARG32;
      uint32_t cacheSize = ARG32;
      uint32_t inetHPP = ARG32;
      emupalmos_trap_in(cacheRefP, trap, 3);
      emupalmos_trap_in(inetHPP, trap, 5);
      MemHandle inetH;
      err = INetLibOpen(refNum, config, flags, NULL, cacheSize, &inetH);
      debug(DEBUG_TRACE, "EmuPalmOS", "INetLibOpen(refNum=%u, config=%u, flags=0x%04X, cacheRef=0x%08X, cacheSize=%u, inetHP=0x%08X): %d",
        refNum, config, flags, cacheRefP, cacheSize, inetHPP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      // Err INetLibClose(UInt16 libRefnum, MemHandle inetH)
      uint16_t refNum = ARG16;
      uint32_t inetHP = ARG32;
      MemHandle inetH = emupalmos_trap_in(inetHP, trap, 1);
      err = INetLibClose(refNum, inetH);
      debug(DEBUG_TRACE, "EmuPalmOS", "INetLibClose(refNum=%d, inetH=0x%08X): %d", refNum, inetHP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      uint16_t refNum = ARG16;
      err = NetLibSleep(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "INetLibSleep(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapWake: {
      uint16_t refNum = ARG16;
      err = NetLibWake(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "INetLibWake(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "INetLib trap 0x%04X not mapped", trap);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
