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

void palmos_TimeSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapTimeToAscii: {
      // void TimeToAscii(UInt8 hours, UInt8 minutes, TimeFormatType timeFormat, Char *pString)
      uint8_t hours = ARG8;
      uint8_t minutes = ARG8;
      uint8_t timeFormat = ARG8;
      uint32_t stringP = ARG32;
      char *string = (char *)emupalmos_trap_in(stringP, trap, 3);
      TimeToAscii(hours, minutes, timeFormat, string);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimeToAscii(%u, %u, %u, 0x%08X \"%s\")", hours, minutes, timeFormat, stringP, string ? string : "");
    }
    break;
    case sysTrapTimeZoneToAscii: {
      // void TimeZoneToAscii(Int16 timeZone, const LmLocaleType *localeP, Char *string)
      int16_t timeZone = ARG16;
      uint32_t localeP = ARG32;
      uint32_t stringP = ARG32;
      emupalmos_trap_in(localeP, trap, 1);
      char *string = (char *)emupalmos_trap_in(stringP, trap, 2);
      LmLocaleType locale;
      decode_locale(localeP, &locale);
      TimeZoneToAscii(timeZone, localeP ? &locale : NULL, string);
      debug(DEBUG_TRACE, "EmuPalmOS", "TimeZoneToAscii(%d, 0x%08X, 0x%08X )", timeZone, localeP, stringP);
    }
    break;
  }
}
