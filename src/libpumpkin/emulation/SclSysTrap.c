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

void palmos_SclSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
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
  }
}
