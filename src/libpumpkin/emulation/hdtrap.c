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

void palmos_highdensitytrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  Err err;

  switch (sel) {
    case HDSelectorBmpGetNextBitmapAnyDensity: {
      // BitmapType *BmpGetNextBitmapAnyDensity(BitmapType *bitmapP)
      uint32_t bitmapP = ARG32;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      BitmapType *next = BmpGetNextBitmapAnyDensity(bitmap);
      uint32_t a = emupalmos_trap_out(next);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetNextBitmapAnyDensity(0x%08X): 0x%08X", bitmapP, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;

    case HDSelectorBmpGetVersion: {
      // UInt8 BmpGetVersion(const BitmapType *bitmapP)
      uint32_t bitmapP = ARG32;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      UInt8 version = BmpGetVersion(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetVersion(0x%08X): %d", bitmapP, version);
      m68k_set_reg(M68K_REG_D0, version);
    }
      break;

    case HDSelectorBmpGetCompressionType: {
      //BitmapCompressionType BmpGetCompressionType(const BitmapType *bitmapP)
      uint32_t bitmapP = ARG32;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      BitmapCompressionType type = BmpGetCompressionType(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetCompressionType(0x%08X): %d", bitmapP, type);
      m68k_set_reg(M68K_REG_D0, type);
    }
    break;

    case HDSelectorBmpGetDensity: {
      // UInt16 BmpGetDensity(const BitmapType *bitmapP)
      uint32_t bitmapP = ARG32;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      UInt16 density = BmpGetDensity(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetDensity(0x%08X): %d", bitmapP, density);
      m68k_set_reg(M68K_REG_D0, density);
    }
    break;

    case HDSelectorBmpSetDensity: {
      // Err BmpSetDensity(BitmapType *bitmapP, UInt16 density)
      uint32_t bitmapP = ARG32;
      uint16_t density = ARG16;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      err = BmpSetDensity(bitmap, density);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpSetDensity(0x%08X, %d): %d", bitmapP, density, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;

    case HDSelectorBmpGetTransparentValue: {
      // Boolean BmpGetTransparentValue(const BitmapType *bitmapP, UInt32 *transparentValueP)
      uint32_t bitmapP = ARG32;
      uint32_t transparentValueP = ARG32;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      UInt32 transparentValue;
      Boolean r = BmpGetTransparentValue(bitmap, &transparentValue);
      if (transparentValueP) m68k_write_memory_32(transparentValueP, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpGetTransparentValue(0x%08X, 0x%08X): %d", bitmapP, transparentValueP, r);
      m68k_set_reg(M68K_REG_D0, r);
    }
    break;

    case HDSelectorBmpSetTransparentValue: {
      // void BmpSetTransparentValue(BitmapType *bitmapP, UInt32 transparentValue)
      uint32_t bitmapP = ARG32;
      uint32_t transparentValue = ARG32;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      BmpSetTransparentValue(bitmap, transparentValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpSetTransparentValue(0x%08X, %d)", bitmapP, transparentValue);
    }
      break;

    case HDSelectorBmpCreateBitmapV3: {
      // BitmapTypeV3 *BmpCreateBitmapV3(const BitmapType *bitmapP, UInt16 density, const void *bitsP, const ColorTableType *colorTableP)
      uint32_t bitmapP = ARG32;
      uint16_t density = ARG16;
      uint32_t bitsP = ARG32;
      uint32_t colorTableP = ARG32;
      BitmapType *bitmap = emupalmos_trap_sel_in(bitmapP, sysTrapPinsDispatch, sel, 0);
      void *bits = emupalmos_trap_sel_in(bitsP, sysTrapPinsDispatch, sel, 2);
      ColorTableType *colorTable = emupalmos_trap_sel_in(colorTableP, sysTrapPinsDispatch, sel, 3);
      BitmapTypeV3 *bmpV3 = BmpCreateBitmapV3(bitmap, density, bits, colorTable);
      uint32_t a = emupalmos_trap_out(bmpV3);
      debug(DEBUG_TRACE, "EmuPalmOS", "BmpCreateBitmapV3(0x%08X, %d, 0x%08X, 0x%08X): 0x%08X", bitmapP, density, bitsP, colorTableP, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
      break;

    case HDSelectorWinSetCoordinateSystem: {
      // UInt16 WinSetCoordinateSystem(UInt16 coordSys)
      uint16_t coordSys = ARG16;
      UInt16 old = WinSetCoordinateSystem(coordSys);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinSetCoordinateSystem(%d): %d", coordSys, old);
      m68k_set_reg(M68K_REG_D0, old);
     }
      break;

    case HDSelectorWinGetCoordinateSystem: {
      // UInt16 WinGetCoordinateSystem(void)
      UInt16 coordSys = WinGetCoordinateSystem();
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGetCoordinateSystem(): %d", coordSys);
      m68k_set_reg(M68K_REG_D0, coordSys);
     }
      break;

    case HDSelectorWinScalePoint: {
      // void WinScalePoint(PointType *pointP, Boolean ceiling)
      uint32_t pointP = ARG32;
      uint8_t ceiling = ARG8;
      PointType point;
      decode_point(pointP, &point);
      WinScalePoint(pointP ? &point : NULL, ceiling);
      encode_point(pointP, &point);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScalePoint(0x%08X [%d,%d], %d)", pointP, point.x, point.y, ceiling);
    }
    break;

    case HDSelectorWinUnscalePoint: {
      // void WinUnscalePoint(PointType *pointP, Boolean ceiling)
      uint32_t pointP = ARG32;
      uint8_t ceiling = ARG8;
      PointType point;
      decode_point(pointP, &point);
      WinUnscalePoint(pointP ? &point : NULL, ceiling);
      encode_point(pointP, &point);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinUnscalePoint(0x%08X [%d,%d], %d)", pointP, point.x, point.y, ceiling);
    }
      break;

    case HDSelectorWinScaleRectangle: {
      // void WinScaleRectangle(RectangleType *rectP)
      uint32_t rectP = ARG32;
      RectangleType rect;
      decode_rectangle(rectP, &rect);
      WinScaleRectangle(rectP ? &rect : NULL);
      encode_rectangle(rectP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScaleRectangle(0x%08X [%d,%d,%d,%d])", rectP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y);
    }
      break;

    case HDSelectorWinUnscaleRectangle: {
      // void WinUnscaleRectangle(RectangleType *rectP)
      uint32_t rectP = ARG32;
      RectangleType rect;
      decode_rectangle(rectP, &rect);
      WinUnscaleRectangle(rectP ? &rect : NULL);
      encode_rectangle(rectP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinUnscaleRectangle(0x%08X [%d,%d,%d,%d])", rectP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y);
    }
      break;

    case HDSelectorWinScreenGetAttribute: {
      // Err WinScreenGetAttribute(WinScreenAttrType selector, UInt32 *attrP)
      uint8_t selector = ARG8;
      uint32_t attrP = ARG32;
      UInt32 attr = 0;
      err = WinScreenGetAttribute(selector, attrP ? &attr : NULL);
      if (attrP) m68k_write_memory_32(attrP, attr);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScreenGetAttribute(%d, 0x%08X [%d]): %d", selector, attrP, attr, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
      break;

    //case HDSelectorWinPaintTiledBitmap:

    case HDSelectorWinGetSupportedDensity: {
      // Err WinGetSupportedDensity(UInt16 *densityP)
      uint32_t densityP = ARG32;
      UInt16 density = densityP ? m68k_read_memory_16(densityP) : 0;
      err = WinGetSupportedDensity(&density);
      if (densityP) m68k_write_memory_16(densityP, density);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinGetSupportedDensity(0x%08X): %d", densityP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;

    case HDSelectorEvtGetPenNative: {
      // void EvtGetPenNative(WinHandle winH, Int16* pScreenX, Int16* pScreenY, Boolean* pPenDown)
      uint32_t w = ARG32;
      uint32_t xP = ARG32;
      uint32_t yP = ARG32;
      uint32_t downP = ARG32;
      WinHandle wh = emupalmos_trap_sel_in(w, sysTrapPinsDispatch, sel, 0);
      Int16 x = 0, y = 0;
      Boolean down = false;
      EvtGetPenNative(wh, xP ? &x : NULL, yP ? &y : NULL, downP ? &down : NULL);
      if (xP) m68k_write_memory_16(xP, x);
      if (yP) m68k_write_memory_16(yP, y);
      if (downP) m68k_write_memory_8(downP, down);
      debug(DEBUG_TRACE, "EmuPalmOS", "EvtGetPenNative(0x%08X, 0x%08X, 0x%08X, 0x%08X)", w, xP, yP, downP);
    }
      break;
    case HDSelectorWinScaleCoord: {
      // Coord WinScaleCoord(Coord coord, Boolean ceiling)
      int16_t coord = ARG16;
      uint8_t ceiling = ARG8;
      Coord res = WinScaleCoord(coord, ceiling);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinScaleCoord(%d, %d): %d", coord, ceiling, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case HDSelectorWinUnscaleCoord: {
      // Coord WinUnscaleCoord(Coord coord, Boolean ceiling)
      int16_t coord = ARG16;
      uint8_t ceiling = ARG8;
      Coord res = WinUnscaleCoord(coord, ceiling);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinUnscaleCoord(%d, %d): %d", coord, ceiling, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;

    case HDSelectorWinPaintRoundedRectangleFrame: {
      // void WinPaintRoundedRectangleFrame(const RectangleType *rP, Coord width, Coord cornerRadius, Coord shadowWidth)
      uint32_t rectP = ARG32;
      RectangleType rect;
      decode_rectangle(rectP, &rect);
      int16_t width = ARG16;
      int16_t cornerRadius = ARG16;
      int16_t shadowWidth = ARG16;
      WinPaintRoundedRectangleFrame(&rect, width, cornerRadius, shadowWidth);
      debug(DEBUG_TRACE, "EmuPalmOS", "WinPaintRoundedRectangleFrame(0x%08X [%d,%d,%d,%d], %d, %d, %d)", rectP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, width, cornerRadius, shadowWidth);
    }
    break;

    //case HDSelectorWinSetScalingMode:
    //case HDSelectorWinGetScalingMode:

    default:
      sys_snprintf(buf, sizeof(buf)-1, "HighDensityDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
