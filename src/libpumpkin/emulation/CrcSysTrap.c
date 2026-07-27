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

void palmos_CrcSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapCrc16CalcBlock: {
      // UInt16 Crc16CalcBlock(const void *bufP, UInt16 count, UInt16 crc)
      uint32_t bufP = ARG32;
      uint16_t count = ARG16;
      uint16_t crc = ARG16;
      void *buf = emupalmos_trap_in(bufP, trap, 0);
      UInt16 res = Crc16CalcBlock(buf, count, crc);
      debug(DEBUG_TRACE, "EmuPalmOS", "Crc16CalcBlock(0x%08X, %d, 0x%04X): 0x%04X", bufP, count, crc, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
