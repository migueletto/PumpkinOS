#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>
#include <INetMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "mutex.h"
#include "storage.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

static Boolean MemSetOrMoveDisplay(uint32_t dstP, uint32_t sP, uint32_t numBytes, uint8_t value, uint32_t start, uint32_t end, Err *err) {
  BitmapType *bmp;
  WinHandle wh;
  UInt32 offset, numPixels, pixelSize, depth;
  Coord width, height, x1, y1, x2, y2;
  Boolean moved = false;

  if (dstP >= start && dstP < end) {
    if (emupalmos_fast_screen_write()) {
      wh = WinGetDisplayWindow();
      bmp = WinGetBitmap(wh);
      depth = BmpGetBitDepth(bmp);

      if (depth == 8 || depth == 16) {
        if (sP) {
          *err = MemMove(emupalmos_trap_in(dstP, sysTrapMemMove, 0), emupalmos_trap_in(sP, sysTrapMemMove, 1), numBytes);
        } else {
          *err = MemSet(emupalmos_trap_in(dstP, sysTrapMemSet, 0), numBytes, value);
        }

        BmpGetDimensions(bmp, &width, &height, NULL);
        pixelSize = depth / 8;
        numPixels = numBytes / pixelSize;
        offset = (dstP - start) / pixelSize;
        x1 = offset % width;
        y1 = offset / width;
        x2 = (offset + numPixels - 1) % width;
        y2 = (offset + numPixels - 1) / width;
        debug(DEBUG_TRACE, "EmuPalmOS", "MemSetOrMove screen x1=%d y1=%d x2=%d y2=%d", x1, y1, x2, y2);
        pumpkin_dirty_region_mode(dirtyRegionBegin);
        pumpkin_screen_dirty(wh, x1, y1, x2, y2);
        pumpkin_dirty_region_mode(dirtyRegionEnd);
        moved = true;
      }
    }
  }

  return moved;
}

void palmos_MemSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {
  switch (trap) {
    case sysTrapMemSet: {
      // Err MemSet(void *dstP, Int32 numBytes, UInt8 value)
      uint32_t dstP = ARG32;
      uint32_t numBytes = ARG32;
      uint8_t value = ARG8;
      UInt32 start, end;
      Err err = errNone;

      WinLegacyGetAddr(&start, &end);
      if ((dstP >= start && dstP < end) ||
          (dstP+numBytes-1 >= start && dstP+numBytes-1 < end) ||
          (dstP < start && dstP+numBytes >= end)) {

        debug(DEBUG_TRACE, "EmuPalmOS", "MemSet(0x%08X, %d, 0x%02X) inside screen", dstP, numBytes, value);
        if (!MemSetOrMoveDisplay(dstP, 0, numBytes, value, start, end, &err)) {
          for (uint32_t i = 0; i < numBytes; i++) {
            m68k_write_memory_8(dstP+i, value);
          }
        }
      } else {
        if (emupalmos_check_address(dstP, numBytes, 0)) {
          err = MemSet(emupalmos_trap_in(dstP, trap, 0), numBytes, value);
        } else {
          err = dmErrInvalidParam;
        }
      }
      debug(DEBUG_TRACE, "logmem", "write %u %u", dstP, numBytes);
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
      Err err = errNone;

      WinLegacyGetAddr(&start, &end);
      if ((dstP >= start && dstP < end) ||
          (dstP+numBytes-1 >= start && dstP+numBytes-1 < end) ||
          (dstP < start && dstP+numBytes >= end) ||
          (sP >= start && sP < end) ||
          (sP+numBytes-1 >= start && sP+numBytes-1 < end) ||
          (sP < start && sP+numBytes >= end)) {

        debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d) inside screen", dstP, sP, numBytes);
        if (!MemSetOrMoveDisplay(dstP, sP, numBytes, 0, start, end, &err)) {
          for (uint32_t i = 0; i < numBytes; i++) {
            uint8_t value = m68k_read_memory_8(sP+i);
            m68k_write_memory_8(dstP+i, value);
          }
        }
      } else {
        if (emupalmos_check_address(dstP, numBytes, 0) && emupalmos_check_address(sP, numBytes, 1)) {
          err = MemMove(emupalmos_trap_in(dstP, trap, 0), emupalmos_trap_in(sP, trap, 1), numBytes);
        } else {
          err = dmErrInvalidParam;
        }
      }
      debug(DEBUG_TRACE, "logmem", "write %u %u", dstP, numBytes);
      debug(DEBUG_TRACE, "EmuPalmOS", "MemMove(0x%08X, 0x%08X, %d): %d", dstP, sP, numBytes, err);
      m68k_set_reg(M68K_REG_D0, err);
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
  }
}
