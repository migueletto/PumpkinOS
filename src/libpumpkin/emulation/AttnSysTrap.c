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

void palmos_AttnSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapAttnListOpen: {
      // void AttnListOpen(void)
      AttnListOpen();
      debug(DEBUG_TRACE, "EmuPalmOS", "AttnListOpen()");
    }
    break;
    case sysTrapAttnIndicatorEnable: {
      // void AttnIndicatorEnable(Boolean enableIt)
      uint16_t enableIt = ARG8;
      AttnIndicatorEnable(enableIt);
      debug(DEBUG_TRACE, "EmuPalmOS", "AttnIndicatorEnable(%d)", enableIt);
    }
    break;
    case sysTrapAttnIterate: {
      // void AttnIterate(UInt16 cardNo, LocalID dbID, UInt32 iterationData)
      uint16_t cardNo = ARG16;
      uint32_t dbID = ARG32;
      uint32_t iterationData = ARG32;
      AttnIterate(cardNo, dbID, iterationData);
      debug(DEBUG_TRACE, "EmuPalmOS", "AttnIterate(%d, 0x%08X, %u)", cardNo, dbID, iterationData);
    }
    break;
  }
}
