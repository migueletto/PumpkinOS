#include <PalmOS.h>
#include <VFSMgr.h>
    
#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"
    
void palmos_tsmtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
    
  switch (sel) {
    case tsmGetFepMode: {
      // TsmFepModeType TsmGetFepMode(void *nullParam)
      uint32_t nullParamP = ARG32;
      void *nullParam = emupalmos_trap_sel_in(nullParamP, sysTrapTsmDispatch, sel, 0);
      TsmFepModeType res = TsmGetFepMode(nullParam);
      debug(DEBUG_TRACE, "EmuPalmOS", "TsmGetFepMode(0x%08X): %d", nullParamP, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    case tsmSetFepMode: {
      // TsmFepModeType TsmSetFepMode(void *nullParam, TsmFepModeType inNewMode)
      uint32_t nullParamP = ARG32;
      uint16_t inNewMode = ARG16;
      void *nullParam = emupalmos_trap_sel_in(nullParamP, sysTrapTsmDispatch, sel, 0);
      TsmFepModeType res = TsmSetFepMode(nullParam, inNewMode);
      debug(DEBUG_TRACE, "EmuPalmOS", "TsmSetFepMode(0x%08X, %d): %d", nullParamP, inNewMode, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "TsmDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
