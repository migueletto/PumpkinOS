#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <SonyCLIE.h>
#include <SonyHRLib.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_sonyhrtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      //Err HROpen(UInt16 refNum)
      uint16_t refNum = ARG16;
      err = HROpen(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "HROpen(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      //Err HRClose(UInt16 refNum)
      uint16_t refNum = ARG16;
      err = HRClose(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "HRClose(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      //Err HRSleep(UInt16 refNum)
      m68k_set_reg(M68K_REG_D0, errNone);
      }
      break;
    case sysLibTrapWake: {
      //Err HRWake(UInt16 refNum)
      m68k_set_reg(M68K_REG_D0, errNone);
      }
      break;
    case HRTrapGetAPIVersion: {
      //Err HRGetAPIVersion(UInt16 refNum, UInt16 *versionP)
      uint16_t refNum = ARG16;
      uint32_t versionP = ARG32;
      UInt16 version;
      err = HRGetAPIVersion(refNum, &version);
      if (versionP) m68k_write_memory_16(versionP, version);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case HRTrapWinScreenMode: {
      //Err HRWinScreenMode(UInt16 refNum, WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP)
      uint16_t refNum = ARG16;
      uint8_t operation = ARG8;
      uint32_t widthP = ARG32;
      uint32_t heightP = ARG32;
      uint32_t depthP = ARG32;
      uint32_t enableColorP = ARG32;
      emupalmos_trap_in(widthP, trap, 2);
      emupalmos_trap_in(heightP, trap, 3);
      emupalmos_trap_in(depthP, trap, 4);
      emupalmos_trap_in(enableColorP, trap, 5);
      uint32_t width = 0, height = 0, depth = 0;
      Boolean enableColor = 0;
      if (widthP) width = m68k_read_memory_32(widthP);
      if (heightP) height = m68k_read_memory_32(heightP);
      if (depthP) depth = m68k_read_memory_32(depthP);
      if (enableColorP) enableColor = m68k_read_memory_8(enableColorP);
      Err err = HRWinScreenMode(refNum, operation, widthP ? &width : NULL, heightP ? &height : NULL, depthP ? &depth : NULL, enableColorP ? &enableColor : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "HRWinScreenMode(%u, %d, 0x%08X [%d], 0x%08X [%d], 0x%08X [%d], 0x%08X [%d]): %d",
        refNum, operation, widthP, width, heightP, height, depthP, depth, enableColorP, enableColor, err);
      if (widthP) m68k_write_memory_32(widthP, width);
      if (heightP) m68k_write_memory_32(heightP, height);
      if (depthP) m68k_write_memory_32(depthP, depth);
      if (enableColorP) m68k_write_memory_8(enableColorP, enableColor);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case HRTrapWinDrawLine: {
      //void HRWinDrawLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2)
      uint16_t refNum = ARG16;
      int16_t x1 = ARG16;
      int16_t y1 = ARG16;
      int16_t x2 = ARG16;
      int16_t y2 = ARG16;
      HRWinDrawLine(refNum, x1, y1, x2, y2);
      debug(DEBUG_TRACE, "EmuPalmOS", "HRWinDrawLine(%u, x1=%d, y1=%d, x2=%d, y2=%d)", refNum, x1, y1, x2, y2);
      }
      break;
    case HRTrapWinDrawRectangle: {
      //void HRWinDrawRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam)
      uint16_t refNum = ARG16;
      uint32_t rP = ARG32;
      uint16_t cornerDiam = ARG16;
      RectangleType rect;
      decode_rectangle(rP, &rect);
      HRWinDrawRectangle(refNum, &rect, cornerDiam);
      encode_rectangle(rP, &rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "HRWinDrawRectangle(%u, rP=0x%08X [%d,%d,%d,%d], cornerDiam=%d)",
        refNum, rP, rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y, cornerDiam);
      }
      break;
    case HRTrapWinDrawBitmap: {
      //void HRWinDrawBitmap(UInt16 refNum, BitmapPtr bitmapP, Coord x, Coord y)
      uint16_t refNum = ARG16;
      uint32_t bitmapP = ARG32;
      int16_t x = ARG16;
      int16_t y = ARG16;
      BitmapType *bitmap = emupalmos_trap_in(bitmapP, trap, 1);
      HRWinDrawBitmap(refNum, bitmap, x, y);
      debug(DEBUG_TRACE, "EmuPalmOS", "HRWinDrawBitmap(%u, bitmapP=0x%08X, x=%d, y=%d)", refNum, bitmapP, x, y);
      }
      break;
    case HRTrapBmpCreate: {
      //BitmapType *HRBmpCreate(UInt16 refNum, Coord width, Coord height, UInt8 depth, ColorTableType *colorTableP, UInt16 *error)
      uint16_t refNum = ARG16;
      int16_t width = ARG16;
      int16_t height = ARG16;
      uint8_t depth = ARG8;
      uint32_t colorTableP = ARG32;
      uint32_t errorP = ARG32;
      ColorTableType *colorTable = emupalmos_trap_in(colorTableP, trap, 4);
      UInt16 error;
      BitmapType *bitmap = HRBmpCreate(refNum, width, height, depth, colorTable, &error);
      if (errorP) m68k_write_memory_16(errorP, error);
      uint32_t bitmapP = emupalmos_trap_out(bitmap);
      debug(DEBUG_TRACE, "EmuPalmOS", "HRBmpCreate(%u, width=%d, height=%d, depth=%d, colorTableP=0x%08X, error=0x%08X [%d]): 0x%08X",
        refNum, width, height, depth, colorTableP, errorP, error, bitmapP);
      m68k_set_reg(M68K_REG_A0, bitmapP);
      }
      break;
    case HRTrapWinCreateBitmapWindow: {
      //WinHandle HRWinCreateBitmapWindow(UInt16 refNum, BitmapType *bitmapP, UInt16 *error)
      uint16_t refNum = ARG16;
      uint32_t bitmapP = ARG32;
      uint32_t errorP = ARG32;
      BitmapType *bitmap = emupalmos_trap_in(bitmapP, trap, 1);
      UInt16 error;
      WinHandle wh = HRWinCreateBitmapWindow(refNum, bitmap, &error);
      if (errorP) m68k_write_memory_16(errorP, error);
      uint32_t whP = emupalmos_trap_out(wh);
      debug(DEBUG_TRACE, "EmuPalmOS", "HRWinCreateBitmapWindow(%u, 0x%08X, 0x%08X [%d]): 0x%08X", refNum, bitmapP, errorP, error, whP);
      m68k_set_reg(M68K_REG_A0, whP);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "SonyHRLib trap 0x%04X not mapped", trap);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
