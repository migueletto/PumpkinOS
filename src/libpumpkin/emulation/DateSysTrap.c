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

void palmos_DateSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapDateSecondsToDate: {
      // void DateSecondsToDate(UInt32 seconds, DateType *dateP)
      uint32_t seconds = ARG32;
      uint32_t dateP = ARG32;
      emupalmos_trap_in(dateP, trap, 1);
      union {
        UInt16 bits;
        DateType fields;
      } date;
      DateSecondsToDate(seconds, dateP ? &date.fields : NULL);
      if (dateP) m68k_write_memory_16(dateP, date.bits);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateSecondsToDate(%u, 0x%08X)", seconds, dateP);
    }
    break;
    case sysTrapDateToDOWDMFormat: {
      // void DateToDOWDMFormat(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString)
      uint8_t months = ARG8;
      uint8_t days = ARG8;
      uint16_t years = ARG16;
      uint8_t dateFormat = ARG8;
      uint32_t stringP = ARG32;
      char *string = (char *)emupalmos_trap_in(stringP, trap, 4);
      DateToDOWDMFormat(months, days, years, dateFormat, string);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToDOWDMFormat(%u, %u, %u, %u, 0x%08X)", months, days, years, dateFormat, stringP);
    }
    break;
    case sysTrapDateToAscii: {
      // void DateToAscii(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString)
      uint8_t months = ARG8;
      uint8_t days = ARG8;
      uint16_t years = ARG16;
      uint8_t dateFormat = ARG8;
      uint32_t stringP = ARG32;
      char *string = (char *)emupalmos_trap_in(stringP, trap, 4);
      DateToAscii(months, days, years, dateFormat, string);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToAscii(%u, %u, %u, %u, 0x%08X)", months, days, years, dateFormat, stringP);
    }
    break;
    case sysTrapDateToDays: {
      // UInt32 DateToDays(DateType date)
      union {
        UInt16 bits;
        DateType fields;
      } date;
      date.bits = ARG16;
      UInt32 res = DateToDays(date.fields);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateToDays(0x%04X [%04d-%02d-%02d]): %d", date.bits, date.fields.year+1904, date.fields.month, date.fields.day, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapDateDaysToDate: {
      // void DateDaysToDate(UInt32 days, DateType *dateP)
      uint32_t days = ARG32;
      uint32_t dateP = ARG32;
      emupalmos_trap_in(dateP, trap, 1);
      union {
        UInt16 bits;
        DateType fields;
      } date;
      DateDaysToDate(days, dateP ? &date.fields : NULL);
      if (dateP) m68k_write_memory_16(dateP, date.bits);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateDaysToDate(%u, 0x%08X)", days, dateP);
    }
    break;
    case sysTrapDateAdjust: {
      // void DateAdjust(DateType *dateP, Int32 adjustment)
      uint32_t dateP = ARG32;
      int32_t adjustment = ARG32;
      emupalmos_trap_in(dateP, trap, 0);
      union {
        UInt16 bits;
        DateType fields;
      } date;
      if (dateP) date.bits = m68k_read_memory_16(dateP);
      DateAdjust(&date.fields, adjustment);
      if (dateP) m68k_write_memory_16(dateP, date.bits);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateAdjust(0x%08X, %d)", dateP, adjustment);
    }
    break;
    case sysTrapDateTemplateToAscii: {
      // UInt16 DateTemplateToAscii(const Char *templateP, UInt8 months, UInt8 days, UInt16 years, Char *stringP, Int16 stringLen)
      uint32_t templateP = ARG32;
      uint8_t months = ARG8;
      uint8_t days = ARG8;
      uint16_t years = ARG16;
      uint32_t stringP = ARG32;
      int16_t stringLen = ARG16;
      char *template = (char *)emupalmos_trap_in(templateP, trap, 0);
      char *string = (char *)emupalmos_trap_in(stringP, trap, 4);
      UInt16 res = DateTemplateToAscii(template, months, days, years, string, stringLen);
      debug(DEBUG_TRACE, "EmuPalmOS", "DateTemplateToAscii(0x%08X, %u, %u, %u, 0x%08X \"%s\", %d): %u", templateP, months, days, years, stringP, string ? string : "", stringLen, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
