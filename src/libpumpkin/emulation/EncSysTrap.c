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

void palmos_EncSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapEncDigestMD5: {
      // Err EncDigestMD5(UInt8 *strP, UInt16 strLen, UInt8 digestP[16])
      uint32_t strP = ARG32;
      uint16_t strLen = ARG16;
      uint32_t digestP = ARG32;
      UInt8 *str = emupalmos_trap_in(strP, trap, 0);
      UInt8 *digest = emupalmos_trap_in(digestP, trap, 2);
      Err res = EncDigestMD5(str, strLen, digest);
      debug(DEBUG_TRACE, "EmuPalmOS", "EncDigestMD5(0x%08X, %u, 0x%08X): %d", strP, strLen, digestP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
