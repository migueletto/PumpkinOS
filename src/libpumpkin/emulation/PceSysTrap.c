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

void palmos_PceSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapPceNativeCall: {
      // UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
    #ifdef ARMEMU
      uint32_t nativeFuncP = ARG32;
      uint32_t userDataP = ARG32;
      emupalmos_trap_in(nativeFuncP, trap, 0);
      emupalmos_trap_in(userDataP, trap, 1);
      UInt32 res = arm_native_call_pce(nativeFuncP, userDataP);
      debug(DEBUG_TRACE, "EmuPalmOS", "PceNativeCall(0x%08X, 0x%08X): %d", nativeFuncP, userDataP, res);
      m68k_set_reg(M68K_REG_A0, res);
      m68k_set_reg(M68K_REG_D0, res);
    #endif
    }
    break;
  }
}
