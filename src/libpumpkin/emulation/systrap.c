#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

#include "sys.h"
#include "mutex.h"
#include "storage.h"
#include "pumpkin.h"
#include "bytes.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "logtrap.h"
#include "emupalmosinc.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "launch_serde.h"
#include "emu_launch_serde.h"
#include "debug.h"

static void palmos_libtrap(uint16_t refNum, uint16_t trap) {
  char buf[256];

  switch (refNum) {
    case NetLibRefNum:
      palmos_netlibtrap(trap);
      break;
    case GPDLibRefNum:
      palmos_gpdlibtrap(trap);
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "trap 0x%04X refNum %d not mapped", trap, refNum);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}

uint32_t palmos_systrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx, selector;
  char buf[256], buf2[8];
  char *s;
  uint8_t *ram = pumpkin_heap_base();
  emu_state_t *state = m68k_get_emu_state();
  Err err;
  uint32_t r = 0;

  // MathLib seems to use trap numbers like 0x0306 instead of 0xA306.
  trap = (trap & 0x0FFF) | 0xA000;
  s = logtrap_trapname(state->lt, trap, &selector, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X begin (%s) pc=0x%08X", trap, s ? s : "unknown", m68k_get_reg(NULL, M68K_REG_PC));

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysTrapFlpDispatch:
      palmos_flptrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapFlpEmDispatch:
      palmos_flpemtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapIntlDispatch:
      palmos_intltrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapFileSystemDispatch:
      palmos_filesystemtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapSerialDispatch:
      palmos_serialtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapHighDensityDispatch:
      if (pumpkin_get_density() == kDensityDouble) {
        palmos_highdensitytrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      } else {
        emupalmos_panic("high density trap called on low density system", EMUPALMOS_INVALID_TRAP);
      }
      break;
    case sysTrapOmDispatch:
      palmos_omtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapPinsDispatch:
      palmos_pinstrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapAccessorDispatch:
      palmos_accessortrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapExpansionDispatch:
      palmos_expansiontrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapTsmDispatch:
      palmos_tsmtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapLmDispatch:
      palmos_lmtrap(sp, idx, m68k_get_reg(NULL, M68K_REG_D2));
      break;
    case sysTrapNavSelector:
      selector = ARG16;
      palmos_navtrap(sp, idx, selector);
      break;

    #include "switch.c"

    case sysTrapPumpkinDebug: {
      // changes in M68K /opt/palmdev/<sdk>/include/Core/CoreTraps.h:
      // #define sysTrapPumpkinDebug 0xA506
      // #define sysTrapLastTrapNumber 0xA507

      // changes in M68K /opt/palmdev/<sdk>/include/Core/System/SysUtils.h:
      // void PumpkinDebug(UInt16 level, Char *sys, Char *buf) SYS_TRAP(sysTrapPumpkinDebug);

      uint16_t level = ARG16;
      uint32_t sysP = ARG32;
      uint32_t bufP = ARG32;
      char *sys = emupalmos_trap_in(sysP, trap, 1);
      char *buf = emupalmos_trap_in(bufP, trap, 2);
      debug(level, sys, "%s", buf);
      break;
    }
    case sysTrapPumpkinDebugBytes: {
      uint16_t level = ARG16;
      uint32_t sysP = ARG32;
      uint32_t bufP = ARG32;
      uint32_t len = ARG32;
      char *sys = emupalmos_trap_in(sysP, trap, 1);
      void *buf = emupalmos_trap_in(bufP, trap, 2);
      debug_bytes(level, sys, (uint8_t *)buf, len);
      break;
    }

    default:
      if (trap > sysLibTrapName) {
        uint16_t refNum = ARG16;
        if (refNum > MAX_SYSLIBS) {
          palmos_libtrap(refNum, trap);
          break;
        }
        uint16_t index = trap - sysLibTrapName;
        uint16_t offset;
        uint16_t num = 0;
        char *s = NULL;
        UInt16 *dispatch = SysLibGetDispatch68K(refNum);

        switch (trap) {
          case sysLibTrapOpen:  s = "Open";  break;
          case sysLibTrapClose: s = "Close"; break;
          case sysLibTrapSleep: s = "Sleep"; break;
          case sysLibTrapWake:  s = "Wake";  break;
          default:
            num = trap - sysLibTrapCustom;
            break;
        }

        if (dispatch) {
          get2b(&offset, (uint8_t *)dispatch, index*2);
          uint8_t *addr = (uint8_t *)dispatch + offset;

          if (s) {
            debug(DEBUG_INFO, "EmuPalmOS", "sysLibTrap%s refNum=%d index=%d", s, refNum, index);
          } else {
            debug(DEBUG_TRACE, "EmuPalmOS", "sysLibTrapCustom %d refNum=%d index=%d", num, refNum, index);
          }
          r = emupalmos_trap_out(addr);
        } else {
          if (s) {
            sys_snprintf(buf, sizeof(buf)-1, "sysLibTrap%s refNum=%d index=%d: no dispatch table", s, refNum, index);
          } else {
            sys_snprintf(buf, sizeof(buf)-1, "sysLibTrapCustom %d refNum=%d index=%d: no dispatch table", num, refNum, index);
          }
          emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
        }
      } else {
        uint16_t selector;
        sys_snprintf(buf, sizeof(buf)-1, "trap 0x%04X %s not mapped", trap, logtrap_trapname(state->lt, trap, &selector, 0));
        emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      }
      break;
  }

  debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X end (int)", trap);
  pumpkin_trace(trap);

  return r;
}
