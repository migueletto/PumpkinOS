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

void palmos_GsiSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
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
  }
}
