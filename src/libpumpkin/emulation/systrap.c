#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>

#include "sys.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "bytes.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "emupalmosinc.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "trapnames.h"
#include "debug.h"

// not mapped:
// FldNewField

static void palmos_libtrap(uint16_t refNum, uint16_t trap) {
  char buf[256];

  switch (refNum) {
    case NetLibRefNum:
      palmos_netlibtrap(trap);
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "trap 0x%04X refNum %d not mapped", trap, refNum);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}

static int palmos_systrap_gen(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  int handled = 1;
  uint8_t *ram = pumpkin_heap_base();

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    #include "switch.c"
    default:
      handled = 0;
  }

  return handled;
}

uint32_t palmos_systrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx, selector;
  char buf[256], buf2[8];
  char *s;
  Err err;
  emu_state_t *state = m68k_get_emu_state();
  uint32_t r = 0;

  // MathLib seems to use trap numbers like 0x0306 instead of 0xA306.
  trap = (trap & 0x0FFF) | 0xA000;
  s = trapName(trap, &selector, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X begin (%s) pc=0x%08X", trap, s ? s : "unknown", m68k_get_reg(NULL, M68K_REG_PC));

  if (palmos_systrap_gen(trap)) {
    debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X end (gen)", trap);
    pumpkin_trace(trap);
    return 0;
  }

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
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
    case sysTrapSysLibFind: { // Return a reference number for a library that is already loaded, given its name.
      // Err SysLibFind(const Char *nameP, UInt16 *refNumP)
      uint32_t nameP = ARG32;
      uint32_t refNumP = ARG32;
      char *name = (char *)emupalmos_trap_in(nameP, trap, 0);
      emupalmos_trap_in(refNumP, trap, 1);
      UInt16 refNum;
      if (name && !StrCompare(name, NetLibName)) {
        refNum = NetLibRefNum;
      } else {
        refNum = SysLibFind68K(name);
      }
      err = refNum ? errNone : sysErrLibNotFound;
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
      pumpkin_id2s(libCreator, buf2);
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibLoad('%s', '%s', 0x%08X) native", buf, buf2, refNumP);
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
      pumpkin_id2s(creator, buf2);
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibNewRefNum68K('%s', '%s', 0x%08X): %d ", buf, buf2, refNumP, exists);
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
      debug(DEBUG_INFO, "EmuPalmOS", "SysLibTblEntry(%d): 0x%08X", refNum, a);
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
    case sysTrapPceNativeCall: {
      // UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
#ifdef ARMEMU
      uint32_t nativeFuncP = ARG32;
      uint32_t userDataP = ARG32;
      emupalmos_trap_in(nativeFuncP, trap, 0);
      emupalmos_trap_in(userDataP, trap, 1);
      UInt32 res = arm_native_call(nativeFuncP, 0, userDataP);
      debug(DEBUG_TRACE, "EmuPalmOS", "PceNativeCall(0x%08X, 0x%08X): %d", nativeFuncP, userDataP, res);
      m68k_set_reg(M68K_REG_A0, res);
      m68k_set_reg(M68K_REG_D0, res);
#endif
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
      char *s = trapName(trapNum, &selector, 0);
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
      char *s = trapName(trapNum, &selector, 0);
      if (s) {
        a = pumpkin_heap_size() + (trapNum << 2);
      }
      debug(DEBUG_INFO, "EmuPalmOS", "SysGetTrapAddress(0x%04X [ %s ]): 0x%08X", trapNum, s ? s : "unknown", a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapHwrGetROMToken: {
      // Err HwrGetROMToken(UInt16 cardNo, UInt32 token, out UInt8 **dataP, out UInt16 *sizeP)
      uint16_t cardNo = ARG16;
      uint32_t token = ARG32;
      uint32_t dataP = ARG32;
      UInt8 *l_dataP;
      uint32_t sizeP = ARG32;
      UInt16 l_sizeP;
      emupalmos_trap_in(dataP, trap, 2);
      emupalmos_trap_in(sizeP, trap, 3);
      Err res = HwrGetROMToken(cardNo, token, &l_dataP, &l_sizeP);
      if (dataP) m68k_write_memory_32(dataP, emupalmos_trap_out(l_dataP));
      if (sizeP) m68k_write_memory_16(sizeP, l_sizeP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "HwrGetROMToken(cardNo=%d, token=%d, dataP=0x%08X, sizeP=0x%08X): %d", cardNo, token, dataP, sizeP, res);
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
    case sysTrapHostControl: {
      // UInt32 HostControl(HostControlTrapNumber selector, ...)
      uint16_t selector = ARG16;
      UInt32 res = 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "HostControl(0x%04X): 0x%08X", selector, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapSysNotifyRegister: {
      // Err SysNotifyRegister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, SysNotifyProcPtr callbackP, Int8 priority, void *userDataP)
      uint16_t cardNo = ARG16;
      uint32_t dbID = ARG32;
      uint32_t notifyType = ARG32;
      uint32_t callbackP = ARG32;
      int32_t priority = ARG8;
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
      int32_t priority = ARG8;
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
      decode_notify(notifyP, &notify, 0);
      Err res = SysNotifyBroadcast(notifyP ? &notify : NULL);
      encode_notify(notifyP, &notify, 0);
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
    case sysTrapResLoadConstant: {
      // UInt32 ResLoadConstant(UInt16 rscID)
      uint16_t rscID = ARG16;
      UInt32 res = ResLoadConstant(rscID);
      debug(DEBUG_TRACE, "EmuPalmOS", "ResLoadConstant(%d): 0x%08X", rscID, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapAttnListOpen: {
      // void AttnListOpen(void)
      AttnListOpen();
      debug(DEBUG_TRACE, "EmuPalmOS", "AttnListOpen()");
      }
      break;
    case sysTrapAttnIndicatorEnable: {
      // void AttnIndicatorEnable(Boolean enableIt)
      uint16_t enableIt = ARG8;
      AttnIndicatorEnable(enableIt);
      debug(DEBUG_TRACE, "EmuPalmOS", "AttnIndicatorEnable(%d)", enableIt);
      }
      break;
    case sysTrapDlkGetSyncInfo: {
      // Err DlkGetSyncInfo(UInt32 *succSyncDateP, UInt32 *lastSyncDateP, DlkSyncStateType *syncStateP, Char *nameBufP, Char *logBufP, Int32 *logLenP)
      uint32_t succSyncDateP = ARG32;
      uint32_t lastSyncDateP = ARG32;
      uint32_t syncStateP = ARG32;
      uint32_t nameBufP = ARG32;
      uint32_t logBufP = ARG32;
      uint32_t logLenP = ARG32;
      UInt32 succSyncDate, lastSyncDate;
      DlkSyncStateType syncState;
      emupalmos_trap_in(succSyncDateP, trap, 0);
      emupalmos_trap_in(lastSyncDateP, trap, 1);
      emupalmos_trap_in(syncStateP, trap, 2);
      char *nameBuf = emupalmos_trap_in(nameBufP, trap, 3);
      char *logBuf = emupalmos_trap_in(logBufP, trap, 4);
      Int32 logLen;
      err = DlkGetSyncInfo(&succSyncDate, &lastSyncDate, &syncState, nameBuf, logBuf, &logLen);
      if (succSyncDateP) m68k_write_memory_32(succSyncDateP, succSyncDate);
      if (lastSyncDateP) m68k_write_memory_32(lastSyncDateP, lastSyncDate);
      if (syncStateP) m68k_write_memory_8(syncStateP, syncState);
      if (logLenP) m68k_write_memory_32(logLenP, logLen);
      debug(DEBUG_TRACE, "EmuPalmOS", "DlkGetSyncInfo(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", succSyncDateP, lastSyncDateP, syncStateP, nameBufP, logBufP, logLenP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapErrDisplayFileLineMsg: {
      // void ErrDisplayFileLineMsg(const Char * const filename, UInt16 lineNo, const Char * const msg)
      uint32_t filenameP = ARG32;
      uint16_t lineNo = ARG16;
      uint32_t msgP = ARG32;
      char *filename = emupalmos_trap_in(filenameP, trap, 0);
      char *msg = emupalmos_trap_in(msgP, trap, 2);
      ErrDisplayFileLineMsg(filename, lineNo, msg);
      debug(DEBUG_INFO, "EmuPalmOS", "ErrDisplayFileLineMsg(0x%08X \"%s\", %d, 0x%08X \"%s\")", filenameP, filename ? filename : "", lineNo, msgP, msg ? msg : "");
      }
      break;
    case sysTrapFileControl: {
      // Err FileControl(FileOpEnum op, FileHand stream, inout void *valueP, inout Int32 *valueLenP)
      uint8_t op = ARG8;
      uint32_t stream = ARG32;
      FileHand l_stream = (FileHand)emupalmos_trap_in(stream, trap, 1);
      uint32_t valueP = ARG32;
      void *s_valueP = emupalmos_trap_in(valueP, trap, 2);
      uint32_t valueLenP = ARG32;
      emupalmos_trap_in(valueLenP, trap, 3);
      Int32 l_valueLenP;
      if (valueLenP) l_valueLenP = m68k_read_memory_32(valueLenP);
      // XXX read valueP
      Err res = FileControl(op, l_stream, s_valueP, &l_valueLenP);
      // XXX fill valueP
      if (valueLenP) m68k_write_memory_32(valueLenP, l_valueLenP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileControl(op=%d, stream=0x%08X, valueP=0x%08X, valueLenP=0x%08X): %d", op, stream, valueP, valueLenP, res);
      }
      break;
    case sysTrapFtrPtrNew: {
      // Err FtrPtrNew(UInt32 creator, UInt16 featureNum, UInt32 size, void **newPtrP)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t size = ARG32;
      uint32_t newPtrP = ARG32;
      emupalmos_trap_in(newPtrP, trap, 3);
      uint8_t *p = MemPtrNew(size);
      if (p) {
        uint32_t a = emupalmos_trap_out(p);
        if (newPtrP) m68k_write_memory_32(newPtrP, a);
        err = FtrSet(creator, featureNum, a);
      } else {
        err = memErrNotEnoughSpace;
      }
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrNew('%s', %d, %d, 0x%08X): %d", buf, featureNum, size, newPtrP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapFtrPtrFree: {
      // Err FtrPtrFree(UInt32 creator, UInt16 featureNum)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t a;
      err = FtrGet(creator, featureNum, &a);
      if (err == errNone && a) {
        uint8_t *p = emupalmos_trap_in(a, trap, -1);
        MemPtrFree(p);
      }
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrFree('%s', %d): %d", buf, featureNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapFtrUnregister: {
      // Err FtrUnregister(UInt32 creator, UInt16 featureNum)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      err = FtrUnregister(creator, featureNum);
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrUnregister('%s', %d): %d", buf, featureNum, err);
      }
      break;
    case sysTrapFtrGet: {
      // Err FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t valueP = ARG32;
      emupalmos_trap_in(valueP, trap, 2);
      uint32_t value;
      pumpkin_id2s(creator, buf);
      err = FtrGet(creator, featureNum, &value);

      if (creator == sysFileCSystem && featureNum == sysFtrNumProcessorID && err == errNone) {
#ifdef ARMEMU
        // If the processor is 68K, Cubis writes directly to the display bitmap. It works ONLY if the display is 8bpp.
        //value = sysFtrNumProcessorEZ;

        // If the processor is ARM, Cubis does not write directly to the display bitmap. It works both on 8pp and 16bpp. No hooks are necessary.
        value = sysFtrNumProcessorARM720T;
#else
        value = sysFtrNumProcessorEZ;
#endif
      }

      debug(DEBUG_TRACE, "EmuPalmOS", "FtrGet('%s', %d, 0x%08X [0x%08X]): %d", buf, featureNum, valueP, value, err);
      m68k_write_memory_32(valueP, value);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapFtrSet: {
      // Err FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue)
      uint32_t creator = ARG32;
      uint16_t featureNum = ARG16;
      uint32_t newValue = ARG32;
      pumpkin_id2s(creator, buf);
      err = FtrSet(creator, featureNum, newValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "FtrSet('%s', %d, %d): %d", buf, featureNum, newValue, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSelectOneTime: {
      // Boolean SelectOneTime(Int16 *hour, Int16 *minute, const Char *titleP)
      uint32_t hourP = ARG32;
      uint32_t minP = ARG32;
      uint32_t titleP = ARG32;
      Int16 hour, min;
      emupalmos_trap_in(hourP, trap, 0);
      emupalmos_trap_in(minP, trap, 1);
      char *title = (char *)emupalmos_trap_in(titleP, trap, 2);
      if (hourP) hour = m68k_read_memory_16(hourP);
      if (minP) min = m68k_read_memory_16(minP);
      Boolean res = SelectOneTime(hourP ? &hour : NULL, minP ? &min : NULL, title);
      if (hourP) m68k_write_memory_16(hourP, hour);
      if (minP) m68k_write_memory_16(minP, min);
      debug(DEBUG_TRACE, "EmuPalmOS", "SelectOneTime(0x%08X, 0x%08X, 0x%08X): %d", hourP, minP, titleP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapSelectDay: {
      // Boolean SelectDay(const SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char *title)
      uint8_t selectDayBy = ARG8;
      uint32_t monthP = ARG32;
      uint32_t dayP = ARG32;
      uint32_t yearP = ARG32;
      uint32_t titleP = ARG32;
      Int16 month, day, year;
      emupalmos_trap_in(monthP, trap, 1);
      emupalmos_trap_in(dayP, trap, 2);
      emupalmos_trap_in(yearP, trap, 3);
      char *title = (char *)emupalmos_trap_in(titleP, trap, 4);
      if (monthP) month = m68k_read_memory_16(monthP);
      if (dayP) day = m68k_read_memory_16(dayP);
      if (yearP) year = m68k_read_memory_16(yearP);
      Boolean res = SelectDay(selectDayBy, monthP ? &month : NULL, dayP ? &day : NULL, yearP ? &year : NULL, title);
      if (monthP) m68k_write_memory_16(monthP, month);
      if (dayP) m68k_write_memory_16(dayP, day);
      if (yearP) m68k_write_memory_16(yearP, year);
      debug(DEBUG_TRACE, "EmuPalmOS", "SelectDay(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", selectDayBy, monthP, dayP, yearP, titleP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDaysInMonth: {
      // Int16 DaysInMonth(Int16 month, Int16 year)
      int16_t month = ARG16;
      int16_t year = ARG16;
      Int16 res = DaysInMonth(month, year);
      debug(DEBUG_TRACE, "EmuPalmOS", "DaysInMonth(%d, %d): %d", month, year, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDayOfWeek: {
      // Int16 DayOfWeek(Int16 month, Int16 day, Int16 year)
      int16_t month = ARG16;
      int16_t day = ARG16;
      int16_t year = ARG16;
      Int16 res = DayOfWeek(month, day, year);
      debug(DEBUG_TRACE, "EmuPalmOS", "DayOfWeek(%d, %d, %d): %d", month, day, year, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDateSecondsToDate: {
      // void DateSecondsToDate(UInt32 seconds, DateType *dateP)
      uint32_t seconds = ARG32;
      uint32_t dateP = ARG32;
      emupalmos_trap_in(dateP, trap, 1);
      union {
        UInt16 bits;
        DateType fields;
      } date;
      DateSecondsToDate(seconds, dateP ? &date.fields : NULL);
      if (dateP) m68k_write_memory_16(dateP, date.bits);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateSecondsToDate(%u, 0x%08X)", seconds, dateP);
      }
      break;
    case sysTrapDateToDOWDMFormat: {
      // void DateToDOWDMFormat(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString)
      uint8_t months = ARG8;
      uint8_t days = ARG8;
      uint16_t years = ARG16;
      uint8_t dateFormat = ARG8;
      uint32_t stringP = ARG32;
      char *string = (char *)emupalmos_trap_in(stringP, trap, 4);
      DateToDOWDMFormat(months, days, years, dateFormat, string);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToDOWDMFormat(%u, %u, %u, %u, 0x%08X)", months, days, years, dateFormat, stringP);
      }
      break;
    case sysTrapDateToAscii: {
      // void DateToAscii(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString)
      uint8_t months = ARG8;
      uint8_t days = ARG8;
      uint16_t years = ARG16;
      uint8_t dateFormat = ARG8;
      uint32_t stringP = ARG32;
      char *string = (char *)emupalmos_trap_in(stringP, trap, 4);
      DateToAscii(months, days, years, dateFormat, string);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToAscii(%u, %u, %u, %u, 0x%08X)", months, days, years, dateFormat, stringP);
      }
      break;
    case sysTrapDateToDays: {
      // UInt32 DateToDays(DateType date)
      union {
        UInt16 bits;
        DateType fields;
      } date;
      date.bits = ARG16;
      UInt32 res = DateToDays(date.fields);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToDays(0x%04X [%04d-%02d-%02d]): %d", date.bits, date.fields.year+1904, date.fields.month, date.fields.day, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDateDaysToDate: {
      // void DateDaysToDate(UInt32 days, DateType *dateP)
      uint32_t days = ARG32;
      uint32_t dateP = ARG32;
      emupalmos_trap_in(dateP, trap, 1);
      union {
        UInt16 bits;
        DateType fields;
      } date;
      DateDaysToDate(days, dateP ? &date.fields : NULL);
      if (dateP) m68k_write_memory_16(dateP, date.bits);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateDaysToDate(%u, 0x%08X)", days, dateP);
      }
      break;
    case sysTrapTimAdjust: {
      // void TimAdjust(DateTimeType *dateTimeP, Int32 adjustment)
      uint32_t dateTimeP = ARG32;
      int32_t adjustment = ARG32;
      emupalmos_trap_in(dateTimeP, trap, 0);
      DateTimeType dateTime;
      decode_datetime(dateTimeP, &dateTime);
      TimAdjust(&dateTime, adjustment);
      encode_datetime(dateTimeP, &dateTime);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimAdjust(0x%08X, %d)", dateTimeP, adjustment);
      }
      break;
    case sysTrapDateAdjust: {
      // void DateAdjust(DateType *dateP, Int32 adjustment)
      uint32_t dateP = ARG32;
      int32_t adjustment = ARG32;
      emupalmos_trap_in(dateP, trap, 0);
      union {
        UInt16 bits;
        DateType fields;
      } date;
      if (dateP) date.bits = m68k_read_memory_16(dateP);
      DateAdjust(&date.fields, adjustment);
      if (dateP) m68k_write_memory_16(dateP, date.bits);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateAdjust(0x%08X, %d)", dateP, adjustment);
      }
      break;
    case sysTrapTimeToAscii: {
      // void TimeToAscii(UInt8 hours, UInt8 minutes, TimeFormatType timeFormat, Char *pString)
      uint8_t hours = ARG8;
      uint8_t minutes = ARG8;
      uint8_t timeFormat = ARG8;
      uint32_t stringP = ARG32;
      char *string = (char *)emupalmos_trap_in(stringP, trap, 3);
      TimeToAscii(hours, minutes, timeFormat, string);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimeToAscii(%u, %u, %u, 0x%08X \"%s\")", hours, minutes, timeFormat, stringP, string ? string : "");
      }
      break;
    case sysTrapDateTemplateToAscii: {
      // UInt16 DateTemplateToAscii(const Char *templateP, UInt8 months, UInt8 days, UInt16 years, Char *stringP, Int16 stringLen)
      uint32_t templateP = ARG32;
      uint8_t months = ARG8;
      uint8_t days = ARG8;
      uint16_t years = ARG16;
      uint32_t stringP = ARG32;
      int16_t stringLen = ARG16;
      char *template = (char *)emupalmos_trap_in(templateP, trap, 0);
      char *string = (char *)emupalmos_trap_in(stringP, trap, 4);
      UInt16 res = DateTemplateToAscii(template, months, days, years, string, stringLen);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateTemplateToAscii(0x%08X, %u, %u, %u, 0x%08X \"%s\", %d): %u", templateP, months, days, years, stringP, string ? string : "", stringLen, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapTimDateTimeToSeconds: {
      // UInt32 TimDateTimeToSeconds(const DateTimeType *dateTimeP)
      uint32_t dateTimeP = ARG32;
      emupalmos_trap_in(dateTimeP, trap, 0);
      DateTimeType dateTime;
      decode_datetime(dateTimeP, &dateTime);
      UInt32 seconds = TimDateTimeToSeconds(&dateTime);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimDateTimeToSeconds(0x%08X [%04d-%02d-%02d %02d:%02d:%02d]): %u", dateTimeP, dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second, seconds);
      m68k_set_reg(M68K_REG_D0, seconds);
      }
      break;
    case sysTrapTimSecondsToDateTime: {
      // void TimSecondsToDateTime(UInt32 seconds, DateTimeType *dateTimeP)
      uint32_t seconds = ARG32;
      uint32_t dateTimeP = ARG32;
      emupalmos_trap_in(dateTimeP, trap, 1);
      DateTimeType dateTime;
      TimSecondsToDateTime(seconds, &dateTime);
      encode_datetime(dateTimeP, &dateTime);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimSecondsToDateTime(%u, 0x%08X [%04d-%02d-%02d %02d:%02d:%02d])", seconds, dateTimeP, dateTime.year, dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second);
      }
      break;
    case sysTrapTimGetSeconds: {
      // UInt32 TimGetSeconds(void)
      UInt32 t = TimGetSeconds();
      debug(DEBUG_TRACE, "EmuPalmOS", "TimGetSeconds(): %u", t);
      m68k_set_reg(M68K_REG_D0, t);
      }
      break;
    case sysTrapTimGetTicks: {
      // UInt32 TimGetTicks(void)
      UInt32 t = TimGetTicks();
      debug(DEBUG_TRACE, "EmuPalmOS", "TimGetTicks(): %u", t);
      m68k_set_reg(M68K_REG_D0, t);
      }
      break;
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
    case sysTrapWinScreenMode: {
      // Err WinScreenMode(WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP)
      uint8_t operation = ARG8;
      uint32_t widthP = ARG32;
      uint32_t heightP = ARG32;
      uint32_t depthP = ARG32;
      uint32_t enableColorP = ARG32;
      emupalmos_trap_in(widthP, trap, 1);
      emupalmos_trap_in(heightP, trap, 2);
      emupalmos_trap_in(depthP, trap, 3);
      emupalmos_trap_in(enableColorP, trap, 4);
      uint32_t width = 0, height = 0, depth = 0;
      Boolean enableColor = 0;
      if (widthP) width = m68k_read_memory_32(widthP);
      if (heightP) height = m68k_read_memory_32(heightP);
      if (depthP) depth = m68k_read_memory_32(depthP);
      if (enableColorP) enableColor = m68k_read_memory_32(enableColorP);
      err = WinScreenMode(operation, widthP ? &width : NULL, heightP ? &height : NULL, depthP ? &depth : NULL, enableColorP ? &enableColor : NULL);
      if (operation == winScreenModeGetSupportedDepths) {
        // do not advertise support for 16 bits color
        depth &= 0xff;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenMode(%d, 0x%08X [%d], 0x%08X [%d], 0x%08X [%d], 0x%08X [%d]): %d",
        operation, widthP, width, heightP, height, depthP, depth, enableColorP, enableColor, err);
      if (widthP) m68k_write_memory_32(widthP, width);
      if (heightP) m68k_write_memory_32(heightP, height);
      if (depthP) m68k_write_memory_32(depthP, depth);
      if (enableColorP) m68k_write_memory_8(enableColorP, enableColor);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapWinPalette: {
      // Err WinPalette(UInt8 operation, Int16 startIndex, UInt16 paletteEntries, RGBColorType *tableP)
      // operation:
      // 0: winPaletteGet
      // 1: winPaletteSet
      // 2: winPaletteSetToDefault
      uint8_t operation = ARG8;
      int16_t startIndex = ARG16;
      uint16_t paletteEntries = ARG16;
      uint32_t tableP = ARG32;
      emupalmos_trap_in(tableP, trap, 3);
      uint32_t i;
      RGBColorType table[256];
      MemSet(table, sizeof(table), 0);
      if (operation == winPaletteSet && tableP) {
        for (i = 0; i < paletteEntries; i++) {
          decode_rgb(tableP + i*4, &table[i]);
          debug(DEBUG_TRACE, "EmuPalmOS", "palette %d: %d,%d,%d", startIndex+i, table[i].r, table[i].g, table[i].b);
        }
      }
      err = WinPalette(operation, startIndex, paletteEntries, tableP ? table : NULL);
      if (operation == winPaletteGet && tableP && err == errNone) {
        for (i = 0; i < paletteEntries; i++) {
          encode_rgb(tableP + i*4, &table[i]);
        }
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "WinPalette(%d, %d, %d, 0x%08X): %d", operation, startIndex, paletteEntries, tableP, err);
      WinHandle wh = WinGetDrawWindow();
      debug(DEBUG_TRACE, "EmuPalmOS", "WinPalette draw window 0x%08X", emupalmos_trap_out(wh));
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapFntDefineFont: {
      // Err FntDefineFont(FontID font, FontPtr fontP)
      uint8_t font = ARG8;
      uint32_t fontP = ARG32;
      FontPtr fontp = (FontPtr)emupalmos_trap_in(fontP, trap, 1);
      err = FntDefineFont(font, fontp);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntDefineFont(%d, 0x%08X): %d", font, fontP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapWinCreateWindow: {
      // WinHandle WinCreateWindow(const RectangleType *bounds, FrameType frame, Boolean modal, Boolean focusable, UInt16 *error)
      uint32_t boundsP = ARG32;
      uint16_t frame = ARG16;
      uint8_t modal = ARG8;
      uint8_t focusable = ARG8;
      uint32_t errorP = ARG32;
      emupalmos_trap_in(boundsP, trap, 0);
      emupalmos_trap_in(errorP, trap, 4);
      RectangleType bounds;
      UInt16 error;
      decode_rectangle(boundsP, &bounds);
      WinHandle wh = WinCreateWindow(boundsP ? &bounds : NULL, frame, modal, focusable, errorP ? &error : NULL);
      encode_rectangle(boundsP, &bounds);
      if (errorP) m68k_write_memory_16(errorP, error);
      uint32_t w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateWindow([%d,%d,%d,%d], %d, %d, %d, 0x%08X [%d]): 0x%08X", bounds.topLeft.x, bounds.topLeft.y, bounds.extent.x, bounds.extent.y, frame, modal, focusable, errorP, error, w);
      m68k_set_reg(M68K_REG_A0, w);
      }
      break;
    case sysTrapWinCreateBitmapWindow: {
      // WinHandle WinCreateBitmapWindow(BitmapType *bitmapP, UInt16 *error)
      uint32_t bitmapP = ARG32;
      uint32_t errorP = ARG32;
      UInt16 error;
      BitmapType *bitmap = (BitmapType *)emupalmos_trap_in(bitmapP, trap, 0);
      WinHandle wh = WinCreateBitmapWindow(bitmap, &error);
      if (errorP) m68k_write_memory_16(errorP, error);
      uint32_t w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateBitmapWindow(0x%08X, 0x%08X [%d]): 0x%08X", bitmapP, errorP, error, w);
      m68k_set_reg(M68K_REG_A0, w);
      }
      break;
    case sysTrapWinCreateOffscreenWindow: {
      // WinHandle WinCreateOffscreenWindow(Coord width, Coord height, WindowFormatType format, UInt16 *error)
      uint16_t width = ARG16;
      uint16_t height = ARG16;
      uint8_t format = ARG8;
      uint32_t errorP = ARG32;
      emupalmos_trap_in(errorP, trap, 3);
      UInt16 error;
      WinHandle wh = WinCreateOffscreenWindow(width, height, format, errorP ? &error : NULL);
      if (errorP) m68k_write_memory_16(errorP, error);
      uint32_t w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinCreateOffscreenWindow(%d, %d, %d, 0x%08X [%d]): 0x%08X", width, height, format, errorP, error, w);
      m68k_set_reg(M68K_REG_A0, w);
      }
      break;
    case sysTrapWinDeleteWindow: {
      // void WinDeleteWindow(WinHandle winHandle, Boolean eraseIt)
      uint32_t w = ARG32;
      uint8_t eraseIt = ARG8;
      WinHandle wh = (WinHandle)emupalmos_trap_in(w, trap, 0);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinDeleteWindow(0x%08X, %d) ...", w, eraseIt);
      WinDeleteWindow(wh, eraseIt);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinDeleteWindow(0x%08X, %d)", w, eraseIt);
      }
      break;
    case sysTrapRctSetRectangle: {
      // void RctSetRectangle(RectangleType *rP, Coord left, Coord top, Coord width, Coord height)
      uint32_t rP = ARG32;
      int16_t left = ARG16;
      int16_t top = ARG16;
      int16_t width = ARG16;
      int16_t height = ARG16;
      emupalmos_trap_in(rP, trap, 0);
      RectangleType rect;
      RctSetRectangle(rP ? &rect : NULL, left, top, width, height);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctSetRectangle(0x%08X [%d,%d,%d,%d], %d, %d, %d, %d)", rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, left, top, width, height);
      }
      break;
    case sysTrapRctInsetRectangle: {
      // void RctInsetRectangle(RectangleType *rP, Coord insetAmt)
      uint32_t rP = ARG32;
      int16_t insetAmt = ARG16;
      emupalmos_trap_in(rP, trap, 0);
      RectangleType rect;
      decode_rectangle(rP, &rect);
      RctInsetRectangle(rP ? &rect : NULL, insetAmt);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctInsetRectangle(0x%08X [%d,%d,%d,%d], %d)",
        rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, insetAmt);
      }
      break;
    case sysTrapRctOffsetRectangle: {
      // void RctOffsetRectangle(RectangleType *rP, Coord deltaX, Coord deltaY)
      uint32_t rP = ARG32;
      int16_t deltaX = ARG16;
      int16_t deltaY = ARG16;
      emupalmos_trap_in(rP, trap, 0);
      RectangleType rect;
      decode_rectangle(rP, &rect);
      RctOffsetRectangle(rP ? &rect : NULL, deltaX, deltaY);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctOffsetRectangle(0x%08X [%d,%d,%d,%d], %d, %d)",
        rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, deltaX, deltaY);
      }
      break;
    case sysTrapRctCopyRectangle: {
      // void RctCopyRectangle(const RectangleType *srcRectP, RectangleType *dstRectP)
      uint32_t srcRectP = ARG32;
      uint32_t dstRectP = ARG32;
      emupalmos_trap_in(srcRectP, trap, 0);
      emupalmos_trap_in(dstRectP, trap, 1);
      RectangleType src, dst;
      decode_rectangle(srcRectP, &src);
      RctCopyRectangle(srcRectP ? &src : NULL, dstRectP ? &dst : NULL);
      encode_rectangle(dstRectP, &dst);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctCopyRectangle(0x%08X [%d,%d,%d,%d], 0x%08X [%d,%d,%d,%d])",
        srcRectP, src.topLeft.x, src.topLeft.y, src.extent.x, src.extent.y,
        dstRectP, dst.topLeft.x, dst.topLeft.y, dst.extent.x, dst.extent.y);
      }
      break;
    case sysTrapRctPtInRectangle: {
      // Boolean RctPtInRectangle(Coord x, Coord y, const RectangleType *rP)
      int16_t x = ARG16;
      int16_t y = ARG16;
      uint32_t rP = ARG32;
      emupalmos_trap_in(rP, trap, 2);
      RectangleType rect;
      decode_rectangle(rP, &rect);
      Boolean res = RctPtInRectangle(x, y, rP ? &rect : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctPtInRectangle(%d, %d, 0x%08X [%d,%d,%d,%d]): %d", x, y, rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapRctGetIntersection: {
      // void RctGetIntersection(const RectangleType *r1P, const RectangleType *r2P, RectangleType *r3P)
      uint32_t r1P = ARG32;
      uint32_t r2P = ARG32;
      uint32_t r3P = ARG32;
      emupalmos_trap_in(r1P, trap, 0);
      emupalmos_trap_in(r2P, trap, 1);
      emupalmos_trap_in(r3P, trap, 2);
      RectangleType rect1, rect2, rect3;
      decode_rectangle(r1P, &rect1);
      decode_rectangle(r2P, &rect2);
      decode_rectangle(r3P, &rect3);
      RctGetIntersection(r1P ? &rect1 : NULL, r2P ? &rect2 : NULL, r3P ? &rect3 : NULL);
      encode_rectangle(r1P, &rect1);
      encode_rectangle(r2P, &rect2);
      encode_rectangle(r3P, &rect3);
      debug(DEBUG_TRACE, "EmuPalmOS", "RctGetIntersection(0x%08X, 0x%08X, 0x%08X)", r1P, r2P, r3P);
      }
      break;
    case sysTrapBmpCreate: {
      // BitmapType *BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *colorTableP, UInt16 *error)
      int16_t width = ARG16;
      int16_t height = ARG16;
      uint8_t depth = ARG8;
      uint32_t colorTableP = ARG32;
      uint32_t errorP = ARG32;
      emupalmos_trap_in(errorP, trap, 4);
      UInt16 error;
      BitmapType *bitmap = BmpCreate(width, height, depth, (ColorTableType *)emupalmos_trap_in(colorTableP, trap, 3), errorP ? &error : NULL);
      uint32_t a = emupalmos_trap_out(bitmap);
      if (errorP) m68k_write_memory_16(errorP, error);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpCreate(width=%d, height=%d, depth=%d, colorTableP=0x%08X, error=0x%08X [%d]): 0x%08X", width, height, depth, colorTableP, errorP, error, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapBmpDelete: {
      // Err BmpDelete(BitmapType *bitmapP)
      uint32_t bitmapP = ARG32;
      BitmapType *bitmap = (BitmapType *)emupalmos_trap_in(bitmapP, trap, 0);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpDelete(0x%08X) ...", bitmapP);
      err = BmpDelete(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpDelete(0x%08X): %d", bitmapP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSecSelectViewStatus: {
      // privateRecordViewEnum SecSelectViewStatus(void)
      privateRecordViewEnum r = SecSelectViewStatus();
      debug(DEBUG_TRACE, "EmuPalmOS", "SecSelectViewStatus(): %d", r);
      m68k_set_reg(M68K_REG_D0, r);
      }
      break;
    case sysTrapFontSelect: {
      // FontID FontSelect(FontID fontID)
      uint8_t fontID = ARG8;
      uint8_t oldFontID = FontSelect(fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FontID(%d): %d", fontID, oldFontID);
      m68k_set_reg(M68K_REG_D0, oldFontID);
      }
      break;
    case sysTrapUIColorPushTable: {
      // Err UIColorPushTable(void)
      err = UIColorPushTable();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorPushTable(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapUIColorPopTable: {
      // Err UIColorPopTable(void)
      err = UIColorPopTable();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorPopTable(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapUIColorSetTableEntry: {
      // Err UIColorSetTableEntry(UIColorTableEntries which, const RGBColorType *rgbP)
      uint8_t which = ARG8;
      uint32_t rgbP = ARG32;
      emupalmos_trap_in(rgbP, trap, 1);
      RGBColorType rgb;
      decode_rgb(rgbP, &rgb);
      err = UIColorSetTableEntry(which, rgbP ? &rgb : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorSetTableEntry(%d, 0x%08X [%d,%d,%d,%d]): %d", which, rgbP, rgb.index, rgb.r, rgb.g, rgb.b, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapUIColorGetTableEntryRGB: {
      // void UIColorGetTableEntryRGB(UIColorTableEntries which, RGBColorType *rgbP)
      uint8_t which = ARG8;
      uint32_t rgbP = ARG32;
      emupalmos_trap_in(rgbP, trap, 1);
      RGBColorType rgb;
      UIColorGetTableEntryRGB(which, rgbP ? &rgb : NULL);
      encode_rgb(rgbP, &rgb);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorGetTableEntryRGB(%d, 0x%08X [%d,%d,%d,%d])", which, rgbP, rgb.index, rgb.r, rgb.g, rgb.b);
      }
      break;
    case sysTrapUIColorGetTableEntryIndex: {
      // IndexedColorType UIColorGetTableEntryIndex(UIColorTableEntries which)
      uint8_t which = ARG8;
      IndexedColorType c = UIColorGetTableEntryIndex(which);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorGetTableEntryIndex(%d): %d", which, c);
      m68k_set_reg(M68K_REG_D0, c);
      }
      break;
    case sysTrapPrefGetPreferences: {
      // void PrefGetPreferences(SystemPreferencesPtr p)
      uint32_t p = ARG32;
      emupalmos_trap_in(p, trap, 0);
      SystemPreferencesType prefs;
      PrefGetPreferences(p ? &prefs : NULL);
      // XXX decode prefs into p
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetPreferences(0x%08X)", p);
      }
      break;
    case sysTrapPrefGetPreference: {
      // UInt32 PrefGetPreference(SystemPreferencesChoice choice)
      uint8_t choice = ARG8;
      uint32_t value = PrefGetPreference(choice);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetPreference(%d): %d", choice, value);
      m68k_set_reg(M68K_REG_D0, value);
      }
      break;
    case sysTrapPrefSetPreference: {
      //void PrefSetPreference(SystemPreferencesChoice choice, UInt32 value)
      uint8_t choice = ARG8;
      uint32_t value = ARG32;
      PrefSetPreference(choice, value);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetPreference(%d, %d)", choice, value);
      }
      break;
    case sysTrapPrefOpenPreferenceDB: {
      // DmOpenRef PrefOpenPreferenceDB(Boolean saved)
      uint8_t saved = ARG8;
      DmOpenRef dbRef = PrefOpenPreferenceDB(saved);
      uint32_t a = emupalmos_trap_out(dbRef);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefOpenPreferenceDB(%d): 0x%08X", saved, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapPrefOpenPreferenceDBV10: {
      // DmOpenRef PrefOpenPreferenceDBV10(void)
      DmOpenRef dbRef = PrefOpenPreferenceDBV10();
      uint32_t a = emupalmos_trap_out(dbRef);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefOpenPreferenceDBV10(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapPrefSetAppPreferences: {
      // void PrefSetAppPreferences(UInt32 creator, UInt16 id, Int16 version, const void *prefs, UInt16 prefsSize, Boolean saved)
      uint32_t creator = ARG32;
      uint16_t id = ARG16;
      int16_t version = ARG16;
      uint32_t prefsP = ARG32;
      uint16_t prefsSize = ARG16;
      uint8_t saved = ARG8;
      PrefSetAppPreferences(creator, id, version, emupalmos_trap_in(prefsP, trap, 3), prefsSize, saved);
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferences('%s', %d, %d, 0x%08X, %d, %d)", buf, id, version, prefsP, prefsSize, saved);
      }
      break;
    case sysTrapPrefSetAppPreferencesV10: {
      // void PrefSetAppPreferencesV10(UInt32 creator, Int16 version, void *prefs, UInt16 prefsSize)
      uint32_t creator = ARG32;
      int16_t version = ARG16;
      uint32_t prefsP = ARG32;
      uint16_t prefsSize = ARG16;
      PrefSetAppPreferencesV10(creator, version, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferencesV10('%s', %d, 0x%08X, %d)", buf, version, prefsP, prefsSize);
      }
      break;
    case sysTrapPrefGetAppPreferences: {
      // Int16 PrefGetAppPreferences(UInt32 creator, UInt16 id, void *prefs, UInt16 *prefsSize, Boolean saved)
      uint32_t creator = ARG32;
      uint16_t id = ARG16;
      uint32_t prefsP = ARG32;
      uint32_t prefsSizeP = ARG32;
      uint8_t saved = ARG8;
      emupalmos_trap_in(prefsSizeP, trap, 3);
      UInt16 prefsSize = prefsSizeP ? m68k_read_memory_16(prefsSizeP) : 0;
      UInt16 version = PrefGetAppPreferences(creator, id, emupalmos_trap_in(prefsP, trap, 2), prefsSizeP ? &prefsSize : NULL, saved);
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferences('%s', %d, 0x%08X, 0x%08X, %d): %d", buf, id, prefsP, prefsSizeP, saved, version);
      if (prefsSizeP) m68k_write_memory_16(prefsSizeP, prefsSize);
      m68k_set_reg(M68K_REG_D0, version);
      }
      break;
    case sysTrapPrefGetAppPreferencesV10: {
      // Boolean PrefGetAppPreferencesV10(UInt32 type, Int16 version, void *prefs, UInt16 prefsSize)
      uint32_t type = ARG32;
      uint16_t version = ARG16;
      uint32_t prefsP = ARG32;
      uint16_t prefsSize = ARG16;
      Boolean b = PrefGetAppPreferencesV10(type, version, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
      pumpkin_id2s(type, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferencesV10('%s', %d, 0x%08X, %d): %d", buf, version, prefsP, prefsSize, b);
      m68k_set_reg(M68K_REG_D0, b);
      }
      break;
    case sysTrapMemSet: {
      // Err MemSet(void *dstP, Int32 numBytes, UInt8 value)
      uint32_t dstP = ARG32;
      uint32_t numBytes = ARG32;
      uint8_t value = ARG8;
      UInt32 start, end;
      WinLegacyGetAddr(&start, &end);
      if ((dstP >= start && dstP < end) ||
          (dstP+numBytes-1 >= start && dstP+numBytes-1 < end) ||
          (dstP < start && dstP+numBytes >= end)) {
        debug(DEBUG_TRACE, "EmuPalmOS", "MemSet(0x%08X, %d, 0x%02X) inside screen", dstP, numBytes, value);
        for (uint32_t i = 0; i < numBytes; i++) {
          m68k_write_memory_8(dstP+i, value);
        }
        err = 0;
      } else {
        if (emupalmos_check_address(dstP, numBytes, 0)) {
          err = MemSet(emupalmos_trap_in(dstP, trap, 0), numBytes, value);
        } else {
          err = dmErrInvalidParam;
        }
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "MemSet(0x%08X, %d, 0x%02X): %d", dstP, numBytes, value, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapMemMove: {
      // Err MemMove(void *dstP, const void *sP, Int32 numBytes)
      uint32_t dstP = ARG32;
      uint32_t sP = ARG32;
      int32_t numBytes = ARG32;
      UInt32 start, end;
      WinLegacyGetAddr(&start, &end);
      if ((dstP >= start && dstP < end) ||
          (dstP+numBytes-1 >= start && dstP+numBytes-1 < end) ||
          (dstP < start && dstP+numBytes >= end) ||
          (sP >= start && sP < end) ||
          (sP+numBytes-1 >= start && sP+numBytes-1 < end) ||
          (sP < start && sP+numBytes >= end)) {
        debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d) inside screen", dstP, sP, numBytes);
        for (uint32_t i = 0; i < numBytes; i++) {
          uint8_t value = m68k_read_memory_8(sP+i);
          m68k_write_memory_8(dstP+i, value);
        }
        err = 0;
      } else {
        if (emupalmos_check_address(dstP, numBytes, 0) && emupalmos_check_address(sP, numBytes, 1)) {
          err = MemMove(emupalmos_trap_in(dstP, trap, 0), emupalmos_trap_in(sP, trap, 1), numBytes);
        } else {
          err = dmErrInvalidParam;
        }
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d): %d", dstP, sP, numBytes, err);
      m68k_set_reg(M68K_REG_D0, err);
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
    case sysTrapDmDetachRecord: {
      // Err DmDetachRecord(DmOpenRef dbP, UInt16 index, MemHandle *oldHP)
      uint32_t dbP = ARG32;
      uint16_t index = ARG16;
      uint32_t oldHP = ARG32;
      emupalmos_trap_in(oldHP, trap, 2);
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      MemHandle old;
      Err res = DmDetachRecord(dbRef, index, oldHP ? &old : NULL);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachRecord(0x%08X, %d, 0x%08X): %d", dbP, index, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDmDetachResource: {
      // Err DmDetachResource(DmOpenRef dbP, UInt16 index, MemHandle *oldHP)
      uint32_t dbP = ARG32;
      uint16_t index = ARG16;
      uint32_t oldHP = ARG32;
      emupalmos_trap_in(oldHP, trap, 2);
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      MemHandle old;
      Err res = DmDetachResource(dbRef, index, oldHP ? &old : NULL);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmDetachsource(0x%08X, %d, 0x%08X): %d", dbP, index, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDmSearchResource: {
      // UInt16 DmSearchResource(DmResType resType, DmResID resID, MemHandle resH, DmOpenRef *dbPP)
      uint32_t type = ARG32;
      uint32_t resID = ARG16;
      uint32_t ih = ARG32;
      uint32_t dbPP = ARG32;
      MemHandle h = emupalmos_trap_in(ih, trap, 2);
      emupalmos_trap_in(dbPP, trap, 3);
      DmOpenRef dbP;
      UInt16 index = DmSearchResource(type, resID, h, dbPP ? &dbP : NULL);
      if (dbPP) m68k_write_memory_32(dbPP, emupalmos_trap_out(dbP));
      pumpkin_id2s(type, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmSearchResource('%s', %d, 0x%08X, 0x%08X): %d", buf, resID, ih, dbPP, index);
      m68k_set_reg(M68K_REG_D0, index);
      }
      break;
    case sysTrapMemHandleLock: {
      // MemPtr MemHandleLock(MemHandle h)
      uint32_t ih = ARG32;
      MemHandle h = emupalmos_trap_in(ih, trap, 0);
      uint8_t *p = MemHandleLock(h);
      uint32_t a = emupalmos_trap_out(p);
      debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleLock(0x%08X): 0x%08X (%p)", ih, a, p);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapDmGetNextDatabaseByTypeCreator: {
      // Err DmGetNextDatabaseByTypeCreator(Boolean newSearch, DmSearchStatePtr stateInfoP, UInt32 type, UInt32 creator, Boolean onlyLatestVers, UInt16 *cardNoP, LocalID *dbIDP)
      uint8_t newSearch = ARG8;
      uint32_t stateInfoP = ARG32;
      uint32_t type = ARG32;
      uint32_t creator = ARG32;
      uint8_t onlyLatestVers = ARG8;
      uint32_t cardNoP = ARG32;
      uint32_t dbIDP = ARG32;
      emupalmos_trap_in(stateInfoP, trap, 1);
      emupalmos_trap_in(cardNoP, trap, 5);
      emupalmos_trap_in(dbIDP, trap, 6);
      DmSearchStateType stateInfo;
      UInt16 cardNo;
      LocalID dbID = 0;
      if (stateInfoP && !newSearch) {
        uint32_t info = m68k_read_memory_32(stateInfoP);
        stateInfo.p = emupalmos_trap_in(info, trap, -1);
      }
      err = DmGetNextDatabaseByTypeCreator(newSearch, stateInfoP ? &stateInfo : NULL, type, creator, onlyLatestVers, cardNoP ? &cardNo : NULL, dbIDP ? &dbID : NULL);
      if (stateInfoP) {
        uint32_t info = emupalmos_trap_out(stateInfo.p);
        m68k_write_memory_32(stateInfoP, info);
      }
      if (cardNoP) m68k_write_memory_16(cardNoP, cardNo);
      if (dbIDP) m68k_write_memory_32(dbIDP, dbID);
      pumpkin_id2s(type, buf);
      pumpkin_id2s(creator, buf2);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmGetNextDatabaseByTypeCreator(%d, 0x%08X, '%s', '%s', %d, 0x%08X, 0x%08X): %d", newSearch, stateInfoP, buf, buf2, onlyLatestVers, cardNoP, dbIDP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapDmInsertionSort: {
      // Err DmInsertionSort(DmOpenRef dbP, DmComparF *comparF, Int16 other)
      uint32_t dbP = ARG32;
      uint32_t comparP = ARG32;
      int16_t other = ARG16;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      Err res = DmInsertionSort68K(dbRef, comparP, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmInsertionSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, other, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDmQuickSort: {
      // Err DmQuickSort(DmOpenRef dbP, DmComparF *comparF, Int16 other)
      uint32_t dbP = ARG32;
      uint32_t comparP = ARG32;
      int16_t other = ARG16;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      emupalmos_trap_in(comparP, trap, 1);
      Err res = DmQuickSort68K(dbRef, comparP, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmQuickSort(0x%08X, 0x%08X, %d): %d", dbP, comparP, other, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDmFindSortPosition: {
      // UInt16 DmFindSortPosition(DmOpenRef dbP, void *newRecord, SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other)
      uint32_t dbP = ARG32;
      uint32_t newRecordP = ARG32;
      uint32_t newRecordInfoP = ARG32;
      uint32_t comparP = ARG32;
      int16_t other = ARG16;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      emupalmos_trap_in(newRecordP, trap, 1);
      emupalmos_trap_in(newRecordInfoP, trap, 2);
      emupalmos_trap_in(comparP, trap, 3);
      UInt16 res = DmFindSortPosition68K(dbRef, newRecordP, newRecordInfoP, comparP, other);
      debug(DEBUG_TRACE, "EmuPalmOS", "DmFindSortPosition(0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", dbP, newRecordP, newRecordInfoP, comparP, other, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapDmAttachRecord: {
      // Err DmAttachRecord(DmOpenRef dbP, UInt16 *atP, MemHandle newH, MemHandle *oldHP)
      uint32_t dbP = ARG32;
      uint32_t atP = ARG32;
      uint32_t newH = ARG32;
      uint32_t oldHP = ARG32;
      DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
      emupalmos_trap_in(atP, trap, 1);
      UInt16 at = atP ? m68k_read_memory_16(atP) : 0;
      MemHandle h = emupalmos_trap_in(newH, trap, 2);
      emupalmos_trap_in(oldHP, trap, 3);
      MemHandle old;
      Err res = DmAttachRecord(dbRef, atP ? &at : NULL, h, oldHP ? &old : NULL);
      if (atP) m68k_write_memory_16(atP, at);
      if (oldHP) m68k_write_memory_32(oldHP, emupalmos_trap_out(old));
      debug(DEBUG_TRACE, "EmuPalmOS", "DmAttachRecord(0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", dbP, atP, newH, oldHP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapStrVPrintF:
    case sysTrapStrPrintF: {
      // Int16 StrVPrintF(Char *s, const Char *formatStr, _Palm_va_list arg)
      // Int16 StrPrintF(Char *s, const Char *formatStr, ...)
      uint32_t str = ARG32;
      uint32_t formatStr = ARG32;
      char *s = emupalmos_trap_in(str, trap, 0);
      char *f = emupalmos_trap_in(formatStr, trap, 1);
      Int16 res = 0;
      if (s && f) {
        int i, j, k = 1, t = 0, sz, arglen;
        uint32_t arg;
        char *p, *q, fmt[16];
        for (i = 0, p = s; f[i]; i++) {
          switch (t) {
            case 0:
              if (f[i] == '%') {
                j = 0;
                fmt[j++] = f[i];
                arglen = -1;
                sz = 2;
                t = 1;
              } else {
                *p++ = f[i];
              }
              break;
            case 1:
              switch (f[i]) {
                case 'h':
                case 'H':
                  fmt[j++] = f[i];
                  sz = 2;
                  break;
                case 'l':
                case 'L':
                  fmt[j++] = f[i];
                  sz = 4;
                  break;
                case 'd':
                case 'i':
                case 'u':
                case 'x':
                  switch (sz) {
                    case 1:  arg = ARG8;  break;
                    case 2:  arg = ARG16; break;
                    case 4:  arg = ARG32; break;
                    default: arg = ARG16; break;
                  }
                  k++;
                  fmt[j++] = f[i];
                  fmt[j] = 0;
                  sys_sprintf(p, fmt, arg);
                  p += sys_strlen(p);
                  t = 0;
                  break;
                case 'c':
                case 'C':
                  arg = ARG8;
                  k++;
                  fmt[j++] = f[i];
                  fmt[j] = 0;
                  sys_sprintf(p, fmt, arg);
                  p += sys_strlen(p);
                  t = 0;
                  break;
                case 's':
                  arg = ARG32;
                  k++;
                  q = emupalmos_trap_in(arg, trap, k);
                  fmt[j++] = f[i];
                  fmt[j] = 0;
                  if (arglen < 0) {
                    sys_sprintf(p, fmt, q);
                  } else {
                    sys_sprintf(p, fmt, arglen, q);
                  }
                  p += sys_strlen(p);
                  t = 0;
                  break;
                case '*':
                  arglen = ARG16;
                  k++;
                  break;
                case '%':
                  *p++ = f[i];
                  t = 0;
                  break;
                default:
                  fmt[j++] = f[i];
                  break;
              }
              break;
          }
        }
        *p = 0;
        res = sys_strlen(s);
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "StrPrintF(0x%08X \"%s\", 0x%08X \"%s\", ...): %d", str, s ? s : "", formatStr, f ? f : "", res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmNewForm: {
      // FormType *FrmNewForm(UInt16 formID, const Char *titleStrP, Coord x, Coord y, Coord width, Coord height, Boolean modal, UInt16 defaultButton, UInt16 helpRscID, UInt16 menuRscID)
      uint16_t formID = ARG16;
      uint32_t titleStrP = ARG32;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint16_t width = ARG16;
      uint16_t height = ARG16;
      uint8_t modal = ARG8;
      uint16_t defaultButton = ARG16;
      uint16_t helpRscID = ARG16;
      uint16_t menuRscID = ARG16;
      char *titleStr = (char *)emupalmos_trap_in(titleStrP, trap, 1);
      FormType *form = FrmNewForm(formID, titleStr, x, y, width, height, modal, defaultButton, helpRscID, menuRscID);
      uint32_t formP = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewForm(%d, 0x%08X [%s], %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X", formID, titleStrP, titleStr, x, y, width, height, modal, defaultButton, helpRscID, menuRscID, formP);
      m68k_set_reg(M68K_REG_A0, formP);
      }
      break;
    case sysTrapFrmInitForm: {
      // FormType *FrmInitForm(UInt16 rscID)
      uint16_t rscID = ARG16;
      FormType *form = FrmInitForm(rscID);
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmInitForm(%d): 0x%08X", rscID, f);
      m68k_set_reg(M68K_REG_A0, f);
      }
      break;
    case sysTrapFrmDeleteForm: {
      // void FrmDeleteForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmDeleteForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDeleteForm(0x%08X)", formP);
      }
      break;
    case sysTrapFrmGetFormId: {
      // UInt16 FrmGetFormId(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 id = FrmGetFormId(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormId(0x%08X): %d", formP, id);
      m68k_set_reg(M68K_REG_D0, id);
      }
      break;
    case sysTrapFrmGetFirstForm: {
      // FormType *FrmGetFirstForm(void)
      FormType *form = FrmGetFirstForm();
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFirstForm(): 0x%08X", f);
      m68k_set_reg(M68K_REG_A0, f);
      }
      break;
    case sysTrapFrmGetFormPtr: {
      // FormType *FrmGetFormPtr(UInt16 formId)
      uint16_t formId = ARG16;
      FormType *form = FrmGetFormPtr(formId);
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormPtr(%d): 0x%08X", formId, f);
      m68k_set_reg(M68K_REG_A0, f);
      }
      break;
    case sysTrapFrmGetObjectIndexFromPtr: {
      // UInt16 FrmGetObjectIndexFromPtr(const FormType *formP, void *objP)
      uint32_t formP = ARG32;
      uint32_t objP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      void *obj = emupalmos_trap_in(objP, trap, 1);
      UInt16 res = FrmGetObjectIndexFromPtr(form, obj);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndexFromPtr(0x%08X, 0x%08X): %d", formP, objP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmGetActiveField: {
      // FieldType *FrmGetActiveField(const FormType* formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FieldType *fld = FrmGetActiveField(form);
      uint32_t f = emupalmos_trap_out(fld);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveField(0x%08X): 0x%08X", formP, f);
      m68k_set_reg(M68K_REG_A0, f);
      }
      break;
    case sysTrapFrmGotoForm: {
      // void FrmGotoForm(UInt16 formId)
      uint16_t formId = ARG16;
      FrmGotoForm(formId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGotoForm(%d)", formId);
      }
      break;
    case sysTrapFrmUpdateForm: {
      // void FrmUpdateForm(UInt16 formId, UInt16 updateCode)
      uint16_t formId = ARG16;
      uint16_t updateCode = ARG16;
      FrmUpdateForm(formId, updateCode);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmUpdateForm(%d, %d)", formId, updateCode);
      }
      break;
    case sysTrapFrmDrawForm: {
      // void FrmDrawForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm begin");
      FrmDrawForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm(0x%08X)", formP);

/*
      if (form) {
        int hasNativeHandler = 0;
        for (uint16_t objIndex = 0;; objIndex++) {
          void *obj = FrmGetObjectPtr(form, objIndex);
          if (obj == NULL) break;
          FormObjectKind objType = FrmGetObjectType(form, objIndex);
          if (objType == frmGadgetObj) {
            FormGadgetType *gadget = (FormGadgetType *)obj;
            if (gadget->m68k_handler) {
              hasNativeHandler = 1;
              break;
            }
          }
        }
        if (hasNativeHandler) {
          debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm(0x%08X) native", formP);
          r = FrmDrawForm_addr;
        }
      }
*/
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDrawForm end");
      }
      break;
    case sysTrapFrmEraseForm: {
      // void FrmEraseForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmEraseForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmEraseForm(0x%08X)", formP);
      }
      break;
    case sysTrapFrmVisible: {
      // Boolean FrmVisible(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      Boolean res = FrmVisible(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmVisible(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmHideObject: {
      // void FrmHideObject(FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t index = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmHideObject(form, index);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHideObject(0x%08X, %d)", formP, index);
      // XXX must handle 68K code because of gadget handler
      }
      break;
    case sysTrapFrmShowObject: {
      // void FrmShowObject(FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t index = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmShowObject(form, index);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmShowObject(0x%08X, %d)", formP, index);
      // XXX must handle 68K code because of gadget handler
      }
      break;
    case sysTrapFrmGetFocus: {
      // UInt16 FrmGetFocus(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetFocus(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFocus(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmSetMenu: {
      // void FrmSetMenu(FormType *formP, UInt16 menuRscID)
      uint32_t formP = ARG32;
      uint16_t menuRscID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetMenu(form, menuRscID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetMenu(0x%08X, %d)", formP, menuRscID);
      }
      break;
    case sysTrapFrmGetTitle: {
      // const Char *FrmGetTitle(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *title = (char *)FrmGetTitle(form);
      uint32_t s = emupalmos_trap_out(title);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetTitle(0x%08X): 0x%08X \"%s\"", formP, s, title ? title : "");
      m68k_set_reg(M68K_REG_A0, s);
      }
      break;
    case sysTrapFrmCopyTitle: {
      // void FrmCopyTitle(FormType *formP, const Char *newTitle)
      uint32_t formP = ARG32;
      uint32_t newTitleP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
      FrmCopyTitle(form, newTitle);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCopyTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
      }
      break;
    case sysTrapFrmSetTitle: {
      // void FrmSetTitle(FormType *formP, Char *newTitle)
      uint32_t formP = ARG32;
      uint32_t newTitleP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *newTitle = (char *)emupalmos_trap_in(newTitleP, trap, 1);
      FrmSetTitle(form, newTitle);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetTitle(0x%08X, 0x%08X \"%s\")", formP, newTitleP, newTitle ? newTitle : "");
      }
      break;
    case sysTrapFrmUpdateScrollers: {
      // void FrmUpdateScrollers(FormType *formP, UInt16 upIndex, UInt16 downIndex, Boolean scrollableUp, Boolean scrollableDown)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      uint16_t upIndex = ARG16;
      uint16_t downIndex = ARG16;
      uint8_t scrollableUp = ARG8;
      uint8_t scrollableDown = ARG8;
      FrmUpdateScrollers(form, upIndex, downIndex, scrollableUp, scrollableDown);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmUpdateScrollers(0x%08X, %d, %d, %d, %d)", formP, upIndex, downIndex, scrollableUp, scrollableDown);
      }
      break;
    case sysTrapFrmSetActiveForm: {
      // void FrmSetActiveForm(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetActiveForm(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetActiveForm(0x%08X)", formP);
      }
      break;
    case sysTrapFrmSetEventHandler: {
      // void FrmSetEventHandler(FormType *formP, FormEventHandlerType *handler)
      uint32_t formP = ARG32;
      uint32_t handlerP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) form->m68k_handler = handlerP;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetEventHandler(0x%08X, 0x%08X)", formP, handlerP);
      }
      break;
    case sysTrapFrmGetEventHandler68K: { // custom trap created for use in 68K code
      // FormEventHandlerType *FrmGetEventHandler68K(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      uint32_t handlerP = form ? form->m68k_handler : 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetEventHandler68K(0x%08X): 0x%08X", formP, handlerP);
      m68k_set_reg(M68K_REG_A0, handlerP);
      }
      break;
    case sysTrapFrmSetGadgetHandler: {
      // void FrmSetGadgetHandler(FormType *formP, UInt16 objIndex, FormGadgetHandlerType *attrP)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t handlerP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) gadget->m68k_handler = handlerP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetGadgetHandler(0x%08X, %d, 0x%08X)", formP, objIndex, handlerP);
      }
      break;
    case sysTrapFrmGetGadgetData: {
      // void *FrmGetGadgetData(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      uint32_t dataP = 0;
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) dataP = gadget->m68k_data;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetGadgetData(0x%08X, %d): 0x%08X", formP, objIndex, dataP);
      m68k_set_reg(M68K_REG_A0, dataP);
      }
      break;
    case sysTrapFrmSetGadgetData: {
      // void FrmSetGadgetData(FormType *formP, UInt16 objIndex, const void *data)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t dataP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) gadget->m68k_data = dataP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetGadgetData(0x%08X, %d, 0x%08X)", formP, objIndex, dataP);
      }
      break;
    case sysTrapFrmGetGadgetPtr68K: {
      // FormGadgetTypeInCallback *FrmGetGadgetPtr68k(FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t gadgetP = 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      if (form) {
        FormGadgetType *gadget = FrmGetObjectPtr(form, objIndex);
        if (gadget) {
          gadgetP = emupalmos_trap_out(gadget);
          if (FrmGetObjectType(form, objIndex) == frmGadgetObj) {
            encode_gadget(gadgetP, gadget);
          }
        }
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetGadgetPtr68k(0x%08X, %d): 0x%08X", formP, objIndex, gadgetP);
      m68k_set_reg(M68K_REG_A0, gadgetP);
      }
      break;
    case sysTrapFrmGetWindowHandle: {
      // WinHandle FrmGetWindowHandle(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      WinHandle wh = FrmGetWindowHandle(form);
      uint32_t w = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetWindowHandle(0x%08X): 0x%08X", formP, w);
      m68k_set_reg(M68K_REG_A0, w);
      }
      break;
    case sysTrapFrmGetFormBounds: {
      // void FrmGetFormBounds(const FormType *formP, RectangleType *rP)
      uint32_t formP = ARG32;
      uint32_t rP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      RectangleType rect;
      FrmGetFormBounds(form, &rect);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetFormBounds(0x%08X, 0x%08X)", formP, rP);
      }
      break;
    case sysTrapFrmSetObjectBounds: {
      // void FrmSetObjectBounds(FormType *formP, UInt16 objIndex, const RectangleType *bounds)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t rP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      RectangleType rect;
      decode_rectangle(rP, &rect);
      FrmSetObjectBounds(form, objIndex, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetObjectBounds(0x%08X, %d, 0x%08X)", formP, objIndex, rP);
      }
      break;
    case sysTrapFrmGetNumberOfObjects: {
      // UInt16 FrmGetNumberOfObjects(const FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetNumberOfObjects(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetNumberOfObjects(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmSetObjectPosition: {
      // void FrmSetObjectPosition(FormType *formP, UInt16 objIndex, Coord x, Coord y)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      Coord x = ARG16;
      Coord y = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetObjectPosition(form, objIndex, x, y);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetObjectPosition(0x%08X, %u, %d, %d)", formP, objIndex, x, y);
      }
      break;
    case sysTrapFrmGetObjectId: {
      // UInt16 FrmGetObjectId(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetObjectId(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectId(0x%08X, %d): %d", formP, objIndex, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmGetObjectPosition: {
      // void FrmGetObjectPosition(const FormType *formP, UInt16 objIndex, Coord *x, Coord *y)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t xP = ARG32;
      uint32_t yP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      Coord x, y;
      FrmGetObjectPosition(form, objIndex, xP ? &x : NULL, yP ? &y : NULL);
      if (xP) m68k_write_memory_16(xP, x);
      if (yP) m68k_write_memory_16(yP, y);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectPosition(0x%08X, %d, 0x%08X, 0x%08X)", formP, objIndex, xP, yP);
      }
      break;
    case sysTrapFrmGetObjectBounds: {
      // void FrmGetObjectBounds(const FormType *formP, UInt16 objIndex, RectangleType *rP)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      uint32_t rP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      RectangleType rect;
      FrmGetObjectBounds(form, objIndex, &rect);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectBounds(0x%08X, %d, 0x%08X)", formP, objIndex, rP);
      }
      break;
    case sysTrapFrmGetControlGroupSelection: {
      // UInt16 FrmGetControlGroupSelection(const FormType *formP, UInt8 groupNum)
      uint32_t formP = ARG32;
      uint8_t groupNum = ARG8;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmGetControlGroupSelection(form, groupNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlGroupSelection(0x%08X, %u): %u", formP, groupNum, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmGetActiveForm: {
      // FormType *FrmGetActiveForm(void)
      FormType *form = FrmGetActiveForm();
      uint32_t f = emupalmos_trap_out(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveForm(): 0x%08X", f);
      m68k_set_reg(M68K_REG_A0, f);
      }
      break;
    case sysTrapFrmGetActiveFormID: {
      // UInt16 FrmGetActiveFormID(void)
      UInt16 id = FrmGetActiveFormID();
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetActiveFormID(): %d", id);
      m68k_set_reg(M68K_REG_D0, id);
      }
      break;
    case sysTrapFrmGetObjectIndex: {
      // UInt16 FrmGetObjectIndex(const FormType *formP, UInt16 objID)
      uint32_t formP = ARG32;
      uint16_t objID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 index = FrmGetObjectIndex(form, objID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectIndex(0x%08X, %d): %d", formP, objID, index);
      m68k_set_reg(M68K_REG_D0, index);
      }
      break;
    case sysTrapFrmGetObjectPtr: {
      // void *FrmGetObjectPtr(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      void *p = FrmGetObjectPtr(form, objIndex);
      uint32_t ptr = emupalmos_trap_out(p);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectPtr(0x%08X, %d): 0x%08X", formP, objIndex, ptr);
      m68k_set_reg(M68K_REG_A0, ptr);
      }
      break;
    case sysTrapFrmGetObjectType: {
      // FormObjectKind FrmGetObjectType(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FormObjectKind objType = FrmGetObjectType(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetObjectType(0x%08X, %d): %d", formP, objIndex, objType);
      m68k_set_reg(M68K_REG_D0, objType);
      }
      break;
    case sysTrapFrmGetLabel: {
      // const Char *FrmGetLabel(const FormType *formP, UInt16 labelID)
      uint32_t formP = ARG32;
      uint16_t labelID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      const Char *label = FrmGetLabel(form, labelID);
      uint32_t a = emupalmos_trap_out((void *)label);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetLabel(0x%08X, %d): 0x%08X", formP, labelID, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapFrmSetFocus: {
      // void FrmSetFocus(FormType *formP, UInt16 fieldIndex)
      uint32_t formP = ARG32;
      uint16_t fieldIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetFocus(form, fieldIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetFocus(0x%08X, %d)", formP, fieldIndex);
      }
      break;
    case sysTrapFrmGetControlValue: {
      // Int16 FrmGetControlValue(const FormType *formP, UInt16 objIndex)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      Int16 value = FrmGetControlValue(form, objIndex);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmGetControlValue(0x%08X, %d): %d", formP, objIndex, value);
      m68k_set_reg(M68K_REG_D0, value);
      }
      break;
    case sysTrapFrmSetControlValue: {
      // void FrmSetControlValue(const FormType *formP, UInt16 objIndex, Int16 newValue)
      uint32_t formP = ARG32;
      uint16_t objIndex = ARG16;
      int16_t newValue = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetControlValue(form, objIndex, newValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetControlValue(0x%08X, %d, %d)", formP, objIndex, newValue);
      }
      break;
    case sysTrapFrmSetControlGroupSelection: {
      // void FrmSetControlGroupSelection(const FormType *formP, UInt8 groupNum, UInt16 controlID)
      uint32_t formP = ARG32;
      uint8_t groupNum = ARG8;
      uint16_t controlID = ARG16;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmSetControlGroupSelection(form, groupNum, controlID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSetControlGroupSelection(0x%08X, %d, %d)", formP, groupNum, controlID);
      }
      break;
    case sysTrapFrmDispatchEvent: {
      // Boolean FrmDispatchEvent(EventType *eventP)
      uint32_t eventP = ARG32;
      EventType event;
      if (eventP) decode_event(eventP, &event);
      Boolean res = FrmDispatchEvent(&event);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDispatchEvent(0x%08X): %d", eventP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmHandleEvent: {
      // Boolean FrmHandleEvent(FormType *formP, EventType *eventP)
      uint32_t formP = ARG32;
      uint32_t eventP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      EventType event;
      if (eventP) decode_event(eventP, &event);
      Boolean res = FrmHandleEvent(form, &event);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHandleEvent(0x%08X, 0x%08X): %d", formP, eventP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmCopyLabel: {
      // void FrmCopyLabel(FormType *formP, UInt16 labelID, const Char *newLabel)
      uint32_t formP = ARG32;
      uint16_t labelID = ARG16;
      uint32_t newLabelP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      char *s = (char *)emupalmos_trap_in(newLabelP, trap, 1);
      FrmCopyLabel(form, labelID, s);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCopyLabel(0x%08X, %d, 0x%08X \"%s\")", formP, labelID, newLabelP, s ? s : "");
      }
      break;
    case sysTrapFrmSaveAllForms:
      // void FrmSaveAllForms(void)
      FrmSaveAllForms();
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmSaveAllForms()");
      break;
    case sysTrapFrmCloseAllForms:
      // void FrmCloseAllForms(void)
      FrmCloseAllForms();
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCloseAllForms()");
      break;
    case sysTrapFrmPopupForm: {
      // void FrmPopupForm(UInt16 formId)
      uint16_t formId = ARG16;
      FrmPopupForm(formId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmPopupForm(%d)", formId);
      }
      break;
    case sysTrapFrmDoDialog: {
      // UInt16 FrmDoDialog(FormType *formP)
      uint32_t formP = ARG32;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      UInt16 res = FrmDoDialog(form);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmDoDialog(0x%08X): %d", formP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmReturnToForm: {
      // void FrmReturnToForm(UInt16 formId)
      uint16_t formId = ARG16;
      FrmReturnToForm(formId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmReturnToForm(%d)", formId);
      }
      break;
    case sysTrapFrmHelp: {
      // void FrmHelp(UInt16 helpMsgId)
      uint16_t helpMsgId = ARG16;
      FrmHelp(helpMsgId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmHelp(%d)", helpMsgId);
      }
      break;
    case sysTrapAbtShowAbout: {
      // void AbtShowAbout(UInt32 creator)
      uint32_t creator = ARG32;
      AbtShowAbout(creator);
      debug(DEBUG_TRACE, "EmuPalmOS", "AbtShowAbout(%d)", creator);
      }
      break;
    case sysTrapFrmCustomAlert: {
      // UInt16 FrmCustomAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3)
      uint16_t alertId = ARG16;
      uint32_t s1P = ARG32;
      uint32_t s2P = ARG32;
      uint32_t s3P = ARG32;
      char *s1 = (char *)emupalmos_trap_in(s1P, trap, 1);
      char *s2 = (char *)emupalmos_trap_in(s2P, trap, 2);
      char *s3 = (char *)emupalmos_trap_in(s3P, trap, 3);
      UInt16 res = FrmCustomAlert(alertId, s1, s2, s3);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomAlert(%d, 0x%08X, 0x%08X, 0x%08X): %d", alertId, s1P, s2P, s3P, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmCustomResponseAlert: {
      // UInt16 FrmCustomResponseAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3, Char *entryStringBuf, Int16 entryStringBufLength, FormCheckResponseFuncPtr callback)
      uint16_t alertId = ARG16;
      uint32_t s1P = ARG32;
      uint32_t s2P = ARG32;
      uint32_t s3P = ARG32;
      uint32_t entryStringBufP = ARG32;
      int16_t entryStringBufLength = ARG16;
      uint32_t callbackP = ARG32;
      char *s1 = (char *)emupalmos_trap_in(s1P, trap, 1);
      char *s2 = (char *)emupalmos_trap_in(s2P, trap, 2);
      char *s3 = (char *)emupalmos_trap_in(s3P, trap, 3);
      char *entryStringBuf = (char *)emupalmos_trap_in(entryStringBufP, trap, 4);
      FormCheckResponseFuncPtr callback = (FormCheckResponseFuncPtr)emupalmos_trap_in(callbackP, trap, 6);
      UInt16 res = FrmCustomResponseAlert(alertId, s1, s2, s3, entryStringBuf, entryStringBufLength, callback);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmCustomResponseAlert(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d, 0x%08X): %d", alertId, s1P, s2P, s3P, entryStringBufP, entryStringBufLength, callbackP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmAlert: {
      // UInt16 FrmAlert(UInt16 alertId)
      uint16_t alertId = ARG16;
      UInt16 res = FrmAlert(alertId);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmAlert(%d): %d", alertId, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapFrmNewBitmap: {
      // FormBitmapType *FrmNewBitmap(FormType **formPP, UInt16 ID, UInt16 rscID, Coord x, Coord y)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      uint16_t rscId = ARG16;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FormBitmapType *bitmap = FrmNewBitmap(&form, id, rscId, x, y);
      uint32_t a = emupalmos_trap_out(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewBitmap(0x%08X, %u, %u, %d, %d): 0x%08X", formPP, id, rscId, x, y, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapFrmNewGadget: {
      // FormGadgetType *FrmNewGadget(FormType **formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint16_t width = ARG16;
      uint16_t height = ARG16;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FormGadgetType *gadget = FrmNewGadget(&form, id, x, y, width, height);
      uint32_t a = emupalmos_trap_out(gadget);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewGadget(0x%08X, %u, %d, %d, %d, %d): 0x%08X", formPP, id, x, y, width, height, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapFrmActiveState: {
      // Err FrmActiveState(FormActiveStateType *stateP, Boolean save)
      uint32_t stateP = ARG32;
      uint8_t save = ARG8;
      FormActiveStateType *state = (FormActiveStateType *)emupalmos_trap_in(stateP, trap, 0);
      Err err = FrmActiveState(state, save);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmActiveState(0x%08X, %d)", stateP, save);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapFrmNewGsi: {
      // FrmGraffitiStateType *FrmNewGsi(FormType **formPP, Coord x, Coord y)
      uint32_t formPP = ARG32;
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      FormType *form = (FormType *)emupalmos_trap_in(formP, trap, 0);
      FrmGraffitiStateType *gsi = FrmNewGsi(&form, x, y);
      uint32_t a = emupalmos_trap_out(gsi);
      debug(DEBUG_TRACE, "EmuPalmOS", "FrmNewGsi(0x%08X, %d, %d): 0x%08X", formPP, x, y, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapCtlNewControl: {
      // ControlType *CtlNewControl(void **formPP, UInt16 ID, ControlStyleType style, const Char *textP, Coord x, Coord y, Coord width, Coord height, FontID font, UInt8 group, Boolean leftAnchor)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      uint8_t style = ARG8;
      uint32_t textP = ARG32;
      int16_t x = ARG16;
      int16_t y = ARG16;
      int16_t width = ARG16;
      int16_t height = ARG16;
      uint8_t font = ARG8;
      uint8_t group = ARG8;
      uint8_t leftAnchor = ARG8;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      void *form = emupalmos_trap_in(formP, trap, 0);
      char *text = (char *)emupalmos_trap_in(textP, trap, 3);
      ControlType *ctl = CtlNewControl(&form, id, style, text, x, y, width, height, font, group, leftAnchor);
      uint32_t a = emupalmos_trap_out(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlNewControl(0x%08X, %u, %d, 0x%08X [%s], %d, %d, %d, %d, %d, %d, %d): 0x%08X", formPP, id, style, textP, text, x, y, width, height, font, group, leftAnchor, a);
      m68k_set_reg(M68K_REG_A0, a);
      }
      break;
    case sysTrapCtlGetStyle68K: { // custom trap created for use in 68K code
      // ControlStyleType CtlGetStyle(ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *control = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
      ControlStyleType style = control ? control->style : 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetStyle(0x%08X): %d", controlP, style);
      m68k_set_reg(M68K_REG_D0, style);
      }
      break;
    case sysTrapCtlGetLabel: {
      // const Char *CtlGetLabel(ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
      Char *res = (Char *)CtlGetLabel(s_controlP);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetLabel(controlP=0x%08X): 0x%08X", controlP, r_res);
      }
      break;
    case sysTrapLstSetDrawFunction: {
      // void LstSetDrawFunction(ListType *listP, ListDrawDataFuncPtr func)
      uint32_t listP = ARG32;
      uint32_t funcP = ARG32;
      ListType *list = (ListType *)emupalmos_trap_in(listP, trap, 0);
      emupalmos_trap_in(funcP, trap, 1);
      if (list) list->m68k_drawfunc = funcP;
      debug(DEBUG_TRACE, "EmuPalmOS", "LstSetDrawFunction(0x%08X, 0x%08X)", listP, funcP);
      }
      break;
    case sysTrapLstDrawList: {
      // void LstDrawList(ListType *listP)
      uint32_t listP = ARG32;
      ListType *list = (ListType *)emupalmos_trap_in(listP, trap, 0);
      LstDrawList(list);
      debug(DEBUG_TRACE, "EmuPalmOS", "LstDrawList(0x%08X)", listP);
      }
      break;
    case sysTrapTblSetCustomDrawProcedure: {
      // void TblSetCustomDrawProcedure(TableType *tableP, Int16 column, TableDrawItemFuncPtr drawCallback)
      uint32_t tableP = ARG32;
      int16_t column = ARG16;
      uint32_t funcP = ARG32;
      TableType *table = (TableType *)emupalmos_trap_in(tableP, trap, 0);
      emupalmos_trap_in(funcP, trap, 2);
      if (table && column >= 0 && column < table->numColumns) {
        table->columnAttrs[column].m68k_drawfunc = funcP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetCustomDrawProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
      }
      break;
    case sysTrapTblSetLoadDataProcedure: {
      // void TblSetLoadDataProcedure(TableType *tableP, Int16 column, TableLoadDataFuncPtr loadDataCallback)
      uint32_t tableP = ARG32;
      int16_t column = ARG16;
      uint32_t funcP = ARG32;
      TableType *table = (TableType *)emupalmos_trap_in(tableP, trap, 0);
      emupalmos_trap_in(funcP, trap, 2);
      if (table && column >= 0 && column < table->numColumns) {
        table->columnAttrs[column].m68k_loadfunc = funcP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetLoadDataProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
      }
      break;
    case sysTrapTblSetSaveDataProcedure: {
      // void TblSetSaveDataProcedure(TableType *tableP, Int16 column, TableSaveDataFuncPtr saveDataCallback)
      uint32_t tableP = ARG32;
      int16_t column = ARG16;
      uint32_t funcP = ARG32;
      TableType *table = (TableType *)emupalmos_trap_in(tableP, trap, 0);
      emupalmos_trap_in(funcP, trap, 2);
      if (table && column >= 0 && column < table->numColumns) {
        table->columnAttrs[column].m68k_savefunc = funcP;
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "TblSetSaveDataProcedure(0x%08X, %d, 0x%08X)", tableP, column, funcP);
      }
      break;
    case sysTrapSclSetScrollBar: {
      // void SclSetScrollBar(ScrollBarType *bar, Int16 value, Int16 min, Int16 max, Int16 pageSize)
      uint32_t barP = ARG32;
      int16_t value = ARG16;
      int16_t min = ARG16;
      int16_t max = ARG16;
      int16_t pageSize = ARG16;
      ScrollBarType *bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
      SclSetScrollBar(bar, value, min, max, pageSize);
      debug(DEBUG_TRACE, "EmuPalmOS", "SclSetScrollBar(0x%08X, %d, %d, %d, %d)", barP, value, min, max, pageSize);
      }
      break;
    case sysTrapSclGetScrollBar: {
      // void SclGetScrollBar(ScrollBarType *bar, Int16 *valueP, Int16 *minP, Int16 *maxP, Int16 *pageSizeP)
      uint32_t barP = ARG32;
      uint32_t valueP = ARG32;
      uint32_t minP = ARG32;
      uint32_t maxP = ARG32;
      uint32_t pageSizeP = ARG32;
      Int16 value, min, max, pageSize;
      ScrollBarType *bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
      emupalmos_trap_in(valueP, trap, 1);
      emupalmos_trap_in(minP, trap, 2);
      emupalmos_trap_in(maxP, trap, 3);
      emupalmos_trap_in(pageSizeP, trap, 4);
      SclGetScrollBar(bar, &value, &min, &max, &pageSize);
      if (valueP) m68k_write_memory_16(valueP, value);
      if (minP) m68k_write_memory_16(minP, min);
      if (maxP) m68k_write_memory_16(maxP, max);
      if (pageSizeP) m68k_write_memory_16(pageSizeP, pageSize);
      }
      break;
    case sysTrapSclDrawScrollBar: {
      // void SclDrawScrollBar(ScrollBarType *bar)
      uint32_t barP = ARG32;
      ScrollBarType *bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
      SclDrawScrollBar(bar);
      }
      break;
    case sysTrapSclHandleEvent: {
      // Boolean SclHandleEvent(ScrollBarType *bar, EventType *event)
      uint32_t barP = ARG32;
      uint32_t eventP = ARG32;
      EventType event;
      if (eventP) decode_event(eventP, &event);
      ScrollBarType *bar = (ScrollBarType *)emupalmos_trap_in(barP, trap, 0);
      Boolean res = SclHandleEvent(bar, &event);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapEvtEnableGraffiti: {
      // void EvtEnableGraffiti(Boolean enable)
      uint8_t enable = ARG8;
      EvtEnableGraffiti(enable);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtEnableGraffiti(%d)", enable);
      }
      break;
    case sysTrapEvtResetAutoOffTimer: {
      // Err EvtResetAutoOffTimer(void)
      err = EvtResetAutoOffTimer();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtResetAutoOffTimer(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapEvtAddUniqueEventToQueue: {
      // void EvtAddUniqueEventToQueue(const EventType *eventP, UInt32 id, Boolean inPlace)
      uint32_t eventP = ARG32;
      uint32_t id = ARG32;
      uint8_t inPlace = ARG8;
      emupalmos_trap_in(eventP, trap, 0);
      EventType event;
      if (eventP) decode_event(eventP, &event);
      EvtAddUniqueEventToQueue(eventP ? &event : NULL, id, inPlace);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddUniqueEventToQueue(0x%08X [0x%04X], %d, %d)", eventP, event.eType, id, inPlace);
      }
      break;
    case sysTrapEvtAddEventToQueue: {
      // void EvtAddEventToQueue(const EventType *event)
      uint32_t eventP = ARG32;
      emupalmos_trap_in(eventP, trap, 0);
      EventType event;
      if (eventP) decode_event(eventP, &event);
      EvtAddEventToQueue(eventP ? &event : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [0x%04X])", eventP, event.eType);
      }
      break;
    case sysTrapEvtEnqueueKey: {
      // Err EvtEnqueueKey(WChar ascii, UInt16 keycode, UInt16 modifiers)
      uint16_t ascii = ARG16;
      uint16_t keycode = ARG16;
      uint16_t modifiers = ARG16;
      err = EvtEnqueueKey(ascii, keycode, modifiers);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtEnqueueKey(0x%04X, 0x%04X, 0x%04X): %d", ascii, keycode, modifiers, err);
      }
      break;
    case sysTrapEvtEventAvail: {
      // Boolean EvtEventAvail(void)
      Boolean res = EvtEventAvail();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtEventAvail(): %d", res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapEvtWakeup: {
      // Err EvtWakeup(void)
      err = EvtWakeup();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtWakeup(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapEvtGetEvent: {
      // void EvtGetEvent(EventType *event, Int32 timeout)
      uint32_t eventP = ARG32;
      int32_t timeout = ARG32;
      if (timeout == 0) timeout = 1; // XXX temporary fix to hungry apps
      emupalmos_trap_in(eventP, trap, 0);
      EventType event;
      MemSet(&event, sizeof(EventType), 0);
      EvtGetEvent(eventP ? &event : NULL, timeout);
      char *eventName = EvtGetEventName(event.eType);
      if (eventName) {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [%s], %d)", eventP, eventName, timeout);
      } else {
        debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetEvent(0x%08X [0x%04X], %d)", eventP, event.eType, timeout);
      }
      if (eventP) encode_event(eventP, &event);
      }
      break;
    case sysTrapEvtCopyEvent: {
      // void EvtCopyEvent(const EventType *source, EventType *dest)
      uint32_t sourceP = ARG32;
      uint32_t destP = ARG32;
      emupalmos_trap_in(sourceP, trap, 0);
      emupalmos_trap_in(destP, trap, 1);
      EventType source, dest;
      if (sourceP) decode_event(sourceP, &source);
      EvtCopyEvent(&source, &dest);
      if (destP) encode_event(destP, &dest);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtCopyEvent(0x%08X [0x%04X], 0x%08X)", sourceP, source.eType, destP);
      }
      break;
    case sysTrapPenSleep: {
      // Err PenSleep(void)
      err = PenSleep();
      debug(DEBUG_TRACE, "EmuPalmOS", "PenSleep(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapPenWake: {
      // Err PenWake(void)
      err = PenWake();
      debug(DEBUG_TRACE, "EmuPalmOS", "PenWake(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapEvtGetPen: {
      // void EvtGetPen(Int16 *pScreenX, Int16 *pScreenY, Boolean *pPenDown)
      uint32_t pScreenX = ARG32;
      uint32_t pScreenY = ARG32;
      uint32_t pPenDown = ARG32;
      emupalmos_trap_in(pScreenX, trap, 0);
      emupalmos_trap_in(pScreenY, trap, 1);
      emupalmos_trap_in(pPenDown, trap, 2);
      Int16 screenX, screenY;
      Boolean penDown;
      EvtGetPen(pScreenX ? &screenX : NULL, pScreenY ? &screenY : NULL, pPenDown ? &penDown : NULL);
      if (pScreenX) m68k_write_memory_16(pScreenX, screenX);
      if (pScreenY) m68k_write_memory_16(pScreenY, screenY);
      if (pPenDown) m68k_write_memory_8(pPenDown, penDown);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetPen(0x%08X, 0x%08X, 0x%08X)", pScreenX, pScreenY, pPenDown);
      }
      break;
    case sysTrapEvtSysEventAvail: {
      // Boolean EvtSysEventAvail(Boolean ignorePenUps)
      Boolean ignorePenUps = ARG8;
      Boolean res = EvtSysEventAvail(ignorePenUps);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtSysEventAvail(): %d", res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapEvtFlushKeyQueue:
      // Err EvtFlushKeyQueue(void)
      err = EvtFlushKeyQueue();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushKeyQueue(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      break;
    case sysTrapEvtFlushPenQueue:
      // Err EvtFlushPenQueue(void)
      err = EvtFlushPenQueue();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushPenQueue(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      break;
    case sysTrapEvtSetNullEventTick: {
      // Boolean EvtSetNullEventTick(UInt32 tick)
      UInt32 tick = ARG32;
      Boolean res = EvtSetNullEventTick(tick);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtSetNullEventTick(%u): %d", tick, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapEvtFlushNextPenStroke:
      // Err EvtFlushNextPenStroke(void)
      err = EvtFlushNextPenStroke();
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushNextPenStroke(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
      break;
    case sysTrapClipboardAddItem: {
      // void ClipboardAddItem(const ClipboardFormatType format, const void *ptr, UInt16 length)
      uint8_t format = ARG8;
      uint32_t ptrP = ARG32;
      uint16_t length = ARG16;
      void *ptr = emupalmos_trap_in(ptrP, trap, 1);
      ClipboardAddItem(format, ptr, length);
      debug(DEBUG_TRACE, "EmuPalmOS", "ClipboardAddItem(%d, 0x%08X, %d)", format, ptrP, length);
      }
      break;
    case sysTrapClipboardGetItem: {
      // MemHandle ClipboardGetItem(const ClipboardFormatType format, UInt16 *length)
      uint8_t format = ARG8;
      uint32_t lengthP = ARG32;
      emupalmos_trap_in(lengthP, trap, 1);
      UInt16 length;
      MemHandle h = ClipboardGetItem(format, &length);
      uint32_t r = emupalmos_trap_out(h);
      debug(DEBUG_TRACE, "EmuPalmOS", "ClipboardGetItem(%d, 0x%08X): 0x%08X", format, lengthP, r);
      m68k_set_reg(M68K_REG_A0, r);
      }
      break;
    case sysTrapExgInit:
    case sysTrapExgConnect:
    case sysTrapExgPut:
    case sysTrapExgGet:
    case sysTrapExgAccept:
    case sysTrapExgDisconnect:
    case sysTrapExgRegisterData:
    case sysTrapExgNotifyReceiveV35:
    case sysTrapExgDBRead:
    case sysTrapExgDBWrite:
    case sysTrapExgDoDialog:
    case sysTrapExgRegisterDatatype:
    case sysTrapExgNotifyReceive:
    case sysTrapExgNotifyGoto:
    case sysTrapExgRequest:
    case sysTrapExgSetDefaultApplication:
    case sysTrapExgGetDefaultApplication:
    case sysTrapExgGetTargetApplication:
    case sysTrapExgGetRegisteredApplications:
    case sysTrapExgGetRegisteredTypes:
    case sysTrapExgNotifyPreview:
    case sysTrapExgControl:
      m68k_set_reg(M68K_REG_D0, sysErrParamErr);
      break;
    case sysTrapExgSend:
    case sysTrapExgReceive:
      m68k_set_reg(M68K_REG_D0, 0);
      break;
    case sysTrapEvtGetSilkscreenAreaList:
    case sysTrapEvtGetPenBtnList: {
      // const SilkscreenAreaType *EvtGetSilkscreenAreaList(UInt16* numAreas)
      // const PenBtnInfoType *EvtGetPenBtnList(UInt16* numButtons)
      uint32_t numP = ARG32;
      emupalmos_trap_in(numP, trap, 0);
      if (numP) m68k_write_memory_16(numP, 0);
      m68k_set_reg(M68K_REG_A0, 0);
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
    case sysTrapUIPickColor: {
      // Boolean UIPickColor(IndexedColorType *indexP, RGBColorType *rgbP, UIPickColorStartType start, const Char *titleP, const Char *tipP)
      uint32_t indexP = ARG32;
      uint32_t rgbP = ARG32;
      uint16_t start = ARG16;
      uint32_t titleP = ARG32;
      uint32_t tipP = ARG32;
      emupalmos_trap_in(indexP, trap, 0);
      emupalmos_trap_in(rgbP, trap, 1);
      IndexedColorType index;
      RGBColorType rgb;
      if (indexP) index = m68k_read_memory_8(indexP);
      decode_rgb(rgbP, &rgb);
      char *title = (char *)emupalmos_trap_in(titleP, trap, 3);
      char *tip = (char *)emupalmos_trap_in(tipP, trap, 4);
      Boolean res = UIPickColor(indexP ? &index : NULL, rgbP ? &rgb : NULL, start, title, tip);
      if (indexP) m68k_write_memory_8(indexP, index);
      encode_rgb(rgbP, &rgb);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIPickColor(indexP=0x%08X, rgbP=0x%08X, start=%d, title=%s, tip=%s)", indexP, rgbP, start, title ? title : "", tip ? tip : "");
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapUIBrightnessAdjust:
      // void UIBrightnessAdjust(void)
      UIBrightnessAdjust();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIBrightnessAdjust()");
      break;
    case sysTrapUIContrastAdjust:
      // void UIContrastAdjust(void)
      UIContrastAdjust();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIContrastAdjust()");
      break;
    case sysTrapLocGetNumberSeparators: {
      // void LocGetNumberSeparators(NumberFormatType numberFormat, Char *thousandSeparator, Char *decimalSeparator)
      uint8_t numberFormat = ARG8;
      uint32_t thousandSeparatorP = ARG32;
      uint32_t decimalSeparatorP = ARG32;
      emupalmos_trap_in(thousandSeparatorP, trap, 0);
      emupalmos_trap_in(decimalSeparatorP, trap, 1);
      char thousandSeparator, decimalSeparator;
      LocGetNumberSeparators(numberFormat, &thousandSeparator, &decimalSeparator);
      if (thousandSeparatorP) m68k_write_memory_8(thousandSeparatorP, thousandSeparator);
      if (decimalSeparatorP) m68k_write_memory_8(decimalSeparatorP, decimalSeparator);
      debug(DEBUG_TRACE, "EmuPalmOS", "LocGetNumberSeparators(%d, %u, %u)", numberFormat, thousandSeparatorP, decimalSeparatorP);
      }
      break;
    case sysTrapSndPlaySmf: {
      // Err SndPlaySmf(void *chanP, SndSmfCmdEnum cmd, UInt8 *smfP, SndSmfOptionsType *selP, SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP, Boolean bNoWait)
      uint32_t chanP = ARG32;
      uint8_t cmd = ARG8;
      uint32_t smfP = ARG32;
      uint32_t selP = ARG32;
      uint32_t chanRangeP = ARG32;
      uint32_t callbacksP = ARG32;
      uint8_t bNoWait = ARG8;
      emupalmos_trap_in(chanP, trap, 0);
      emupalmos_trap_in(selP, trap, 3);
      emupalmos_trap_in(chanRangeP, trap, 4);
      emupalmos_trap_in(callbacksP, trap, 5);
      SndSmfOptionsType options;
      decode_smfoptions(selP, &options);
      Err res = SndPlaySmf(NULL, cmd, (UInt8 *)emupalmos_trap_in(smfP, trap, 2), selP ? &options : NULL, NULL, NULL, bNoWait);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmf(0x%08X, %d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", chanP, cmd, smfP, selP, chanRangeP, callbacksP, bNoWait, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapSndCreateMidiList: {
      // Boolean SndCreateMidiList(UInt32 creator, Boolean multipleDBs, UInt16 *wCountP, MemHandle *entHP)
      uint32_t creator = ARG32;
      uint8_t multipleDBs = ARG8;
      uint32_t wCountP = ARG32;
      uint32_t entHP = ARG32;
      emupalmos_trap_in(wCountP, trap, 2);
      emupalmos_trap_in(entHP, trap, 3);
      UInt16 wCount;
      MemHandle entH;
      Boolean res = SndCreateMidiList(creator, multipleDBs, wCountP ? &wCount : NULL, entHP ? &entH : NULL);
      if (wCountP) m68k_write_memory_16(wCountP, wCount);
      if (entHP) m68k_write_memory_32(entHP, emupalmos_trap_out(entH));
      pumpkin_id2s(creator, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndCreateMidiList('%s', %d, 0x%08X, 0x%08X): %d", buf, multipleDBs, wCountP, entHP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case sysTrapSndPlaySystemSound: {
      // void SndPlaySystemSound(SndSysBeepType beepID)
      uint8_t beepID = ARG8;
      SndPlaySystemSound(beepID);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndSysBeepType(%d)", beepID);
      }
      break;
    case sysTrapSndPlayResource: {
      // Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags)
      uint32_t sndP = ARG32;
      int32_t volume = ARG32;
      uint32_t flags = ARG32;
      void *sndPtr = (void *)emupalmos_trap_in(sndP, trap, 0);
      Err res = SndPlayResource(sndPtr, volume, flags);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndPlayResource(0x%08X, %d, 0x%08X): %d", sndP, volume, flags, res);
      m68k_set_reg(M68K_REG_D0, res);
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
    case sysTrapSndDoCmd: {
      // Err SndDoCmd(void *channelP, SndCommandPtr cmdP, Boolean noWait)
      uint32_t channelP = ARG32;
      uint32_t cmdP = ARG32;
      uint8_t noWait = ARG8;
      emupalmos_trap_in(channelP, trap, 0);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "SndDoCmd(0x%08X, 0x%08X, %d): %d", channelP, cmdP, noWait, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndGetDefaultVolume: {
      // void SndGetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *masterAmpP)
      uint32_t alarmAmpP = ARG32;
      uint32_t sysAmpP = ARG32;
      uint32_t masterAmpP = ARG32;
      emupalmos_trap_in(alarmAmpP, trap, 0);
      emupalmos_trap_in(sysAmpP, trap, 1);
      emupalmos_trap_in(masterAmpP, trap, 2);
      UInt16 alarmAmp, sysAmp, masterAmp;
      SndGetDefaultVolume(alarmAmpP ? &alarmAmp : NULL, sysAmpP ? &sysAmp : NULL, masterAmpP ? &masterAmp : NULL);
      if (alarmAmpP) m68k_write_memory_16(alarmAmpP, alarmAmp);
      if (sysAmpP) m68k_write_memory_16(sysAmpP, sysAmp);
      if (masterAmpP) m68k_write_memory_16(masterAmpP, masterAmp);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndGetDefaultVolume(0x%08X, 0x%08X, 0x%08X)", alarmAmpP, sysAmpP, masterAmpP);
      }
      break;
    case sysTrapSndSetDefaultVolume: {
      // void SndSetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *defAmpP)
      uint32_t alarmAmpP = ARG32;
      uint32_t sysAmpP = ARG32;
      uint32_t defAmpP = ARG32;
      emupalmos_trap_in(alarmAmpP, trap, 0);
      emupalmos_trap_in(sysAmpP, trap, 1);
      emupalmos_trap_in(defAmpP, trap, 2);
      UInt16 alarmAmp = alarmAmpP ? m68k_read_memory_16(alarmAmpP) : 0;
      UInt16 sysAmp = sysAmpP ? m68k_read_memory_16(sysAmpP) : 0;
      UInt16 defAmp = defAmpP ? m68k_read_memory_16(defAmpP) : 0;
      SndSetDefaultVolume(alarmAmpP ? &alarmAmp : NULL, sysAmpP ? &sysAmp : NULL, defAmpP ? &defAmp : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndSetDefaultVolume(0x%08X, 0x%08X, 0x%08X)", alarmAmpP, sysAmpP, defAmpP);
      }
      break;
    case sysTrapSndStreamCreate: {
      // Err SndStreamCreate( SndStreamRef *channel, SndStreamMode mode, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamBufferCallback func, void *userdata UInt32 buffsize, Boolean armNative)
      uint32_t channelP = ARG32;
      uint8_t mode = ARG8;
      uint32_t samplerate = ARG32;
      uint16_t type = ARG16;
      uint8_t width = ARG8;
      uint32_t funcP = ARG32;
      uint32_t userdataP = ARG32;
      uint32_t buffsize = ARG32;
      uint8_t armNative = ARG8;
      emupalmos_trap_in(channelP, trap, 0);
      emupalmos_trap_in(funcP, trap, 5);
      emupalmos_trap_in(userdataP, trap, 6);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreate(0x%08X, %d, %d, %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
        channelP, mode, samplerate, type, width, funcP, userdataP, buffsize, armNative, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamDelete: {
      // Err SndStreamDelete(SndStreamRef channel)
      uint32_t channelP = ARG32;
      emupalmos_trap_in(channelP, trap, 0);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamDelete(0x%08X): %d", channelP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamSetVolume: {
      // Err SndStreamSetVolume(SndStreamRef channel, Int32 volume)
      uint32_t channelP = ARG32;
      uint32_t volume = ARG32;
      emupalmos_trap_in(channelP, trap, 0);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamSetVolume(0x%08X, %d): %d", channelP, volume, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSndStreamStart: {
      // Err SndStreamStart(SndStreamRef channel)
      uint32_t channelP = ARG32;
      emupalmos_trap_in(channelP, trap, 0);
      err = errNone;
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStart(0x%08X): %d", channelP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapGrfGetState: {
      // Err GrfGetState(Boolean *capsLockP, Boolean *numLockP, UInt16 *tempShiftP, Boolean *autoShiftedP)
      uint32_t capsLockP = ARG32;
      uint32_t numLockP = ARG32;
      uint32_t tempShiftP = ARG32;
      uint32_t autoShiftedP = ARG32;
      emupalmos_trap_in(capsLockP, trap, 0);
      emupalmos_trap_in(numLockP, trap, 1);
      emupalmos_trap_in(tempShiftP, trap, 2);
      emupalmos_trap_in(autoShiftedP, trap, 3);
      Boolean capsLock, numLock, autoShifted;
      UInt16 tempShift;
      err = GrfGetState(&capsLock, &numLock, &tempShift, &autoShifted);
      debug(DEBUG_TRACE, "EmuPalmOS", "GrfGetState(%d, %d, %d, %d): %d", capsLock, numLock, tempShift, autoShifted, err);
      if (capsLockP) m68k_write_memory_8(capsLockP, capsLock);
      if (numLockP) m68k_write_memory_8(numLockP, numLock);
      if (tempShiftP) m68k_write_memory_16(tempShiftP, tempShift);
      if (autoShiftedP) m68k_write_memory_8(autoShiftedP, autoShifted);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapGrfSetState: {
      // Err GrfSetState(Boolean capsLock, Boolean numLock, Boolean upperShift)
      uint8_t capsLock = ARG8;
      uint8_t numLock = ARG8;
      uint8_t upperShift = ARG8;
      err = GrfSetState(capsLock, numLock, upperShift);
      debug(DEBUG_TRACE, "EmuPalmOS", "GrfSetState(%d, %d, %d): %d", capsLock, numLock, upperShift, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysTrapSysNotifyBroadcastDeferred: {
      // Err SysNotifyBroadcastDeferred(SysNotifyParamType *notify, Int16 paramSize)
      uint32_t notifyP = ARG32;
      int16_t paramSize = ARG16;
      emupalmos_trap_in(notifyP, trap, 0);
      SysNotifyParamType notify;
      decode_notify(notifyP, &notify, 0);
      err = SysNotifyBroadcastDeferred(notifyP ? &notify : NULL, paramSize);
      encode_notify(notifyP, &notify, 0);
      debug(DEBUG_TRACE, "EmuPalmOS", "SysNotifyBroadcastDeferred(0x%08X, %d): %d", notifyP, paramSize, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapCrc16CalcBlock: {
      // UInt16 Crc16CalcBlock(const void *bufP, UInt16 count, UInt16 crc)
      uint32_t bufP = ARG32;
      uint16_t count = ARG16;
      uint16_t crc = ARG16;
      void *buf = emupalmos_trap_in(bufP, trap, 0);
      UInt16 res = Crc16CalcBlock(buf, count, crc);
      debug(DEBUG_TRACE, "EmuPalmOS", "Crc16CalcBlock(0x%08X, %d, 0x%04X): 0x%04X", bufP, count, crc, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case sysTrapGsiSetShiftState: {
      // void GsiSetShiftState(const UInt16 lockFlags, const UInt16 tempShift)
      uint16_t lockFlags = ARG16;
      uint16_t tempShift = ARG16;
      GsiSetShiftState(lockFlags, tempShift);
      debug(DEBUG_TRACE, "EmuPalmOS", "GsiSetShiftState(0x%04X, 0x%04X)", lockFlags, tempShift);
    }
      break;
    case sysTrapGsiEnable: {
      // void GsiEnable(const Boolean enableIt)
      uint8_t enableIt = ARG8;
      GsiEnable(enableIt);
      debug(DEBUG_TRACE, "EmuPalmOS", "GsiEnable(%d)", enableIt);
    }
      break;
    case sysTrapGsiSetLocation: {
      // void GsiSetLocation(const Int16 x, const Int16 y)
      uint16_t x = ARG16;
      uint16_t y = ARG16;
      GsiSetLocation(x, y);
      debug(DEBUG_TRACE, "EmuPalmOS", "GsiSetLocation(%d, %d)", x, y);
    }
      break;
    case sysTrapEncDigestMD5: {
      // Err EncDigestMD5(UInt8 *strP, UInt16 strLen, UInt8 digestP[16])
      uint32_t strP = ARG32;
      uint16_t strLen = ARG16;
      uint32_t digestP = ARG32;
      UInt8 *str = emupalmos_trap_in(strP, trap, 0);
      UInt8 *digest = emupalmos_trap_in(digestP, trap, 2);
      Err res = EncDigestMD5(str, strLen, digest);
      debug(DEBUG_TRACE, "EmuPalmOS", "EncDigestMD5(0x%08X, %u, 0x%08X): %d", strP, strLen, digestP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case sysTrapGetCharCaselessValue: {
      // const UInt8 *GetCharCaselessValue(void)
      UInt8 *res = (UInt8 *)GetCharCaselessValue();
      uint32_t a = emupalmos_trap_out(res);
      debug(DEBUG_TRACE, "EmuPalmOS", "GetCharCaselessValue(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
      break;
    case sysTrapErrExceptionList: {
      // MemPtr *ErrExceptionList(void)
      uint8_t *e = (uint8_t *)ErrExceptionList();
      uint32_t a = emupalmos_trap_out(e);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrExceptionList(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
      break;
    case sysTrapErrThrow: {
      // void ErrThrow(Int32 err)
      uint32_t code = ARG32;
      uint8_t *e = (uint8_t *)ErrExceptionList();
      uint32_t a = emupalmos_trap_out(e);
      uint32_t exceptionP = m68k_read_memory_32(a);

      // typedef struct ErrExceptionType {
      //   struct ErrExceptionType *nextP;  // next exception type
      //   ErrJumpBuf state;                // setjmp/longjmp storage
      //   Int32 err;                       // Error code
      // } ErrExceptionType;
      uint32_t nextP = m68k_read_memory_32(exceptionP);
      m68k_write_memory_32(a, nextP);
      uint32_t bufP = exceptionP + 4;
      uint32_t aux = m68k_read_memory_32(bufP);
      m68k_set_reg(M68K_REG_D3, aux);
      aux = m68k_read_memory_32(bufP + 4);
      m68k_set_reg(M68K_REG_D4, aux);
      aux = m68k_read_memory_32(bufP + 8);
      m68k_set_reg(M68K_REG_D5, aux);
      aux = m68k_read_memory_32(bufP + 12);
      m68k_set_reg(M68K_REG_D6, aux);
      aux = m68k_read_memory_32(bufP + 16);
      m68k_set_reg(M68K_REG_D7, aux);
      aux = m68k_read_memory_32(bufP + 20);
      m68k_set_reg(M68K_REG_PC, aux);
      aux = m68k_read_memory_32(bufP + 24);
      m68k_set_reg(M68K_REG_A2, aux);
      aux = m68k_read_memory_32(bufP + 28);
      m68k_set_reg(M68K_REG_A3, aux);
      aux = m68k_read_memory_32(bufP + 32);
      m68k_set_reg(M68K_REG_A4, aux);
      aux = m68k_read_memory_32(bufP + 36);
      m68k_set_reg(M68K_REG_A5, aux);
      aux = m68k_read_memory_32(bufP + 40);
      m68k_set_reg(M68K_REG_A6, aux);
      aux = m68k_read_memory_32(bufP + 44);
      m68k_set_reg(M68K_REG_A7, aux);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrThrow(%d)", code);
      m68k_set_reg(M68K_REG_D0, code);
    }
      break;
    case sysTrapErrSetJump: {
      // Int16 ErrSetJump(ErrJumpBuf buf)
      uint32_t bufP = ARG32;
      emupalmos_trap_in(bufP, trap, 0);
      // typedef long *ErrJumpBuf[12];  // D3-D7,PC,A2-A7
      uint32_t aux = m68k_get_reg(NULL, M68K_REG_D3);
      m68k_write_memory_32(bufP, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D4);
      m68k_write_memory_32(bufP + 4, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D5);
      m68k_write_memory_32(bufP + 8, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D6);
      m68k_write_memory_32(bufP + 12, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D7);
      m68k_write_memory_32(bufP + 16, aux);
      aux = m68k_get_reg(NULL, M68K_REG_PC);
      m68k_write_memory_32(bufP + 20, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A2);
      m68k_write_memory_32(bufP + 24, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A3);
      m68k_write_memory_32(bufP + 28, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A4);
      m68k_write_memory_32(bufP + 32, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A5);
      m68k_write_memory_32(bufP + 36, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A6);
      m68k_write_memory_32(bufP + 40, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A7);
      m68k_write_memory_32(bufP + 44, aux);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrSetJump(0x%08X): %d", bufP, 0);
      m68k_set_reg(M68K_REG_D0, 0); // XXX not calling ErrSetJump()
    }
      break;
    case sysTrapErrLongJump: {
      // void ErrLongJump(ErrJumpBuf buf, Int16 result)
      uint32_t bufP = ARG32;
      int16_t result = ARG16;
      emupalmos_trap_in(bufP, trap, 0);
      uint32_t aux = m68k_read_memory_32(bufP);
      m68k_set_reg(M68K_REG_D3, aux);
      aux = m68k_read_memory_32(bufP + 4);
      m68k_set_reg(M68K_REG_D4, aux);
      aux = m68k_read_memory_32(bufP + 8);
      m68k_set_reg(M68K_REG_D5, aux);
      aux = m68k_read_memory_32(bufP + 12);
      m68k_set_reg(M68K_REG_D6, aux);
      aux = m68k_read_memory_32(bufP + 16);
      m68k_set_reg(M68K_REG_D7, aux);
      aux = m68k_read_memory_32(bufP + 20);
      m68k_set_reg(M68K_REG_PC, aux);
      aux = m68k_read_memory_32(bufP + 24);
      m68k_set_reg(M68K_REG_A2, aux);
      aux = m68k_read_memory_32(bufP + 28);
      m68k_set_reg(M68K_REG_A3, aux);
      aux = m68k_read_memory_32(bufP + 32);
      m68k_set_reg(M68K_REG_A4, aux);
      aux = m68k_read_memory_32(bufP + 36);
      m68k_set_reg(M68K_REG_A5, aux);
      aux = m68k_read_memory_32(bufP + 40);
      m68k_set_reg(M68K_REG_A6, aux);
      aux = m68k_read_memory_32(bufP + 44);
      m68k_set_reg(M68K_REG_A7, aux);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrLongJump(0x%08X, %d)", bufP, result);
      m68k_set_reg(M68K_REG_D0, result);
    }
      break;
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
        sys_snprintf(buf, sizeof(buf)-1, "trap %s not mapped", trapName(trap, &selector, 0));
        emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      }
      break;
  }

  debug(DEBUG_TRACE, "EmuPalmOS", "trap 0x%04X end (int)", trap);
  pumpkin_trace(trap);

  return r;
}
