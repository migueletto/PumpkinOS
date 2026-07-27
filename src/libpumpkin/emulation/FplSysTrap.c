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

void palmos_FplSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapFplInit:
      // Err FplInit(void)
      debug(DEBUG_TRACE, "EmuPalmOS", "FplInit()");
      m68k_set_reg(M68K_REG_D0, 0);
    break;
    case sysTrapFplFree:
      // void FplFree(void)
      debug(DEBUG_TRACE, "EmuPalmOS", "FplFree()");
    break;
  }
}
