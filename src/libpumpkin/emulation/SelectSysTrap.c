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

void palmos_SelectSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapSelectOneTime: {
      // Boolean SelectOneTime(Int16 *hour, Int16 *minute, const Char *titleP)
      uint32_t hourP = ARG32;
      uint32_t minP = ARG32;
      uint32_t titleP = ARG32;
      Int16 hour, min;
      emupalmos_trap_in(hourP, trap, 0);
      emupalmos_trap_in(minP, trap, 1);
      char *title = (char *)emupalmos_trap_in(titleP, trap, 2);
      if (hourP) hour = m68k_read_memory_16(hourP);
      if (minP) min = m68k_read_memory_16(minP);
      Boolean res = SelectOneTime(hourP ? &hour : NULL, minP ? &min : NULL, title);
      if (hourP) m68k_write_memory_16(hourP, hour);
      if (minP) m68k_write_memory_16(minP, min);
      debug(DEBUG_TRACE, "EmuPalmOS", "SelectOneTime(0x%08X, 0x%08X, 0x%08X): %d", hourP, minP, titleP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSelectDay: {
      // Boolean SelectDay(const SelectDayType selectDayBy, Int16 *month, Int16 *day, Int16 *year, const Char *title)
      uint8_t selectDayBy = ARG8;
      uint32_t monthP = ARG32;
      uint32_t dayP = ARG32;
      uint32_t yearP = ARG32;
      uint32_t titleP = ARG32;
      Int16 month, day, year;
      emupalmos_trap_in(monthP, trap, 1);
      emupalmos_trap_in(dayP, trap, 2);
      emupalmos_trap_in(yearP, trap, 3);
      char *title = (char *)emupalmos_trap_in(titleP, trap, 4);
      if (monthP) month = m68k_read_memory_16(monthP);
      if (dayP) day = m68k_read_memory_16(dayP);
      if (yearP) year = m68k_read_memory_16(yearP);
      Boolean res = SelectDay(selectDayBy, monthP ? &month : NULL, dayP ? &day : NULL, yearP ? &year : NULL, title);
      if (monthP) m68k_write_memory_16(monthP, month);
      if (dayP) m68k_write_memory_16(dayP, day);
      if (yearP) m68k_write_memory_16(yearP, year);
      debug(DEBUG_TRACE, "EmuPalmOS", "SelectDay(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", selectDayBy, monthP, dayP, yearP, titleP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
