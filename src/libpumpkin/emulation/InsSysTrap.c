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

void palmos_InsSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
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
  }
}
