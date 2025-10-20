#include <PalmOS.h>
#include <VFSMgr.h>
    
#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"
    
void palmos_lmtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  Err err;
    
  switch (sel) {
    case lmGetNumLocales: {
      // UInt16 LmGetNumLocales(void)
      UInt16 num = LmGetNumLocales();
      debug(DEBUG_TRACE, "EmuPalmOS", "LmGetNumLocales(): %u", num);
      m68k_set_reg(M68K_REG_D0, num);
    }
    break;
    case lmLocaleToIndex: {
      // Err LmLocaleToIndex(const LmLocaleType *iLocale, UInt16 *oLocaleIndex)
      uint32_t localeP = ARG32;
      uint32_t indexP = ARG32;
      emupalmos_trap_sel_in(localeP, sysTrapLmDispatch, sel, 0);
      emupalmos_trap_sel_in(indexP, sysTrapLmDispatch, sel, 1);
      LmLocaleType locale;
      decode_locale(localeP, &locale);
      UInt16 index;
      err = LmLocaleToIndex(localeP ? &locale : NULL, indexP ? &index : NULL);
      if (indexP) m68k_write_memory_16(indexP, index);
      debug(DEBUG_TRACE, "EmuPalmOS", "LmLocaleToIndex(0x%08X, 0x%08X): %d", localeP, indexP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case lmGetLocaleSetting: {
      // Err LmGetLocaleSetting(UInt16 iLocaleIndex, LmLocaleSettingChoice iChoice, void *oValue, UInt16 iValueSize)
      uint16_t index = ARG16;
      uint16_t choice = ARG16;
      uint32_t valueP = ARG32;
      uint16_t size = ARG16;
      emupalmos_trap_sel_in(valueP, sysTrapLmDispatch, sel, 2);
      uint8_t buf[256];
      uint32_t len;
      err = LmGetLocaleSetting(index, choice, buf, sizeof(buf));
      if (valueP) {
        switch (choice) {
          case lmChoiceLocale:
            encode_locale(valueP, (LmLocaleType *)buf);
            break;
          case lmChoiceCountryName:
            len = sizeof(buf);
            if (len > size) len = size;
            encode_string(valueP, (char *)buf, len);
            break;
          case lmChoiceDateFormat:
          case lmChoiceLongDateFormat:
          case lmChoiceTimeFormat:
          case lmChoiceWeekStartDay:
          case lmChoiceNumberFormat:
          case lmChoiceMeasurementSystem:
            m68k_write_memory_8(valueP, buf[0]);
            break;
          case lmChoiceTimeZone:
            m68k_write_memory_16(valueP, *(UInt16 *)buf);
            break;
        }
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "LmGetLocaleSetting(%u, %u, 0x%08X, %u): %d", index, choice, valueP, size, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "LmDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
