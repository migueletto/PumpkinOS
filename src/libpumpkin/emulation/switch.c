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
case sysTrapPceNativeCall: {
  // UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
#ifdef ARMEMU
  uint32_t nativeFuncP = ARG32;
  uint32_t userDataP = ARG32;
  emupalmos_trap_in(nativeFuncP, trap, 0);
  emupalmos_trap_in(userDataP, trap, 1);
  UInt32 res = arm_native_call_pce(nativeFuncP, userDataP);
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
case sysTrapAttnIterate: {
  // void AttnIterate(UInt16 cardNo, LocalID dbID, UInt32 iterationData)
  uint16_t cardNo = ARG16;
  uint32_t dbID = ARG32;
  uint32_t iterationData = ARG32;
  AttnIterate(cardNo, dbID, iterationData);
  debug(DEBUG_TRACE, "EmuPalmOS", "AttnIterate(%d, 0x%08X, %u)", cardNo, dbID, iterationData);
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
  Err err = DlkGetSyncInfo(&succSyncDate, &lastSyncDate, &syncState, nameBuf, logBuf, &logLen);
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
  Err err;
  if (p) {
    uint32_t a = emupalmos_trap_out(p);
    if (newPtrP) m68k_write_memory_32(newPtrP, a);
    err = FtrSet(creator, featureNum, a);
  } else {
    err = memErrNotEnoughSpace;
  }
  char screator[8];
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrNew('%s', %d, %d, 0x%08X): %d", screator, featureNum, size, newPtrP, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFtrPtrFree: {
  // Err FtrPtrFree(UInt32 creator, UInt16 featureNum)
  uint32_t creator = ARG32;
  uint16_t featureNum = ARG16;
  uint32_t a;
  Err err = FtrGet(creator, featureNum, &a);
  if (err == errNone && a) {
    uint8_t *p = emupalmos_trap_in(a, trap, -1);
    MemPtrFree(p);
  }
  char screator[8];
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrPtrFree('%s', %d): %d", screator, featureNum, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFtrUnregister: {
  // Err FtrUnregister(UInt32 creator, UInt16 featureNum)
  uint32_t creator = ARG32;
  uint16_t featureNum = ARG16;
  Err err = FtrUnregister(creator, featureNum);
  char screator[8];
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrUnregister('%s', %d): %d", screator, featureNum, err);
}
break;
case sysTrapFtrGet: {
  // Err FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP)
  uint32_t creator = ARG32;
  uint16_t featureNum = ARG16;
  uint32_t valueP = ARG32;
  emupalmos_trap_in(valueP, trap, 2);
  uint32_t value;
  char screator[8];
  pumpkin_id2s(creator, screator);
  Err err = FtrGet(creator, featureNum, &value);

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

  debug(DEBUG_TRACE, "EmuPalmOS", "FtrGet('%s', %d, 0x%08X [0x%08X]): %d", screator, featureNum, valueP, value, err);
  m68k_write_memory_32(valueP, value);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapFtrSet: {
  // Err FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue)
  uint32_t creator = ARG32;
  uint16_t featureNum = ARG16;
  uint32_t newValue = ARG32;
  char screator[8];
  pumpkin_id2s(creator, screator);
  Err err = FtrSet(creator, featureNum, newValue);
  debug(DEBUG_TRACE, "EmuPalmOS", "FtrSet('%s', %d, %d): %d", screator, featureNum, newValue, err);
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
case sysTrapTimSetSeconds: {
  // void TimSetSeconds(UInt32 seconds)
  uint32_t seconds = ARG32;
  TimSetSeconds(seconds);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimSetSeconds(%u)", seconds);
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
case sysTrapTimeZoneToAscii: {
  // void TimeZoneToAscii(Int16 timeZone, const LmLocaleType *localeP, Char *string)
  int16_t timeZone = ARG16;
  uint32_t localeP = ARG32;
  uint32_t stringP = ARG32;
  emupalmos_trap_in(localeP, trap, 1);
  char *string = (char *)emupalmos_trap_in(stringP, trap, 2);
  LmLocaleType locale;
  decode_locale(localeP, &locale);
  TimeZoneToAscii(timeZone, localeP ? &locale : NULL, string);
  debug(DEBUG_TRACE, "EmuPalmOS", "TimeZoneToAscii(%d, 0x%08X, 0x%08X )", timeZone, localeP, stringP);
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
case sysTrapFplInit:
  // Err FplInit(void)
  debug(DEBUG_TRACE, "EmuPalmOS", "FplInit()");
  m68k_set_reg(M68K_REG_D0, 0);
break;
case sysTrapFplFree:
  // void FplFree(void)
  debug(DEBUG_TRACE, "EmuPalmOS", "FplFree()");
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
  if (enableColorP) enableColor = m68k_read_memory_8(enableColorP);
  Err err = WinScreenMode(operation, widthP ? &width : NULL, heightP ? &height : NULL, depthP ? &depth : NULL, enableColorP ? &enableColor : NULL);
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
    if (startIndex == WinUseTableIndexes) {
      for (i = 0; i < paletteEntries && i < 256; i++) {
        uint32_t index = m68k_read_memory_8(tableP + i*4);
        decode_rgb(tableP + i*4, &table[i]);
        debug(DEBUG_TRACE, "EmuPalmOS", "palette %d: %u,%u,%u (i=%d)", index, table[i].r, table[i].g, table[i].b, i);
      }
    } else {
      for (i = 0; i < paletteEntries && i < 256; i++) {
        if (startIndex+i >= 0 && startIndex+i < 256) {
          decode_rgb(tableP + i*4, &table[i]);
          debug(DEBUG_TRACE, "EmuPalmOS", "palette %d: %u,%u,%u (start=%d, i=%d)", startIndex+i, table[i].r, table[i].g, table[i].b, startIndex, i);
        }
      }
    }
  }
  Err err = WinPalette(operation, startIndex, paletteEntries, tableP ? table : NULL);
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
  Err err = FntDefineFont(font, fontp);
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
  UInt16 error = 0;
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
  UInt16 error = 0;
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
  UInt16 error = 0;
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
  Err err = BmpDelete(bitmap);
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
  Err err = UIColorPushTable();
  debug(DEBUG_TRACE, "EmuPalmOS", "UIColorPushTable(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapUIColorPopTable: {
  // Err UIColorPopTable(void)
  Err err = UIColorPopTable();
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
  Err err = UIColorSetTableEntry(which, rgbP ? &rgb : NULL);
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
case sysTrapPrefSetPreferences: {
  // void PrefSetPreferences(SystemPreferencesPtr p)
  uint32_t p = ARG32;
  emupalmos_trap_in(p, trap, 0);
  SystemPreferencesType prefs;
  // XXX encode p into prefs
  PrefSetPreferences(p ? &prefs : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetPreferences(0x%08X)", p);
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
  char screator[8];
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferences('%s', %d, %d, 0x%08X, %d, %d)", screator, id, version, prefsP, prefsSize, saved);
}
break;
case sysTrapPrefSetAppPreferencesV10: {
  // void PrefSetAppPreferencesV10(UInt32 creator, Int16 version, void *prefs, UInt16 prefsSize)
  uint32_t creator = ARG32;
  int16_t version = ARG16;
  uint32_t prefsP = ARG32;
  uint16_t prefsSize = ARG16;
  PrefSetAppPreferencesV10(creator, version, emupalmos_trap_in(prefsP, trap, 2), prefsSize);
  char screator[8];
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefSetAppPreferencesV10('%s', %d, 0x%08X, %d)", screator, version, prefsP, prefsSize);
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
  char screator[8];
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferences('%s', %d, 0x%08X, 0x%08X, %d): %d", screator, id, prefsP, prefsSizeP, saved, version);
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
  char screator[8];
  pumpkin_id2s(type, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrefGetAppPreferencesV10('%s', %d, 0x%08X, %d): %d", screator, version, prefsP, prefsSize, b);
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
  Err err;
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
  Err err;
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
  char stype[8];
  pumpkin_id2s(type, stype);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSearchResource('%s', %d, 0x%08X, 0x%08X): %d", stype, resID, ih, dbPP, index);
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
  Err err = DmGetNextDatabaseByTypeCreator(newSearch, stateInfoP ? &stateInfo : NULL, type, creator, onlyLatestVers, cardNoP ? &cardNo : NULL, dbIDP ? &dbID : NULL);
  if (stateInfoP) {
    uint32_t info = emupalmos_trap_out(stateInfo.p);
    m68k_write_memory_32(stateInfoP, info);
  }
  if (cardNoP) m68k_write_memory_16(cardNoP, cardNo);
  if (dbIDP) m68k_write_memory_32(dbIDP, dbID);
  char stype[8];
  char screator[8];
  pumpkin_id2s(type, stype);
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetNextDatabaseByTypeCreator(%d, 0x%08X, '%s', '%s', %d, 0x%08X, 0x%08X): %d", newSearch, stateInfoP, stype, screator, onlyLatestVers, cardNoP, dbIDP, err);
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
case sysTrapDmFindSortPositionV10: {
  // UInt16 DmFindSortPositionV10(DmOpenRef dbP, void *newRecord, DmComparF *compar, Int16 other)
  uint32_t dbP = ARG32;
  uint32_t newRecordP = ARG32;
  uint32_t comparP = ARG32;
  int16_t other = ARG16;
  DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  emupalmos_trap_in(newRecordP, trap, 1);
  emupalmos_trap_in(comparP, trap, 2);
  UInt16 res = DmFindSortPosition68K(dbRef, newRecordP, 0, comparP, other);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindSortPositionV10(0x%08X, 0x%08X, 0x%08X, %d): %d", dbP, newRecordP, comparP, other, res);
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
case sysTrapDmSync:
  // void DmSync(void)
  DmSync();
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSync()");
break;
case sysTrapDmSyncDatabase: {
  // Err DmSyncDatabase(DmOpenRef dbRef)
  uint32_t dbP = ARG32;
  DmOpenRef dbRef = (DmOpenRef)emupalmos_trap_in(dbP, trap, 0);
  Err res = DmSyncDatabase(dbRef);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSyncDatabase(0x%08X): %d", dbP, res);
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
  int vararg = trap == sysTrapStrVPrintF;
  Int16 res = 0;
  if (s && f) {
    int i, j = 0, k = 1, t = 0, sz = 0, arglen = 0;
    uint32_t arg, v_arg;
    char *p, *q, fmt[16];
    if (vararg) {
      v_arg = ARG32;
    } else {
      v_arg = 0;
    }
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
            case 'X':
              if (vararg) {
                switch (sz) {
                  case 1:  arg = m68k_read_memory_16(v_arg) & 0xff; v_arg += 2; break;
                  case 2:  arg = m68k_read_memory_16(v_arg); v_arg += 2; break;
                  case 4:  arg = m68k_read_memory_32(v_arg); v_arg += 4; break;
                  default: arg = m68k_read_memory_16(v_arg); v_arg += 2; break;
                }
              } else {
                switch (sz) {
                  case 1:  arg = ARG8;  break;
                  case 2:  arg = ARG16; break;
                  case 4:  arg = ARG32; break;
                  default: arg = ARG16; break;
                }
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
              if (vararg) {
                arg = m68k_read_memory_16(v_arg) & 0xff;
                v_arg += 2;
              } else {
                arg = ARG8;
              }
              k++;
              fmt[j++] = f[i];
              fmt[j] = 0;
              sys_sprintf(p, fmt, arg);
              p += sys_strlen(p);
              t = 0;
              break;
            case 's':
              if (vararg) {
                arg = m68k_read_memory_32(v_arg);
                v_arg += 4;
              } else {
                arg = ARG32;
              }
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
              if (vararg) {
                arglen = m68k_read_memory_16(v_arg);
                v_arg += 2;
              } else {
                arglen = ARG16;
              }
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
case sysTrapFrmGetEventHandler68K: {
  // FormEventHandlerType *FrmGetEventHandler68K(FormType *formP)
  // custom trap created for use in 68K code
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
case sysTrapCtlGetStyle68K: {
  // ControlStyleType CtlGetStyle(ControlType *controlP)
  // custom trap created for use in 68K code
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
  Err err = EvtResetAutoOffTimer();
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
  char *eventName = EvtGetEventName(event.eType);
  if (eventName) {
    debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [%s])", eventP, eventName);
  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "EvtAddEventToQueue(0x%08X [0x%04X])", eventP, event.eType);
  }
  if (eventP) encode_event(eventP, &event);
}
break;
case sysTrapEvtEnqueueKey: {
  // Err EvtEnqueueKey(WChar ascii, UInt16 keycode, UInt16 modifiers)
  uint16_t ascii = ARG16;
  uint16_t keycode = ARG16;
  uint16_t modifiers = ARG16;
  Err err = EvtEnqueueKey(ascii, keycode, modifiers);
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
  Err err = EvtWakeup();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtWakeup(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapEvtGetEvent: {
  // void EvtGetEvent(EventType *event, Int32 timeout)
  uint32_t eventP = ARG32;
  int32_t timeout = ARG32;
  timeout = pumpkin_event_timeout(timeout);
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
case sysTrapPenResetCalibration: {
  // Err PenResetCalibration(void)
  Err err = PenResetCalibration();
  debug(DEBUG_TRACE, "EmuPalmOS", "PenResetCalibration(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapPenCalibrate: {
  // Err PenCalibrate(PointType *digTopLeftP, PointType *digBotRightP, PointType *scrTopLeftP, PointType *scrBotRightP)
  Err err = PenCalibrate(NULL, NULL, NULL, NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "PenCalibrate %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapPenSleep: {
  // Err PenSleep(void)
  Err err = PenSleep();
  debug(DEBUG_TRACE, "EmuPalmOS", "PenSleep(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapPenWake: {
  // Err PenWake(void)
  Err err = PenWake();
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
case sysTrapEvtFlushKeyQueue: {
  // Err EvtFlushKeyQueue(void)
  Err err = EvtFlushKeyQueue();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushKeyQueue(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapEvtFlushPenQueue: {
  // Err EvtFlushPenQueue(void)
  Err err = EvtFlushPenQueue();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushPenQueue(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapEvtSetNullEventTick: {
  // Boolean EvtSetNullEventTick(UInt32 tick)
  UInt32 tick = ARG32;
  Boolean res = EvtSetNullEventTick(tick);
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtSetNullEventTick(%u): %d", tick, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapEvtFlushNextPenStroke: {
  // Err EvtFlushNextPenStroke(void)
  Err err = EvtFlushNextPenStroke();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtFlushNextPenStroke(): %d", err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapEvtKeyQueueEmpty: {
  // Boolean EvtKeyQueueEmpty(void)
  Boolean res = EvtKeyQueueEmpty();
  debug(DEBUG_TRACE, "EmuPalmOS", "EvtKeyQueueEmpty(): %d", res);
  m68k_set_reg(M68K_REG_D0, res);
}
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
case sysTrapSndPlaySmfResource: {
  //Err SndPlaySmfResource(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector)
  uint32_t resType = ARG32;
  int16_t resID = ARG32;
  uint8_t volumeSelector = ARG8;
  Err res = SndPlaySmfResource(resType, resID, volumeSelector);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmfResource(0x%08X, %d, %d): %d", resType, resID, volumeSelector, res);
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
  char screator[8];
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndCreateMidiList('%s', %d, 0x%08X, 0x%08X): %d", screator, multipleDBs, wCountP, entHP, res);
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
  SndCommandType cmd;
  decode_sndcmd(cmdP, &cmd);
  Err err = SndDoCmd(NULL, cmdP ? &cmd : NULL, noWait);
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
  // Err SndStreamCreate(SndStreamRef *channel, SndStreamMode mode, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamBufferCallback func, void *userdata, UInt32 buffsize, Boolean armNative)
  uint32_t channelP = ARG32;
  uint8_t mode = ARG8;
  uint32_t samplerate = ARG32;
  uint16_t type = ARG16;
  uint8_t width = ARG8;
  uint32_t funcP = ARG32;
  uint32_t userdataP = ARG32;
  uint32_t buffsize = ARG32;
  uint8_t armNative = ARG8;
  SndStreamRef *channel = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
  SndStreamBufferCallback func = (SndStreamBufferCallback)emupalmos_trap_in(funcP, trap, 5);
  void *userdata = emupalmos_trap_in(userdataP, trap, 6);
  Err err = SndStreamCreate(channel, mode, samplerate, type, width, func, userdata, buffsize, armNative);
  if (channelP) m68k_write_memory_32(channelP, *channel);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreate(0x%08X, %d, %d, %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
    channelP, mode, samplerate, type, width, funcP, userdataP, buffsize, armNative, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamCreateExtended: {
  // Err SndStreamCreateExtended(SndStreamRef *channel, SndStreamMode mode, SndFormatType format, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamVariableBufferCallback func, void *userdata, UInt32 buffsize, Boolean armNative)
  uint32_t channelP = ARG32;
  uint8_t mode = ARG8;
  uint32_t format = ARG32;
  uint32_t samplerate = ARG32;
  uint16_t type = ARG16;
  uint8_t width = ARG8;
  uint32_t funcP = ARG32;
  uint32_t userdataP = ARG32;
  uint32_t buffsize = ARG32;
  uint8_t armNative = ARG8;
  SndStreamRef *channel = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
  SndStreamVariableBufferCallback func = (SndStreamVariableBufferCallback)emupalmos_trap_in(funcP, trap, 6);
  void *userdata = emupalmos_trap_in(userdataP, trap, 7);
  Err err = SndStreamCreateExtended(channel, mode, format, samplerate, type, width, func, userdata, buffsize, armNative);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreateExtented(0x%08X, %d, %d, %d %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
    channelP, mode, format, samplerate, type, width, funcP, userdataP, buffsize, armNative, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamDelete: {
  // Err SndStreamDelete(SndStreamRef channel)
  uint32_t channel = ARG32;
  Err err = SndStreamDelete(channel);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamDelete(0x%08X): %d", channel, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamSetVolume: {
  // Err SndStreamSetVolume(SndStreamRef channel, Int32 volume)
  uint32_t channel = ARG32;
  uint32_t volume = ARG32;
  Err err = SndStreamSetVolume(channel, volume);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamSetVolume(0x%08X, %d): %d", channel, volume, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamStart: {
  // Err SndStreamStart(SndStreamRef channel)
  uint32_t channel = ARG32;
  Err err = SndStreamStart(channel);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStart(0x%08X): %d", channel, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapSndStreamStop: {
  // Err SndStreamStop(SndStreamRef channel)
  uint32_t channel = ARG32;
  Err err = SndStreamStop(channel);
  debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStop(0x%08X): %d", channel, err);
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
  Err err = GrfGetState(&capsLock, &numLock, &tempShift, &autoShifted);
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
  Err err = GrfSetState(capsLock, numLock, upperShift);
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
  decode_notify(notifyP, &notify);
  Err err = SysNotifyBroadcastDeferred(notifyP ? &notify : NULL, paramSize);
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
case sysTrapGsiInitialize: {
  // void GsiInitialize(void)
  GsiInitialize();
  debug(DEBUG_TRACE, "EmuPalmOS", "GsiInitialize()");
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
case sysTrapPrgStartDialogV31: {
  // ProgressPtr PrgStartDialogV31(const Char *title, PrgCallbackFunc textCallback)
  uint32_t titleP = ARG32;
  uint32_t textCallbackP = ARG32;
  char *title = emupalmos_trap_in(titleP, trap, 0);
  void *textCallback = emupalmos_trap_in(textCallbackP, trap, 1);
  ProgressPtr prg = PrgStartDialogV31(title, textCallback);
  uint32_t prgP = emupalmos_trap_out(prg);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgStartDialogV31(0x%08X [%s], 0x%08X): 0x%08X", titleP, title ? title : "", textCallbackP, prgP);
  m68k_set_reg(M68K_REG_A0, prgP);
}
break;
case sysTrapPrgStartDialog: {
  // ProgressPtr PrgStartDialog(const Char *title, PrgCallbackFunc textCallback, void *userDataP)
  uint32_t titleP = ARG32;
  uint32_t textCallbackP = ARG32;
  uint32_t userDataP = ARG32;
  char *title = emupalmos_trap_in(titleP, trap, 0);
  void *textCallback = emupalmos_trap_in(textCallbackP, trap, 1);
  void *userData = emupalmos_trap_in(userDataP, trap, 2);
  ProgressPtr prg = PrgStartDialog(title, textCallback, userData);
  uint32_t prgP = emupalmos_trap_out(prg);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgStartDialog(0x%08X [%s], 0x%08X, 0x%08X): 0x%08X", titleP, title ? title : "", textCallbackP, userDataP, prgP);
  m68k_set_reg(M68K_REG_A0, prgP);
}
break;
case sysTrapPrgStopDialog: {
  // void PrgStopDialog(ProgressPtr prgP, Boolean force)
  uint32_t prgP = ARG32;
  uint8_t force = ARG8;
  void *prg = emupalmos_trap_in(prgP, trap, 0);
  PrgStopDialog(prg, force);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgStopDialog(0x%08X)", prgP);
}
break;
case sysTrapPrgHandleEvent: {
  // Boolean PrgHandleEvent(ProgressPtr prgGP, EventType *eventP)
  uint32_t prgP = ARG32;
  uint32_t eventP = ARG32;
  void *prg = emupalmos_trap_in(prgP, trap, 0);
  emupalmos_trap_in(eventP, trap, 1);
  EventType event;
  if (eventP) decode_event(eventP, &event);
  Boolean res = PrgHandleEvent(prg, &event);
  char *eventName = EvtGetEventName(event.eType);
  if (eventName) {
    debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [%s]): %d", prgP, eventP, eventName, res);
  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "PrgHandleEvent(0x%08X, 0x%08X [0x%04X]): %d", prgP, eventP, event.eType, res);
  }
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapPrgUpdateDialog: {
  // void PrgUpdateDialog(ProgressPtr prgGP, UInt16 err, UInt16 stage, const Char *messageP, Boolean updateNow)
  uint32_t prgP = ARG32;
  uint16_t err = ARG16;
  uint16_t stage = ARG16;
  uint32_t messageP = ARG32;
  uint8_t updateNow = ARG8;
  void *prg = emupalmos_trap_in(prgP, trap, 0);
  char *message = emupalmos_trap_in(messageP, trap, 3);
  PrgUpdateDialog(prg, err, stage, message, updateNow);
  debug(DEBUG_TRACE, "EmuPalmOS", "PrgUpdateDialog(0x%08X, %d, %d, 0x%08X [%s], %d)", prgP, err, stage, messageP, messageP ? message : "", updateNow);
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
case sysTrapGetCharSortValue: {
  // const UInt8 *GetCharSortValue(void) 
  UInt8 *res = (UInt8 *)GetCharSortValue();
  uint32_t a = emupalmos_trap_out(res);
  debug(DEBUG_TRACE, "EmuPalmOS", "GetCharSortValue(): 0x%08X", a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapAlmSetAlarm: {
  // Err AlmSetAlarm(UInt16 cardNo, LocalID dbID, UInt32 ref, UInt32 alarmSeconds, Boolean quiet)
  uint16_t cardNo = ARG16;
  uint32_t dbID = ARG32;
  uint32_t ref = ARG32;
  uint32_t alarmSeconds = ARG32;
  uint8_t quiet = ARG8;
  Err err = AlmSetAlarm(cardNo, dbID, ref, alarmSeconds, quiet);
  debug(DEBUG_TRACE, "EmuPalmOS", "AlmSetAlarm(%d, 0x%08X, %u, %u, %u): %d", cardNo, dbID, ref, alarmSeconds, quiet, err);
  m68k_set_reg(M68K_REG_D0, err);
}
break;
case sysTrapAlmGetAlarm: {
  // UInt32 AlmGetAlarm(UInt16 cardNo, LocalID dbID, UInt32 *refP)
  uint16_t cardNo = ARG16;
  uint32_t dbID = ARG32;
  uint32_t refP = ARG32;
  emupalmos_trap_in(refP, trap, 2);
  UInt32 ref;
  UInt32 res = AlmGetAlarm(cardNo, dbID, refP ? &ref : NULL);
  if (refP) m68k_write_memory_32(refP, ref);
  debug(DEBUG_TRACE, "EmuPalmOS", "AlmGetAlarm(%d, 0x%08X, 0x%08X): %u", cardNo, dbID, refP, res);
  m68k_set_reg(M68K_REG_D0, res);
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
case sysTrapErrAlertCustom: {
  // Int16 ErrAlertCustom(Err errCode, Char *errMsgP, Char *preMsgP, Char *postMsgP)
  uint16_t errCode = ARG16;
  uint32_t errMsgP = ARG32;
  uint32_t preMsgP = ARG32;
  uint32_t postMsgP = ARG32;
  char *errMsg = emupalmos_trap_in(errMsgP, trap, 1);
  char *preMsg = emupalmos_trap_in(preMsgP, trap, 2);
  char *postMsg = emupalmos_trap_in(postMsgP, trap, 3);
  Int16 res = ErrAlertCustom(errCode, errMsg, preMsg, postMsg);
  debug(DEBUG_TRACE, "EmuPalmOS", "ErrAlertCustom(%u, 0x%08X, 0x%08X, 0x%08X): %d", errCode, errMsgP, preMsgP, postMsgP, res);
  m68k_set_reg(M68K_REG_D0, res);
}
break;
case sysTrapMemInit: {
  // Err MemInit(void)
  Err res = MemInit();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemInit(): %d", res);
}
break;
case sysTrapMemKernelInit: {
  // Err MemKernelInit(void)
  Err res = MemKernelInit();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemKernelInit(): %d", res);
}
break;
case sysTrapMemInitHeapTable: {
  // Err MemInitHeapTable(UInt16 cardNo)
  uint16_t cardNo = ARG16;
  Err res = MemInitHeapTable(cardNo);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemInitHeapTable(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapMemNumCards: {
  // UInt16 MemNumCards(void)
  UInt16 res = MemNumCards();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemNumCards(): %d", res);
}
break;
case sysTrapMemCardInfo: {
  // Err MemCardInfo(UInt16 cardNo, out Char *cardNameP, out Char *manufNameP, out UInt16 *versionP, out UInt32 *crDateP, out UInt32 *romSizeP, out UInt32 *ramSizeP, out UInt32 *freeBytesP)
  uint16_t cardNo = ARG16;
  uint32_t cardNameP = ARG32;
  char *s_cardNameP = emupalmos_trap_in(cardNameP, trap, 1);
  uint32_t manufNameP = ARG32;
  char *s_manufNameP = emupalmos_trap_in(manufNameP, trap, 2);
  uint32_t versionP = ARG32;
  UInt16 l_versionP = 0;
  uint32_t crDateP = ARG32;
  UInt32 l_crDateP = 0;
  uint32_t romSizeP = ARG32;
  UInt32 l_romSizeP = 0;
  uint32_t ramSizeP = ARG32;
  UInt32 l_ramSizeP = 0;
  uint32_t freeBytesP = ARG32;
  UInt32 l_freeBytesP = 0;
  Err res = MemCardInfo(cardNo, cardNameP ? s_cardNameP : NULL, manufNameP ? s_manufNameP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, romSizeP ? &l_romSizeP : NULL, ramSizeP ? &l_ramSizeP : NULL, freeBytesP ? &l_freeBytesP : NULL);
  if (versionP) m68k_write_memory_16(versionP, l_versionP);
  if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
  if (romSizeP) m68k_write_memory_32(romSizeP, l_romSizeP);
  if (ramSizeP) m68k_write_memory_32(ramSizeP, l_ramSizeP);
  if (freeBytesP) m68k_write_memory_32(freeBytesP, l_freeBytesP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemCardInfo(cardNo=%d, cardNameP=0x%08X [%s], manufNameP=0x%08X [%s], versionP=0x%08X [%d], crDateP=0x%08X [%d], romSizeP=0x%08X [%d], ramSizeP=0x%08X [%d], freeBytesP=0x%08X [%d]): %d", cardNo, cardNameP, s_cardNameP, manufNameP, s_manufNameP, versionP, l_versionP, crDateP, l_crDateP, romSizeP, l_romSizeP, ramSizeP, l_ramSizeP, freeBytesP, l_freeBytesP, res);
}
break;
case sysTrapMemNumHeaps: {
  // UInt16 MemNumHeaps(UInt16 cardNo)
  uint16_t cardNo = ARG16;
  UInt16 res = MemNumHeaps(cardNo);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemNumHeaps(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapMemNumRAMHeaps: {
  // UInt16 MemNumRAMHeaps(UInt16 cardNo)
  uint16_t cardNo = ARG16;
  UInt16 res = MemNumRAMHeaps(cardNo);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemNumRAMHeaps(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapMemHeapID: {
  // UInt16 MemHeapID(UInt16 cardNo, UInt16 heapIndex)
  uint16_t cardNo = ARG16;
  uint16_t heapIndex = ARG16;
  UInt16 res = MemHeapID(cardNo, heapIndex);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapID(cardNo=%d, heapIndex=%d): %d", cardNo, heapIndex, res);
}
break;
case sysTrapMemHeapDynamic: {
  // Boolean MemHeapDynamic(UInt16 heapID)
  uint16_t heapID = ARG16;
  Boolean res = MemHeapDynamic(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapDynamic(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapFreeBytes: {
  // Err MemHeapFreeBytes(UInt16 heapID, out UInt32 *freeP, out UInt32 *maxP)
  uint16_t heapID = ARG16;
  uint32_t freeP = ARG32;
  UInt32 l_freeP = 0;
  uint32_t maxP = ARG32;
  UInt32 l_maxP = 0;
  Err res = MemHeapFreeBytes(heapID, freeP ? &l_freeP : NULL, maxP ? &l_maxP : NULL);
  if (freeP) m68k_write_memory_32(freeP, l_freeP);
  if (maxP) m68k_write_memory_32(maxP, l_maxP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapFreeBytes(heapID=%d, freeP=0x%08X [%d], maxP=0x%08X [%d]): %d", heapID, freeP, l_freeP, maxP, l_maxP, res);
}
break;
case sysTrapMemHeapSize: {
  // UInt32 MemHeapSize(UInt16 heapID)
  uint16_t heapID = ARG16;
  UInt32 res = MemHeapSize(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapSize(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapFlags: {
  // UInt16 MemHeapFlags(UInt16 heapID)
  uint16_t heapID = ARG16;
  UInt16 res = MemHeapFlags(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapFlags(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapCompact: {
  // Err MemHeapCompact(UInt16 heapID)
  uint16_t heapID = ARG16;
  Err res = MemHeapCompact(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapCompact(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapInit: {
  // Err MemHeapInit(UInt16 heapID, Int16 numHandles, Boolean initContents)
  uint16_t heapID = ARG16;
  int16_t numHandles = ARG16;
  uint8_t initContents = ARG8;
  Err res = MemHeapInit(heapID, numHandles, initContents);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapInit(heapID=%d, numHandles=%d, initContents=%d): %d", heapID, numHandles, initContents, res);
}
break;
case sysTrapMemHeapFreeByOwnerID: {
  // Err MemHeapFreeByOwnerID(UInt16 heapID, UInt16 ownerID)
  uint16_t heapID = ARG16;
  uint16_t ownerID = ARG16;
  Err res = MemHeapFreeByOwnerID(heapID, ownerID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapFreeByOwnerID(heapID=%d, ownerID=%d): %d", heapID, ownerID, res);
}
break;
case sysTrapMemChunkNew: {
  // MemPtr MemChunkNew(UInt16 heapID, UInt32 size, UInt16 attr)
  uint16_t heapID = ARG16;
  uint32_t size = ARG32;
  uint16_t attr = ARG16;
  MemPtr res = MemChunkNew(heapID, size, attr);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemChunkNew(heapID=%d, size=%d, attr=%d): 0x%08X", heapID, size, attr, r_res);
}
break;
case sysTrapMemChunkFree: {
  // Err MemChunkFree(MemPtr chunkDataP)
  uint32_t chunkDataP = ARG32;
  char *l_chunkDataP = emupalmos_trap_in(chunkDataP, trap, 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemChunkFree(chunkDataP=0x%08X) ...", chunkDataP);
  Err res = MemChunkFree(chunkDataP ? l_chunkDataP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemChunkFree(chunkDataP=0x%08X): %d", chunkDataP, res);
}
break;
case sysTrapMemPtrNew: {
  // MemPtr MemPtrNew(UInt32 size)
  uint32_t size = ARG32;
  MemPtr res = MemPtrNew(size);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrNew(size=%u): 0x%08X to 0x%08X", size, r_res, r_res + size - 1);
}
break;
case sysTrapMemPtrRecoverHandle: {
  // MemHandle MemPtrRecoverHandle(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  MemHandle res = MemPtrRecoverHandle(p ? l_p : 0);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrRecoverHandle(p=0x%08X): %p", p, res);
}
break;
case sysTrapMemPtrFlags: {
  // UInt16 MemPtrFlags(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  UInt16 res = MemPtrFlags(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrFlags(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrSize: {
  // UInt32 MemPtrSize(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  UInt32 res = MemPtrSize(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrSize(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrOwner: {
  // UInt16 MemPtrOwner(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  UInt16 res = MemPtrOwner(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrOwner(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrHeapID: {
  // UInt16 MemPtrHeapID(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  UInt16 res = MemPtrHeapID(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrHeapID(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrDataStorage: {
  // Boolean MemPtrDataStorage(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  Boolean res = MemPtrDataStorage(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrDataStorage(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrCardNo: {
  // UInt16 MemPtrCardNo(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  UInt16 res = MemPtrCardNo(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrCardNo(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrToLocalID: {
  // LocalID MemPtrToLocalID(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  LocalID res = MemPtrToLocalID(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrToLocalID(p=0x%08X): 0x%08X", p, res);
}
break;
case sysTrapMemPtrSetOwner: {
  // Err MemPtrSetOwner(MemPtr p, UInt16 owner)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  uint16_t owner = ARG16;
  Err res = MemPtrSetOwner(p ? l_p : 0, owner);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrSetOwner(p=0x%08X, owner=%d): %d", p, owner, res);
}
break;
case sysTrapMemPtrResize: {
  // Err MemPtrResize(MemPtr p, UInt32 newSize)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  uint32_t newSize = ARG32;
  Err res = MemPtrResize(p ? l_p : 0, newSize);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrResize(p=0x%08X, newSize=%d): %d", p, newSize, res);
}
break;
case sysTrapMemPtrResetLock: {
  // Err MemPtrResetLock(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  Err res = MemPtrResetLock(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrResetLock(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemPtrUnlock: {
  // Err MemPtrUnlock(MemPtr p)
  uint32_t p = ARG32;
  void *l_p = emupalmos_trap_in(p, trap, 0);
  Err res = MemPtrUnlock(p ? l_p : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrUnlock(p=0x%08X): %d", p, res);
}
break;
case sysTrapMemHandleNew: {
  // MemHandle MemHandleNew(UInt32 size)
  uint32_t size = ARG32;
  MemHandle res = MemHandleNew(size);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleNew(size=%d): 0x%08X", size, r_res);
}
break;
case sysTrapMemHandleFree: {
  // Err MemHandleFree(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  Err res = MemHandleFree(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleFree(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleFlags: {
  // UInt16 MemHandleFlags(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  UInt16 res = MemHandleFlags(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleFlags(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleSize: {
  // UInt32 MemHandleSize(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  UInt32 res = MemHandleSize(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleSize(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleOwner: {
  // UInt16 MemHandleOwner(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  UInt16 res = MemHandleOwner(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleOwner(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleLockCount: {
  // UInt16 MemHandleLockCount(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  UInt16 res = MemHandleLockCount(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleLockCount(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleHeapID: {
  // UInt16 MemHandleHeapID(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  UInt16 res = MemHandleHeapID(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleHeapID(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleDataStorage: {
  // Boolean MemHandleDataStorage(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  Boolean res = MemHandleDataStorage(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleDataStorage(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleCardNo: {
  // UInt16 MemHandleCardNo(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  UInt16 res = MemHandleCardNo(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleCardNo(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleToLocalID: {
  // LocalID MemHandleToLocalID(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  LocalID res = MemHandleToLocalID(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleToLocalID(h=0x%08X): 0x%08X", h, res);
}
break;
case sysTrapMemHandleSetOwner: {
  // Err MemHandleSetOwner( MemHandle h, UInt16 owner)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  uint16_t owner = ARG16;
  Err res = MemHandleSetOwner(h ? l_h : 0, owner);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleSetOwner(h=0x%08X, owner=%d): %d", h, owner, res);
}
break;
case sysTrapMemHandleResize: {
  // Err MemHandleResize(MemHandle h, UInt32 newSize)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  uint32_t newSize = ARG32;
  Err res = MemHandleResize(h ? l_h : 0, newSize);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleResize(h=0x%08X, newSize=%d): %d", h, newSize, res);
}
break;
case sysTrapMemHandleUnlock: {
  // Err MemHandleUnlock(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  Err res = MemHandleUnlock(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleUnlock(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemHandleResetLock: {
  // Err MemHandleResetLock(MemHandle h)
  uint32_t h = ARG32;
  MemHandle l_h = emupalmos_trap_in(h, trap, 0);
  Err res = MemHandleResetLock(h ? l_h : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHandleResetLock(h=0x%08X): %d", h, res);
}
break;
case sysTrapMemLocalIDToGlobal: {
  // MemPtr MemLocalIDToGlobal(LocalID local, UInt16 cardNo)
  LocalID local = ARG32;
  uint16_t cardNo = ARG16;
  MemPtr res = MemLocalIDToGlobal(local, cardNo);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDToGlobal(local=0x%08X, cardNo=%d): 0x%08X", local, cardNo, r_res);
}
break;
case sysTrapMemLocalIDKind: {
  // LocalIDKind MemLocalIDKind(LocalID local)
  LocalID local = ARG32;
  LocalIDKind res = MemLocalIDKind(local);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDKind(local=0x%08X): %d", local, res);
}
break;
case sysTrapMemLocalIDToPtr: {
  // MemPtr MemLocalIDToPtr(LocalID local, UInt16 cardNo)
  LocalID local = ARG32;
  uint16_t cardNo = ARG16;
  MemPtr res = MemLocalIDToPtr(local, cardNo);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDToPtr(local=0x%08X, cardNo=%d): 0x%08X", local, cardNo, r_res);
}
break;
case sysTrapMemLocalIDToLockedPtr: {
  // MemPtr MemLocalIDToLockedPtr(LocalID local, UInt16 cardNo)
  LocalID local = ARG32;
  uint16_t cardNo = ARG16;
  MemPtr res = MemLocalIDToLockedPtr(local, cardNo);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemLocalIDToLockedPtr(local=0x%08X, cardNo=%d): 0x%08X", local, cardNo, r_res);
}
break;
case sysTrapMemCmp: {
  // Int16 MemCmp(in void *s1, in void *s2, Int32 numBytes)
  uint32_t s1 = ARG32;
  void *s_s1 = emupalmos_trap_in(s1, trap, 0);
  uint32_t s2 = ARG32;
  void *s_s2 = emupalmos_trap_in(s2, trap, 1);
  int32_t numBytes = ARG32;
  Int16 res = MemCmp(s1 ? s_s1 : NULL, s2 ? s_s2 : NULL, numBytes);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemCmp(s1=0x%08X, s2=0x%08X, numBytes=%d): %d", s1, s2, numBytes, res);
}
break;
case sysTrapMemSemaphoreReserve: {
  // Err MemSemaphoreReserve(Boolean writeAccess)
  uint8_t writeAccess = ARG8;
  Err res = MemSemaphoreReserve(writeAccess);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemSemaphoreReserve(writeAccess=%d): %d", writeAccess, res);
}
break;
case sysTrapMemSemaphoreRelease: {
  // Err MemSemaphoreRelease(Boolean writeAccess)
  uint8_t writeAccess = ARG8;
  Err res = MemSemaphoreRelease(writeAccess);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemSemaphoreRelease(writeAccess=%d): %d", writeAccess, res);
}
break;
case sysTrapMemDebugMode: {
  // UInt16 MemDebugMode(void)
  UInt16 res = MemDebugMode();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemDebugMode(): %d", res);
}
break;
case sysTrapMemSetDebugMode: {
  // Err MemSetDebugMode(UInt16 flags)
  uint16_t flags = ARG16;
  Err res = MemSetDebugMode(flags);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemSetDebugMode(flags=%d): %d", flags, res);
}
break;
case sysTrapMemHeapScramble: {
  // Err MemHeapScramble(UInt16 heapID)
  uint16_t heapID = ARG16;
  Err res = MemHeapScramble(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapScramble(heapID=%d): %d", heapID, res);
}
break;
case sysTrapMemHeapCheck: {
  // Err MemHeapCheck(UInt16 heapID)
  uint16_t heapID = ARG16;
  Err res = MemHeapCheck(heapID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MemHeapCheck(heapID=%d): %d", heapID, res);
}
break;
case sysTrapDmInit: {
  // Err DmInit(void)
  Err res = DmInit();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmInit(): %d", res);
}
break;
case sysTrapDmCreateDatabase: {
  // Err DmCreateDatabase(UInt16 cardNo, in Char *nameP, UInt32 creator, UInt32 type, Boolean resDB)
  uint16_t cardNo = ARG16;
  uint32_t nameP = ARG32;
  char *s_nameP = emupalmos_trap_in(nameP, trap, 1);
  uint32_t creator = ARG32;
  uint32_t type = ARG32;
  uint8_t resDB = ARG8;
  Err res = DmCreateDatabase(cardNo, nameP ? s_nameP : NULL, creator, type, resDB);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmCreateDatabase(cardNo=%d, nameP=0x%08X [%s], creator=%d, type=%d, resDB=%d): %d", cardNo, nameP, s_nameP, creator, type, resDB, res);
}
break;
case sysTrapDmCreateDatabaseFromImage: {
  // Err DmCreateDatabaseFromImage(MemPtr bufferP)
  uint32_t bufferP = ARG32;
  void *l_bufferP = emupalmos_trap_in(bufferP, trap, 0);
  Err res = DmCreateDatabaseFromImage(bufferP ? l_bufferP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmCreateDatabaseFromImage(bufferP=0x%08X): %d", bufferP, res);
}
break;
case sysTrapDmDeleteDatabase: {
  // Err DmDeleteDatabase(UInt16 cardNo, LocalID dbID)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  Err res = DmDeleteDatabase(cardNo, dbID);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteDatabase(cardNo=%d, dbID=0x%08X): %d", cardNo, dbID, res);
}
break;
case sysTrapDmNumDatabases: {
  // UInt16 DmNumDatabases(UInt16 cardNo)
  uint16_t cardNo = ARG16;
  UInt16 res = DmNumDatabases(cardNo);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumDatabases(cardNo=%d): %d", cardNo, res);
}
break;
case sysTrapDmGetDatabase: {
  // LocalID DmGetDatabase(UInt16 cardNo, UInt16 index)
  uint16_t cardNo = ARG16;
  uint16_t index = ARG16;
  LocalID res = DmGetDatabase(cardNo, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetDatabase(cardNo=%d, index=%d): 0x%08X", cardNo, index, res);
}
break;
case sysTrapDmFindDatabase: {
  // LocalID DmFindDatabase(UInt16 cardNo, in Char *nameP)
  uint16_t cardNo = ARG16;
  uint32_t nameP = ARG32;
  char *s_nameP = emupalmos_trap_in(nameP, trap, 1);
  LocalID res = DmFindDatabase(cardNo, nameP ? s_nameP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindDatabase(cardNo=%d, nameP=0x%08X [%s]): 0x%08X", cardNo, nameP, s_nameP, res);
}
break;
case sysTrapDmDatabaseInfo: {
  // Err DmDatabaseInfo(UInt16 cardNo, LocalID dbID, out Char *nameP, out UInt16 *attributesP, out UInt16 *versionP, out UInt32 *crDateP, out UInt32 *modDateP, out UInt32 *bckUpDateP, out UInt32 *modNumP, out LocalID *appInfoIDP, out LocalID *sortInfoIDP, out UInt32 *typeP, out UInt32 *creatorP)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  uint32_t nameP = ARG32;
  char *s_nameP = emupalmos_trap_in(nameP, trap, 2);
  uint32_t attributesP = ARG32;
  UInt16 l_attributesP = 0;
  uint32_t versionP = ARG32;
  UInt16 l_versionP = 0;
  uint32_t crDateP = ARG32;
  UInt32 l_crDateP = 0;
  uint32_t modDateP = ARG32;
  UInt32 l_modDateP = 0;
  uint32_t bckUpDateP = ARG32;
  UInt32 l_bckUpDateP = 0;
  uint32_t modNumP = ARG32;
  UInt32 l_modNumP = 0;
  uint32_t appInfoIDP = ARG32;
  LocalID l_appInfoIDP = 0;
  uint32_t sortInfoIDP = ARG32;
  LocalID l_sortInfoIDP = 0;
  uint32_t typeP = ARG32;
  UInt32 l_typeP = 0;
  uint32_t creatorP = ARG32;
  UInt32 l_creatorP = 0;
  Err res = DmDatabaseInfo(cardNo, dbID, nameP ? s_nameP : NULL, attributesP ? &l_attributesP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, modDateP ? &l_modDateP : NULL, bckUpDateP ? &l_bckUpDateP : NULL, modNumP ? &l_modNumP : NULL, appInfoIDP ? &l_appInfoIDP : NULL, sortInfoIDP ? &l_sortInfoIDP : NULL, typeP ? &l_typeP : NULL, creatorP ? &l_creatorP : NULL);
  if (attributesP) m68k_write_memory_16(attributesP, l_attributesP);
  if (versionP) m68k_write_memory_16(versionP, l_versionP);
  if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
  if (modDateP) m68k_write_memory_32(modDateP, l_modDateP);
  if (bckUpDateP) m68k_write_memory_32(bckUpDateP, l_bckUpDateP);
  if (modNumP) m68k_write_memory_32(modNumP, l_modNumP);
  if (appInfoIDP) m68k_write_memory_32(appInfoIDP, l_appInfoIDP);
  if (sortInfoIDP) m68k_write_memory_32(sortInfoIDP, l_sortInfoIDP);
  if (typeP) m68k_write_memory_32(typeP, l_typeP);
  if (creatorP) m68k_write_memory_32(creatorP, l_creatorP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseInfo(cardNo=%d, dbID=0x%08X, nameP=0x%08X [%s], attributesP=0x%08X [%d], versionP=0x%08X [%d], crDateP=0x%08X [%d], modDateP=0x%08X [%d], bckUpDateP=0x%08X [%d], modNumP=0x%08X [%d], appInfoIDP=0x%08X, sortInfoIDP=0x%08X, typeP=0x%08X [%d], creatorP=0x%08X [%d]): %d", cardNo, dbID, nameP, s_nameP, attributesP, l_attributesP, versionP, l_versionP, crDateP, l_crDateP, modDateP, l_modDateP, bckUpDateP, l_bckUpDateP, modNumP, l_modNumP, appInfoIDP, sortInfoIDP, typeP, l_typeP, creatorP, l_creatorP, res);
}
break;
case sysTrapDmSetDatabaseInfo: {
  // Err DmSetDatabaseInfo(UInt16 cardNo, LocalID dbID, in Char *nameP, in UInt16 *attributesP, in UInt16 *versionP, in UInt32 *crDateP, in UInt32 *modDateP, in UInt32 *bckUpDateP, in UInt32 *modNumP, in LocalID *appInfoIDP, in LocalID *sortInfoIDP, in UInt32 *typeP, in UInt32 *creatorP)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  uint32_t nameP = ARG32;
  char *s_nameP = emupalmos_trap_in(nameP, trap, 2);
  uint32_t attributesP = ARG32;
  UInt16 l_attributesP = 0;
  if (attributesP) l_attributesP = m68k_read_memory_16(attributesP);
  uint32_t versionP = ARG32;
  UInt16 l_versionP = 0;
  if (versionP) l_versionP = m68k_read_memory_16(versionP);
  uint32_t crDateP = ARG32;
  UInt32 l_crDateP = 0;
  if (crDateP) l_crDateP = m68k_read_memory_32(crDateP);
  uint32_t modDateP = ARG32;
  UInt32 l_modDateP = 0;
  if (modDateP) l_modDateP = m68k_read_memory_32(modDateP);
  uint32_t bckUpDateP = ARG32;
  UInt32 l_bckUpDateP = 0;
  if (bckUpDateP) l_bckUpDateP = m68k_read_memory_32(bckUpDateP);
  uint32_t modNumP = ARG32;
  UInt32 l_modNumP = 0;
  if (modNumP) l_modNumP = m68k_read_memory_32(modNumP);
  uint32_t appInfoIDP = ARG32;
  LocalID l_appInfoIDP = 0;
  if (appInfoIDP) l_appInfoIDP = m68k_read_memory_32(appInfoIDP);
  uint32_t sortInfoIDP = ARG32;
  LocalID l_sortInfoIDP = 0;
  if (sortInfoIDP) l_sortInfoIDP = m68k_read_memory_32(sortInfoIDP);
  uint32_t typeP = ARG32;
  UInt32 l_typeP = 0;
  if (typeP) l_typeP = m68k_read_memory_32(typeP);
  uint32_t creatorP = ARG32;
  UInt32 l_creatorP = 0;
  if (creatorP) l_creatorP = m68k_read_memory_32(creatorP);
  Err res = DmSetDatabaseInfo(cardNo, dbID, nameP ? s_nameP : NULL, attributesP ? &l_attributesP : NULL, versionP ? &l_versionP : NULL, crDateP ? &l_crDateP : NULL, modDateP ? &l_modDateP : NULL, bckUpDateP ? &l_bckUpDateP : NULL, modNumP ? &l_modNumP : NULL, appInfoIDP ? &l_appInfoIDP : NULL, sortInfoIDP ? &l_sortInfoIDP : NULL, typeP ? &l_typeP : NULL, creatorP ? &l_creatorP : NULL);
  if (attributesP) m68k_write_memory_16(attributesP, l_attributesP);
  if (versionP) m68k_write_memory_16(versionP, l_versionP);
  if (crDateP) m68k_write_memory_32(crDateP, l_crDateP);
  if (modDateP) m68k_write_memory_32(modDateP, l_modDateP);
  if (bckUpDateP) m68k_write_memory_32(bckUpDateP, l_bckUpDateP);
  if (modNumP) m68k_write_memory_32(modNumP, l_modNumP);
  if (typeP) m68k_write_memory_32(typeP, l_typeP);
  if (creatorP) m68k_write_memory_32(creatorP, l_creatorP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSetDatabaseInfo(cardNo=%d, dbID=0x%08X, nameP=0x%08X [%s], attributesP=0x%08X [%d], versionP=0x%08X [%d], crDateP=0x%08X [%d], modDateP=0x%08X [%d], bckUpDateP=0x%08X [%d], modNumP=0x%08X [%d], appInfoIDP=0x%08X, sortInfoIDP=0x%08X, typeP=0x%08X [%d], creatorP=0x%08X [%d]): %d", cardNo, dbID, nameP, s_nameP, attributesP, l_attributesP, versionP, l_versionP, crDateP, l_crDateP, modDateP, l_modDateP, bckUpDateP, l_bckUpDateP, modNumP, l_modNumP, appInfoIDP, sortInfoIDP, typeP, l_typeP, creatorP, l_creatorP, res);
}
break;
case sysTrapDmDatabaseSize: {
  // Err DmDatabaseSize(UInt16 cardNo, LocalID dbID, out UInt32 *numRecordsP, out UInt32 *totalBytesP, out UInt32 *dataBytesP)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  uint32_t numRecordsP = ARG32;
  UInt32 l_numRecordsP = 0;
  uint32_t totalBytesP = ARG32;
  UInt32 l_totalBytesP = 0;
  uint32_t dataBytesP = ARG32;
  UInt32 l_dataBytesP = 0;
  Err res = DmDatabaseSize(cardNo, dbID, numRecordsP ? &l_numRecordsP : NULL, totalBytesP ? &l_totalBytesP : NULL, dataBytesP ? &l_dataBytesP : NULL);
  if (numRecordsP) m68k_write_memory_32(numRecordsP, l_numRecordsP);
  if (totalBytesP) m68k_write_memory_32(totalBytesP, l_totalBytesP);
  if (dataBytesP) m68k_write_memory_32(dataBytesP, l_dataBytesP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseSize(cardNo=%d, dbID=0x%08X, numRecordsP=0x%08X [%d], totalBytesP=0x%08X [%d], dataBytesP=0x%08X [%d]): %d", cardNo, dbID, numRecordsP, l_numRecordsP, totalBytesP, l_totalBytesP, dataBytesP, l_dataBytesP, res);
}
break;
case sysTrapDmDatabaseProtect: {
  // Err DmDatabaseProtect(UInt16 cardNo, LocalID dbID, Boolean protect)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  uint8_t protect = ARG8;
  Err res = DmDatabaseProtect(cardNo, dbID, protect);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDatabaseProtect(cardNo=%d, dbID=0x%08X, protect=%d): %d", cardNo, dbID, protect, res);
}
break;
case sysTrapDmOpenDatabase: {
  // DmOpenRef DmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  uint16_t mode = ARG16;
  DmOpenRef res = DmOpenDatabase(cardNo, dbID, mode);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabase(cardNo=%d, dbID=0x%08X, mode=%d): 0x%08X", cardNo, dbID, mode, r_res);
}
break;
case sysTrapDmOpenDatabaseByTypeCreator: {
  // DmOpenRef DmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode)
  uint32_t type = ARG32;
  uint32_t creator = ARG32;
  uint16_t mode = ARG16;
  DmOpenRef res = DmOpenDatabaseByTypeCreator(type, creator, mode);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  char stype[8], screator[8];
  pumpkin_id2s(type, stype);
  pumpkin_id2s(creator, screator);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabaseByTypeCreator(type='%s', creator='%s', mode=%d): 0x%08X", stype, screator, mode, r_res);
}
break;
case sysTrapDmOpenDBNoOverlay: {
  // DmOpenRef DmOpenDBNoOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  uint16_t mode = ARG16;
  DmOpenRef res = DmOpenDBNoOverlay(cardNo, dbID, mode);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDBNoOverlay(cardNo=%d, dbID=0x%08X, mode=%d): 0x%08X", cardNo, dbID, mode, r_res);
}
break;
case sysTrapDmCloseDatabase: {
  // Err DmCloseDatabase(DmOpenRef dbP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  Err res = DmCloseDatabase(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmCloseDatabase(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmNextOpenDatabase: {
  // DmOpenRef DmNextOpenDatabase(DmOpenRef currentP)
  uint32_t currentP = ARG32;
  DmOpenRef l_currentP = emupalmos_trap_in(currentP, trap, 0);
  DmOpenRef res = DmNextOpenDatabase(currentP ? l_currentP : 0);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNextOpenDatabase(currentP=0x%08X): 0x%08X", currentP, r_res);
}
break;
case sysTrapDmOpenDatabaseInfo: {
  // Err DmOpenDatabaseInfo(DmOpenRef dbP, out LocalID *dbIDP, out UInt16 *openCountP, out UInt16 *modeP, out UInt16 *cardNoP, out Boolean *resDBP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t dbIDP = ARG32;
  LocalID l_dbIDP = 0;
  uint32_t openCountP = ARG32;
  UInt16 l_openCountP = 0;
  uint32_t modeP = ARG32;
  UInt16 l_modeP = 0;
  uint32_t cardNoP = ARG32;
  UInt16 l_cardNoP = 0;
  uint32_t resDBP = ARG32;
  Boolean l_resDBP = false;
  Err res = DmOpenDatabaseInfo(dbP ? l_dbP : 0, dbIDP ? &l_dbIDP : NULL, openCountP ? &l_openCountP : NULL, modeP ? &l_modeP : NULL, cardNoP ? &l_cardNoP : NULL, resDBP ? &l_resDBP : NULL);
  if (dbIDP) m68k_write_memory_32(dbIDP, l_dbIDP);
  if (openCountP) m68k_write_memory_16(openCountP, l_openCountP);
  if (modeP) m68k_write_memory_16(modeP, l_modeP);
  if (cardNoP) m68k_write_memory_16(cardNoP, l_cardNoP);
  if (resDBP) m68k_write_memory_8(resDBP, l_resDBP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmOpenDatabaseInfo(dbP=0x%08X, dbIDP=0x%08X, openCountP=0x%08X [%d], modeP=0x%08X [%d], cardNoP=0x%08X [%d], resDBP=0x%08X): %d", dbP, dbIDP, openCountP, l_openCountP, modeP, l_modeP, cardNoP, l_cardNoP, resDBP, res);
}
break;
case sysTrapDmGetAppInfoID: {
  // LocalID DmGetAppInfoID(DmOpenRef dbP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  LocalID res = DmGetAppInfoID(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetAppInfoID(dbP=0x%08X): 0x%08X", dbP, res);
}
break;
case sysTrapDmGetDatabaseLockState: {
  // void DmGetDatabaseLockState(DmOpenRef dbR, out UInt8 *highest, out UInt32 *count, out UInt32 *busy)
  uint32_t dbR = ARG32;
  DmOpenRef l_dbR = emupalmos_trap_in(dbR, trap, 0);
  uint32_t highest = ARG32;
  UInt8 l_highest = 0;
  uint32_t count = ARG32;
  UInt32 l_count = 0;
  uint32_t busy = ARG32;
  UInt32 l_busy = 0;
  DmGetDatabaseLockState(dbR ? l_dbR : 0, highest ? &l_highest : NULL, count ? &l_count : NULL, busy ? &l_busy : NULL);
  if (highest) m68k_write_memory_8(highest, l_highest);
  if (count) m68k_write_memory_32(count, l_count);
  if (busy) m68k_write_memory_32(busy, l_busy);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetDatabaseLockState(dbR=0x%08X, highest=0x%08X, count=0x%08X [%d], busy=0x%08X [%d])", dbR, highest, count, l_count, busy, l_busy);
}
break;
case sysTrapDmResetRecordStates: {
  // Err DmResetRecordStates(DmOpenRef dbP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  Err res = DmResetRecordStates(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResetRecordStates(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmGetLastErr: {
  // Err DmGetLastErr(void)
  Err res = DmGetLastErr();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetLastErr(): %d", res);
}
break;
case sysTrapDmNumRecords: {
  // UInt16 DmNumRecords(DmOpenRef dbP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  UInt16 res = DmNumRecords(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumRecords(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmNumRecordsInCategory: {
  // UInt16 DmNumRecordsInCategory(DmOpenRef dbP, UInt16 category)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t category = ARG16;
  UInt16 res = DmNumRecordsInCategory(dbP ? l_dbP : 0, category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumRecordsInCategory(dbP=0x%08X, category=%d): %d", dbP, category, res);
}
break;
case sysTrapDmRecordInfo: {
  // Err DmRecordInfo(DmOpenRef dbP, UInt16 index, out UInt16 *attrP, out UInt32 *uniqueIDP, out LocalID *chunkIDP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  uint32_t attrP = ARG32;
  UInt16 l_attrP = 0;
  uint32_t uniqueIDP = ARG32;
  UInt32 l_uniqueIDP = 0;
  uint32_t chunkIDP = ARG32;
  LocalID l_chunkIDP = 0;
  Err res = DmRecordInfo(dbP ? l_dbP : 0, index, attrP ? &l_attrP : NULL, uniqueIDP ? &l_uniqueIDP : NULL, chunkIDP ? &l_chunkIDP : NULL);
  if (attrP) m68k_write_memory_16(attrP, l_attrP);
  if (uniqueIDP) m68k_write_memory_32(uniqueIDP, l_uniqueIDP);
  if (chunkIDP) m68k_write_memory_32(chunkIDP, l_chunkIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRecordInfo(dbP=0x%08X, index=%d, attrP=0x%08X [%d], uniqueIDP=0x%08X [%d], chunkIDP=0x%08X): %d", dbP, index, attrP, l_attrP, uniqueIDP, l_uniqueIDP, chunkIDP, res);
}
break;
case sysTrapDmSetRecordInfo: {
  // Err DmSetRecordInfo(DmOpenRef dbP, UInt16 index, in UInt16 *attrP, in UInt32 *uniqueIDP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  uint32_t attrP = ARG32;
  UInt16 l_attrP = 0;
  if (attrP) l_attrP = m68k_read_memory_16(attrP);
  uint32_t uniqueIDP = ARG32;
  UInt32 l_uniqueIDP = 0;
  if (uniqueIDP) l_uniqueIDP = m68k_read_memory_32(uniqueIDP);
  Err res = DmSetRecordInfo(dbP ? l_dbP : 0, index, attrP ? &l_attrP : NULL, uniqueIDP ? &l_uniqueIDP : NULL);
  if (attrP) m68k_write_memory_16(attrP, l_attrP);
  if (uniqueIDP) m68k_write_memory_32(uniqueIDP, l_uniqueIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSetRecordInfo(dbP=0x%08X, index=%d, attrP=0x%08X [%d], uniqueIDP=0x%08X [%d]): %d", dbP, index, attrP, l_attrP, uniqueIDP, l_uniqueIDP, res);
}
break;
case sysTrapDmMoveRecord: {
  // Err DmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t from = ARG16;
  uint16_t to = ARG16;
  Err res = DmMoveRecord(dbP ? l_dbP : 0, from, to);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmMoveRecord(dbP=0x%08X, from=%d, to=%d): %d", dbP, from, to, res);
}
break;
case sysTrapDmNewRecord: {
  // MemHandle DmNewRecord(DmOpenRef dbP, inout UInt16 *atP, UInt32 size)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t atP = ARG32;
  UInt16 l_atP = 0;
  if (atP) l_atP = m68k_read_memory_16(atP);
  uint32_t size = ARG32;
  MemHandle res = DmNewRecord(dbP ? l_dbP : 0, atP ? &l_atP : NULL, size);
  if (atP) m68k_write_memory_16(atP, l_atP);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNewRecord(dbP=0x%08X, atP=0x%08X [%d], size=%d): 0x%08X", dbP, atP, l_atP, size, r_res);
}
break;
case sysTrapDmRemoveRecord: {
  // Err DmRemoveRecord(DmOpenRef dbP, UInt16 index)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  Err res = DmRemoveRecord(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmDeleteRecord: {
  // Err DmDeleteRecord(DmOpenRef dbP, UInt16 index)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  Err res = DmDeleteRecord(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmArchiveRecord: {
  // Err DmArchiveRecord(DmOpenRef dbP, UInt16 index)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  Err res = DmArchiveRecord(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmArchiveRecord(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmNewHandle: {
  // MemHandle DmNewHandle(DmOpenRef dbP, UInt32 size)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t size = ARG32;
  MemHandle res = DmNewHandle(dbP ? l_dbP : 0, size);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNewHandle(dbP=0x%08X, size=%d): 0x%08X", dbP, size, r_res);
}
break;
case sysTrapDmRemoveSecretRecords: {
  // Err DmRemoveSecretRecords(DmOpenRef dbP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  Err res = DmRemoveSecretRecords(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveSecretRecords(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmFindRecordByID: {
  // Err DmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, out UInt16 *indexP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t uniqueID = ARG32;
  uint32_t indexP = ARG32;
  UInt16 l_indexP = 0;
  Err res = DmFindRecordByID(dbP ? l_dbP : 0, uniqueID, indexP ? &l_indexP : NULL);
  if (indexP) m68k_write_memory_16(indexP, l_indexP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindRecordByID(dbP=0x%08X, uniqueID=%d, indexP=0x%08X [%d]): %d", dbP, uniqueID, indexP, l_indexP, res);
}
break;
case sysTrapDmQueryRecord: {
  // MemHandle DmQueryRecord(DmOpenRef dbP, UInt16 index)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  MemHandle res = DmQueryRecord(dbP ? l_dbP : 0, index);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmQueryRecord(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
}
break;
case sysTrapDmGetRecord: {
  // MemHandle DmGetRecord(DmOpenRef dbP, UInt16 index)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  MemHandle res = DmGetRecord(dbP ? l_dbP : 0, index);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetRecord(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
}
break;
case sysTrapDmQueryNextInCategory: {
  // MemHandle DmQueryNextInCategory(DmOpenRef dbP, inout UInt16 *indexP, UInt16 category)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t indexP = ARG32;
  UInt16 l_indexP = 0;
  if (indexP) l_indexP = m68k_read_memory_16(indexP);
  uint16_t category = ARG16;
  MemHandle res = DmQueryNextInCategory(dbP ? l_dbP : 0, indexP ? &l_indexP : NULL, category);
  if (indexP) m68k_write_memory_16(indexP, l_indexP);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmQueryNextInCategory(dbP=0x%08X, indexP=0x%08X [%d], category=%d): 0x%08X", dbP, indexP, l_indexP, category, r_res);
}
break;
case sysTrapDmPositionInCategory: {
  // UInt16 DmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  uint16_t category = ARG16;
  UInt16 res = DmPositionInCategory(dbP ? l_dbP : 0, index, category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmPositionInCategory(dbP=0x%08X, index=%d, category=%d): %d", dbP, index, category, res);
}
break;
case sysTrapDmSeekRecordInCategory: {
  // Err DmSeekRecordInCategory(DmOpenRef dbP, inout UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t indexP = ARG32;
  UInt16 l_indexP = 0;
  if (indexP) l_indexP = m68k_read_memory_16(indexP);
  uint16_t offset = ARG16;
  int16_t direction = ARG16;
  uint16_t category = ARG16;
  Err res = DmSeekRecordInCategory(dbP ? l_dbP : 0, indexP ? &l_indexP : NULL, offset, direction, category);
  if (indexP) m68k_write_memory_16(indexP, l_indexP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSeekRecordInCategory(dbP=0x%08X, indexP=0x%08X [%d], offset=%d, direction=%d, category=%d): %d", dbP, indexP, l_indexP, offset, direction, category, res);
}
break;
case sysTrapDmResizeRecord: {
  // MemHandle DmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  uint32_t newSize = ARG32;
  MemHandle res = DmResizeRecord(dbP ? l_dbP : 0, index, newSize);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResizeRecord(dbP=0x%08X, index=%d, newSize=%d): %p", dbP, index, newSize, res);
}
break;
case sysTrapDmReleaseRecord: {
  // Err DmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  uint8_t dirty = ARG8;
  Err res = DmReleaseRecord(dbP ? l_dbP : 0, index, dirty);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmReleaseRecord(dbP=0x%08X, index=%d, dirty=%d): %d", dbP, index, dirty, res);
}
break;
case sysTrapDmMoveCategory: {
  // Err DmMoveCategory(DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t toCategory = ARG16;
  uint16_t fromCategory = ARG16;
  uint8_t dirty = ARG8;
  Err res = DmMoveCategory(dbP ? l_dbP : 0, toCategory, fromCategory, dirty);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmMoveCategory(dbP=0x%08X, toCategory=%d, fromCategory=%d, dirty=%d): %d", dbP, toCategory, fromCategory, dirty, res);
}
break;
case sysTrapDmDeleteCategory: {
  // Err DmDeleteCategory(DmOpenRef dbR, UInt16 categoryNum)
  uint32_t dbR = ARG32;
  DmOpenRef l_dbR = emupalmos_trap_in(dbR, trap, 0);
  uint16_t categoryNum = ARG16;
  Err res = DmDeleteCategory(dbR ? l_dbR : 0, categoryNum);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmDeleteCategory(dbR=0x%08X, categoryNum=%d): %d", dbR, categoryNum, res);
}
break;
case sysTrapDmWriteCheck: {
  // Err DmWriteCheck(out void *recordP, UInt32 offset, UInt32 bytes)
  uint32_t recordP = ARG32;
  void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
  uint32_t offset = ARG32;
  uint32_t bytes = ARG32;
  Err res = DmWriteCheck(recordP ? s_recordP : NULL, offset, bytes);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmWriteCheck(recordP=0x%08X, offset=%d, bytes=%d): %d", recordP, offset, bytes, res);
}
break;
case sysTrapDmWrite: {
  // Err DmWrite(out void *recordP, UInt32 offset, in void *srcP, UInt32 bytes)
  uint32_t recordP = ARG32;
  void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
  uint32_t offset = ARG32;
  uint32_t srcP = ARG32;
  void *s_srcP = emupalmos_trap_in(srcP, trap, 2);
  uint32_t bytes = ARG32;
  Err res;
  if (emupalmos_check_address(recordP + offset, bytes, 0) && emupalmos_check_address(srcP, bytes, 1)) {
    res = DmWrite(recordP ? s_recordP : NULL, offset, srcP ? s_srcP : NULL, bytes);
  } else {
    res = dmErrInvalidParam;
  }
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmWrite(recordP=0x%08X, offset=%d, srcP=0x%08X, bytes=%d): %d", recordP, offset, srcP, bytes, res);
}
break;
case sysTrapDmStrCopy: {
  // Err DmStrCopy(out void *recordP, UInt32 offset, in Char *srcP)
  uint32_t recordP = ARG32;
  void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
  uint32_t offset = ARG32;
  uint32_t srcP = ARG32;
  char *s_srcP = emupalmos_trap_in(srcP, trap, 2);
  Err res = DmStrCopy(recordP ? s_recordP : NULL, offset, srcP ? s_srcP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmStrCopy(recordP=0x%08X, offset=%d, srcP=0x%08X [%s]): %d", recordP, offset, srcP, s_srcP, res);
}
break;
case sysTrapDmSet: {
  // Err DmSet(out void *recordP, UInt32 offset, UInt32 bytes, UInt8 value)
  uint32_t recordP = ARG32;
  void *s_recordP = emupalmos_trap_in(recordP, trap, 0);
  uint32_t offset = ARG32;
  uint32_t bytes = ARG32;
  uint8_t value = ARG8;
  Err res = DmSet(recordP ? s_recordP : NULL, offset, bytes, value);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSet(recordP=0x%08X, offset=%d, bytes=%d, value=%d): %d", recordP, offset, bytes, value, res);
}
break;
case sysTrapDmGetResource: {
  // MemHandle DmGetResource(DmResType type, DmResID resID)
  uint32_t type = ARG32;
  char buf[8];
  pumpkin_id2s(type, buf);
  uint16_t resID = ARG16;
  MemHandle res = DmGetResource(type, resID);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetResource(type=%s, resID=%d): 0x%08X", buf, resID, r_res);
}
break;
case sysTrapDmGet1Resource: {
  // MemHandle DmGet1Resource(DmResType type, DmResID resID)
  uint32_t type = ARG32;
  char buf[8];
  pumpkin_id2s(type, buf);
  uint16_t resID = ARG16;
  MemHandle res = DmGet1Resource(type, resID);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGet1Resource(type=%s, resID=%d): 0x%08X", buf, resID, r_res);
}
break;
case sysTrapDmReleaseResource: {
  // Err DmReleaseResource(MemHandle resourceH)
  uint32_t resourceH = ARG32;
  MemHandle l_resourceH = emupalmos_trap_in(resourceH, trap, 0);
  Err res = DmReleaseResource(resourceH ? l_resourceH : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmReleaseResource(resourceH=0x%08X): %d", resourceH, res);
}
break;
case sysTrapDmResizeResource: {
  // MemHandle DmResizeResource(MemHandle resourceH, UInt32 newSize)
  uint32_t resourceH = ARG32;
  MemHandle l_resourceH = emupalmos_trap_in(resourceH, trap, 0);
  uint32_t newSize = ARG32;
  MemHandle res = DmResizeResource(resourceH ? l_resourceH : 0, newSize);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResizeResource(resourceH=0x%08X, newSize=%d): %p", resourceH, newSize, res);
}
break;
case sysTrapDmNextOpenResDatabase: {
  // DmOpenRef DmNextOpenResDatabase(DmOpenRef dbP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  DmOpenRef res = DmNextOpenResDatabase(dbP ? l_dbP : 0);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNextOpenResDatabase(dbP=0x%08X): 0x%08X", dbP, r_res);
}
break;
case sysTrapDmFindResourceType: {
  // UInt16 DmFindResourceType(DmOpenRef dbP, DmResType resType, UInt16 typeIndex)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t resType = ARG32;
  uint16_t typeIndex = ARG16;
  UInt16 res = DmFindResourceType(dbP ? l_dbP : 0, resType, typeIndex);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindResourceType(dbP=0x%08X, resType=%d, typeIndex=%d): %d", dbP, resType, typeIndex, res);
}
break;
case sysTrapDmFindResource: {
  // UInt16 DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t resType = ARG32;
  uint16_t resID = ARG16;
  uint32_t resH = ARG32;
  MemHandle l_resH = emupalmos_trap_in(resH, trap, 3);
  UInt16 res = DmFindResource(dbP ? l_dbP : 0, resType, resID, resH ? l_resH : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmFindResource(dbP=0x%08X, resType=%d, resID=%d, resH=%d): %d", dbP, resType, resID, resH, res);
}
break;
case sysTrapDmNumResources: {
  // UInt16 DmNumResources(DmOpenRef dbP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  UInt16 res = DmNumResources(dbP ? l_dbP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNumResources(dbP=0x%08X): %d", dbP, res);
}
break;
case sysTrapDmResourceInfo: {
  // Err DmResourceInfo(DmOpenRef dbP, UInt16 index, out DmResType *resTypeP, out DmResID *resIDP, out LocalID *chunkLocalIDP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  uint32_t resTypeP = ARG32;
  DmResType l_resTypeP;
  uint32_t resIDP = ARG32;
  DmResID l_resIDP;
  uint32_t chunkLocalIDP = ARG32;
  LocalID l_chunkLocalIDP;
  Err res = DmResourceInfo(dbP ? l_dbP : 0, index, resTypeP ? &l_resTypeP : NULL, resIDP ? &l_resIDP : NULL, chunkLocalIDP ? &l_chunkLocalIDP : NULL);
  if (resTypeP) m68k_write_memory_32(resTypeP, l_resTypeP);
  if (resIDP) m68k_write_memory_16(resIDP, l_resIDP);
  if (chunkLocalIDP) m68k_write_memory_32(chunkLocalIDP, l_chunkLocalIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmResourceInfo(dbP=0x%08X, index=%d, resTypeP=0x%08X, resIDP=0x%08X, chunkLocalIDP=0x%08X): %d", dbP, index, resTypeP, resIDP, chunkLocalIDP, res);
}
break;
case sysTrapDmSetResourceInfo: {
  // Err DmSetResourceInfo(DmOpenRef dbP, UInt16 index, in DmResType *resTypeP, in DmResID *resIDP)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  uint32_t resTypeP = ARG32;
  DmResType l_resTypeP;
  if (resTypeP) l_resTypeP = m68k_read_memory_32(resTypeP);
  uint32_t resIDP = ARG32;
  DmResID l_resIDP;
  if (resIDP) l_resIDP = m68k_read_memory_16(resIDP);
  Err res = DmSetResourceInfo(dbP ? l_dbP : 0, index, resTypeP ? &l_resTypeP : NULL, resIDP ? &l_resIDP : NULL);
  if (resTypeP) m68k_write_memory_32(resTypeP, l_resTypeP);
  if (resIDP) m68k_write_memory_16(resIDP, l_resIDP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmSetResourceInfo(dbP=0x%08X, index=%d, resTypeP=0x%08X, resIDP=0x%08X): %d", dbP, index, resTypeP, resIDP, res);
}
break;
case sysTrapDmNewResource: {
  // MemHandle DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID, UInt32 size)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint32_t resType = ARG32;
  uint16_t resID = ARG16;
  uint32_t size = ARG32;
  MemHandle res = DmNewResource(dbP ? l_dbP : 0, resType, resID, size);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmNewResource(dbP=0x%08X, resType=%d, resID=%d, size=%d): %p", dbP, resType, resID, size, res);
}
break;
case sysTrapDmRemoveResource: {
  // Err DmRemoveResource(DmOpenRef dbP, UInt16 index)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  Err res = DmRemoveResource(dbP ? l_dbP : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmRemoveResource(dbP=0x%08X, index=%d): %d", dbP, index, res);
}
break;
case sysTrapDmGetResourceIndex: {
  // MemHandle DmGetResourceIndex(DmOpenRef dbP, UInt16 index)
  uint32_t dbP = ARG32;
  DmOpenRef l_dbP = emupalmos_trap_in(dbP, trap, 0);
  uint16_t index = ARG16;
  MemHandle res = DmGetResourceIndex(dbP ? l_dbP : 0, index);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "DmGetResourceIndex(dbP=0x%08X, index=%d): 0x%08X", dbP, index, r_res);
}
break;
case sysTrapWinValidateHandle: {
  // Boolean WinValidateHandle(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  Boolean res = WinValidateHandle(winHandle ? l_winHandle : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinValidateHandle(winHandle=0x%08X): %d", winHandle, res);
}
break;
case sysTrapWinInitializeWindow: {
  // void WinInitializeWindow(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinInitializeWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInitializeWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinAddWindow: {
  // void WinAddWindow(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinAddWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinAddWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinRemoveWindow: {
  // void WinRemoveWindow(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  WinHandle l_winHandle = winHandle ? (WinHandle)(ram + winHandle) : NULL;
  WinRemoveWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinRemoveWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinSetActiveWindow: {
  // void WinSetActiveWindow(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinSetActiveWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetActiveWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinSetDrawWindow: {
  // WinHandle WinSetDrawWindow(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinHandle res = WinSetDrawWindow(winHandle ? l_winHandle : 0);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetDrawWindow(winHandle=0x%08X): 0x%08X", winHandle, r_res);
}
break;
case sysTrapWinGetDrawWindow: {
  // WinHandle WinGetDrawWindow(void)
  WinHandle res = WinGetDrawWindow();
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDrawWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinGetActiveWindow: {
  // WinHandle WinGetActiveWindow(void)
  WinHandle res = WinGetActiveWindow();
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetActiveWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinGetDisplayWindow: {
  // WinHandle WinGetDisplayWindow(void)
  WinHandle res = WinGetDisplayWindow();
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDisplayWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinGetFirstWindow: {
  // WinHandle WinGetFirstWindow(void)
  WinHandle res = WinGetFirstWindow();
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetFirstWindow(): 0x%08X", r_res);
}
break;
case sysTrapWinEnableWindow: {
  // void WinEnableWindow(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinEnableWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEnableWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinDisableWindow: {
  // void WinDisableWindow(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  WinDisableWindow(winHandle ? l_winHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDisableWindow(winHandle=0x%08X)", winHandle);
}
break;
case sysTrapWinGetWindowFrameRect: {
  // void WinGetWindowFrameRect(WinHandle winHandle, out RectangleType *r)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  uint32_t r = ARG32;
  RectangleType l_r;
  WinGetWindowFrameRect(winHandle ? l_winHandle : 0, r ? &l_r : NULL);
  encode_rectangle(r, &l_r);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetWindowFrameRect(winHandle=0x%08X, r=0x%08X [%d,%d,%d,%d])", winHandle, r, l_r.topLeft.x, l_r.topLeft.y, l_r.extent.x, l_r.extent.y);
}
break;
case sysTrapWinDrawWindowFrame: {
  // void WinDrawWindowFrame(void)
  WinDrawWindowFrame();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawWindowFrame()");
}
break;
case sysTrapWinEraseWindow: {
  // void WinEraseWindow(void)
  WinEraseWindow();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseWindow()");
}
break;
case sysTrapWinSaveBits: {
  // WinHandle WinSaveBits(in RectangleType *source, out UInt16 *error)
  uint32_t source = ARG32;
  RectangleType l_source;
  decode_rectangle(source, &l_source);
  uint32_t error = ARG32;
  UInt16 l_error = 0;
  WinHandle res = WinSaveBits(source ? &l_source : NULL, error ? &l_error : NULL);
  if (error) m68k_write_memory_16(error, l_error);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSaveBits(source=0x%08X [%d,%d,%d,%d], error=0x%08X [%d]): 0x%08X", source, l_source.topLeft.x, l_source.topLeft.y, l_source.extent.x, l_source.extent.y, error, l_error, r_res);
}
break;
case sysTrapWinRestoreBits: {
  // void WinRestoreBits(WinHandle winHandle, Coord destX, Coord destY)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  int16_t destX = ARG16;
  int16_t destY = ARG16;
  WinRestoreBits(winHandle ? l_winHandle : 0, destX, destY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinRestoreBits(winHandle=0x%08X, destX=%d, destY=%d)", winHandle, destX, destY);
}
break;
case sysTrapWinCopyRectangle: {
  // void WinCopyRectangle(WinHandle srcWin, WinHandle dstWin, in RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode)
  uint32_t srcWin = ARG32;
  WinHandle l_srcWin = emupalmos_trap_in(srcWin, trap, 0);
  uint32_t dstWin = ARG32;
  WinHandle l_dstWin = emupalmos_trap_in(dstWin, trap, 1);
  uint32_t srcRect = ARG32;
  RectangleType l_srcRect;
  decode_rectangle(srcRect, &l_srcRect);
  int16_t destX = ARG16;
  int16_t destY = ARG16;
  uint8_t mode = ARG8;
  WinCopyRectangle(srcWin ? l_srcWin : 0, dstWin ? l_dstWin : 0, srcRect ? &l_srcRect : NULL, destX, destY, mode);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinCopyRectangle(srcWin=0x%08X, dstWin=0x%08X, srcRect=0x%08X [%d,%d,%d,%d], destX=%d, destY=%d, mode=%d)", srcWin, dstWin, srcRect, l_srcRect.topLeft.x, l_srcRect.topLeft.y, l_srcRect.extent.x, l_srcRect.extent.y, destX, destY, mode);
}
break;
case sysTrapWinScrollRectangle: {
  // void WinScrollRectangle(in RectangleType *rP, WinDirectionType direction, Coord distance, out RectangleType *vacatedP)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  uint8_t direction = ARG8;
  int16_t distance = ARG16;
  uint32_t vacatedP = ARG32;
  RectangleType l_vacatedP;
  WinScrollRectangle(rP ? &l_rP : NULL, direction, distance, vacatedP ? &l_vacatedP : NULL);
  encode_rectangle(vacatedP, &l_vacatedP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScrollRectangle(rP=0x%08X [%d,%d,%d,%d], direction=%d, distance=%d, vacatedP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, direction, distance, vacatedP, l_vacatedP.topLeft.x, l_vacatedP.topLeft.y, l_vacatedP.extent.x, l_vacatedP.extent.y);
}
break;
case sysTrapWinGetDisplayExtent: {
  // void WinGetDisplayExtent(out Coord *extentX, out Coord *extentY)
  uint32_t extentX = ARG32;
  Coord l_extentX = 0;
  uint32_t extentY = ARG32;
  Coord l_extentY = 0;
  WinGetDisplayExtent(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDisplayExtent(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinGetDrawWindowBounds: {
  // void WinGetDrawWindowBounds(out RectangleType *rP)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  WinGetDrawWindowBounds(rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetDrawWindowBounds(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinGetBounds: {
  // void WinGetBounds(WinHandle winH, out RectangleType *rP)
  uint32_t winH = ARG32;
  WinHandle l_winH = emupalmos_trap_in(winH, trap, 0);
  uint32_t rP = ARG32;
  RectangleType l_rP;
  WinGetBounds(winH ? l_winH : 0, rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetBounds(winH=0x%08X, rP=0x%08X [%d,%d,%d,%d])", winH, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinSetBounds: {
  // void WinSetBounds(WinHandle winHandle, in RectangleType *rP)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinSetBounds(winHandle ? l_winHandle : 0, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetBounds(winHandle=0x%08X, rP=0x%08X [%d,%d,%d,%d])", winHandle, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinGetWindowExtent: {
  // void WinGetWindowExtent(out Coord *extentX, out Coord *extentY)
  uint32_t extentX = ARG32;
  Coord l_extentX = 0;
  uint32_t extentY = ARG32;
  Coord l_extentY = 0;
  WinGetWindowExtent(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetWindowExtent(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinDisplayToWindowPt: {
  // void WinDisplayToWindowPt(inout Coord *extentX, inout Coord *extentY)
  uint32_t extentX = ARG32;
  Coord l_extentX = 0;
  if (extentX) l_extentX = m68k_read_memory_16(extentX);
  uint32_t extentY = ARG32;
  Coord l_extentY = 0;
  if (extentY) l_extentY = m68k_read_memory_16(extentY);
  WinDisplayToWindowPt(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDisplayToWindowPt(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinWindowToDisplayPt: {
  // void WinWindowToDisplayPt(inout Coord *extentX, inout Coord *extentY)
  uint32_t extentX = ARG32;
  Coord l_extentX = 0;
  if (extentX) l_extentX = m68k_read_memory_16(extentX);
  uint32_t extentY = ARG32;
  Coord l_extentY = 0;
  if (extentY) l_extentY = m68k_read_memory_16(extentY);
  WinWindowToDisplayPt(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinWindowToDisplayPt(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinGetBitmap: {
  // BitmapType *WinGetBitmap(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  BitmapType *res = WinGetBitmap(winHandle ? l_winHandle : 0);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetBitmap(winHandle=0x%08X): 0x%08X", winHandle, r_res);
}
break;
case sysTrapWinGetClip: {
  // void WinGetClip(out RectangleType *rP)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  WinGetClip(rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetClip(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinSetClip: {
  // void WinSetClip(in RectangleType *rP)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinSetClip(rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetClip(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinResetClip: {
  // void WinResetClip(void)
  WinResetClip();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinResetClip()");
}
break;
case sysTrapWinClipRectangle: {
  // void WinClipRectangle(out RectangleType *rP)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinClipRectangle(rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinClipRectangle(rP=0x%08X [%d,%d,%d,%d])", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinModal: {
  // Boolean WinModal(WinHandle winHandle)
  uint32_t winHandle = ARG32;
  WinHandle l_winHandle = emupalmos_trap_in(winHandle, trap, 0);
  Boolean res = WinModal(winHandle ? l_winHandle : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinModal(winHandle=0x%08X): %d", winHandle, res);
}
break;
case sysTrapWinGetPixel: {
  // IndexedColorType WinGetPixel(Coord x, Coord y)
  int16_t x = ARG16;
  int16_t y = ARG16;
  IndexedColorType res = WinGetPixel(x, y);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPixel(x=%d, y=%d): %d", x, y, res);
}
break;
case sysTrapWinGetPixelRGB: {
  // Err WinGetPixelRGB(Coord x, Coord y, out RGBColorType *rgbP)
  int16_t x = ARG16;
  int16_t y = ARG16;
  uint32_t rgbP = ARG32;
  RGBColorType l_rgbP;
  Err res = WinGetPixelRGB(x, y, rgbP ? &l_rgbP : NULL);
  encode_rgb(rgbP, &l_rgbP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPixelRGB(x=%d, y=%d, rgbP=0x%08X): %d", x, y, rgbP, res);
}
break;
case sysTrapWinPaintPixel: {
  // void WinPaintPixel(Coord x, Coord y)
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinPaintPixel(x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintPixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinPaintPixels: {
  // void WinPaintPixels(UInt16 numPoints, PointType pts[])
  uint16_t numPoints = ARG16;
  uint32_t pts = ARG32;
  uint16_t i;
  emupalmos_trap_in(pts, trap, 1);
  for (i = 0; i < numPoints; i++) {
    int16_t x = m68k_read_memory_16(pts);
    pts += 2;
    int16_t y = m68k_read_memory_16(pts);
    pts += 2;
    WinPaintPixel(x, y);
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintPixels(numPoints=%u, pts=0x%08X)", numPoints, pts);
}
break;
case sysTrapWinDrawPixel: {
  // void WinDrawPixel(Coord x, Coord y)
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinDrawPixel(x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawPixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinErasePixel: {
  // void WinErasePixel(Coord x, Coord y)
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinErasePixel(x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinErasePixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinInvertPixel: {
  // void WinInvertPixel(Coord x, Coord y)
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinInvertPixel(x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertPixel(x=%d, y=%d)", x, y);
}
break;
case sysTrapWinPaintLine: {
  // void WinPaintLine(Coord x1, Coord y1, Coord x2, Coord y2)
  int16_t x1 = ARG16;
  int16_t y1 = ARG16;
  int16_t x2 = ARG16;
  int16_t y2 = ARG16;
  WinPaintLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinDrawLine: {
  // void WinDrawLine(Coord x1, Coord y1, Coord x2, Coord y2)
  int16_t x1 = ARG16;
  int16_t y1 = ARG16;
  int16_t x2 = ARG16;
  int16_t y2 = ARG16;
  WinDrawLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinDrawGrayLine: {
  // void WinDrawGrayLine(Coord x1, Coord y1, Coord x2, Coord y2)
  int16_t x1 = ARG16;
  int16_t y1 = ARG16;
  int16_t x2 = ARG16;
  int16_t y2 = ARG16;
  WinDrawGrayLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawGrayLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinEraseLine: {
  // void WinEraseLine(Coord x1, Coord y1, Coord x2, Coord y2)
  int16_t x1 = ARG16;
  int16_t y1 = ARG16;
  int16_t x2 = ARG16;
  int16_t y2 = ARG16;
  WinEraseLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinInvertLine: {
  // void WinInvertLine(Coord x1, Coord y1, Coord x2, Coord y2)
  int16_t x1 = ARG16;
  int16_t y1 = ARG16;
  int16_t x2 = ARG16;
  int16_t y2 = ARG16;
  WinInvertLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinFillLine: {
  // void WinFillLine(Coord x1, Coord y1, Coord x2, Coord y2)
  int16_t x1 = ARG16;
  int16_t y1 = ARG16;
  int16_t x2 = ARG16;
  int16_t y2 = ARG16;
  WinFillLine(x1, y1, x2, y2);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinFillLine(x1=%d, y1=%d, x2=%d, y2=%d)", x1, y1, x2, y2);
}
break;
case sysTrapWinPaintRectangle: {
  // void WinPaintRectangle(in RectangleType *rP, UInt16 cornerDiam)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  uint16_t cornerDiam = ARG16;
  WinPaintRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinDrawRectangle: {
  // void WinDrawRectangle(in RectangleType *rP, UInt16 cornerDiam)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  uint16_t cornerDiam = ARG16;
  WinDrawRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinEraseRectangle: {
  // void WinEraseRectangle(in RectangleType *rP, UInt16 cornerDiam)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  uint16_t cornerDiam = ARG16;
  WinEraseRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinInvertRectangle: {
  // void WinInvertRectangle(in RectangleType *rP, UInt16 cornerDiam)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  uint16_t cornerDiam = ARG16;
  WinInvertRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinFillRectangle: {
  // void WinFillRectangle(in RectangleType *rP, UInt16 cornerDiam)
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  uint16_t cornerDiam = ARG16;
  WinFillRectangle(rP ? &l_rP : NULL, cornerDiam);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinFillRectangle(rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)", rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y, cornerDiam);
}
break;
case sysTrapWinPaintRectangleFrame: {
  // void WinPaintRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinPaintRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawRectangleFrame: {
  // void WinDrawRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinDrawRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawGrayRectangleFrame: {
  // void WinDrawGrayRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinDrawGrayRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawGrayRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinEraseRectangleFrame: {
  // void WinEraseRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinEraseRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinInvertRectangleFrame: {
  // void WinInvertRectangleFrame(FrameType frame, in RectangleType *rP)
  uint16_t frame = ARG16;
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  WinInvertRectangleFrame(frame, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertRectangleFrame(frame=%d, rP=0x%08X [%d,%d,%d,%d])", frame, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapWinDrawBitmap: {
  // void WinDrawBitmap(in BitmapType *bitmapP, Coord x, Coord y)
  uint32_t bitmapP = ARG32;
  //BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  BitmapType *l_bitmapP = emupalmos_trap_in(bitmapP, trap, 0);
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinDrawBitmap(bitmapP ? l_bitmapP : NULL, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawBitmap(bitmapP=0x%08X, x=%d, y=%d)", bitmapP, x, y);
}
break;
case sysTrapWinPaintBitmap: {
  // void WinPaintBitmap(in BitmapType *bitmapP, Coord x, Coord y)
  uint32_t bitmapP = ARG32;
  //BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  BitmapType *l_bitmapP = emupalmos_trap_in(bitmapP, trap, 0);
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinPaintBitmap(bitmapP ? l_bitmapP : NULL, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintBitmap(bitmapP=0x%08X, x=%d, y=%d)", bitmapP, x, y);
}
break;
case sysTrapWinDrawChar: {
  // void WinDrawChar(WChar theChar, Coord x, Coord y)
  uint16_t theChar = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinDrawChar(theChar, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawChar(theChar=%d, x=%d, y=%d)", theChar, x, y);
}
break;
case sysTrapWinDrawChars: {
  // void WinDrawChars(in Char *chars, Int16 len, Coord x, Coord y)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  int16_t len = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  if (s_chars) WinDrawChars(s_chars, len, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawChars(chars=0x%08X %p [%.*s], len=%d, x=%d, y=%d)", chars, s_chars, len, s_chars, len, x, y);
}
break;
case sysTrapWinPaintChar: {
  // void WinPaintChar(WChar theChar, Coord x, Coord y)
  uint16_t theChar = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  WinPaintChar(theChar, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintChar(theChar=%d, x=%d, y=%d)", theChar, x, y);
}
break;
case sysTrapWinPaintChars: {
  // void WinPaintChars(in Char *chars, Int16 len, Coord x, Coord y)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  int16_t len = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  if (s_chars) WinPaintChars(s_chars, len, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinDrawInvertedChars: {
  // void WinDrawInvertedChars(in Char *chars, Int16 len, Coord x, Coord y)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  int16_t len = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  if (s_chars) WinDrawInvertedChars(s_chars, len, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawInvertedChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinDrawTruncChars: {
  // void WinDrawTruncChars(in Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  int16_t len = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  int16_t maxWidth = ARG16;
  if (s_chars) WinDrawTruncChars(s_chars, len, x, y, maxWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinDrawTruncChars(chars=0x%08X [%s], len=%d, x=%d, y=%d, maxWidth=%d)", chars, s_chars, len, x, y, maxWidth);
}
break;
case sysTrapWinEraseChars: {
  // void WinEraseChars(in Char *chars, Int16 len, Coord x, Coord y)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  int16_t len = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  if (s_chars) WinEraseChars(s_chars, len, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinEraseChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinInvertChars: {
  // void WinInvertChars(in Char *chars, Int16 len, Coord x, Coord y)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  int16_t len = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  if (s_chars) WinInvertChars(s_chars, len, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinInvertChars(chars=0x%08X [%s], len=%d, x=%d, y=%d)", chars, s_chars, len, x, y);
}
break;
case sysTrapWinSetUnderlineMode: {
  // UnderlineModeType WinSetUnderlineMode(UnderlineModeType mode)
  uint8_t mode = ARG8;
  UnderlineModeType res = WinSetUnderlineMode(mode);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetUnderlineMode(mode=%d): %d", mode, res);
}
break;
case sysTrapWinPushDrawState: {
  // void WinPushDrawState(void)
  WinPushDrawState();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPushDrawState()");
}
break;
case sysTrapWinPopDrawState: {
  // void WinPopDrawState(void)
  WinPopDrawState();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinPopDrawState()");
}
break;
case sysTrapWinSetDrawMode: {
  // WinDrawOperation WinSetDrawMode(WinDrawOperation newMode)
  uint8_t newMode = ARG8;
  WinDrawOperation res = WinSetDrawMode(newMode);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetDrawMode(newMode=%d): %d", newMode, res);
}
break;
case sysTrapWinSetForeColor: {
  // IndexedColorType WinSetForeColor(IndexedColorType foreColor)
  uint8_t foreColor = ARG8;
  IndexedColorType res = WinSetForeColor(foreColor);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetForeColor(foreColor=%d): %d", foreColor, res);
}
break;
case sysTrapWinSetBackColor: {
  // IndexedColorType WinSetBackColor(IndexedColorType backColor)
  uint8_t backColor = ARG8;
  IndexedColorType res = WinSetBackColor(backColor);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetBackColor(backColor=%d): %d", backColor, res);
}
break;
case sysTrapWinSetTextColor: {
  // IndexedColorType WinSetTextColor(IndexedColorType textColor)
  uint8_t textColor = ARG8;
  IndexedColorType res = WinSetTextColor(textColor);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetTextColor(textColor=%d): %d", textColor, res);
}
break;
case sysTrapWinSetForeColorRGB: {
  // void WinSetForeColorRGB(in RGBColorType *newRgbP, out RGBColorType *prevRgbP)
  uint32_t newRgbP = ARG32;
  RGBColorType l_newRgbP;
  decode_rgb(newRgbP, &l_newRgbP);
  uint32_t prevRgbP = ARG32;
  RGBColorType l_prevRgbP;
  WinSetForeColorRGB(newRgbP ? &l_newRgbP : NULL, prevRgbP ? &l_prevRgbP : NULL);
  encode_rgb(prevRgbP, &l_prevRgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetForeColorRGB(newRgbP=0x%08X, prevRgbP=0x%08X)", newRgbP, prevRgbP);
}
break;
case sysTrapWinSetBackColorRGB: {
  // void WinSetBackColorRGB(in RGBColorType *newRgbP, out RGBColorType *prevRgbP)
  uint32_t newRgbP = ARG32;
  RGBColorType l_newRgbP;
  decode_rgb(newRgbP, &l_newRgbP);
  uint32_t prevRgbP = ARG32;
  RGBColorType l_prevRgbP;
  WinSetBackColorRGB(newRgbP ? &l_newRgbP : NULL, prevRgbP ? &l_prevRgbP : NULL);
  encode_rgb(prevRgbP, &l_prevRgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetBackColorRGB(newRgbP=0x%08X, prevRgbP=0x%08X)", newRgbP, prevRgbP);
}
break;
case sysTrapWinSetTextColorRGB: {
  // void WinSetTextColorRGB(in RGBColorType *newRgbP, out RGBColorType *prevRgbP)
  uint32_t newRgbP = ARG32;
  RGBColorType l_newRgbP;
  decode_rgb(newRgbP, &l_newRgbP);
  uint32_t prevRgbP = ARG32;
  RGBColorType l_prevRgbP;
  WinSetTextColorRGB(newRgbP ? &l_newRgbP : NULL, prevRgbP ? &l_prevRgbP : NULL);
  encode_rgb(prevRgbP, &l_prevRgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetTextColorRGB(newRgbP=0x%08X, prevRgbP=0x%08X)", newRgbP, prevRgbP);
}
break;
case sysTrapWinGetPattern: {
  // void WinGetPattern(out CustomPatternType *patternP)
  uint32_t patternP = ARG32;
  CustomPatternType *s_patternP = emupalmos_trap_in(patternP, trap, 0);
  if (s_patternP) WinGetPattern(s_patternP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPattern(patternP=0x%08X)", patternP);
}
break;
case sysTrapWinGetPatternType: {
  // PatternType WinGetPatternType(void)
  PatternType res = WinGetPatternType();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetPatternType(): %d", res);
}
break;
case sysTrapWinSetPattern: {
  // void WinSetPattern(in CustomPatternType *patternP)
  uint32_t patternP = ARG32;
  CustomPatternType *s_patternP = emupalmos_trap_in(patternP, trap, 0);
  if (s_patternP) WinSetPattern(s_patternP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetPattern(patternP=0x%08X)", patternP);
}
break;
case sysTrapWinSetPatternType: {
  // void WinSetPatternType(PatternType newPattern)
  uint8_t newPattern = ARG8;
  WinSetPatternType(newPattern);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetPatternType(newPattern=%d)", newPattern);
}
break;
case sysTrapWinRGBToIndex: {
  // IndexedColorType WinRGBToIndex(in RGBColorType *rgbP)
  uint32_t rgbP = ARG32;
  RGBColorType l_rgbP;
  decode_rgb(rgbP, &l_rgbP);
  IndexedColorType res = WinRGBToIndex(rgbP ? &l_rgbP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinRGBToIndex(rgbP=0x%08X): %d", rgbP, res);
}
break;
case sysTrapWinIndexToRGB: {
  // void WinIndexToRGB(IndexedColorType i, out RGBColorType *rgbP)
  uint8_t i = ARG8;
  uint32_t rgbP = ARG32;
  RGBColorType l_rgbP;
  WinIndexToRGB(i, rgbP ? &l_rgbP : NULL);
  encode_rgb(rgbP, &l_rgbP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinIndexToRGB(i=%d, rgbP=0x%08X)", i, rgbP);
}
break;
case sysTrapWinSetColors: {
  // void WinSetColors(in RGBColorType *newForeColorP, out RGBColorType *oldForeColorP, in RGBColorType *newBackColorP, out RGBColorType *oldBackColorP)
  uint32_t newForeColorP = ARG32;
  RGBColorType l_newForeColorP;
  decode_rgb(newForeColorP, &l_newForeColorP);
  uint32_t oldForeColorP = ARG32;
  RGBColorType l_oldForeColorP;
  uint32_t newBackColorP = ARG32;
  RGBColorType l_newBackColorP;
  decode_rgb(newBackColorP, &l_newBackColorP);
  uint32_t oldBackColorP = ARG32;
  RGBColorType l_oldBackColorP;
  WinSetColors(newForeColorP ? &l_newForeColorP : NULL, oldForeColorP ? &l_oldForeColorP : NULL, newBackColorP ? &l_newBackColorP : NULL, oldBackColorP ? &l_oldBackColorP : NULL);
  encode_rgb(oldForeColorP, &l_oldForeColorP);
  encode_rgb(oldBackColorP, &l_oldBackColorP);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinSetColors(newForeColorP=0x%08X, oldForeColorP=0x%08X, newBackColorP=0x%08X, oldBackColorP=0x%08X)", newForeColorP, oldForeColorP, newBackColorP, oldBackColorP);
}
break;
case sysTrapWinScreenInit: {
  // void WinScreenInit(void)
  WinScreenInit();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenInit()");
}
break;
case sysTrapWinScreenLock: {
  // UInt8 *WinScreenLock(WinLockInitType initMode)
  uint8_t initMode = ARG8;
  UInt8 *res = WinScreenLock(initMode);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenLock(initMode=%d): 0x%08X", initMode, r_res);
}
break;
case sysTrapWinScreenUnlock: {
  // void WinScreenUnlock(void)
  WinScreenUnlock();
  debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenUnlock()");
}
break;
case sysTrapBmpCompress: {
  // Err BmpCompress(in BitmapType *bitmapP, BitmapCompressionType compType)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  uint8_t compType = ARG8;
  Err res = BmpCompress(bitmapP ? l_bitmapP : NULL, compType);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpCompress(bitmapP=0x%08X, compType=%d): %d", bitmapP, compType, res);
}
break;
case sysTrapBmpGetBits: {
  // void *BmpGetBits(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  void *res = BmpGetBits(bitmapP ? l_bitmapP : NULL);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetBits(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapBmpGetColortable: {
  // ColorTableType *BmpGetColortable(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  ColorTableType *res = BmpGetColortable(bitmapP ? l_bitmapP : NULL);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetColortable(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapBmpSize: {
  // UInt16 BmpSize(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt16 res = BmpSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpBitsSize: {
  // UInt16 BmpBitsSize(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt16 res = BmpBitsSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpBitsSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetSizes: {
  // void BmpGetSizes(in BitmapType *bitmapP, out UInt32 *dataSizeP, out UInt32 *headerSizeP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  uint32_t dataSizeP = ARG32;
  UInt32 l_dataSizeP = 0;
  uint32_t headerSizeP = ARG32;
  UInt32 l_headerSizeP = 0;
  BmpGetSizes(bitmapP ? l_bitmapP : NULL, dataSizeP ? &l_dataSizeP : NULL, headerSizeP ? &l_headerSizeP : NULL);
  if (dataSizeP) m68k_write_memory_32(dataSizeP, l_dataSizeP);
  if (headerSizeP) m68k_write_memory_32(headerSizeP, l_headerSizeP);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetSizes(bitmapP=0x%08X, dataSizeP=0x%08X [%d], headerSizeP=0x%08X [%d])", bitmapP, dataSizeP, l_dataSizeP, headerSizeP, l_headerSizeP);
}
break;
case sysTrapBmpColortableSize: {
  // UInt16 BmpColortableSize(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt16 res = BmpColortableSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpColortableSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetDimensions: {
  // void BmpGetDimensions(in BitmapType *bitmapP, out Coord *widthP, out Coord *heightP, out UInt16 *rowBytesP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  uint32_t widthP = ARG32;
  Coord l_widthP = 0;
  uint32_t heightP = ARG32;
  Coord l_heightP = 0;
  uint32_t rowBytesP = ARG32;
  UInt16 l_rowBytesP = 0;
  BmpGetDimensions(bitmapP ? l_bitmapP : NULL, widthP ? &l_widthP : NULL, heightP ? &l_heightP : NULL, rowBytesP ? &l_rowBytesP : NULL);
  if (widthP) m68k_write_memory_16(widthP, l_widthP);
  if (heightP) m68k_write_memory_16(heightP, l_heightP);
  if (rowBytesP) m68k_write_memory_16(rowBytesP, l_rowBytesP);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetDimensions(bitmapP=0x%08X, widthP=0x%08X [%d], heightP=0x%08X [%d], rowBytesP=0x%08X [%d])", bitmapP, widthP, l_widthP, heightP, l_heightP, rowBytesP, l_rowBytesP);
}
break;
case sysTrapBmpGetBitDepth: {
  // UInt8 BmpGetBitDepth(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt8 res = BmpGetBitDepth(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetBitDepth(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetNextBitmap: {
  // BitmapType *BmpGetNextBitmap(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  BitmapType *res = BmpGetNextBitmap(bitmapP ? l_bitmapP : NULL);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetNextBitmap(bitmapP=0x%08X): 0x%08X", bitmapP, r_res);
}
break;
case sysTrapFntGetFont: {
  // FontID FntGetFont(void)
  FontID res = FntGetFont();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntGetFont(): %d", res);
}
break;
case sysTrapFntSetFont: {
  // FontID FntSetFont(FontID font)
  uint8_t font = ARG8;
  FontID res = FntSetFont(font);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntSetFont(font=%d): %d", font, res);
}
break;
case sysTrapFntGetFontPtr: {
  // FontType *FntGetFontPtr(void)
  FontType *res = FntGetFontPtr();
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntGetFontPtr(): 0x%08X", r_res);
}
break;
case sysTrapFntBaseLine: {
  // Int16 FntBaseLine(void)
  Int16 res = FntBaseLine();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntBaseLine(): %d", res);
}
break;
case sysTrapFntCharHeight: {
  // Int16 FntCharHeight(void)
  Int16 res = FntCharHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharHeight(): %d", res);
}
break;
case sysTrapFntLineHeight: {
  // Int16 FntLineHeight(void)
  Int16 res = FntLineHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntLineHeight(): %d", res);
}
break;
case sysTrapFntAverageCharWidth: {
  // Int16 FntAverageCharWidth(void)
  Int16 res = FntAverageCharWidth();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntAverageCharWidth(): %d", res);
}
break;
case sysTrapFntCharWidth: {
  // Int16 FntCharWidth(Char ch)
  int8_t ch = ARG8;
  Int16 res = FntCharWidth(ch);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharWidth(ch=%d): %d", ch, res);
}
break;
case sysTrapFntWCharWidth: {
  // Int16 FntWCharWidth(WChar iChar)
  uint16_t iChar = ARG16;
  Int16 res = FntWCharWidth(iChar);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWCharWidth(iChar=%d): %d", iChar, res);
}
break;
case sysTrapFntCharsWidth: {
  // Int16 FntCharsWidth(in Char *chars, Int16 len)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  int16_t len = ARG16;
  Int16 res = s_chars ? FntCharsWidth(s_chars, len) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharsWidth(chars=0x%08X [%.*s], len=%d): %d", chars, len, s_chars, len, res);
}
break;
case sysTrapFntWidthToOffset: {
  // Int16 FntWidthToOffset(in Char *pChars, UInt16 length, Int16 pixelWidth, out Boolean *leadingEdge, out Int16 *truncWidth)
  uint32_t pChars = ARG32;
  char *s_pChars = emupalmos_trap_in(pChars, trap, 0);
  uint16_t length = ARG16;
  int16_t pixelWidth = ARG16;
  uint32_t leadingEdge = ARG32;
  Boolean l_leadingEdge = false;
  uint32_t truncWidth = ARG32;
  Int16 l_truncWidth = 0;
  Int16 res = s_pChars ? FntWidthToOffset(s_pChars, length, pixelWidth, leadingEdge ? &l_leadingEdge : NULL, truncWidth ? &l_truncWidth : NULL) : 0;
  if (leadingEdge) m68k_write_memory_8(leadingEdge, l_leadingEdge);
  if (truncWidth) m68k_write_memory_16(truncWidth, l_truncWidth);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWidthToOffset(pChars=0x%08X [%s], length=%d, pixelWidth=%d, leadingEdge=0x%08X, truncWidth=0x%08X [%d]): %d", pChars, s_pChars, length, pixelWidth, leadingEdge, truncWidth, l_truncWidth, res);
}
break;
case sysTrapFntCharsInWidth: {
  // void FntCharsInWidth(in Char *string, inout Int16 *stringWidthP, inout Int16 *stringLengthP, out Boolean *fitWithinWidth)
  uint32_t string = ARG32;
  char *s_string = emupalmos_trap_in(string, trap, 0);
  uint32_t stringWidthP = ARG32;
  Int16 l_stringWidthP = 0;
  if (stringWidthP) l_stringWidthP = m68k_read_memory_16(stringWidthP);
  uint32_t stringLengthP = ARG32;
  Int16 l_stringLengthP = 0;
  if (stringLengthP) l_stringLengthP = m68k_read_memory_16(stringLengthP);
  uint32_t fitWithinWidth = ARG32;
  Boolean l_fitWithinWidth = 0;
  if (s_string) FntCharsInWidth(s_string, stringWidthP ? &l_stringWidthP : NULL, stringLengthP ? &l_stringLengthP : NULL, fitWithinWidth ? &l_fitWithinWidth : NULL);
  if (stringWidthP) m68k_write_memory_16(stringWidthP, l_stringWidthP);
  if (stringLengthP) m68k_write_memory_16(stringLengthP, l_stringLengthP);
  if (fitWithinWidth) m68k_write_memory_8(fitWithinWidth, l_fitWithinWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntCharsInWidth(string=0x%08X [%s], stringWidthP=0x%08X [%d], stringLengthP=0x%08X [%d], fitWithinWidth=0x%08X [%d])", string, s_string, stringWidthP, l_stringWidthP, stringLengthP, l_stringLengthP, fitWithinWidth, l_fitWithinWidth);
}
break;
case sysTrapFntDescenderHeight: {
  // Int16 FntDescenderHeight(void)
  Int16 res = FntDescenderHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntDescenderHeight(): %d", res);
}
break;
case sysTrapFntLineWidth: {
  // Int16 FntLineWidth(in Char *pChars, UInt16 length)
  uint32_t pChars = ARG32;
  char *s_pChars = emupalmos_trap_in(pChars, trap, 0);
  uint16_t length = ARG16;
  Int16 res = s_pChars ? FntLineWidth(s_pChars, length) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntLineWidth(pChars=0x%08X [%s], length=%d): %d", pChars, s_pChars, length, res);
}
break;
case sysTrapFntWordWrap: {
  // UInt16 FntWordWrap(in Char *chars, UInt16 maxWidth)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  uint16_t maxWidth = ARG16;
  UInt16 res = s_chars ? FntWordWrap(s_chars, maxWidth) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWordWrap(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
}
break;
case sysTrapFntWordWrapReverseNLines: {
  // void FntWordWrapReverseNLines(in Char *chars, UInt16 maxWidth, inout UInt16 *linesToScrollP, inout UInt16 *scrollPosP)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  uint16_t maxWidth = ARG16;
  uint32_t linesToScrollP = ARG32;
  UInt16 l_linesToScrollP = 0;
  if (linesToScrollP) l_linesToScrollP = m68k_read_memory_16(linesToScrollP);
  uint32_t scrollPosP = ARG32;
  UInt16 l_scrollPosP = 0;
  if (scrollPosP) l_scrollPosP = m68k_read_memory_16(scrollPosP);
  if (s_chars) FntWordWrapReverseNLines(s_chars, maxWidth, linesToScrollP ? &l_linesToScrollP : NULL, scrollPosP ? &l_scrollPosP : NULL);
  if (linesToScrollP) m68k_write_memory_16(linesToScrollP, l_linesToScrollP);
  if (scrollPosP) m68k_write_memory_16(scrollPosP, l_scrollPosP);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntWordWrapReverseNLines(chars=0x%08X [%s], maxWidth=%d, linesToScrollP=0x%08X [%d], scrollPosP=0x%08X [%d])", chars, s_chars, maxWidth, linesToScrollP, l_linesToScrollP, scrollPosP, l_scrollPosP);
}
break;
case sysTrapFntGetScrollValues: {
  // void FntGetScrollValues(in Char *chars, UInt16 width, UInt16 scrollPos, out UInt16 *linesP, out UInt16 *topLine)
  uint32_t chars = ARG32;
  char *s_chars = emupalmos_trap_in(chars, trap, 0);
  uint16_t width = ARG16;
  uint16_t scrollPos = ARG16;
  uint32_t linesP = ARG32;
  UInt16 l_linesP = 0;
  uint32_t topLine = ARG32;
  UInt16 l_topLine = 0;
  if (s_chars) FntGetScrollValues(s_chars, width, scrollPos, linesP ? &l_linesP : NULL, topLine ? &l_topLine : NULL);
  if (linesP) m68k_write_memory_16(linesP, l_linesP);
  if (topLine) m68k_write_memory_16(topLine, l_topLine);
  debug(DEBUG_TRACE, "EmuPalmOS", "FntGetScrollValues(chars=0x%08X [%s], width=%d, scrollPos=%d, linesP=0x%08X [%d], topLine=0x%08X [%d])", chars, s_chars, width, scrollPos, linesP, l_linesP, topLine, l_topLine);
}
break;
case sysTrapStrCopy: {
  // Char *StrCopy(out Char *dst, in Char *src)
  uint32_t dst = ARG32;
  char *s_dst = emupalmos_trap_in(dst, trap, 0);
  uint32_t src = ARG32;
  char *s_src = emupalmos_trap_in(src, trap, 1);
  Char *res = NULL;
  if (s_dst && s_src) {
    if (emupalmos_check_address(dst, sys_strlen(s_src)+1, 0)) {
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCopy %d bytes", (int)sys_strlen(s_src)+1);
      res = StrCopy(s_dst, s_src);
    }
  }
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCopy(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
}
break;
case sysTrapStrNCopy: {
  // Char *StrNCopy(out Char *dst, in Char *src, Int16 n)
  uint32_t dst = ARG32;
  char *s_dst = emupalmos_trap_in(dst, trap, 0);
  uint32_t src = ARG32;
  char *s_src = emupalmos_trap_in(src, trap, 1);
  int16_t n = ARG16;
  Char *res = s_dst && s_src ? StrNCopy(s_dst, s_src, n) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCopy(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst, s_dst, src, s_src, n, r_res);
}
break;
case sysTrapStrCat: {
  // Char *StrCat(out Char *dst, in Char *src)
  uint32_t dst = ARG32;
  char *s_dst = emupalmos_trap_in(dst, trap, 0);
  uint32_t src = ARG32;
  char *s_src = emupalmos_trap_in(src, trap, 1);
  Char *res = NULL;
  if (s_dst && s_src) {
    if (emupalmos_check_address(dst + sys_strlen(s_dst), sys_strlen(s_src)+1, 0)) {
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCat %d bytes into %d bytes with %d total", (int)sys_strlen(s_src)+1, (int)sys_strlen(s_dst), (int)sys_strlen(s_dst) + (int)sys_strlen(s_src)+1);
      res = StrCat(s_dst, s_src);
    }
  }
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCat(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
}
break;
case sysTrapStrNCat: {
  // Char *StrNCat(out Char *dst, in Char *src, Int16 n)
  uint32_t dst = ARG32;
  char *s_dst = emupalmos_trap_in(dst, trap, 0);
  uint32_t src = ARG32;
  char *s_src = emupalmos_trap_in(src, trap, 1);
  int16_t n = ARG16;
  Char *res = s_dst && s_src ? StrNCat(s_dst, s_src, n) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCat(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst, s_dst, src, s_src, n, r_res);
}
break;
case sysTrapStrLen: {
  // UInt16 StrLen(in Char *src)
  uint32_t src = ARG32;
  char *s_src = emupalmos_trap_in(src, trap, 0);
  UInt16 res = s_src ? StrLen(s_src) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrLen(src=0x%08X [%s]): %d", src, s_src, res);
}
break;
case sysTrapStrCompareAscii: {
  // Int16 StrCompareAscii(in Char *s1, in Char *s2)
  uint32_t s1 = ARG32;
  char *s_s1 = emupalmos_trap_in(s1, trap, 0);
  uint32_t s2 = ARG32;
  char *s_s2 = emupalmos_trap_in(s2, trap, 1);
  Int16 res = s_s1 && s_s2 ? StrCompareAscii(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
}
break;
case sysTrapStrCompare: {
  // Int16 StrCompare(in Char *s1, in Char *s2)
  uint32_t s1 = ARG32;
  char *s_s1 = emupalmos_trap_in(s1, trap, 0);
  uint32_t s2 = ARG32;
  char *s_s2 = emupalmos_trap_in(s2, trap, 1);
  Int16 res = s_s1 && s_s2 ? StrCompare(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
}
break;
case sysTrapStrNCompareAscii: {
  // Int16 StrNCompareAscii(in Char *s1, in Char *s2, Int32 n)
  uint32_t s1 = ARG32;
  char *s_s1 = emupalmos_trap_in(s1, trap, 0);
  uint32_t s2 = ARG32;
  char *s_s2 = emupalmos_trap_in(s2, trap, 1);
  int32_t n = ARG32;
  Int16 res = s_s1 && s_s2 ? StrNCompareAscii(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
}
break;
case sysTrapStrNCompare: {
  // Int16 StrNCompare(in Char *s1, in Char *s2, Int32 n)
  uint32_t s1 = ARG32;
  char *s_s1 = emupalmos_trap_in(s1, trap, 0);
  uint32_t s2 = ARG32;
  char *s_s2 = emupalmos_trap_in(s2, trap, 1);
  int32_t n = ARG32;
  Int16 res = s_s1 && s_s2 ? StrNCompare(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
}
break;
case sysTrapStrCaselessCompare: {
  // Int16 StrCaselessCompare(in Char *s1, in Char *s2)
  uint32_t s1 = ARG32;
  char *s_s1 = emupalmos_trap_in(s1, trap, 0);
  uint32_t s2 = ARG32;
  char *s_s2 = emupalmos_trap_in(s2, trap, 1);
  Int16 res = s_s1 && s_s2 ? StrCaselessCompare(s_s1, s_s2) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
}
break;
case sysTrapStrNCaselessCompare: {
  // Int16 StrNCaselessCompare(in Char *s1, in Char *s2, Int32 n)
  uint32_t s1 = ARG32;
  char *s_s1 = emupalmos_trap_in(s1, trap, 0);
  uint32_t s2 = ARG32;
  char *s_s2 = emupalmos_trap_in(s2, trap, 1);
  int32_t n = ARG32;
  Int16 res = s_s1 && s_s2 ?StrNCaselessCompare(s_s1, s_s2, n) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrNCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
}
break;
case sysTrapStrToLower: {
  // Char *StrToLower(out Char *dst, in Char *src)
  uint32_t dst = ARG32;
  char *s_dst = emupalmos_trap_in(dst, trap, 0);
  uint32_t src = ARG32;
  char *s_src = emupalmos_trap_in(src, trap, 1);
  Char *res = s_dst && s_src ? StrToLower(s_dst, s_src) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrToLower(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
}
break;
case sysTrapStrIToA: {
  // Char *StrIToA(out Char *s, Int32 i)
  uint32_t s = ARG32;
  char *s_s = emupalmos_trap_in(s, trap, 0);
  int32_t i = ARG32;
  Char *res = s_s ? StrIToA(s_s, i) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrIToA(s=0x%08X [%s], i=%d): 0x%08X", s, s_s, i, r_res);
}
break;
case sysTrapStrIToH: {
  // Char *StrIToH(out Char *s, UInt32 i)
  uint32_t s = ARG32;
  char *s_s = emupalmos_trap_in(s, trap, 0);
  uint32_t i = ARG32;
  Char *res = s_s ? StrIToH(s_s, i) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrIToH(s=0x%08X [%s], i=%d): 0x%08X", s, s_s, i, r_res);
}
break;
case sysTrapStrLocalizeNumber: {
  // Char *StrLocalizeNumber(out Char *s, Char thousandSeparator, Char decimalSeparator)
  uint32_t s = ARG32;
  char *s_s = emupalmos_trap_in(s, trap, 0);
  int8_t thousandSeparator = ARG8;
  int8_t decimalSeparator = ARG8;
  Char *res = s_s ? StrLocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrLocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s, s_s, thousandSeparator, decimalSeparator, r_res);
}
break;
case sysTrapStrDelocalizeNumber: {
  // Char *StrDelocalizeNumber(out Char *s, Char thousandSeparator, Char decimalSeparator)
  uint32_t s = ARG32;
  char *s_s = emupalmos_trap_in(s, trap, 0);
  int8_t thousandSeparator = ARG8;
  int8_t decimalSeparator = ARG8;
  Char *res = s_s ? StrDelocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrDelocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s, s_s, thousandSeparator, decimalSeparator, r_res);
}
break;
case sysTrapStrChr: {
  // Char *StrChr(in Char *str, WChar chr)
  uint32_t str = ARG32;
  char *s_str = emupalmos_trap_in(str, trap, 0);
  uint16_t chr = ARG16;
  Char *res = s_str ? StrChr(s_str, chr) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrChr(str=0x%08X [%s], chr=%d): 0x%08X", str, s_str, chr, r_res);
}
break;
case sysTrapStrStr: {
  // Char *StrStr(in Char *str, in Char *token)
  uint32_t str = ARG32;
  char *s_str = emupalmos_trap_in(str, trap, 0);
  uint32_t token = ARG32;
  char *s_token = emupalmos_trap_in(token, trap, 1);
  Char *res = s_str && s_token ? StrStr(s_str, s_token) : NULL;
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrStr(str=0x%08X [%s], token=0x%08X [%s]): 0x%08X", str, s_str, token, s_token, r_res);
}
break;
case sysTrapStrAToI: {
  // Int32 StrAToI(in Char *str)
  uint32_t str = ARG32;
  char *s_str = emupalmos_trap_in(str, trap, 0);
  Int32 res = s_str ? StrAToI(s_str) : 0;
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "StrAToI(str=0x%08X [%s]): %d", str, s_str, res);
}
break;
case sysTrapFldCopy: {
  // void FldCopy(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldCopy(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCopy(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldCut: {
  // void FldCut(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldCut(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCut(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldDrawField: {
  // void FldDrawField(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldDrawField(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldDrawField(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldEraseField: {
  // void FldEraseField(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldEraseField(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldEraseField(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldFreeMemory: {
  // void FldFreeMemory(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldFreeMemory(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldFreeMemory(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldGetBounds: {
  // void FldGetBounds(in FieldType *fldP, out RectangleType *rect)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t rect = ARG32;
  RectangleType l_rect;
  FldGetBounds(fldP ? s_fldP : NULL, rect ? &l_rect : NULL);
  encode_rectangle(rect, &l_rect);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetBounds(fldP=0x%08X, rect=0x%08X [%d,%d,%d,%d])", fldP, rect, l_rect.topLeft.x, l_rect.topLeft.y, l_rect.extent.x, l_rect.extent.y);
}
break;
case sysTrapFldGetFont: {
  // FontID FldGetFont(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FontID res = FldGetFont(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetFont(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldGetSelection: {
  // void FldGetSelection(in FieldType *fldP, out UInt16 *startPosition, out UInt16 *endPosition)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t startPosition = ARG32;
  UInt16 l_startPosition = 0;
  uint32_t endPosition = ARG32;
  UInt16 l_endPosition = 0;
  FldGetSelection(fldP ? s_fldP : NULL, startPosition ? &l_startPosition : NULL, endPosition ? &l_endPosition : NULL);
  if (startPosition) m68k_write_memory_16(startPosition, l_startPosition);
  if (endPosition) m68k_write_memory_16(endPosition, l_endPosition);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetSelection(fldP=0x%08X, startPosition=0x%08X [%d], endPosition=0x%08X [%d])", fldP, startPosition, l_startPosition, endPosition, l_endPosition);
}
break;
case sysTrapFldGetTextHandle: {
  // MemHandle FldGetTextHandle(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  MemHandle res = FldGetTextHandle(fldP ? s_fldP : NULL);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextHandle(fldP=0x%08X): 0x%08X", fldP, r_res);
}
break;
case sysTrapFldGetTextPtr: {
  // Char *FldGetTextPtr(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  Char *res = FldGetTextPtr(fldP ? s_fldP : NULL);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextPtr(fldP=0x%08X): 0x%08X", fldP, r_res);
}
break;
case sysTrapFldHandleEvent: {
  // Boolean FldHandleEvent(in FieldType *fldP, in EventType *eventP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t eventP = ARG32;
  EventType l_eventP;
  decode_event(eventP, &l_eventP);
  Boolean res = FldHandleEvent(fldP ? s_fldP : NULL, eventP ? &l_eventP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldHandleEvent(fldP=0x%08X, eventP=0x%08X): %d", fldP, eventP, res);
}
break;
case sysTrapFldPaste: {
  // void FldPaste(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldPaste(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldPaste(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldRecalculateField: {
  // void FldRecalculateField(in FieldType *fldP, Boolean redraw)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint8_t redraw = ARG8;
  FldRecalculateField(fldP ? s_fldP : NULL, redraw);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldRecalculateField(fldP=0x%08X, redraw=%d)", fldP, redraw);
}
break;
case sysTrapFldSetBounds: {
  // void FldSetBounds(in FieldType *fldP, in RectangleType *rP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  FldSetBounds(fldP ? s_fldP : NULL, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetBounds(fldP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", fldP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapFldSetFont: {
  // void FldSetFont(in FieldType *fldP, FontID fontID)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint8_t fontID = ARG8;
  FldSetFont(fldP ? s_fldP : NULL, fontID);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetFont(fldP=0x%08X, fontID=%d)", fldP, fontID);
}
break;
case sysTrapFldSetText: {
  // void FldSetText(in FieldType *fldP, MemHandle textHandle, UInt16 offset, UInt16 size)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t textHandle = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  MemHandle l_textHandle = textHandle ? ram + textHandle : NULL;
  uint16_t offset = ARG16;
  uint16_t size = ARG16;
  FldSetText(fldP ? s_fldP : NULL, textHandle ? l_textHandle : 0, offset, size);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetText(fldP=0x%08X, textHandle=0x%08X, offset=%d, size=%d)", fldP, textHandle, offset, size);
}
break;
case sysTrapFldSetTextHandle: {
  // void FldSetTextHandle(in FieldType *fldP, MemHandle textHandle)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t textHandle = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  MemHandle l_textHandle = textHandle ? ram + textHandle : NULL;
  FldSetTextHandle(fldP ? s_fldP : NULL, textHandle ? l_textHandle : 0);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextHandle(fldP=0x%08X, textHandle=0x%08X)", fldP, textHandle);
}
break;
case sysTrapFldSetTextPtr: {
  // void FldSetTextPtr(in FieldType *fldP, in Char *textP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t textP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_textP = textP ? (char *)(ram + textP) : NULL;
  FldSetTextPtr(fldP ? s_fldP : NULL, textP ? s_textP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextPtr(fldP=0x%08X, textP=0x%08X [%s])", fldP, textP, s_textP);
}
break;
case sysTrapFldSetUsable: {
  // void FldSetUsable(in FieldType *fldP, Boolean usable)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint8_t usable = ARG8;
  FldSetUsable(fldP ? s_fldP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetUsable(fldP=0x%08X, usable=%d)", fldP, usable);
}
break;
case sysTrapFldSetSelection: {
  // void FldSetSelection(in FieldType *fldP, UInt16 startPosition, UInt16 endPosition)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t startPosition = ARG16;
  uint16_t endPosition = ARG16;
  FldSetSelection(fldP ? s_fldP : NULL, startPosition, endPosition);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetSelection(fldP=0x%08X, startPosition=%d, endPosition=%d)", fldP, startPosition, endPosition);
}
break;
case sysTrapFldGrabFocus: {
  // void FldGrabFocus(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldGrabFocus(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGrabFocus(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldReleaseFocus: {
  // void FldReleaseFocus(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldReleaseFocus(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldReleaseFocus(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldGetInsPtPosition: {
  // UInt16 FldGetInsPtPosition(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetInsPtPosition(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetInsPtPosition(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetInsPtPosition: {
  // void FldSetInsPtPosition(in FieldType *fldP, UInt16 pos)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t pos = ARG16;
  FldSetInsPtPosition(fldP ? s_fldP : NULL, pos);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetInsPtPosition(fldP=0x%08X, pos=%d)", fldP, pos);
}
break;
case sysTrapFldSetInsertionPoint: {
  // void FldSetInsertionPoint(in FieldType *fldP, UInt16 pos)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t pos = ARG16;
  FldSetInsertionPoint(fldP ? s_fldP : NULL, pos);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetInsertionPoint(fldP=0x%08X, pos=%d)", fldP, pos);
}
break;
case sysTrapFldGetScrollPosition: {
  // UInt16 FldGetScrollPosition(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetScrollPosition(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetScrollPosition(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetScrollPosition: {
  // void FldSetScrollPosition(in FieldType *fldP, UInt16 pos)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t pos = ARG16;
  FldSetScrollPosition(fldP ? s_fldP : NULL, pos);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetScrollPosition(fldP=0x%08X, pos=%d)", fldP, pos);
}
break;
case sysTrapFldGetScrollValues: {
  // void FldGetScrollValues(in FieldType *fldP, out UInt16 *scrollPosP, out UInt16 *textHeightP, out UInt16 *fieldHeightP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t scrollPosP = ARG32;
  UInt16 l_scrollPosP = 0;
  uint32_t textHeightP = ARG32;
  UInt16 l_textHeightP = 0;
  uint32_t fieldHeightP = ARG32;
  UInt16 l_fieldHeightP = 0;
  FldGetScrollValues(fldP ? s_fldP : NULL, scrollPosP ? &l_scrollPosP : NULL, textHeightP ? &l_textHeightP : NULL, fieldHeightP ? &l_fieldHeightP : NULL);
  if (scrollPosP) m68k_write_memory_16(scrollPosP, l_scrollPosP);
  if (textHeightP) m68k_write_memory_16(textHeightP, l_textHeightP);
  if (fieldHeightP) m68k_write_memory_16(fieldHeightP, l_fieldHeightP);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetScrollValues(fldP=0x%08X, scrollPosP=0x%08X [%d], textHeightP=0x%08X [%d], fieldHeightP=0x%08X [%d])", fldP, scrollPosP, l_scrollPosP, textHeightP, l_textHeightP, fieldHeightP, l_fieldHeightP);
}
break;
case sysTrapFldGetTextLength: {
  // UInt16 FldGetTextLength(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetTextLength(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextLength(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldScrollField: {
  // void FldScrollField(in FieldType *fldP, UInt16 linesToScroll, WinDirectionType direction)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t linesToScroll = ARG16;
  uint8_t direction = ARG8;
  FldScrollField(fldP ? s_fldP : NULL, linesToScroll, direction);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldScrollField(fldP=0x%08X, linesToScroll=%d, direction=%d)", fldP, linesToScroll, direction);
}
break;
case sysTrapFldScrollable: {
  // Boolean FldScrollable(in FieldType *fldP, WinDirectionType direction)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint8_t direction = ARG8;
  Boolean res = FldScrollable(fldP ? s_fldP : NULL, direction);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldScrollable(fldP=0x%08X, direction=%d): %d", fldP, direction, res);
}
break;
case sysTrapFldGetVisibleLines: {
  // UInt16 FldGetVisibleLines(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetVisibleLines(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetVisibleLines(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldGetTextHeight: {
  // UInt16 FldGetTextHeight(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetTextHeight(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextHeight(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldCalcFieldHeight: {
  // UInt16 FldCalcFieldHeight(in Char *chars, UInt16 maxWidth)
  uint32_t chars = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_chars = chars ? (char *)(ram + chars) : NULL;
  uint16_t maxWidth = ARG16;
  UInt16 res = FldCalcFieldHeight(chars ? s_chars : NULL, maxWidth);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCalcFieldHeight(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
}
break;
case sysTrapFldWordWrap: {
  // UInt16 FldWordWrap(in Char *chars, Int16 maxWidth)
  uint32_t chars = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_chars = chars ? (char *)(ram + chars) : NULL;
  int16_t maxWidth = ARG16;
  UInt16 res = FldWordWrap(chars ? s_chars : NULL, maxWidth);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldWordWrap(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
}
break;
case sysTrapFldCompactText: {
  // void FldCompactText(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldCompactText(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldCompactText(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldDirty: {
  // Boolean FldDirty(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  Boolean res = FldDirty(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldDirty(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetDirty: {
  // void FldSetDirty(in FieldType *fldP, Boolean dirty)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint8_t dirty = ARG8;
  FldSetDirty(fldP ? s_fldP : NULL, dirty);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetDirty(fldP=0x%08X, dirty=%d)", fldP, dirty);
}
break;
case sysTrapFldGetMaxChars: {
  // UInt16 FldGetMaxChars(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetMaxChars(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetMaxChars(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetMaxChars: {
  // void FldSetMaxChars(in FieldType *fldP, UInt16 maxChars)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t maxChars = ARG16;
  FldSetMaxChars(fldP ? s_fldP : NULL, maxChars);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetMaxChars(fldP=0x%08X, maxChars=%d)", fldP, maxChars);
}
break;
case sysTrapFldInsert: {
  // Boolean FldInsert(in FieldType *fldP, in Char *insertChars, UInt16 insertLen)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t insertChars = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_insertChars = insertChars ? (char *)(ram + insertChars) : NULL;
  uint16_t insertLen = ARG16;
  Boolean res = FldInsert(fldP ? s_fldP : NULL, insertChars ? s_insertChars : NULL, insertLen);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldInsert(fldP=0x%08X, insertChars=0x%08X [%s], insertLen=%d): %d", fldP, insertChars, s_insertChars, insertLen, res);
}
break;
case sysTrapFldDelete: {
  // void FldDelete(in FieldType *fldP, UInt16 start, UInt16 end)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t start = ARG16;
  uint16_t end = ARG16;
  FldDelete(fldP ? s_fldP : NULL, start, end);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldDelete(fldP=0x%08X, start=%d, end=%d)", fldP, start, end);
}
break;
case sysTrapFldUndo: {
  // void FldUndo(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldUndo(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldUndo(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldGetTextAllocatedSize: {
  // UInt16 FldGetTextAllocatedSize(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetTextAllocatedSize(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextAllocatedSize(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetTextAllocatedSize: {
  // void FldSetTextAllocatedSize(in FieldType *fldP, UInt16 allocatedSize)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t allocatedSize = ARG16;
  FldSetTextAllocatedSize(fldP ? s_fldP : NULL, allocatedSize);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextAllocatedSize(fldP=0x%08X, allocatedSize=%d)", fldP, allocatedSize);
}
break;
case sysTrapFldGetAttributes: {
  // void FldGetAttributes(in FieldType *fldP, in FieldAttrType *attrP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t attrP = ARG32;
  UInt16 attrBits;
  FieldAttrType attrFields;
  FldGetAttributes(fldP ? s_fldP : NULL, attrP ? &attrFields : NULL);
  if (attrP) {
    attrBits = 0;
    if (attrFields.usable)       attrBits |= 0x8000;
    if (attrFields.visible)      attrBits |= 0x4000;
    if (attrFields.editable)     attrBits |= 0x2000;
    if (attrFields.singleLine)   attrBits |= 0x1000;
    if (attrFields.hasFocus)     attrBits |= 0x0800;
    if (attrFields.dynamicSize)  attrBits |= 0x0400;
    if (attrFields.insPtVisible) attrBits |= 0x0200;
    if (attrFields.dirty)        attrBits |= 0x0100;
    attrBits |= attrFields.underlined    << 6;
    attrBits |= attrFields.justification << 4;
    if (attrFields.autoShift)    attrBits |= 0x0008;
    if (attrFields.hasScrollBar) attrBits |= 0x0004;
    if (attrFields.numeric)      attrBits |= 0x0002;
    if (attrFields.reserved)     attrBits |= 0x0001;
    m68k_write_memory_16(attrP, attrBits);
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetAttributes(fldP=0x%08X, attrP=0x%08X)", fldP, attrP);
}
break;
case sysTrapFldSetAttributes: {
  // void FldSetAttributes(in FieldType *fldP, in FieldAttrType *attrP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t attrP = ARG32;
  UInt16 attrBits;
  FieldAttrType attrFields;
  attrBits = attrP ? m68k_read_memory_16(attrP) : 0;
  if (attrP) {
    attrFields.usable        = (attrBits & 0x8000) ? 1 : 0;
    attrFields.visible       = (attrBits & 0x4000) ? 1 : 0;
    attrFields.editable      = (attrBits & 0x2000) ? 1 : 0;
    attrFields.singleLine    = (attrBits & 0x1000) ? 1 : 0;
    attrFields.hasFocus      = (attrBits & 0x0800) ? 1 : 0;
    attrFields.dynamicSize   = (attrBits & 0x0400) ? 1 : 0;
    attrFields.insPtVisible  = (attrBits & 0x0200) ? 1 : 0;
    attrFields.dirty         = (attrBits & 0x0100) ? 1 : 0;
    attrFields.underlined    = (attrBits & 0x00c0) >> 6;
    attrFields.justification = (attrBits & 0x0030) >> 4;
    attrFields.autoShift     = (attrBits & 0x0008) ? 1 : 0;
    attrFields.hasScrollBar  = (attrBits & 0x0004) ? 1 : 0;
    attrFields.numeric       = (attrBits & 0x0002) ? 1 : 0;
    attrFields.reserved      = (attrBits & 0x0001) ? 1 : 0;
  }
  FldSetAttributes(fldP ? s_fldP : NULL, attrP ? &attrFields : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetAttributes(fldP=0x%08X, attrP=0x%08X)", fldP, attrP);
}
break;
case sysTrapFldSendChangeNotification: {
  // void FldSendChangeNotification(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  FldSendChangeNotification(fldP ? s_fldP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSendChangeNotification(fldP=0x%08X)", fldP);
}
break;
case sysTrapFldSendHeightChangeNotification: {
  // void FldSendHeightChangeNotification(in FieldType *fldP, UInt16 pos, Int16 numLines)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint16_t pos = ARG16;
  int16_t numLines = ARG16;
  FldSendHeightChangeNotification(fldP ? s_fldP : NULL, pos, numLines);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSendHeightChangeNotification(fldP=0x%08X, pos=%d, numLines=%d)", fldP, pos, numLines);
}
break;
case sysTrapFldMakeFullyVisible: {
  // Boolean FldMakeFullyVisible(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  Boolean res = FldMakeFullyVisible(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldMakeFullyVisible(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldGetNumberOfBlankLines: {
  // UInt16 FldGetNumberOfBlankLines(in FieldType *fldP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  UInt16 res = FldGetNumberOfBlankLines(fldP ? s_fldP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetNumberOfBlankLines(fldP=0x%08X): %d", fldP, res);
}
break;
case sysTrapFldSetMaxVisibleLines: {
  // void FldSetMaxVisibleLines(in FieldType *fldP, UInt8 maxLines)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint8_t maxLines = ARG8;
  FldSetMaxVisibleLines(fldP ? s_fldP : NULL, maxLines);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldSetMaxVisibleLines(fldP=0x%08X, maxLines=%d)", fldP, maxLines);
}
break;
case sysTrapFldNewField: {
  // FieldType *FldNewField(void **formPP, UInt16 id,
  //   Coord x, Coord y, Coord width, Coord height,
  //   FontID font, UInt32 maxChars, Boolean editable, Boolean underlined,
  //   Boolean singleLine, Boolean dynamicSize, JustificationType justification,
  //   Boolean autoShift, Boolean hasScrollBar, Boolean numeric)
  uint32_t formPP = ARG32;
  uint16_t id = ARG16;
  int16_t x = ARG16;
  int16_t y = ARG16;
  int16_t width = ARG16;
  int16_t height = ARG16;
  uint8_t font = ARG8;
  uint32_t maxChars = ARG32;
  uint8_t editable = ARG8;
  uint8_t underlined = ARG8;
  uint8_t singleLine = ARG8;
  uint8_t dynamicSize = ARG8;
  uint8_t justification = ARG8;
  uint8_t autoShift = ARG8;
  uint8_t hasScrollBar = ARG8;
  uint8_t numeric = ARG8;
  uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
  void *form = emupalmos_trap_in(formP, trap, 0);
  FieldType *fld = FldNewField(&form, id, x, y, width, height,
    font, maxChars, editable, underlined,
    singleLine, dynamicSize, justification,
    autoShift, hasScrollBar, numeric);
  uint32_t a = emupalmos_trap_out(fld);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldNewField(0x%08X, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X",
    formPP, id, x, y, width, height, font, maxChars,
    editable, underlined, singleLine, dynamicSize, justification, autoShift, hasScrollBar, numeric, a);
  m68k_set_reg(M68K_REG_A0, a);
}
break;
case sysTrapTblDrawTable: {
  // void TblDrawTable(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblDrawTable(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblDrawTable(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblRedrawTable: {
  // void TblRedrawTable(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblRedrawTable(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRedrawTable(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblEraseTable: {
  // void TblEraseTable(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblEraseTable(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblEraseTable(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblHandleEvent: {
  // Boolean TblHandleEvent(in TableType *tableP, in EventType *event)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  uint32_t event = ARG32;
  EventType l_event;
  decode_event(event, &l_event);
  Boolean res = TblHandleEvent(tableP ? s_tableP : NULL, event ? &l_event : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblHandleEvent(tableP=0x%08X, event=0x%08X): %d", tableP, event, res);
}
break;
case sysTrapTblGetItemBounds: {
  // void TblGetItemBounds(in TableType *tableP, Int16 row, Int16 column, out RectangleType *rP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  uint32_t rP = ARG32;
  RectangleType l_rP;
  TblGetItemBounds(tableP ? s_tableP : NULL, row, column, rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemBounds(tableP=0x%08X, row=%d, column=%d, rP=0x%08X [%d,%d,%d,%d])", tableP, row, column, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapTblSelectItem: {
  // void TblSelectItem(in TableType *tableP, Int16 row, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  TblSelectItem(tableP ? s_tableP : NULL, row, column);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSelectItem(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
}
break;
case sysTrapTblGetItemInt: {
  // Int16 TblGetItemInt(in TableType *tableP, Int16 row, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  Int16 res = TblGetItemInt(tableP ? s_tableP : NULL, row, column);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemInt(tableP=0x%08X, row=%d, column=%d): %d", tableP, row, column, res);
}
break;
case sysTrapTblSetItemInt: {
  // void TblSetItemInt(in TableType *tableP, Int16 row, Int16 column, Int16 value)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  int16_t value = ARG16;
  TblSetItemInt(tableP ? s_tableP : NULL, row, column, value);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemInt(tableP=0x%08X, row=%d, column=%d, value=%d)", tableP, row, column, value);
}
break;
case sysTrapTblSetItemPtr: {
  // void TblSetItemPtr(in TableType *tableP, Int16 row, Int16 column, in void *value)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  uint32_t value = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  void *s_value = value ? (void *)(ram + value) : NULL;
  TblSetItemPtr(tableP ? s_tableP : NULL, row, column, value ? s_value : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemPtr(tableP=0x%08X, row=%d, column=%d, value=0x%08X)", tableP, row, column, value);
}
break;
case sysTrapTblSetItemStyle: {
  // void TblSetItemStyle(in TableType *tableP, Int16 row, Int16 column, TableItemStyleType type)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  uint8_t type = ARG8;
  TblSetItemStyle(tableP ? s_tableP : NULL, row, column, type);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemStyle(tableP=0x%08X, row=%d, column=%d, type=%d)", tableP, row, column, type);
}
break;
case sysTrapTblUnhighlightSelection: {
  // void TblUnhighlightSelection(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblUnhighlightSelection(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblUnhighlightSelection(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblRowUsable: {
  // Boolean TblRowUsable(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  Boolean res = TblRowUsable(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowUsable(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblSetRowUsable: {
  // void TblSetRowUsable(in TableType *tableP, Int16 row, Boolean usable)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint8_t usable = ARG8;
  TblSetRowUsable(tableP ? s_tableP : NULL, row, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowUsable(tableP=0x%08X, row=%d, usable=%d)", tableP, row, usable);
}
break;
case sysTrapTblGetLastUsableRow: {
  // Int16 TblGetLastUsableRow(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  Int16 res = TblGetLastUsableRow(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetLastUsableRow(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblSetColumnUsable: {
  // void TblSetColumnUsable(in TableType *tableP, Int16 column, Boolean usable)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t column = ARG16;
  uint8_t usable = ARG8;
  TblSetColumnUsable(tableP ? s_tableP : NULL, column, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnUsable(tableP=0x%08X, column=%d, usable=%d)", tableP, column, usable);
}
break;
case sysTrapTblSetRowSelectable: {
  // void TblSetRowSelectable(in TableType *tableP, Int16 row, Boolean selectable)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint8_t selectable = ARG8;
  TblSetRowSelectable(tableP ? s_tableP : NULL, row, selectable);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowSelectable(tableP=0x%08X, row=%d, selectable=%d)", tableP, row, selectable);
}
break;
case sysTrapTblRowSelectable: {
  // Boolean TblRowSelectable(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  Boolean res = TblRowSelectable(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowSelectable(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblGetNumberOfRows: {
  // Int16 TblGetNumberOfRows(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  Int16 res = TblGetNumberOfRows(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetNumberOfRows(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblGetBounds: {
  // void TblGetBounds(in TableType *tableP, out RectangleType *rP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  uint32_t rP = ARG32;
  RectangleType l_rP;
  TblGetBounds(tableP ? s_tableP : NULL, rP ? &l_rP : NULL);
  encode_rectangle(rP, &l_rP);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetBounds(tableP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", tableP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapTblSetBounds: {
  // void TblSetBounds(in TableType *tableP, in RectangleType *rP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  uint32_t rP = ARG32;
  RectangleType l_rP;
  decode_rectangle(rP, &l_rP);
  TblSetBounds(tableP ? s_tableP : NULL, rP ? &l_rP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetBounds(tableP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", tableP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
}
break;
case sysTrapTblGetRowHeight: {
  // Coord TblGetRowHeight(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  Coord res = TblGetRowHeight(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowHeight(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblSetRowHeight: {
  // void TblSetRowHeight(in TableType *tableP, Int16 row, Coord height)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t height = ARG16;
  TblSetRowHeight(tableP ? s_tableP : NULL, row, height);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowHeight(tableP=0x%08X, row=%d, height=%d)", tableP, row, height);
}
break;
case sysTrapTblGetColumnWidth: {
  // Coord TblGetColumnWidth(in TableType *tableP, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t column = ARG16;
  Coord res = TblGetColumnWidth(tableP ? s_tableP : NULL, column);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetColumnWidth(tableP=0x%08X, column=%d): %d", tableP, column, res);
}
break;
case sysTrapTblSetColumnWidth: {
  // void TblSetColumnWidth(in TableType *tableP, Int16 column, Coord width)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t column = ARG16;
  int16_t width = ARG16;
  TblSetColumnWidth(tableP ? s_tableP : NULL, column, width);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnWidth(tableP=0x%08X, column=%d, width=%d)", tableP, column, width);
}
break;
case sysTrapTblGetColumnSpacing: {
  // Coord TblGetColumnSpacing(in TableType *tableP, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t column = ARG16;
  Coord res = TblGetColumnSpacing(tableP ? s_tableP : NULL, column);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetColumnSpacing(tableP=0x%08X, column=%d): %d", tableP, column, res);
}
break;
case sysTrapTblSetColumnSpacing: {
  // void TblSetColumnSpacing(in TableType *tableP, Int16 column, Coord spacing)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t column = ARG16;
  int16_t spacing = ARG16;
  TblSetColumnSpacing(tableP ? s_tableP : NULL, column, spacing);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnSpacing(tableP=0x%08X, column=%d, spacing=%d)", tableP, column, spacing);
}
break;
case sysTrapTblFindRowID: {
  // Boolean TblFindRowID(in TableType *tableP, UInt16 id, out Int16 *rowP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  uint16_t id = ARG16;
  uint32_t rowP = ARG32;
  Int16 l_rowP = 0;
  Boolean res = TblFindRowID(tableP ? s_tableP : NULL, id, rowP ? &l_rowP : NULL);
  if (rowP) m68k_write_memory_16(rowP, l_rowP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblFindRowID(tableP=0x%08X, id=%d, rowP=0x%08X [%d]): %d", tableP, id, rowP, l_rowP, res);
}
break;
case sysTrapTblFindRowData: {
  // Boolean TblFindRowData(in TableType *tableP, UInt32 data, out Int16 *rowP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  uint32_t data = ARG32;
  uint32_t rowP = ARG32;
  Int16 l_rowP = 0;
  Boolean res = TblFindRowData(tableP ? s_tableP : NULL, data, rowP ? &l_rowP : NULL);
  if (rowP) m68k_write_memory_16(rowP, l_rowP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblFindRowData(tableP=0x%08X, data=%d, rowP=0x%08X [%d]): %d", tableP, data, rowP, l_rowP, res);
}
break;
case sysTrapTblGetRowID: {
  // UInt16 TblGetRowID(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  UInt16 res = TblGetRowID(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowID(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblSetRowID: {
  // void TblSetRowID(in TableType *tableP, Int16 row, UInt16 id)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint16_t id = ARG16;
  TblSetRowID(tableP ? s_tableP : NULL, row, id);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowID(tableP=0x%08X, row=%d, id=%d)", tableP, row, id);
}
break;
case sysTrapTblGetRowData: {
  // UInt32 TblGetRowData(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint32_t res = TblGetRowData(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetRowData(tableP=0x%08X, row=%d): 0x%08X", tableP, row, res);
}
break;
case sysTrapTblSetRowData: {
  // void TblSetRowData(in TableType *tableP, Int16 row, UInt32 data)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint32_t data = ARG32;
  TblSetRowData(tableP ? s_tableP : NULL, row, data);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowData(tableP=0x%08X, row=%d, data=0x%08X)", tableP, row, data);
}
break;
case sysTrapTblRowInvalid: {
  // Boolean TblRowInvalid(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  Boolean res = TblRowInvalid(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowInvalid(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblMarkRowInvalid: {
  // void TblMarkRowInvalid(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  TblMarkRowInvalid(tableP ? s_tableP : NULL, row);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblMarkRowInvalid(tableP=0x%08X, row=%d)", tableP, row);
}
break;
case sysTrapTblMarkTableInvalid: {
  // void TblMarkTableInvalid(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblMarkTableInvalid(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblMarkTableInvalid(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblGetSelection: {
  // Boolean TblGetSelection(in TableType *tableP, out Int16 *rowP, out Int16 *columnP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  uint32_t rowP = ARG32;
  Int16 l_rowP = 0;
  uint32_t columnP = ARG32;
  Int16 l_columnP = 0;
  Boolean res = TblGetSelection(tableP ? s_tableP : NULL, rowP ? &l_rowP : NULL, columnP ? &l_columnP : NULL);
  if (rowP) m68k_write_memory_16(rowP, l_rowP);
  if (columnP) m68k_write_memory_16(columnP, l_columnP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetSelection(tableP=0x%08X, rowP=0x%08X [%d], columnP=0x%08X [%d]): %d", tableP, rowP, l_rowP, columnP, l_columnP, res);
}
break;
case sysTrapTblInsertRow: {
  // void TblInsertRow(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  TblInsertRow(tableP ? s_tableP : NULL, row);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblInsertRow(tableP=0x%08X, row=%d)", tableP, row);
}
break;
case sysTrapTblRemoveRow: {
  // void TblRemoveRow(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  TblRemoveRow(tableP ? s_tableP : NULL, row);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRemoveRow(tableP=0x%08X, row=%d)", tableP, row);
}
break;
case sysTrapTblReleaseFocus: {
  // void TblReleaseFocus(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  TblReleaseFocus(tableP ? s_tableP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblReleaseFocus(tableP=0x%08X)", tableP);
}
break;
case sysTrapTblEditing: {
  // Boolean TblEditing(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  Boolean res = TblEditing(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblEditing(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblGetCurrentField: {
  // FieldType *TblGetCurrentField(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  FieldType *res = TblGetCurrentField(tableP ? s_tableP : NULL);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetCurrentField(tableP=0x%08X): 0x%08X", tableP, r_res);
}
break;
case sysTrapTblGrabFocus: {
  // void TblGrabFocus(in TableType *tableP, Int16 row, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  TblGrabFocus(tableP ? s_tableP : NULL, row, column);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGrabFocus(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
}
break;
case sysTrapTblSetColumnEditIndicator: {
  // void TblSetColumnEditIndicator(in TableType *tableP, Int16 column, Boolean editIndicator)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t column = ARG16;
  uint8_t editIndicator = ARG8;
  TblSetColumnEditIndicator(tableP ? s_tableP : NULL, column, editIndicator);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnEditIndicator(tableP=0x%08X, column=%d, editIndicator=%d)", tableP, column, editIndicator);
}
break;
case sysTrapTblSetRowStaticHeight: {
  // void TblSetRowStaticHeight(in TableType *tableP, Int16 row, Boolean staticHeight)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint8_t staticHeight = ARG8;
  TblSetRowStaticHeight(tableP ? s_tableP : NULL, row, staticHeight);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowStaticHeight(tableP=0x%08X, row=%d, staticHeight=%d)", tableP, row, staticHeight);
}
break;
case sysTrapTblHasScrollBar: {
  // void TblHasScrollBar(in TableType *tableP, Boolean hasScrollBar)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  uint8_t hasScrollBar = ARG8;
  TblHasScrollBar(tableP ? s_tableP : NULL, hasScrollBar);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblHasScrollBar(tableP=0x%08X, hasScrollBar=%d)", tableP, hasScrollBar);
}
break;
case sysTrapTblGetItemFont: {
  // FontID TblGetItemFont(in TableType *tableP, Int16 row, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  FontID res = TblGetItemFont(tableP ? s_tableP : NULL, row, column);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemFont(tableP=0x%08X, row=%d, column=%d): %d", tableP, row, column, res);
}
break;
case sysTrapTblSetItemFont: {
  // void TblSetItemFont(in TableType *tableP, Int16 row, Int16 column, FontID fontID)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  uint8_t fontID = ARG8;
  TblSetItemFont(tableP ? s_tableP : NULL, row, column, fontID);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetItemFont(tableP=0x%08X, row=%d, column=%d, fontID=%d)", tableP, row, column, fontID);
}
break;
case sysTrapTblGetItemPtr: {
  // void *TblGetItemPtr(in TableType *tableP, Int16 row, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  void *res = TblGetItemPtr(tableP ? s_tableP : NULL, row, column);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetItemPtr(tableP=0x%08X, row=%d, column=%d): 0x%08X", tableP, row, column, r_res);
}
break;
case sysTrapTblRowMasked: {
  // Boolean TblRowMasked(in TableType *tableP, Int16 row)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  Boolean res = TblRowMasked(tableP ? s_tableP : NULL, row);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblRowMasked(tableP=0x%08X, row=%d): %d", tableP, row, res);
}
break;
case sysTrapTblSetRowMasked: {
  // void TblSetRowMasked(in TableType *tableP, Int16 row, Boolean masked)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint8_t masked = ARG8;
  TblSetRowMasked(tableP ? s_tableP : NULL, row, masked);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetRowMasked(tableP=0x%08X, row=%d, masked=%d)", tableP, row, masked);
}
break;
case sysTrapTblSetColumnMasked: {
  // void TblSetColumnMasked(in TableType *tableP, Int16 column, Boolean masked)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t column = ARG16;
  uint8_t masked = ARG8;
  TblSetColumnMasked(tableP ? s_tableP : NULL, column, masked);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetColumnMasked(tableP=0x%08X, column=%d, masked=%d)", tableP, column, masked);
}
break;
case sysTrapTblGetNumberOfColumns: {
  // Int16 TblGetNumberOfColumns(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  Int16 res = TblGetNumberOfColumns(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetNumberOfColumns(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblGetTopRow: {
  // Int16 TblGetTopRow(in TableType *tableP)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  Int16 res = TblGetTopRow(tableP ? s_tableP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblGetTopRow(tableP=0x%08X): %d", tableP, res);
}
break;
case sysTrapTblSetSelection: {
  // void TblSetSelection(in TableType *tableP, Int16 row, Int16 column)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  int16_t column = ARG16;
  TblSetSelection(tableP ? s_tableP : NULL, row, column);
  debug(DEBUG_TRACE, "EmuPalmOS", "TblSetSelection(tableP=0x%08X, row=%d, column=%d)", tableP, row, column);
}
break;
case sysTrapLstEraseList: {
  // void LstEraseList(in ListType *listP)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  LstEraseList(listP ? s_listP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstEraseList(listP=0x%08X)", listP);
}
break;
case sysTrapLstGetSelection: {
  // Int16 LstGetSelection(in ListType *listP)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  Int16 res = LstGetSelection(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetSelection(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstGetSelectionText: {
  // Char *LstGetSelectionText(in ListType *listP, Int16 itemNum)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  int16_t itemNum = ARG16;
  Char *res = LstGetSelectionText(listP ? s_listP : NULL, itemNum);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetSelectionText(listP=0x%08X, itemNum=%d): 0x%08X", listP, itemNum, r_res);
}
break;
case sysTrapLstHandleEvent: {
  // Boolean LstHandleEvent(in ListType *listP, in EventType *eventP)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  uint32_t eventP = ARG32;
  EventType l_eventP;
  decode_event(eventP, &l_eventP);
  Boolean res = LstHandleEvent(listP ? s_listP : NULL, eventP ? &l_eventP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstHandleEvent(listP=0x%08X, eventP=0x%08X): %d", listP, eventP, res);
}
break;
case sysTrapLstSetHeight: {
  // void LstSetHeight(in ListType *listP, Int16 visibleItems)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  int16_t visibleItems = ARG16;
  LstSetHeight(listP ? s_listP : NULL, visibleItems);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetHeight(listP=0x%08X, visibleItems=%d)", listP, visibleItems);
}
break;
case sysTrapLstSetPosition: {
  // void LstSetPosition(in ListType *listP, Coord x, Coord y)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  int16_t x = ARG16;
  int16_t y = ARG16;
  LstSetPosition(listP ? s_listP : NULL, x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetPosition(listP=0x%08X, x=%d, y=%d)", listP, x, y);
}
break;
case sysTrapLstSetSelection: {
  // void LstSetSelection(in ListType *listP, Int16 itemNum)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  int16_t itemNum = ARG16;
  LstSetSelection(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetSelection(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstSetListChoices: {
  // void LstSetListChoices(in ListType *listP, in Char **itemsText, Int16 numItems)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  uint32_t itemsText = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char **s_itemsText = itemsText ? (char **)(ram + itemsText) : NULL;
  int16_t numItems = ARG16;
  LstSetListChoices(listP ? s_listP : NULL, itemsText ? s_itemsText : NULL, numItems);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetListChoices(listP=0x%08X, itemsText=0x%08X, numItems=%d)", listP, itemsText, numItems);
}
break;
case sysTrapLstSetTopItem: {
  // void LstSetTopItem(in ListType *listP, Int16 itemNum)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  int16_t itemNum = ARG16;
  LstSetTopItem(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstSetTopItem(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstMakeItemVisible: {
  // void LstMakeItemVisible(in ListType *listP, Int16 itemNum)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  int16_t itemNum = ARG16;
  LstMakeItemVisible(listP ? s_listP : NULL, itemNum);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstMakeItemVisible(listP=0x%08X, itemNum=%d)", listP, itemNum);
}
break;
case sysTrapLstGetNumberOfItems: {
  // Int16 LstGetNumberOfItems(in ListType *listP)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  Int16 res = LstGetNumberOfItems(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetNumberOfItems(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstPopupList: {
  // Int16 LstPopupList(in ListType *listP)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  Int16 res = LstPopupList(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstPopupList(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstScrollList: {
  // Boolean LstScrollList(in ListType *listP, WinDirectionType direction, Int16 itemCount)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  uint8_t direction = ARG8;
  int16_t itemCount = ARG16;
  Boolean res = LstScrollList(listP ? s_listP : NULL, direction, itemCount);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstScrollList(listP=0x%08X, direction=%d, itemCount=%d): %d", listP, direction, itemCount, res);
}
break;
case sysTrapLstGetVisibleItems: {
  // Int16 LstGetVisibleItems(in ListType *listP)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  Int16 res = LstGetVisibleItems(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetVisibleItems(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapLstGetTopItem: {
  // Int16 LstGetTopItem(in ListType *listP)
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  Int16 res = LstGetTopItem(listP ? s_listP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "LstGetTopItem(listP=0x%08X): %d", listP, res);
}
break;
case sysTrapMenuInit: {
  // MenuBarType *MenuInit(UInt16 resourceId)
  uint16_t resourceId = ARG16;
  MenuBarType *res = MenuInit(resourceId);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuInit(resourceId=%d): 0x%08X", resourceId, r_res);
}
break;
case sysTrapMenuGetActiveMenu: {
  // MenuBarType *MenuGetActiveMenu(void)
  MenuBarType *res = MenuGetActiveMenu();
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuGetActiveMenu(): 0x%08X", r_res);
}
break;
case sysTrapMenuSetActiveMenu: {
  // MenuBarType *MenuSetActiveMenu(in MenuBarType *menuP)
  uint32_t menuP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuBarType *res = MenuSetActiveMenu(menuP ? s_menuP : NULL);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuSetActiveMenu(menuP=0x%08X): 0x%08X", menuP, r_res);
}
break;
case sysTrapMenuDispose: {
  // void MenuDispose(in MenuBarType *menuP)
  uint32_t menuP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuDispose(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuDispose(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuHandleEvent: {
  // Boolean MenuHandleEvent(in MenuBarType *menuP, in EventType *event, out UInt16 *error)
  uint32_t menuP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  uint32_t event = ARG32;
  EventType l_event;
  decode_event(event, &l_event);
  uint32_t error = ARG32;
  UInt16 l_error = 0;
  Boolean res = MenuHandleEvent(menuP ? s_menuP : NULL, event ? &l_event : NULL, error ? &l_error : NULL);
  if (error) m68k_write_memory_16(error, l_error);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuHandleEvent(menuP=0x%08X, event=0x%08X, error=0x%08X [%d]): %d", menuP, event, error, l_error, res);
}
break;
case sysTrapMenuDrawMenu: {
  // void MenuDrawMenu(in MenuBarType *menuP)
  uint32_t menuP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuDrawMenu(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuDrawMenu(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuEraseStatus: {
  // void MenuEraseStatus(in MenuBarType *menuP)
  uint32_t menuP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuEraseStatus(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuEraseStatus(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuSetActiveMenuRscID: {
  // void MenuSetActiveMenuRscID(UInt16 resourceId)
  uint16_t resourceId = ARG16;
  MenuSetActiveMenuRscID(resourceId);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuSetActiveMenuRscID(resourceId=%d)", resourceId);
}
break;
case sysTrapMenuCmdBarAddButton: {
  // Err MenuCmdBarAddButton(UInt8 where, UInt16 bitmapId, MenuCmdBarResultType resultType, UInt32 result, in Char *nameP)
  uint8_t where = ARG8;
  uint16_t bitmapId = ARG16;
  uint8_t resultType = ARG8;
  uint32_t result = ARG32;
  uint32_t nameP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  Err res = MenuCmdBarAddButton(where, bitmapId, resultType, result, nameP ? s_nameP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarAddButton(where=%d, bitmapId=%d, resultType=%d, result=%d, nameP=0x%08X [%s]): %d", where, bitmapId, resultType, result, nameP, s_nameP, res);
}
break;
case sysTrapMenuCmdBarGetButtonData: {
  // Boolean MenuCmdBarGetButtonData(Int16 buttonIndex, out UInt16 *bitmapIdP, out MenuCmdBarResultType *resultTypeP, out UInt32 *resultP, out Char *nameP)
  int16_t buttonIndex = ARG16;
  uint32_t bitmapIdP = ARG32;
  UInt16 l_bitmapIdP = 0;
  uint32_t resultTypeP = ARG32;
  MenuCmdBarResultType l_resultTypeP;
  uint32_t resultP = ARG32;
  UInt32 l_resultP = 0;
  uint32_t nameP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  Boolean res = MenuCmdBarGetButtonData(buttonIndex, bitmapIdP ? &l_bitmapIdP : NULL, resultTypeP ? &l_resultTypeP : NULL, resultP ? &l_resultP : NULL, nameP ? s_nameP : NULL);
  if (bitmapIdP) m68k_write_memory_16(bitmapIdP, l_bitmapIdP);
  if (resultTypeP) m68k_write_memory_8(resultTypeP, l_resultTypeP);
  if (resultP) m68k_write_memory_32(resultP, l_resultP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarGetButtonData(buttonIndex=%d, bitmapIdP=0x%08X [%d], resultTypeP=0x%08X, resultP=0x%08X [%d], nameP=0x%08X [%s]): %d", buttonIndex, bitmapIdP, l_bitmapIdP, resultTypeP, resultP, l_resultP, nameP, s_nameP, res);
}
break;
case sysTrapMenuCmdBarDisplay: {
  // void MenuCmdBarDisplay(void)
  MenuCmdBarDisplay();
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuCmdBarDisplay()");
}
break;
case sysTrapMenuShowItem: {
  // Boolean MenuShowItem(UInt16 id)
  uint16_t id = ARG16;
  Boolean res = MenuShowItem(id);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuShowItem(id=%d): %d", id, res);
}
break;
case sysTrapMenuHideItem: {
  // Boolean MenuHideItem(UInt16 id)
  uint16_t id = ARG16;
  Boolean res = MenuHideItem(id);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuHideItem(id=%d): %d", id, res);
}
break;
case sysTrapMenuAddItem: {
  // Err MenuAddItem(UInt16 positionId, UInt16 id, Char cmd, in Char *textP)
  uint16_t positionId = ARG16;
  uint16_t id = ARG16;
  int8_t cmd = ARG8;
  uint32_t textP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_textP = textP ? (char *)(ram + textP) : NULL;
  Err res = MenuAddItem(positionId, id, cmd, textP ? s_textP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuAddItem(positionId=%d, id=%d, cmd=%d, textP=0x%08X [%s]): %d", positionId, id, cmd, textP, s_textP, res);
}
break;
case sysTrapInsPtInitialize: {
  // void InsPtInitialize(void)
  InsPtInitialize();
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtInitialize()");
}
break;
case sysTrapInsPtSetLocation: {
  // void InsPtSetLocation(Int16 x, Int16 y)
  int16_t x = ARG16;
  int16_t y = ARG16;
  InsPtSetLocation(x, y);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtSetLocation(x=%d, y=%d)", x, y);
}
break;
case sysTrapInsPtGetLocation: {
  // void InsPtGetLocation(out Int16 *x, out Int16 *y)
  uint32_t x = ARG32;
  Int16 l_x = 0;
  uint32_t y = ARG32;
  Int16 l_y = 0;
  InsPtGetLocation(x ? &l_x : NULL, y ? &l_y : NULL);
  if (x) m68k_write_memory_16(x, l_x);
  if (y) m68k_write_memory_16(y, l_y);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtGetLocation(x=0x%08X [%d], y=0x%08X [%d])", x, l_x, y, l_y);
}
break;
case sysTrapInsPtEnable: {
  // void InsPtEnable(Boolean enableIt)
  uint8_t enableIt = ARG8;
  InsPtEnable(enableIt);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtEnable(enableIt=%d)", enableIt);
}
break;
case sysTrapInsPtEnabled: {
  // Boolean InsPtEnabled(void)
  Boolean res = InsPtEnabled();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtEnabled(): %d", res);
}
break;
case sysTrapInsPtSetHeight: {
  // void InsPtSetHeight(Int16 height)
  int16_t height = ARG16;
  InsPtSetHeight(height);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtSetHeight(height=%d)", height);
}
break;
case sysTrapInsPtGetHeight: {
  // Int16 InsPtGetHeight(void)
  Int16 res = InsPtGetHeight();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtGetHeight(): %d", res);
}
break;
case sysTrapInsPtCheckBlink: {
  // void InsPtCheckBlink(void)
  InsPtCheckBlink();
  debug(DEBUG_TRACE, "EmuPalmOS", "InsPtCheckBlink()");
}
break;
case sysTrapCtlDrawControl: {
  // void CtlDrawControl(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlDrawControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlDrawControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlEraseControl: {
  // void CtlEraseControl(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlEraseControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlEraseControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlHideControl: {
  // void CtlHideControl(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlHideControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHideControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlShowControl: {
  // void CtlShowControl(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlShowControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlShowControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlEnabled: {
  // Boolean CtlEnabled(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  Boolean res = CtlEnabled(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlEnabled(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapCtlSetEnabled: {
  // void CtlSetEnabled(in ControlType *controlP, Boolean usable)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  uint8_t usable = ARG8;
  CtlSetEnabled(controlP ? s_controlP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetEnabled(controlP=0x%08X, usable=%d)", controlP, usable);
}
break;
case sysTrapCtlSetUsable: {
  // void CtlSetUsable(in ControlType *controlP, Boolean usable)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  uint8_t usable = ARG8;
  CtlSetUsable(controlP ? s_controlP : NULL, usable);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetUsable(controlP=0x%08X, usable=%d)", controlP, usable);
}
break;
case sysTrapCtlGetValue: {
  // Int16 CtlGetValue(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  Int16 res = CtlGetValue(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetValue(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapCtlSetValue: {
  // void CtlSetValue(in ControlType *controlP, Int16 newValue)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  int16_t newValue = ARG16;
  CtlSetValue(controlP ? s_controlP : NULL, newValue);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetValue(controlP=0x%08X, newValue=%d)", controlP, newValue);
}
break;
case sysTrapCtlSetLabel: {
  // void CtlSetLabel(in ControlType *controlP, in Char *newLabel)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  uint32_t newLabel = ARG32;
  char *s_newLabel = emupalmos_trap_in(newLabel, trap, 1);
  CtlSetLabel(controlP ? s_controlP : NULL, newLabel ? s_newLabel : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetLabel(controlP=0x%08X, newLabel=0x%08X [%s])", controlP, newLabel, s_newLabel);
}
break;
case sysTrapCtlSetGraphics: {
  // void CtlSetGraphics(in ControlType *ctlP, DmResID newBitmapID, DmResID newSelectedBitmapID)
  uint32_t ctlP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  uint16_t newBitmapID = ARG16;
  uint16_t newSelectedBitmapID = ARG16;
  CtlSetGraphics(ctlP ? s_ctlP : NULL, newBitmapID, newSelectedBitmapID);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetGraphics(ctlP=0x%08X, newBitmapID=%d, newSelectedBitmapID=%d)", ctlP, newBitmapID, newSelectedBitmapID);
}
break;
case sysTrapCtlSetSliderValues: {
  // void CtlSetSliderValues(in ControlType *ctlP, in UInt16 *minValueP, in UInt16 *maxValueP, in UInt16 *pageSizeP, in UInt16 *valueP)
  uint32_t ctlP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  uint32_t minValueP = ARG32;
  UInt16 l_minValueP = 0;
  if (minValueP) l_minValueP = m68k_read_memory_16(minValueP);
  uint32_t maxValueP = ARG32;
  UInt16 l_maxValueP = 0;
  if (maxValueP) l_maxValueP = m68k_read_memory_16(maxValueP);
  uint32_t pageSizeP = ARG32;
  UInt16 l_pageSizeP = 0;
  if (pageSizeP) l_pageSizeP = m68k_read_memory_16(pageSizeP);
  uint32_t valueP = ARG32;
  UInt16 l_valueP = 0;
  if (valueP) l_valueP = m68k_read_memory_16(valueP);
  CtlSetSliderValues(ctlP ? s_ctlP : NULL, minValueP ? &l_minValueP : NULL, maxValueP ? &l_maxValueP : NULL, pageSizeP ? &l_pageSizeP : NULL, valueP ? &l_valueP : NULL);
  if (minValueP) m68k_write_memory_16(minValueP, l_minValueP);
  if (maxValueP) m68k_write_memory_16(maxValueP, l_maxValueP);
  if (pageSizeP) m68k_write_memory_16(pageSizeP, l_pageSizeP);
  if (valueP) m68k_write_memory_16(valueP, l_valueP);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetSliderValues(ctlP=0x%08X, minValueP=0x%08X [%d], maxValueP=0x%08X [%d], pageSizeP=0x%08X [%d], valueP=0x%08X [%d])", ctlP, minValueP, l_minValueP, maxValueP, l_maxValueP, pageSizeP, l_pageSizeP, valueP, l_valueP);
}
break;
case sysTrapCtlGetSliderValues: {
  // void CtlGetSliderValues(in ControlType *ctlP, out UInt16 *minValueP, out UInt16 *maxValueP, out UInt16 *pageSizeP, out UInt16 *valueP)
  uint32_t ctlP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  uint32_t minValueP = ARG32;
  UInt16 l_minValueP = 0;
  uint32_t maxValueP = ARG32;
  UInt16 l_maxValueP = 0;
  uint32_t pageSizeP = ARG32;
  UInt16 l_pageSizeP = 0;
  uint32_t valueP = ARG32;
  UInt16 l_valueP = 0;
  CtlGetSliderValues(ctlP ? s_ctlP : NULL, minValueP ? &l_minValueP : NULL, maxValueP ? &l_maxValueP : NULL, pageSizeP ? &l_pageSizeP : NULL, valueP ? &l_valueP : NULL);
  if (minValueP) m68k_write_memory_16(minValueP, l_minValueP);
  if (maxValueP) m68k_write_memory_16(maxValueP, l_maxValueP);
  if (pageSizeP) m68k_write_memory_16(pageSizeP, l_pageSizeP);
  if (valueP) m68k_write_memory_16(valueP, l_valueP);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetSliderValues(ctlP=0x%08X, minValueP=0x%08X [%d], maxValueP=0x%08X [%d], pageSizeP=0x%08X [%d], valueP=0x%08X [%d])", ctlP, minValueP, l_minValueP, maxValueP, l_maxValueP, pageSizeP, l_pageSizeP, valueP, l_valueP);
}
break;
case sysTrapCtlHitControl: {
  // void CtlHitControl(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  CtlHitControl(controlP ? s_controlP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHitControl(controlP=0x%08X)", controlP);
}
break;
case sysTrapCtlHandleEvent: {
  // Boolean CtlHandleEvent(in ControlType *controlP, in EventType *pEvent)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  uint32_t pEvent = ARG32;
  EventType l_pEvent;
  decode_event(pEvent, &l_pEvent);
  Boolean res = CtlHandleEvent(controlP ? s_controlP : NULL, pEvent ? &l_pEvent : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlHandleEvent(controlP=0x%08X, pEvent=0x%08X): %d", controlP, pEvent, res);
}
break;
case sysTrapCtlValidatePointer: {
  // Boolean CtlValidatePointer(in ControlType *controlP)
  uint32_t controlP = ARG32;
  ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
  Boolean res = CtlValidatePointer(controlP ? s_controlP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CtlValidatePointer(controlP=0x%08X): %d", controlP, res);
}
break;
case sysTrapFileOpen: {
  // FileHand FileOpen(UInt16 cardNo, in Char *nameP, UInt32 type, UInt32 creator, UInt32 openMode, out Err *errP)
  uint16_t cardNo = ARG16;
  uint32_t nameP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  uint32_t type = ARG32;
  uint32_t creator = ARG32;
  uint32_t openMode = ARG32;
  uint32_t errP = ARG32;
  Err l_errP;
  FileHand res = FileOpen(cardNo, nameP ? s_nameP : NULL, type, creator, openMode, errP ? &l_errP : NULL);
  if (errP) m68k_write_memory_16(errP, l_errP);
  uint32_t r_res = emupalmos_trap_out(res);
  m68k_set_reg(M68K_REG_A0, r_res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileOpen(cardNo=%d, nameP=0x%08X [%s], type=%d, creator=%d, openMode=0x%08X, errP=0x%08X): 0x%08X", cardNo, nameP, s_nameP, type, creator, openMode, errP, r_res);
}
break;
case sysTrapFileClose: {
  // Err FileClose(FileHand stream)
  uint32_t stream = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
  Err res = FileClose(stream ? l_stream : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileClose(stream=0x%08X): %d", stream, res);
}
break;
case sysTrapFileDelete: {
  // Err FileDelete(UInt16 cardNo, in Char *nameP)
  uint16_t cardNo = ARG16;
  uint32_t nameP = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  Err res = FileDelete(cardNo, nameP ? s_nameP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileDelete(cardNo=%d, nameP=0x%08X [%s]): %d", cardNo, nameP, s_nameP, res);
}
break;
case sysTrapFileReadLow: {
  // Int32 FileReadLow(FileHand stream, out void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize, Int32 numObj, out Err *errP)
  uint32_t stream = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
  uint32_t baseP = ARG32;
  void *s_baseP = baseP ? (void *)(ram + baseP) : NULL;
  int32_t offset = ARG32;
  uint8_t dataStoreBased = ARG8;
  int32_t objSize = ARG32;
  int32_t numObj = ARG32;
  uint32_t errP = ARG32;
  Err l_errP;
  Int32 res = FileReadLow(stream ? l_stream : 0, baseP ? s_baseP : NULL, offset, dataStoreBased, objSize, numObj, errP ? &l_errP : NULL);
  if (errP) m68k_write_memory_16(errP, l_errP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileReadLow(stream=0x%08X, baseP=0x%08X, offset=%d, dataStoreBased=%d, objSize=%d, numObj=%d, errP=0x%08X): %d", stream, baseP, offset, dataStoreBased, objSize, numObj, errP, res);
}
break;
case sysTrapFileWrite: {
  // Int32 FileWrite(FileHand stream, in void *dataP, Int32 objSize, Int32 numObj, out Err *errP)
  uint32_t stream = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
  uint32_t dataP = ARG32;
  void *s_dataP = dataP ? (void *)(ram + dataP) : NULL;
  int32_t objSize = ARG32;
  int32_t numObj = ARG32;
  uint32_t errP = ARG32;
  Err l_errP;
  Int32 res = FileWrite(stream ? l_stream : 0, dataP ? s_dataP : NULL, objSize, numObj, errP ? &l_errP : NULL);
  if (errP) m68k_write_memory_16(errP, l_errP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileWrite(stream=0x%08X, dataP=0x%08X, objSize=%d, numObj=%d, errP=0x%08X): %d", stream, dataP, objSize, numObj, errP, res);
}
break;
case sysTrapFileSeek: {
  // Err FileSeek(FileHand stream, Int32 offset, FileOriginEnum origin)
  uint32_t stream = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
  int32_t offset = ARG32;
  uint8_t origin = ARG8;
  Err res = FileSeek(stream ? l_stream : 0, offset, origin);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileSeek(stream=0x%08X, offset=%d, origin=%d): %d", stream, offset, origin, res);
}
break;
case sysTrapFileTell: {
  // Int32 FileTell(FileHand stream, out Int32 *fileSizeP, out Err *errP)
  uint32_t stream = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
  uint32_t fileSizeP = ARG32;
  Int32 l_fileSizeP = 0;
  uint32_t errP = ARG32;
  Err l_errP;
  Int32 res = FileTell(stream ? l_stream : 0, fileSizeP ? &l_fileSizeP : NULL, errP ? &l_errP : NULL);
  if (fileSizeP) m68k_write_memory_32(fileSizeP, l_fileSizeP);
  if (errP) m68k_write_memory_16(errP, l_errP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileTell(stream=0x%08X, fileSizeP=0x%08X [%d], errP=0x%08X): %d", stream, fileSizeP, l_fileSizeP, errP, res);
}
break;
case sysTrapFileTruncate: {
  // Err FileTruncate(FileHand stream, Int32 newSize)
  uint32_t stream = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
  int32_t newSize = ARG32;
  Err res = FileTruncate(stream ? l_stream : 0, newSize);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileTruncate(stream=0x%08X, newSize=%d): %d", stream, newSize, res);
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
case sysTrapKeyCurrentState: {
  // UInt32 KeyCurrentState(void)
  UInt32 res = KeyCurrentState();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "KeyCurrentState(): %d", res);
}
break;
case sysTrapKeyRates: {
  // Err KeyRates(Boolean set, inout UInt16 *initDelayP, inout UInt16 *periodP, inout UInt16 *doubleTapDelayP, inout Boolean *queueAheadP)
  uint8_t set = ARG8;
  uint32_t initDelayP = ARG32;
  UInt16 l_initDelayP;
  if (initDelayP) l_initDelayP = m68k_read_memory_16(initDelayP);
  uint32_t periodP = ARG32;
  UInt16 l_periodP;
  if (periodP) l_periodP = m68k_read_memory_16(periodP);
  uint32_t doubleTapDelayP = ARG32;
  UInt16 l_doubleTapDelayP;
  if (doubleTapDelayP) l_doubleTapDelayP = m68k_read_memory_16(doubleTapDelayP);
  uint32_t queueAheadP = ARG32;
  Boolean l_queueAheadP;
  if (queueAheadP) l_queueAheadP = m68k_read_memory_8(queueAheadP);
  Err res = KeyRates(set, initDelayP ? &l_initDelayP : NULL, periodP ? &l_periodP : NULL, doubleTapDelayP ? &l_doubleTapDelayP : NULL, queueAheadP ? &l_queueAheadP : NULL);
  if (initDelayP) m68k_write_memory_16(initDelayP, l_initDelayP);
  if (periodP) m68k_write_memory_16(periodP, l_periodP);
  if (doubleTapDelayP) m68k_write_memory_16(doubleTapDelayP, l_doubleTapDelayP);
  if (queueAheadP) m68k_write_memory_8(queueAheadP, l_queueAheadP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "KeyRates(set=%d, initDelayP=0x%08X [%d], periodP=0x%08X [%d], doubleTapDelayP=0x%08X [%d], queueAheadP=0x%08X): %d", set, initDelayP, l_initDelayP, periodP, l_periodP, doubleTapDelayP, l_doubleTapDelayP, queueAheadP, res);
}
break;
case sysTrapKeySetMask: {
  // UInt32 KeySetMask(UInt32 keyMask)
  uint32_t keyMask = ARG32;
  UInt32 res = KeySetMask(keyMask);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "KeySetMask(keyMask=%d): %d", keyMask, res);
}
break;
case sysTrapCategoryCreateListV10: {
  // void CategoryCreateListV10(DmOpenRef db, in ListType *lst, UInt16 currentCategory, Boolean showAll)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t lst = ARG32;
  ListType *s_lst = lst ? (ListType *)(ram + lst) : NULL;
  uint16_t currentCategory = ARG16;
  uint8_t showAll = ARG8;
  CategoryCreateListV10(db ? l_db : 0, lst ? s_lst : NULL, currentCategory, showAll);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryCreateListV10(db=0x%08X, lst=0x%08X, currentCategory=%d, showAll=%d)", db, lst, currentCategory, showAll);
}
break;
case sysTrapCategoryCreateList: {
  // void CategoryCreateList(DmOpenRef db, in ListType *listP, UInt16 currentCategory, Boolean showAll, Boolean showUneditables, UInt8 numUneditableCategories, UInt32 editingStrID, Boolean resizeList)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  uint16_t currentCategory = ARG16;
  uint8_t showAll = ARG8;
  uint8_t showUneditables = ARG8;
  uint8_t numUneditableCategories = ARG8;
  uint32_t editingStrID = ARG32;
  uint8_t resizeList = ARG8;
  CategoryCreateList(db ? l_db : 0, listP ? s_listP : NULL, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryCreateList(db=0x%08X, listP=0x%08X, currentCategory=%d, showAll=%d, showUneditables=%d, numUneditableCategories=%d, editingStrID=%d, resizeList=%d)", db, listP, currentCategory, showAll, showUneditables, numUneditableCategories, editingStrID, resizeList);
}
break;
case sysTrapCategoryFreeListV10: {
  // void CategoryFreeListV10(DmOpenRef db, in ListType *lst)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t lst = ARG32;
  ListType *s_lst = lst ? (ListType *)(ram + lst) : NULL;
  CategoryFreeListV10(db ? l_db : 0, lst ? s_lst : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFreeListV10(db=0x%08X, lst=0x%08X)", db, lst);
}
break;
case sysTrapCategoryFreeList: {
  // void CategoryFreeList(DmOpenRef db, in ListType *listP, Boolean showAll, UInt32 editingStrID)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t listP = ARG32;
  ListType *s_listP = emupalmos_trap_in(listP, trap, 0);
  uint8_t showAll = ARG8;
  uint32_t editingStrID = ARG32;
  CategoryFreeList(db ? l_db : 0, listP ? s_listP : NULL, showAll, editingStrID);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFreeList(db=0x%08X, listP=0x%08X, showAll=%d, editingStrID=%d)", db, listP, showAll, editingStrID);
}
break;
case sysTrapCategoryFind: {
  // UInt16 CategoryFind(DmOpenRef db, in Char *name)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t name = ARG32;
  char *s_name = name ? (char *)(ram + name) : NULL;
  UInt16 res = CategoryFind(db ? l_db : 0, name ? s_name : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryFind(db=0x%08X, name=0x%08X [%s]): %d", db, name, s_name, res);
}
break;
case sysTrapCategoryGetName: {
  // void CategoryGetName(DmOpenRef db, UInt16 index, out Char *name)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint16_t index = ARG16;
  uint32_t name = ARG32;
  char *s_name = name ? (char *)(ram + name) : NULL;
  CategoryGetName(db ? l_db : 0, index, name ? s_name : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryGetName(db=0x%08X, index=%d, name=0x%08X [%s])", db, index, name, s_name);
}
break;
case sysTrapCategoryEditV10: {
  // Boolean CategoryEditV10(DmOpenRef db, inout UInt16 *category)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t category = ARG32;
  UInt16 l_category = 0;
  if (category) l_category = m68k_read_memory_16(category);
  Boolean res = CategoryEditV10(db ? l_db : 0, category ? &l_category : NULL);
  if (category) m68k_write_memory_16(category, l_category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEditV10(db=0x%08X, category=0x%08X [%d]): %d", db, category, l_category, res);
}
break;
case sysTrapCategoryEditV20: {
  // Boolean CategoryEditV20(DmOpenRef db, inout UInt16 *category, UInt32 titleStrID)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t category = ARG32;
  UInt16 l_category = 0;
  if (category) l_category = m68k_read_memory_16(category);
  uint32_t titleStrID = ARG32;
  Boolean res = CategoryEditV20(db ? l_db : 0, category ? &l_category : NULL, titleStrID);
  if (category) m68k_write_memory_16(category, l_category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEditV20(db=0x%08X, category=0x%08X [%d], titleStrID=%d): %d", db, category, l_category, titleStrID, res);
}
break;
case sysTrapCategoryEdit: {
  // Boolean CategoryEdit(DmOpenRef db, inout UInt16 *category, UInt32 titleStrID, UInt8 numUneditableCategories)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t category = ARG32;
  UInt16 l_category = 0;
  if (category) l_category = m68k_read_memory_16(category);
  uint32_t titleStrID = ARG32;
  uint8_t numUneditableCategories = ARG8;
  Boolean res = CategoryEdit(db ? l_db : 0, category ? &l_category : NULL, titleStrID, numUneditableCategories);
  if (category) m68k_write_memory_16(category, l_category);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryEdit(db=0x%08X, category=0x%08X [%d], titleStrID=%d, numUneditableCategories=%d): %d", db, category, l_category, titleStrID, numUneditableCategories, res);
}
break;
case sysTrapCategorySelectV10: {
  // Boolean CategorySelectV10(DmOpenRef db, in FormType *frm, UInt16 ctlID, UInt16 lstID, Boolean title, out UInt16 *categoryP, out Char *categoryName)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t frm = ARG32;
  FormType *s_frm = frm ? (FormType *)(ram + frm) : NULL;
  uint16_t ctlID = ARG16;
  uint16_t lstID = ARG16;
  uint8_t title = ARG8;
  uint32_t categoryP = ARG32;
  UInt16 l_categoryP = 0;
  uint32_t categoryName = ARG32;
  char *s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
  Boolean res = CategorySelectV10(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL);
  if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelectV10(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s]): %d", db, frm, ctlID, lstID, title, categoryP, l_categoryP, categoryName, s_categoryName, res);
}
break;
case sysTrapCategorySelect: {
  // Boolean CategorySelect(DmOpenRef db, in FormType *frm, UInt16 ctlID, UInt16 lstID, Boolean title, out UInt16 *categoryP, out Char *categoryName, UInt8 numUneditableCategories, UInt32 editingStrID)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t frm = ARG32;
  FormType *s_frm = frm ? (FormType *)(ram + frm) : NULL;
  uint16_t ctlID = ARG16;
  uint16_t lstID = ARG16;
  uint8_t title = ARG8;
  uint32_t categoryP = ARG32;
  UInt16 l_categoryP = 0;
  uint32_t categoryName = ARG32;
  char *s_categoryName = categoryName ? (char *)(ram + categoryName) : NULL;
  uint8_t numUneditableCategories = ARG8;
  uint32_t editingStrID = ARG32;
  Boolean res = CategorySelect(db ? l_db : 0, frm ? s_frm : NULL, ctlID, lstID, title, categoryP ? &l_categoryP : NULL, categoryName ? s_categoryName : NULL, numUneditableCategories, editingStrID);
  if (categoryP) m68k_write_memory_16(categoryP, l_categoryP);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySelect(db=0x%08X, frm=0x%08X, ctlID=%d, lstID=%d, title=%d, categoryP=0x%08X [%d], categoryName=0x%08X [%s], numUneditableCategories=%d, editingStrID=%d): %d", db, frm, ctlID, lstID, title, categoryP, l_categoryP, categoryName, s_categoryName, numUneditableCategories, editingStrID, res);
}
break;
case sysTrapCategoryGetNext: {
  // UInt16 CategoryGetNext(DmOpenRef db, UInt16 index)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint16_t index = ARG16;
  UInt16 res = CategoryGetNext(db ? l_db : 0, index);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryGetNext(db=0x%08X, index=%d): %d", db, index, res);
}
break;
case sysTrapCategorySetTriggerLabel: {
  // void CategorySetTriggerLabel(in ControlType *ctl, Char *name)
  uint32_t ctl = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  ControlType *s_ctl = ctl ? (ControlType *)(ram + ctl) : NULL;
  uint32_t name = ARG32;
  char *s_name = name ? (char *)(ram + name) : NULL;
  CategorySetTriggerLabel(ctl ? s_ctl : NULL, name ? s_name : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetTriggerLabel(ctl=0x%08X, name=0x%08X [%s])", ctl, name, s_name);
}
break;
case sysTrapCategoryTruncateName: {
  // void CategoryTruncateName(inout Char *name, UInt16 maxWidth)
  uint32_t name = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  char *s_name = name ? (char *)(ram + name) : NULL;
  uint16_t maxWidth = ARG16;
  CategoryTruncateName(name ? s_name : NULL, maxWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryTruncateName(name=0x%08X [%s], maxWidth=%d)", name, s_name, maxWidth);
}
break;
case sysTrapCategoryInitialize: {
  // void CategoryInitialize(inout AppInfoType *appInfoP, UInt16 localizedAppInfoStrID)
  uint32_t appInfoP = ARG32;
  AppInfoType *l_appInfoP;
  if ((l_appInfoP = MemPtrNew(sizeof(AppInfoType))) != NULL) {
    decode_appinfo(appInfoP, l_appInfoP);
    uint16_t localizedAppInfoStrID = ARG16;
    CategoryInitialize(appInfoP ? l_appInfoP : NULL, localizedAppInfoStrID);
    encode_appinfo(appInfoP, l_appInfoP);
    MemPtrFree(l_appInfoP);
    debug(DEBUG_TRACE, "EmuPalmOS", "CategoryInitialize(appInfoP=0x%08X, localizedAppInfoStrID=%d)", appInfoP, localizedAppInfoStrID);
  }
}
break;
case sysTrapCategorySetName: {
  // void CategorySetName(DmOpenRef db, UInt16 index, in Char *nameP)
  uint32_t db = ARG32;
  uint8_t *ram = pumpkin_heap_base();
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint16_t index = ARG16;
  uint32_t nameP = ARG32;
  char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  CategorySetName(db ? l_db : 0, index, nameP ? s_nameP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetName(db=0x%08X, index=%d, nameP=0x%08X [%s])", db, index, nameP, s_nameP);
}
break;
case sysTrapPwdExists: {
  // Boolean PwdExists(void)
  Boolean res = PwdExists();
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "PwdExists(): %d", res);
}
break;
