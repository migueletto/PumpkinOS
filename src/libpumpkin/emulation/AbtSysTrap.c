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

void palmos_AbtSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapAbtShowAbout: {
      // void AbtShowAbout(UInt32 creator)
      uint32_t creator = ARG32;
      AbtShowAbout(creator);
      debug(DEBUG_TRACE, "EmuPalmOS", "AbtShowAbout(%d)", creator);
    }
    break;
  }
}
