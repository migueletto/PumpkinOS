#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

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

void palmos_BmpSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
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
  }
}
