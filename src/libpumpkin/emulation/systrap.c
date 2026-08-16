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
#include "syslibs.h"
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
    case SonyHRLibRefNum:
      palmos_sonyhrtrap(trap);
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
      Err err = SysTaskDelay(delay);
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
      Err err = refNum ? errNone : sysErrLibNotFound;
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
      Err err = SysLibRegister68K(refNum, dbID, emupalmos_trap_in(code, trap, 2), size, emupalmos_trap_in(dispatchTblP, trap, 4), emupalmos_trap_in(globalsP, trap, 5));
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
    case sysTrapSysGetStackInfo: {
      // Boolean SysGetStackInfo(MemPtr *startPP, MemPtr *endPP)
      uint32_t startPP = ARG32;
      uint32_t endPP = ARG32;
      emupalmos_trap_in(startPP, trap, 0);
      emupalmos_trap_in(endPP, trap, 1);
      // XXX
      //if (startPP) m68k_write_memory_32(startPP, state->stackStart);
      //if (endPP) m68k_write_memory_32(endPP, state->stackStart + stackSize);
      if (startPP) m68k_write_memory_32(startPP, state->stackStart + stackSize);
      if (endPP) m68k_write_memory_32(endPP, state->stackStart);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysGetStackInfo(0x%08X [0x%08X], 0x%08X [0x%08X]): %d", startPP, state->stackStart, endPP, state->stackStart + stackSize, true);
      m68k_set_reg(M68K_REG_D0, true);
    }
    break;
    case sysTrapSysSetTrapAddress: {
      // Err SysSetTrapAddress(UInt16 trapNum, void *procP)
      uint16_t trapNum = ARG16;
      uint32_t procP = ARG32;
      uint16_t selector;
      emupalmos_trap_in(procP, trap, 1);
      char *s = logtrap_trapname(state->lt, trap, &selector, 0);
      Err res = sysErrParamErr;
      debug(DEBUG_INFO, "EmuPalmOS", "SysSetTrapAddress(0x%04X [ %s ], 0x%08X): %d", trapNum, s ? s : "unknown", procP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSysGetTrapAddress: {
      // void *SysGetTrapAddress(UInt16 trapNum)
      uint16_t trapNum = ARG16;
      uint32_t a = 0;
      uint16_t selector;
      char *s = logtrap_trapname(state->lt, trap, &selector, 0);
      if (s) {
       a = pumpkin_heap_size() + (trapNum << 2);
      }
      debug(DEBUG_INFO, "EmuPalmOS", "SysGetTrapAddress(0x%04X [ %s ]): 0x%08X", trapNum, s ? s : "unknown", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapSysCreatePanelList: {
      // Boolean SysCreatePanelList(UInt16 *panelCount, MemHandle *panelIDs)
      uint32_t panelCountP = ARG32;
      uint32_t panelIDsP = ARG32;
      UInt16 panelCount;
      MemHandle panelIDs;
      emupalmos_trap_in(panelCountP, trap, 0);
      emupalmos_trap_in(panelIDsP, trap, 1);
      Boolean res = SysCreatePanelList(&panelCount, &panelIDs);
      if (panelCountP) m68k_write_memory_16(panelCountP, panelCount);
      if (panelIDsP) m68k_write_memory_32(panelIDsP, emupalmos_trap_out(panelIDs));
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysCreatePanelList(panelCount=0x%08X, panelIDs=0x%08X): %d", panelCountP, panelIDsP, res);
    }
    break;
    case sysTrapSysInsertionSort:
    case sysTrapSysQSort: {
      // void SysQSort(void *baseP, UInt16 numOfElements, Int16 width, CmpFuncPtr comparF, Int32 other)
      uint32_t baseP = ARG32;
      uint16_t numOfElements = ARG16;
      int16_t width = ARG16;
      uint32_t comparF = ARG32;
      int32_t other = ARG32;
      uint8_t *base = emupalmos_trap_in(baseP, trap, 0);
      emupalmos_trap_in(comparF, trap, 3);
      SysQSort68k(base, numOfElements, width, comparF, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysQSort68k(0x%08X, %d, %d, 0x%08X, %d)", baseP, numOfElements, width, comparF, other);
    }
    break;
    case sysTrapSysBinarySearch: {
      // Boolean SysBinarySearch(void const *baseP, UInt16 numOfElements, Int16 width, SearchFuncPtr searchF, void const *searchData, Int32 other, Int32 *position, Boolean findFirst)
      uint32_t baseP = ARG32;
      uint16_t numOfElements = ARG16;
      int16_t width = ARG16;
      uint32_t searchF = ARG32;
      uint32_t searchData = ARG32;
      int32_t other = ARG32;
      uint32_t positionP = ARG32;
      uint8_t findFirst = ARG8;
      emupalmos_trap_in(baseP, trap, 0);
      emupalmos_trap_in(searchF, trap, 3);
      emupalmos_trap_in(searchData, trap, 4);
      emupalmos_trap_in(positionP, trap, 6);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysBinarySearch(0x%08X, %d, %d, 0x%08X, 0x%08X, %d, 0x%08X, %d) native 0x%08X", baseP, numOfElements, width, searchF, searchData, other, positionP, findFirst, state->SysQSort_addr);
      r = state->SysBinarySearch_addr;
    }
    break;
    case sysTrapSysNotifyRegister: {
      // Err SysNotifyRegister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, SysNotifyProcPtr callbackP, Int8 priority, void *userDataP)
      uint16_t cardNo = ARG16;
      uint32_t dbID = ARG32;
      uint32_t notifyType = ARG32;
      uint32_t callbackP = ARG32;
      int8_t priority = ARG8;
      uint32_t userDataP = ARG32;
      SysNotifyProcPtr callback = emupalmos_trap_in(callbackP, trap, 3);
      void *userData = emupalmos_trap_in(userDataP, trap, 5);
      Err res = SysNotifyRegister(cardNo, dbID, notifyType, callback, priority, userData);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyRegister(%d, 0x%08X, 0x%08X, 0x%08X, %u, 0x%08X): %d", cardNo, dbID, notifyType, callbackP, priority, userDataP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSysNotifyUnregister: {
      // Err SysNotifyUnregister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, Int8 priority)
      uint16_t cardNo = ARG16;
      uint32_t dbID = ARG32;
      uint32_t notifyType = ARG32;
      int8_t priority = ARG8;
      Err res = SysNotifyUnregister(cardNo, dbID, notifyType, priority);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyUnregister(%d, 0x%08X, 0x%08X, %u): %d", cardNo, dbID, notifyType, priority, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSysNotifyBroadcast: {
      // Err SysNotifyBroadcast(SysNotifyParamType *notify)
      uint32_t notifyP = ARG32;
      emupalmos_trap_in(notifyP, trap, 0);
      SysNotifyParamType notify;
      decode_notify(notifyP, &notify);
      Err res = SysNotifyBroadcast(notifyP ? &notify : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyBroadcast(0x%08X): %d", notifyP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSysFormPointerArrayToStrings: {
      // MemHandle SysFormPointerArrayToStrings(Char *c, Int16 stringCount)
      uint32_t c = ARG32;
      emupalmos_trap_in(c, trap, 0);
      int16_t stringCount = ARG16;
      debug(DEBUG_TRACE, "EmuPalmOS", "SysFormPointerArrayToStrings(0x%08X, %d) native 0x%08X", c, stringCount, state->SysFormPointerArrayToStrings_addr);
      r = state->SysFormPointerArrayToStrings_addr;
    }
    break;
    case sysTrapSysCopyStringResource: {
      // void SysCopyStringResource(Char *string, Int16 theID)
      uint32_t stringP = ARG32;
      int16_t theID = ARG16;
      char *string = emupalmos_trap_in(stringP, trap, 0);
      SysCopyStringResource(string, theID);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysCopyStringResource(0x%08X, %d)", stringP, theID);
    }
    break;
    case sysTrapSysStringByIndex: {
      // Char *SysStringByIndex(UInt16 resID, UInt16 index, Char *strP, UInt16 maxLen)
      uint16_t resID = ARG16;
      uint16_t index = ARG16;
      uint32_t strP = ARG32;
      uint16_t maxLen = ARG16;
      char *str = emupalmos_trap_in(strP, trap, 2);
      char *res = SysStringByIndex(resID, index, strP ? str : NULL, maxLen);
      uint32_t p = emupalmos_trap_out(res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysStringByIndex(%d, %d, 0x%08X, %d): 0x%08X", resID, index, strP, maxLen, p);
      m68k_set_reg(M68K_REG_A0, p);
    }
    break;
    case sysTrapSysReset:
      // void SysReset(void)
      SysReset();
      debug(DEBUG_TRACE, "EmuPalmOS", "SysReset()");
    break;
    case sysTrapSysErrString: {
      // Char *SysErrString(Err err, Char *strP, UInt16 maxLen)
      int16_t err = ARG16;
      uint32_t strP = ARG32;
      uint16_t maxLen = ARG16;
      char *str = emupalmos_trap_in(strP, trap, 1);
      char *res = SysErrString(err, str, maxLen);
      uint32_t p = emupalmos_trap_out(res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysErrString(%d, 0x%08X, %u): 0x%08X", err, strP, maxLen, p);
      m68k_set_reg(M68K_REG_A0, p);
    }
    break;
    case sysTrapSysRandom: {
      // Int16 SysRandom(Int32 newSeed)
      int32_t newSeed = ARG32;
      Int16 res = SysRandom(newSeed);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysRandom(%d): %d", newSeed, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSysSetAutoOffTime:
      m68k_set_reg(M68K_REG_D0, 0);
    break;
    case sysTrapSysCreateDataBaseList: {
      // Boolean SysCreateDataBaseList(UInt32 type, UInt32 creator, UInt16 *dbCount, MemHandle *dbIDs, Boolean lookupName)
      uint32_t type = ARG32;
      uint32_t creator = ARG32;
      uint32_t countP = ARG32;
      uint32_t listP = ARG32;
      uint8_t lookupName = ARG8;
      emupalmos_trap_in(countP, trap, 2);
      emupalmos_trap_in(listP, trap, 3);
      UInt16 count;
      MemHandle list;
      Boolean r = SysCreateDataBaseList68K(type, creator, &count, &list, lookupName);
      if (countP) m68k_write_memory_16(countP, count);
      if (listP) m68k_write_memory_32(listP, emupalmos_trap_out(list));
      debug(DEBUG_TRACE, "EmuPalmOS", "SysCreateDataBaseList(0x%08X, 0x%08X, 0x%08X, 0x%08X, %d)", type, creator, countP, listP, lookupName);
      m68k_set_reg(M68K_REG_D0, r);
    }
    break;
    case sysTrapSysKeyboardDialogV10: {
      // void SysKeyboardDialogV10(void)
      SysKeyboardDialogV10();
      debug(DEBUG_TRACE, "EmuPalmOS", "SysKeyboardDialogV10()");
    }
    break;
    case sysTrapSysKeyboardDialog: {
      // void SysKeyboardDialog(KeyboardType kbd)
      uint8_t kbd = ARG8;
      SysKeyboardDialog(kbd);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysKeyboardDialog(%d)", kbd);
    }
    break;
    case sysTrapSysNotifyBroadcastDeferred: {
      // Err SysNotifyBroadcastDeferred(SysNotifyParamType *notify, Int16 paramSize)
      uint32_t notifyP = ARG32;
      int16_t paramSize = ARG16;
      emupalmos_trap_in(notifyP, trap, 0);
      SysNotifyParamType notify;
      decode_notify(notifyP, &notify);
      Err err = SysNotifyBroadcastDeferred(notifyP ? &notify : NULL, paramSize);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyBroadcastDeferred(0x%08X, %d): %d", notifyP, paramSize, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSysTicksPerSecond: {
      // UInt16 SysTicksPerSecond(void)
      UInt16 res = SysTicksPerSecond();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysTicksPerSecond(): %d", res);
    }
    break;
    case sysTrapSysHandleEvent: {
      // Boolean SysHandleEvent(in EventType *eventP)
      uint32_t eventP = ARG32;
      EventType l_eventP;
      decode_event(eventP, &l_eventP);
      Boolean res = SysHandleEvent(eventP ? &l_eventP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysHandleEvent(eventP=0x%08X): %d", eventP, res);
    }
    break;
    case sysTrapSysUIBusy: {
      // UInt16 SysUIBusy(Boolean set, Boolean value)
      uint8_t set = ARG8;
      uint8_t value = ARG8;
      UInt16 res = SysUIBusy(set, value);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysUIBusy(%u, %u): %u", set, value, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSysUIAppSwitch: {
      // Err SysUIAppSwitch(UInt16 cardNo, LocalID dbID, UInt16 cmd, MemPtr cmdPBP)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint16_t cmd = ARG16;
      uint32_t cmdPBP = ARG32;
      launch_union_t param;
      int r = 0;
      if (cmdPBP) {
        r = decode_launch(cmd, cmdPBP, &param);
      }
      Err res = sysErrParamErr;
      if (r == 0) {
        res = SysUIAppSwitch(cardNo, dbID, cmd, cmdPBP ? &param : NULL);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "SysUIAppSwitch invalid param type %d", cmd);
      }
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysUIAppSwitch(cardNo=%d, dbID=0x%08X, cmd=%d, cmdPBP=0x%08X)", cardNo, dbID, cmd, cmdPBP);
    }
    break;
    case sysTrapSysCurAppDatabase: {
      // Err SysCurAppDatabase(out UInt16 *cardNoP, out LocalID *dbIDP)
      uint32_t cardNoP = ARG32;
      UInt16 l_cardNoP = 0;
      uint32_t dbIDP = ARG32;
      LocalID l_dbIDP;
      Err res = SysCurAppDatabase(cardNoP ? &l_cardNoP : NULL, dbIDP ? &l_dbIDP : NULL);
      if (cardNoP) m68k_write_memory_16(cardNoP, l_cardNoP);
      if (dbIDP) m68k_write_memory_32(dbIDP, l_dbIDP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysCurAppDatabase(cardNoP=0x%08X [%d], dbIDP=0x%08X): %d", cardNoP, l_cardNoP, dbIDP, res);
    }
    break;
    case sysTrapSysAppLaunch: {
      // Err SysAppLaunch(UInt16 cardNo, LocalID dbID, UInt16 launchFlags, UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP)
      uint16_t cardNo = ARG16;
      LocalID dbID = ARG32;
      uint16_t launchFlags = ARG16;
      uint16_t cmd = ARG16;
      uint32_t cmdPBP = ARG32;
      uint32_t resultP = ARG32;
      UInt32 l_resultP = 0;
      launch_union_t param;
      int r = 0;
      if (cmdPBP) {
        r = decode_launch(cmd, cmdPBP, &param);
      }
      Err res = sysErrParamErr;
      if (r == 0) {
        res = SysAppLaunch(cardNo, dbID, launchFlags, cmd, cmdPBP ? &param : NULL, resultP ? &l_resultP : NULL);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "SysAppLaunch invalid param type %d", cmd);
      }
      if (resultP) m68k_write_memory_32(resultP, l_resultP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysAppLaunch(cardNo=%d, dbID=0x%08X, launchFlags=%d, cmd=%d, cmdPBP=0x%08X, resultP=0x%08X [%d]): %d", cardNo, dbID, launchFlags, cmd, cmdPBP, resultP, l_resultP, res);
    }
    break;
    case sysTrapSysLCDContrast: {
      // UInt8 SysLCDContrast(Boolean set, UInt8 newContrastLevel)
      uint8_t set = ARG8;
      uint8_t newContrastLevel = ARG8;
      UInt8 res = SysLCDContrast(set, newContrastLevel);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysLCDContrast(set=%d, newContrastLevel=%d): %d", set, newContrastLevel, res);
    }
    break;
    case sysTrapSysLCDBrightness: {
      // UInt8 SysLCDBrightness(Boolean set, UInt8 newBrightnessLevel)
      uint8_t set = ARG8;
      uint8_t newBrightnessLevel = ARG8;
      UInt8 res = SysLCDBrightness(set, newBrightnessLevel);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysLCDBrightness(set=%d, newBrightnessLevel=%d): %d", set, newBrightnessLevel, res);
    }
    break;
    case sysTrapSysGetOSVersionString: {
      // Char *SysGetOSVersionString()
      Char *res = SysGetOSVersionString();
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysGetOSVersionString(): 0x%08X", r_res);
    }
    break;
    case sysTrapSysBatteryInfo: {
      // UInt16 SysBatteryInfo(Boolean set, out UInt16 *warnThresholdP, out UInt16 *criticalThresholdP, out Int16 *maxTicksP, out SysBatteryKind *kindP, out Boolean *pluggedIn, out UInt8 *percentP)
      uint8_t set = ARG8;
      uint32_t warnThresholdP = ARG32;
      UInt16 l_warnThresholdP = 0;
      uint32_t criticalThresholdP = ARG32;
      UInt16 l_criticalThresholdP = 0;
      uint32_t maxTicksP = ARG32;
      Int16 l_maxTicksP = 0;
      uint32_t kindP = ARG32;
      SysBatteryKind l_kindP;
      uint32_t pluggedIn = ARG32;
      Boolean l_pluggedIn;
      uint32_t percentP = ARG32;
      UInt8 l_percentP;
      UInt16 res = SysBatteryInfo(set, warnThresholdP ? &l_warnThresholdP : NULL, criticalThresholdP ? &l_criticalThresholdP : NULL, maxTicksP ? &l_maxTicksP : NULL, kindP ? &l_kindP : NULL, pluggedIn ? &l_pluggedIn : NULL, percentP ? &l_percentP : NULL);
      if (warnThresholdP) m68k_write_memory_16(warnThresholdP, l_warnThresholdP);
      if (criticalThresholdP) m68k_write_memory_16(criticalThresholdP, l_criticalThresholdP);
      if (maxTicksP) m68k_write_memory_16(maxTicksP, l_maxTicksP);
      if (pluggedIn) m68k_write_memory_8(pluggedIn, l_pluggedIn);
      if (percentP) m68k_write_memory_8(percentP, l_percentP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysBatteryInfo(set=%d, warnThresholdP=0x%08X [%d], criticalThresholdP=0x%08X [%d], maxTicksP=0x%08X [%d], kindP=0x%08X, pluggedIn=0x%08X, percentP=0x%08X): %d", set, warnThresholdP, l_warnThresholdP, criticalThresholdP, l_criticalThresholdP, maxTicksP, l_maxTicksP, kindP, pluggedIn, percentP, res);
    }
    break;
    case sysTrapSysBatteryInfoV20: {
      // UInt16 SysBatteryInfoV20(Boolean set, UInt16 *warnThresholdP, UInt16 *criticalThresholdP, UInt16 *maxTicksP, SysBatteryKind *kindP, Boolean *pluggedIn)
      uint8_t set = ARG8;
      uint32_t warnThresholdP = ARG32;
      UInt16 l_warnThresholdP = 0;
      uint32_t criticalThresholdP = ARG32;
      UInt16 l_criticalThresholdP = 0;
      uint32_t maxTicksP = ARG32;
      Int16 l_maxTicksP = 0;
      uint32_t kindP = ARG32;
      SysBatteryKind l_kindP;
      uint32_t pluggedIn = ARG32;
      Boolean l_pluggedIn;
      UInt16 res = SysBatteryInfoV20(set, warnThresholdP ? &l_warnThresholdP : NULL, criticalThresholdP ? &l_criticalThresholdP : NULL, maxTicksP ? &l_maxTicksP : NULL, kindP ? &l_kindP : NULL, pluggedIn ? &l_pluggedIn : NULL);
      if (warnThresholdP) m68k_write_memory_16(warnThresholdP, l_warnThresholdP);
      if (criticalThresholdP) m68k_write_memory_16(criticalThresholdP, l_criticalThresholdP);
      if (maxTicksP) m68k_write_memory_16(maxTicksP, l_maxTicksP);
      if (pluggedIn) m68k_write_memory_8(pluggedIn, l_pluggedIn);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysBatteryInfoV20(set=%d, warnThresholdP=0x%08X [%d], criticalThresholdP=0x%08X [%d], maxTicksP=0x%08X [%d], kindP=0x%08X, pluggedIn=0x%08X): %d", set, warnThresholdP, l_warnThresholdP, criticalThresholdP, l_criticalThresholdP, maxTicksP, l_maxTicksP, kindP, pluggedIn, res);
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
        if (refNum > BASE_SYSLIBS) {
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
