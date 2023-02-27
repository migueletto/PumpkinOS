#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_omtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    //case omInit:
    //case omOpenOverlayDatabase:
    //case omLocaleToOverlayDBName:
    //case omOverlayDBNameToLocale:
    case omGetCurrentLocale: {
      // void OmGetCurrentLocale(LmLocaleType *currentLocale)
      uint32_t localeP = ARG32;
      emupalmos_trap_sel_in(localeP, sysTrapOmDispatch, sel, 0);
      LmLocaleType locale;
      OmGetCurrentLocale(&locale);
      encode_locale(localeP, &locale);
      debug(DEBUG_TRACE, "EmuPalmOS", "OmGetCurrentLocale(0x%08X)", localeP);
    }
      break;
    //case omGetIndexedLocale:
    case omGetSystemLocale: {
      // void OmGetSystemLocale(LmLocaleType *systemLocale)
      uint32_t localeP = ARG32;
      emupalmos_trap_sel_in(localeP, sysTrapOmDispatch, sel, 0);
      LmLocaleType locale;
      OmGetSystemLocale(&locale);
      encode_locale(localeP, &locale);
      debug(DEBUG_TRACE, "EmuPalmOS", "OmGetSystemLocale(0x%08X)", localeP);
    }
    break;
    //case omSetSystemLocale:
    //case omGetRoutineAddress:
    //case omGetNextSystemLocale:
    default:
      sys_snprintf(buf, sizeof(buf)-1, "OmDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
