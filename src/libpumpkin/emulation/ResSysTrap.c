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

void palmos_ResSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapResLoadConstant: {
      // UInt32 ResLoadConstant(UInt16 rscID)
      uint16_t rscID = ARG16;
      UInt32 res = ResLoadConstant(rscID);
      debug(DEBUG_TRACE, "EmuPalmOS", "ResLoadConstant(%d): 0x%08X", rscID, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
