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

void palmos_HostSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapHostControl: {
      // UInt32 HostControl(HostControlTrapNumber selector, ...)
      uint16_t selector = ARG16;
      UInt32 res = 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "HostControl(0x%04X): 0x%08X", selector, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
