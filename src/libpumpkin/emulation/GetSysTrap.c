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

void palmos_GetSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapGetCharCaselessValue: {
      // const UInt8 *GetCharCaselessValue(void)
      UInt8 *res = (UInt8 *)GetCharCaselessValue();
      uint32_t a = emupalmos_trap_out(res);
      debug(DEBUG_TRACE, "EmuPalmOS", "GetCharCaselessValue(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapGetCharSortValue: {
      // const UInt8 *GetCharSortValue(void) 
      UInt8 *res = (UInt8 *)GetCharSortValue();
      uint32_t a = emupalmos_trap_out(res);
      debug(DEBUG_TRACE, "EmuPalmOS", "GetCharSortValue(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
  }
}
