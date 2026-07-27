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

void palmos_AlmSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapAlmSetAlarm: {
      // Err AlmSetAlarm(UInt16 cardNo, LocalID dbID, UInt32 ref, UInt32 alarmSeconds, Boolean quiet)
      uint16_t cardNo = ARG16;
      uint32_t dbID = ARG32;
      uint32_t ref = ARG32;
      uint32_t alarmSeconds = ARG32;
      uint8_t quiet = ARG8;
      Err err = AlmSetAlarm(cardNo, dbID, ref, alarmSeconds, quiet);
      debug(DEBUG_TRACE, "EmuPalmOS", "AlmSetAlarm(%d, 0x%08X, %u, %u, %u): %d", cardNo, dbID, ref, alarmSeconds, quiet, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapAlmGetAlarm: {
      // UInt32 AlmGetAlarm(UInt16 cardNo, LocalID dbID, UInt32 *refP)
      uint16_t cardNo = ARG16;
      uint32_t dbID = ARG32;
      uint32_t refP = ARG32;
      emupalmos_trap_in(refP, trap, 2);
      UInt32 ref;
      UInt32 res = AlmGetAlarm(cardNo, dbID, refP ? &ref : NULL);
      if (refP) m68k_write_memory_32(refP, ref);
      debug(DEBUG_TRACE, "EmuPalmOS", "AlmGetAlarm(%d, 0x%08X, 0x%08X): %u", cardNo, dbID, refP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
