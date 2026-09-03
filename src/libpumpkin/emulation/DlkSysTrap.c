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

void palmos_DlkSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapDlkGetSyncInfo: {
      // Err DlkGetSyncInfo(UInt32 *succSyncDateP, UInt32 *lastSyncDateP, DlkSyncStateType *syncStateP, Char *nameBufP, Char *logBufP, Int32 *logLenP)
      uint32_t succSyncDateP = ARG32;
      uint32_t lastSyncDateP = ARG32;
      uint32_t syncStateP = ARG32;
      uint32_t nameBufP = ARG32;
      uint32_t logBufP = ARG32;
      uint32_t logLenP = ARG32;
      UInt32 succSyncDate, lastSyncDate;
      DlkSyncStateType syncState;
      emupalmos_trap_in(succSyncDateP, trap, 0);
      emupalmos_trap_in(lastSyncDateP, trap, 1);
      emupalmos_trap_in(syncStateP, trap, 2);
      char *nameBuf = emupalmos_trap_in(nameBufP, trap, 3);
      char *logBuf = emupalmos_trap_in(logBufP, trap, 4);
      Int32 logLen;
      Err err = DlkGetSyncInfo(&succSyncDate, &lastSyncDate, &syncState, nameBuf, logBuf, &logLen);
      if (succSyncDateP) m68k_write_memory_32(succSyncDateP, succSyncDate);
      if (lastSyncDateP) m68k_write_memory_32(lastSyncDateP, lastSyncDate);
      if (syncStateP) m68k_write_memory_8(syncStateP, syncState);
      if (logLenP) m68k_write_memory_32(logLenP, logLen);
      debug(DEBUG_TRACE, "EmuPalmOS", "DlkGetSyncInfo(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", succSyncDateP, lastSyncDateP, syncStateP, nameBufP, logBufP, logLenP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
  }
}
