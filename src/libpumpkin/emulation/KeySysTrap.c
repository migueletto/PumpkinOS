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

void palmos_KeySysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapKeyCurrentState: {
      // UInt32 KeyCurrentState(void)
      UInt32 res = KeyCurrentState();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "KeyCurrentState(): %d", res);
    }
    break;
    case sysTrapKeyRates: {
      // Err KeyRates(Boolean set, inout UInt16 *initDelayP, inout UInt16 *periodP, inout UInt16 *doubleTapDelayP, inout Boolean *queueAheadP)
      uint8_t set = ARG8;
      uint32_t initDelayP = ARG32;
      UInt16 l_initDelayP;
      if (initDelayP) l_initDelayP = m68k_read_memory_16(initDelayP);
      uint32_t periodP = ARG32;
      UInt16 l_periodP;
      if (periodP) l_periodP = m68k_read_memory_16(periodP);
      uint32_t doubleTapDelayP = ARG32;
      UInt16 l_doubleTapDelayP;
      if (doubleTapDelayP) l_doubleTapDelayP = m68k_read_memory_16(doubleTapDelayP);
      uint32_t queueAheadP = ARG32;
      Boolean l_queueAheadP;
      if (queueAheadP) l_queueAheadP = m68k_read_memory_8(queueAheadP);
      Err res = KeyRates(set, initDelayP ? &l_initDelayP : NULL, periodP ? &l_periodP : NULL, doubleTapDelayP ? &l_doubleTapDelayP : NULL, queueAheadP ? &l_queueAheadP : NULL);
      if (initDelayP) m68k_write_memory_16(initDelayP, l_initDelayP);
      if (periodP) m68k_write_memory_16(periodP, l_periodP);
      if (doubleTapDelayP) m68k_write_memory_16(doubleTapDelayP, l_doubleTapDelayP);
      if (queueAheadP) m68k_write_memory_8(queueAheadP, l_queueAheadP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "KeyRates(set=%d, initDelayP=0x%08X [%d], periodP=0x%08X [%d], doubleTapDelayP=0x%08X [%d], queueAheadP=0x%08X): %d", set, initDelayP, l_initDelayP, periodP, l_periodP, doubleTapDelayP, l_doubleTapDelayP, queueAheadP, res);
    }
    break;
    case sysTrapKeySetMask: {
      // UInt32 KeySetMask(UInt32 keyMask)
      uint32_t keyMask = ARG32;
      UInt32 res = KeySetMask(keyMask);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "KeySetMask(keyMask=%d): %d", keyMask, res);
    }
    break;
  }
}
