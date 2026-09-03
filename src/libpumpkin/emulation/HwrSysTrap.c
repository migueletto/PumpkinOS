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

void palmos_HwrSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapHwrGetROMToken: {
      // Err HwrGetROMToken(UInt16 cardNo, UInt32 token, out UInt8 **dataP, out UInt16 *sizeP)
      uint16_t cardNo = ARG16;
      uint32_t token = ARG32;
      uint32_t dataP = ARG32;
      UInt8 *l_dataP;
      uint32_t sizeP = ARG32;
      UInt16 l_sizeP;
      emupalmos_trap_in(dataP, trap, 2);
      emupalmos_trap_in(sizeP, trap, 3);
      Err res = HwrGetROMToken(cardNo, token, &l_dataP, &l_sizeP);
      if (dataP) m68k_write_memory_32(dataP, emupalmos_trap_out(l_dataP));
      if (sizeP) m68k_write_memory_16(sizeP, l_sizeP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "HwrGetROMToken(cardNo=%d, token=%d, dataP=0x%08X, sizeP=0x%08X): %d", cardNo, token, dataP, sizeP, res);
    }
    break;
  }
}
