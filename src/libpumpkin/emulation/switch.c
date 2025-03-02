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
  UInt16 l_versionP;
  uint32_t crDateP = ARG32;
  UInt32 l_crDateP;
  uint32_t romSizeP = ARG32;
  UInt32 l_romSizeP;
  uint32_t ramSizeP = ARG32;
  UInt32 l_ramSizeP;
  uint32_t freeBytesP = ARG32;
  UInt32 l_freeBytesP;
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
  UInt32 l_freeP;
  uint32_t maxP = ARG32;
  UInt32 l_maxP;
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
  debug(DEBUG_TRACE, "EmuPalmOS", "MemPtrNew(size=%u): 0x%08X", size, r_res);
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
  UInt16 l_attributesP;
  uint32_t versionP = ARG32;
  UInt16 l_versionP;
  uint32_t crDateP = ARG32;
  UInt32 l_crDateP;
  uint32_t modDateP = ARG32;
  UInt32 l_modDateP;
  uint32_t bckUpDateP = ARG32;
  UInt32 l_bckUpDateP;
  uint32_t modNumP = ARG32;
  UInt32 l_modNumP;
  uint32_t appInfoIDP = ARG32;
  LocalID l_appInfoIDP;
  uint32_t sortInfoIDP = ARG32;
  LocalID l_sortInfoIDP;
  uint32_t typeP = ARG32;
  UInt32 l_typeP;
  uint32_t creatorP = ARG32;
  UInt32 l_creatorP;
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
  UInt16 l_attributesP;
  if (attributesP) l_attributesP = m68k_read_memory_16(attributesP);
  uint32_t versionP = ARG32;
  UInt16 l_versionP;
  if (versionP) l_versionP = m68k_read_memory_16(versionP);
  uint32_t crDateP = ARG32;
  UInt32 l_crDateP;
  if (crDateP) l_crDateP = m68k_read_memory_32(crDateP);
  uint32_t modDateP = ARG32;
  UInt32 l_modDateP;
  if (modDateP) l_modDateP = m68k_read_memory_32(modDateP);
  uint32_t bckUpDateP = ARG32;
  UInt32 l_bckUpDateP;
  if (bckUpDateP) l_bckUpDateP = m68k_read_memory_32(bckUpDateP);
  uint32_t modNumP = ARG32;
  UInt32 l_modNumP;
  if (modNumP) l_modNumP = m68k_read_memory_32(modNumP);
  uint32_t appInfoIDP = ARG32;
  LocalID l_appInfoIDP;
  if (appInfoIDP) l_appInfoIDP = m68k_read_memory_32(appInfoIDP);
  uint32_t sortInfoIDP = ARG32;
  LocalID l_sortInfoIDP;
  if (sortInfoIDP) l_sortInfoIDP = m68k_read_memory_32(sortInfoIDP);
  uint32_t typeP = ARG32;
  UInt32 l_typeP;
  if (typeP) l_typeP = m68k_read_memory_32(typeP);
  uint32_t creatorP = ARG32;
  UInt32 l_creatorP;
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
  UInt32 l_numRecordsP;
  uint32_t totalBytesP = ARG32;
  UInt32 l_totalBytesP;
  uint32_t dataBytesP = ARG32;
  UInt32 l_dataBytesP;
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
  LocalID l_dbIDP;
  uint32_t openCountP = ARG32;
  UInt16 l_openCountP;
  uint32_t modeP = ARG32;
  UInt16 l_modeP;
  uint32_t cardNoP = ARG32;
  UInt16 l_cardNoP;
  uint32_t resDBP = ARG32;
  Boolean l_resDBP;
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
  UInt8 l_highest;
  uint32_t count = ARG32;
  UInt32 l_count;
  uint32_t busy = ARG32;
  UInt32 l_busy;
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
  UInt16 l_attrP;
  uint32_t uniqueIDP = ARG32;
  UInt32 l_uniqueIDP;
  uint32_t chunkIDP = ARG32;
  LocalID l_chunkIDP;
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
  UInt16 l_attrP;
  if (attrP) l_attrP = m68k_read_memory_16(attrP);
  uint32_t uniqueIDP = ARG32;
  UInt32 l_uniqueIDP;
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
  UInt16 l_atP;
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
  UInt16 l_indexP;
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
  UInt16 l_indexP;
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
  UInt16 l_indexP;
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
  Err res = DmWrite(recordP ? s_recordP : NULL, offset, srcP ? s_srcP : NULL, bytes);
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
  UInt16 l_error;
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
  Coord l_extentX;
  uint32_t extentY = ARG32;
  Coord l_extentY;
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
  Coord l_extentX;
  uint32_t extentY = ARG32;
  Coord l_extentY;
  WinGetWindowExtent(extentX ? &l_extentX : NULL, extentY ? &l_extentY : NULL);
  if (extentX) m68k_write_memory_16(extentX, l_extentX);
  if (extentY) m68k_write_memory_16(extentY, l_extentY);
  debug(DEBUG_TRACE, "EmuPalmOS", "WinGetWindowExtent(extentX=0x%08X [%d], extentY=0x%08X [%d])", extentX, l_extentX, extentY, l_extentY);
}
break;
case sysTrapWinDisplayToWindowPt: {
  // void WinDisplayToWindowPt(inout Coord *extentX, inout Coord *extentY)
  uint32_t extentX = ARG32;
  Coord l_extentX;
  if (extentX) l_extentX = m68k_read_memory_16(extentX);
  uint32_t extentY = ARG32;
  Coord l_extentY;
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
  Coord l_extentX;
  if (extentX) l_extentX = m68k_read_memory_16(extentX);
  uint32_t extentY = ARG32;
  Coord l_extentY;
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
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt16 res = BmpSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpBitsSize: {
  // UInt16 BmpBitsSize(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt16 res = BmpBitsSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpBitsSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetSizes: {
  // void BmpGetSizes(in BitmapType *bitmapP, out UInt32 *dataSizeP, out UInt32 *headerSizeP)
  uint32_t bitmapP = ARG32;
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  uint32_t dataSizeP = ARG32;
  UInt32 l_dataSizeP;
  uint32_t headerSizeP = ARG32;
  UInt32 l_headerSizeP;
  BmpGetSizes(bitmapP ? l_bitmapP : NULL, dataSizeP ? &l_dataSizeP : NULL, headerSizeP ? &l_headerSizeP : NULL);
  if (dataSizeP) m68k_write_memory_32(dataSizeP, l_dataSizeP);
  if (headerSizeP) m68k_write_memory_32(headerSizeP, l_headerSizeP);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetSizes(bitmapP=0x%08X, dataSizeP=0x%08X [%d], headerSizeP=0x%08X [%d])", bitmapP, dataSizeP, l_dataSizeP, headerSizeP, l_headerSizeP);
}
break;
case sysTrapBmpColortableSize: {
  // UInt16 BmpColortableSize(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt16 res = BmpColortableSize(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpColortableSize(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetDimensions: {
  // void BmpGetDimensions(in BitmapType *bitmapP, out Coord *widthP, out Coord *heightP, out UInt16 *rowBytesP)
  uint32_t bitmapP = ARG32;
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  uint32_t widthP = ARG32;
  Coord l_widthP = 0;
  uint32_t heightP = ARG32;
  Coord l_heightP = 0;
  uint32_t rowBytesP = ARG32;
  UInt16 l_rowBytesP;
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
  BitmapType *l_bitmapP = bitmapP ? (BitmapType *)(ram + bitmapP) : NULL;
  UInt8 res = BmpGetBitDepth(bitmapP ? l_bitmapP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetBitDepth(bitmapP=0x%08X): %d", bitmapP, res);
}
break;
case sysTrapBmpGetNextBitmap: {
  // BitmapType *BmpGetNextBitmap(in BitmapType *bitmapP)
  uint32_t bitmapP = ARG32;
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
  Boolean l_leadingEdge;
  uint32_t truncWidth = ARG32;
  Int16 l_truncWidth;
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
  Int16 l_stringWidthP;
  if (stringWidthP) l_stringWidthP = m68k_read_memory_16(stringWidthP);
  uint32_t stringLengthP = ARG32;
  Int16 l_stringLengthP;
  if (stringLengthP) l_stringLengthP = m68k_read_memory_16(stringLengthP);
  uint32_t fitWithinWidth = ARG32;
  Boolean l_fitWithinWidth;
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
  UInt16 l_linesToScrollP;
  if (linesToScrollP) l_linesToScrollP = m68k_read_memory_16(linesToScrollP);
  uint32_t scrollPosP = ARG32;
  UInt16 l_scrollPosP;
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
  UInt16 l_linesP;
  uint32_t topLine = ARG32;
  UInt16 l_topLine;
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
  Char *res = s_dst && s_src ? StrCopy(s_dst, s_src) : NULL;
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
  Char *res = s_dst && s_src ? StrCat(s_dst, s_src) : NULL;
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
  UInt16 l_startPosition;
  uint32_t endPosition = ARG32;
  UInt16 l_endPosition;
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
  UInt16 l_scrollPosP;
  uint32_t textHeightP = ARG32;
  UInt16 l_textHeightP;
  uint32_t fieldHeightP = ARG32;
  UInt16 l_fieldHeightP;
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
  union {
    UInt16 bits;
    FieldAttrType fields;
  } attr;
  FldGetAttributes(fldP ? s_fldP : NULL, attrP ? &attr.fields : NULL);
  if (attrP) m68k_write_memory_16(attrP, attr.bits);
  debug(DEBUG_TRACE, "EmuPalmOS", "FldGetAttributes(fldP=0x%08X, attrP=0x%08X)", fldP, attrP);
}
break;
case sysTrapFldSetAttributes: {
  // void FldSetAttributes(in FieldType *fldP, in FieldAttrType *attrP)
  uint32_t fldP = ARG32;
  FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
  uint32_t attrP = ARG32;
  union {
    UInt16 bits;
    FieldAttrType fields;
  } attr;
  attr.bits = attrP ? m68k_read_memory_16(attrP) : 0;
  FldSetAttributes(fldP ? s_fldP : NULL, attrP ? &attr.fields : NULL);
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
  Int16 l_rowP;
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
  Int16 l_rowP;
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
  debug(DEBUG_INFO, "EmuPalmOS", "TblGetRowData(tableP=0x%08X, row=%d): 0x%08X", tableP, row, res);
}
break;
case sysTrapTblSetRowData: {
  // void TblSetRowData(in TableType *tableP, Int16 row, UInt32 data)
  uint32_t tableP = ARG32;
  TableType *s_tableP = emupalmos_trap_in(tableP, trap, 0);
  int16_t row = ARG16;
  uint32_t data = ARG32;
  TblSetRowData(tableP ? s_tableP : NULL, row, data);
  debug(DEBUG_INFO, "EmuPalmOS", "TblSetRowData(tableP=0x%08X, row=%d, data=0x%08X)", tableP, row, data);
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
  Int16 l_rowP;
  uint32_t columnP = ARG32;
  Int16 l_columnP;
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
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuDispose(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuDispose(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuHandleEvent: {
  // Boolean MenuHandleEvent(in MenuBarType *menuP, in EventType *event, out UInt16 *error)
  uint32_t menuP = ARG32;
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  uint32_t event = ARG32;
  EventType l_event;
  decode_event(event, &l_event);
  uint32_t error = ARG32;
  UInt16 l_error;
  Boolean res = MenuHandleEvent(menuP ? s_menuP : NULL, event ? &l_event : NULL, error ? &l_error : NULL);
  if (error) m68k_write_memory_16(error, l_error);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuHandleEvent(menuP=0x%08X, event=0x%08X, error=0x%08X [%d]): %d", menuP, event, error, l_error, res);
}
break;
case sysTrapMenuDrawMenu: {
  // void MenuDrawMenu(in MenuBarType *menuP)
  uint32_t menuP = ARG32;
  MenuBarType *s_menuP = menuP ? (MenuBarType *)(ram + menuP) : NULL;
  MenuDrawMenu(menuP ? s_menuP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "MenuDrawMenu(menuP=0x%08X)", menuP);
}
break;
case sysTrapMenuEraseStatus: {
  // void MenuEraseStatus(in MenuBarType *menuP)
  uint32_t menuP = ARG32;
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
  UInt16 l_bitmapIdP;
  uint32_t resultTypeP = ARG32;
  MenuCmdBarResultType l_resultTypeP;
  uint32_t resultP = ARG32;
  UInt32 l_resultP;
  uint32_t nameP = ARG32;
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
  Int16 l_x;
  uint32_t y = ARG32;
  Int16 l_y;
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
  ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  uint32_t minValueP = ARG32;
  UInt16 l_minValueP;
  if (minValueP) l_minValueP = m68k_read_memory_16(minValueP);
  uint32_t maxValueP = ARG32;
  UInt16 l_maxValueP;
  if (maxValueP) l_maxValueP = m68k_read_memory_16(maxValueP);
  uint32_t pageSizeP = ARG32;
  UInt16 l_pageSizeP;
  if (pageSizeP) l_pageSizeP = m68k_read_memory_16(pageSizeP);
  uint32_t valueP = ARG32;
  UInt16 l_valueP;
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
  ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
  uint32_t minValueP = ARG32;
  UInt16 l_minValueP;
  uint32_t maxValueP = ARG32;
  UInt16 l_maxValueP;
  uint32_t pageSizeP = ARG32;
  UInt16 l_pageSizeP;
  uint32_t valueP = ARG32;
  UInt16 l_valueP;
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
  char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  Err res = FileDelete(cardNo, nameP ? s_nameP : NULL);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "FileDelete(cardNo=%d, nameP=0x%08X [%s]): %d", cardNo, nameP, s_nameP, res);
}
break;
case sysTrapFileReadLow: {
  // Int32 FileReadLow(FileHand stream, out void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize, Int32 numObj, out Err *errP)
  uint32_t stream = ARG32;
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
  FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
  uint32_t fileSizeP = ARG32;
  Int32 l_fileSizeP;
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
case sysTrapSysUIAppSwitch: {
  // Err SysUIAppSwitch(UInt16 cardNo, LocalID dbID, UInt16 cmd, MemPtr cmdPBP)
  uint16_t cardNo = ARG16;
  LocalID dbID = ARG32;
  uint16_t cmd = ARG16;
  uint32_t cmdPBP = ARG32;
  void *l_cmdPBP = cmdPBP ? ram + cmdPBP : NULL;
  Err res = SysUIAppSwitch(cardNo, dbID, cmd, cmdPBP ? l_cmdPBP : 0);
  m68k_set_reg(M68K_REG_D0, res);
  debug(DEBUG_TRACE, "EmuPalmOS", "SysUIAppSwitch(cardNo=%d, dbID=0x%08X, cmd=%d, cmdPBP=0x%08X): %d", cardNo, dbID, cmd, cmdPBP, res);
}
break;
case sysTrapSysCurAppDatabase: {
  // Err SysCurAppDatabase(out UInt16 *cardNoP, out LocalID *dbIDP)
  uint32_t cardNoP = ARG32;
  UInt16 l_cardNoP;
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
  void *l_cmdPBP = cmdPBP ? ram + cmdPBP : NULL;
  uint32_t resultP = ARG32;
  UInt32 l_resultP;
  Err res = SysAppLaunch(cardNo, dbID, launchFlags, cmd, cmdPBP ? l_cmdPBP : 0, resultP ? &l_resultP : NULL);
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
  UInt16 l_warnThresholdP;
  uint32_t criticalThresholdP = ARG32;
  UInt16 l_criticalThresholdP;
  uint32_t maxTicksP = ARG32;
  Int16 l_maxTicksP;
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
  UInt16 l_warnThresholdP;
  uint32_t criticalThresholdP = ARG32;
  UInt16 l_criticalThresholdP;
  uint32_t maxTicksP = ARG32;
  Int16 l_maxTicksP;
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
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t category = ARG32;
  UInt16 l_category;
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
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t category = ARG32;
  UInt16 l_category;
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
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t category = ARG32;
  UInt16 l_category;
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
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t frm = ARG32;
  FormType *s_frm = frm ? (FormType *)(ram + frm) : NULL;
  uint16_t ctlID = ARG16;
  uint16_t lstID = ARG16;
  uint8_t title = ARG8;
  uint32_t categoryP = ARG32;
  UInt16 l_categoryP;
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
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint32_t frm = ARG32;
  FormType *s_frm = frm ? (FormType *)(ram + frm) : NULL;
  uint16_t ctlID = ARG16;
  uint16_t lstID = ARG16;
  uint8_t title = ARG8;
  uint32_t categoryP = ARG32;
  UInt16 l_categoryP;
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
  char *s_name = name ? (char *)(ram + name) : NULL;
  uint16_t maxWidth = ARG16;
  CategoryTruncateName(name ? s_name : NULL, maxWidth);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryTruncateName(name=0x%08X [%s], maxWidth=%d)", name, s_name, maxWidth);
}
break;
case sysTrapCategoryInitialize: {
  // void CategoryInitialize(inout AppInfoType *appInfoP, UInt16 localizedAppInfoStrID)
  uint32_t appInfoP = ARG32;
  AppInfoType l_appInfoP;
  decode_appinfo(appInfoP, &l_appInfoP);
  uint16_t localizedAppInfoStrID = ARG16;
  CategoryInitialize(appInfoP ? &l_appInfoP : NULL, localizedAppInfoStrID);
  encode_appinfo(appInfoP, &l_appInfoP);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategoryInitialize(appInfoP=0x%08X, localizedAppInfoStrID=%d)", appInfoP, localizedAppInfoStrID);
}
break;
case sysTrapCategorySetName: {
  // void CategorySetName(DmOpenRef db, UInt16 index, in Char *nameP)
  uint32_t db = ARG32;
  DmOpenRef l_db = db ? (DmOpenRef)(ram + db) : NULL;
  uint16_t index = ARG16;
  uint32_t nameP = ARG32;
  char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
  CategorySetName(db ? l_db : 0, index, nameP ? s_nameP : NULL);
  debug(DEBUG_TRACE, "EmuPalmOS", "CategorySetName(db=0x%08X, index=%d, nameP=0x%08X [%s])", db, index, nameP, s_nameP);
}
break;
