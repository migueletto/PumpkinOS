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

#include "sc_prot.h"

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
  char buf[256], screator[8];
  char *s;
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

    //#include "switch.c"

    case sysTrapSysAppStartup: {
      // Err SysAppStartup(SysAppInfoPtr *appInfoPP, MemPtr *prevGlobalsP, MemPtr *globalsPtrP)
      uint32_t appInfoPP = ARG32;
      uint32_t prevGlobalsP = ARG32;
      uint32_t globalsPtrP = ARG32;
      if (appInfoPP) m68k_write_memory_32(appInfoPP, state->sysAppInfoStart);
      if (prevGlobalsP) m68k_write_memory_32(prevGlobalsP, 0);
      if (globalsPtrP) m68k_write_memory_32(globalsPtrP, 0);
      debug(DEBUG_INFO, "EmuPalmOS", "SysAppStartup called");
      m68k_set_reg(M68K_REG_D0, 0);
    }
    break;
    case sysTrapSysAppExit:
      // Err SysAppExit(SysAppInfoPtr appInfoP, MemPtr prevGlobalsP, MemPtr globalsP)
      debug(DEBUG_INFO, "EmuPalmOS", "SysAppExit called");
      m68k_set_reg(M68K_REG_D0, 0);
      m68k_pulse_halt();
      emupalmos_finish(1);
    break;
    case sysTrapSysGetAppInfo: {
      // SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP)
      // XXX uiAppPP and actionCodeAppPP ignored
      debug(DEBUG_TRACE, "EmuPalmOS", "SysGetAppInfo(): 0x%08X", state->sysAppInfoStart);
      m68k_set_reg(M68K_REG_A0, state->sysAppInfoStart);
    }
    break;
    case sysTrapSysTaskDelay: {
      // Err SysTaskDelay(Int32 delay)
      int32_t delay = ARG32;
      err = SysTaskDelay(delay);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysTaskDelay(%d): %d", delay, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSysLibFind: {
      // Err SysLibFind(const Char *nameP, UInt16 *refNumP)
      // Return a reference number for a library that is already loaded, given its name.
      uint32_t nameP = ARG32;
      uint32_t refNumP = ARG32;
      char *name = (char *)emupalmos_trap_in(nameP, trap, 0);
      emupalmos_trap_in(refNumP, trap, 1);
      UInt16 refNum;
      if (SysLibFind(name, &refNum) != errNone || refNum == 0) {
        refNum = SysLibFind68K(name);
      }
      err = refNum ? errNone : sysErrLibNotFound;
      if (refNum == 0) refNum = 0xffff;
      if (refNumP) m68k_write_memory_16(refNumP, refNum);
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibFind(0x%08X \"%s\", 0x%08X): %d", nameP, name ? name : "", refNumP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSysLibLoad: {
      // Err SysLibLoad(UInt32 libType, UInt32 libCreator, UInt16 *refNumP)
      uint32_t libType = ARG32;
      uint32_t libCreator = ARG32;
      uint32_t refNumP = ARG32;
      emupalmos_trap_in(refNumP, trap, 2);
      pumpkin_id2s(libType, buf);
      pumpkin_id2s(libCreator, screator);
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibLoad('%s', '%s', 0x%08X) native", buf, screator, refNumP);
      r = state->SysLibLoad_addr;
    }
    break;
    case sysTrapSysLibNewRefNum68K: {
      // Boolean SysLibNewRefNum68K(UInt32 type, UInt32 creator, UInt16 *refNum)
      uint32_t type = ARG32;
      uint32_t creator = ARG32;
      uint32_t refNumP = ARG32;
      emupalmos_trap_in(refNumP, trap, 2);
      UInt16 refNum;
      Boolean exists = SysLibNewRefNum68K(type, creator, &refNum);
      if (refNumP) m68k_write_memory_16(refNumP, refNum);
      pumpkin_id2s(type, buf);
      pumpkin_id2s(creator, screator);
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibNewRefNum68K('%s', '%s', 0x%08X): %d ", buf, screator, refNumP, exists);
      m68k_set_reg(M68K_REG_D0, exists);
    }
    break;
    case sysTrapSysLibRegister68K: {
      // Err SysLibRegister68K(UInt16 refNum, LocalID dbID, void *code, UInt32 size, UInt16 *dispatchTblP, UInt8 *globalsP)
      uint16_t refNum = ARG16;
      uint32_t id = ARG32;
      uint32_t code = ARG32;
      uint32_t size = ARG32;
      uint32_t dispatchTblP = ARG32;
      uint32_t globalsP = ARG32;
      LocalID dbID = id;
      err = SysLibRegister68K(refNum, dbID, emupalmos_trap_in(code, trap, 2), size, emupalmos_trap_in(dispatchTblP, trap, 4), emupalmos_trap_in(globalsP, trap, 5));
      if (err == errNone) {
        SysLibTblEntryType tbl;
        uint8_t *p = SysLibTblEntry68K(refNum, &tbl);
        if (p) {
      uint32_t pP = emupalmos_trap_out(p);
      m68k_write_memory_32(pP +  0, emupalmos_trap_out(tbl.dispatchTblP));
      m68k_write_memory_32(pP +  4, emupalmos_trap_out(tbl.globalsP));
      m68k_write_memory_32(pP +  8, tbl.dbID);
      m68k_write_memory_32(pP + 12, 0); // XXX codeResH
        }
      }
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibRegister68K(%d, 0x%08X, 0x%08X, %d, 0x%08X, 0x%08X)", refNum, id, code, size, dispatchTblP, globalsP);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSysLibCancelRefNum68K: {
      // void SysLibCancelRefNum68K(UInt16 refNum)
      uint16_t refNum = ARG16;
      SysLibCancelRefNum68K(refNum);
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibCancelRefNum68K(%d)", refNum);
    }
    break;
    case sysTrapSysLibTblEntry: {
      // SysLibTblEntryType *SysLibTblEntry(UInt16 refNum)
      uint16_t refNum = ARG16;
      SysLibTblEntryType tbl;
      uint8_t *p = SysLibTblEntry68K(refNum, &tbl);
      uint32_t a = emupalmos_trap_out(p);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysLibTblEntry(%d): 0x%08X", refNum, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapSysLibRemove: {
      // Err SysLibRemove(UInt16 refNum)
      uint16_t refNum = ARG16;
      SysLibCancelRefNum68K(refNum);
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibRemove(%d): 0", refNum);
      m68k_set_reg(M68K_REG_D0, errNone);
    }
    break;

    #include "sc_case.c"

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
