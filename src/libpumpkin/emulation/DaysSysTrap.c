#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

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

void palmos_DaysSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapDaysInMonth: {
      // Int16 DaysInMonth(Int16 month, Int16 year)
      int16_t month = ARG16;
      int16_t year = ARG16;
      Int16 res = DaysInMonth(month, year);
      debug(DEBUG_TRACE, "EmuPalmOS", "DaysInMonth(%d, %d): %d", month, year, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
