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

void palmos_RctSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
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
  }
}
