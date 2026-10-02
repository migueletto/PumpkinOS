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
      debug(DEBUG_TRACE, "EmuPalmOS", "HsKeyCurrentStateExt(0x%08X [0x%08X 0x%08X 0x%08X])",
        keysP, keys ? keys[0] : 0, keys ? keys[1] : 0, keys ? keys[2] : 0);
      }
      break;
    case hsSelKeySetMaskExt: {
      // void HsKeySetMaskExt(const UInt32 keyMaskNew[3], UInt32 keyMaskOld[3])
      UInt32 keyMaskNewP = ARG32;
      UInt32 keyMaskOldP = ARG32;
      UInt32 *keyMaskNew = (UInt32 *)emupalmos_trap_in(keyMaskNewP, sysTrapOmDispatch, 0);
      UInt32 *keyMaskOld = (UInt32 *)emupalmos_trap_in(keyMaskOldP, sysTrapOmDispatch, 1);
      HsKeySetMaskExt(keyMaskNew, keyMaskOld);
      debug(DEBUG_TRACE, "EmuPalmOS", "HsKeySetMaskExt(0x%08X [0x%08X 0x%08X 0x%08X] 0x%08X [0x%08X 0x%08X 0x%08X])",
        keyMaskNewP, keyMaskNew ? keyMaskNew[0] : 0, keyMaskNew ? keyMaskNew[1] : 0, keyMaskNew ? keyMaskNew[2] : 0,
        keyMaskOldP, keyMaskOld ? keyMaskOld[0] : 0, keyMaskOld ? keyMaskOld[1] : 0, keyMaskOld ? keyMaskOld[2] : 0);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "OEMDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
