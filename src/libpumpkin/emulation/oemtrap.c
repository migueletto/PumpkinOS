#include <PalmOS.h>
#include <VFSMgr.h>
#include <INetMgr.h>
#include <HsNavCommon.h>
#include <HsExtTraps.h>
#include <HsExt.h>

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

void palmos_oemtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    case hsSelKeyCurrentStateExt: {
      // void HsKeyCurrentStateExt(UInt32 keys[3])
      UInt32 keysP = ARG32;
      UInt32 *keys = (UInt32 *)emupalmos_trap_in(keysP, sysTrapOmDispatch, 0);
      HsKeyCurrentStateExt(keys);
      debug(DEBUG_TRACE, "EmuPalmOS", "HsKeyCurrentStateExt(0x%08X)", keysP);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "OEMDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
