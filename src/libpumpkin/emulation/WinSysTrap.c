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

void palmos_WinSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
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
  }
}
