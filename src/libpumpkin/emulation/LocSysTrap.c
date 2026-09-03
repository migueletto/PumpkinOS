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

void palmos_LocSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapLocGetNumberSeparators: {
      // void LocGetNumberSeparators(NumberFormatType numberFormat, Char *thousandSeparator, Char *decimalSeparator)
      uint8_t numberFormat = ARG8;
      uint32_t thousandSeparatorP = ARG32;
      uint32_t decimalSeparatorP = ARG32;
      emupalmos_trap_in(thousandSeparatorP, trap, 0);
      emupalmos_trap_in(decimalSeparatorP, trap, 1);
      char thousandSeparator, decimalSeparator;
      LocGetNumberSeparators(numberFormat, &thousandSeparator, &decimalSeparator);
      if (thousandSeparatorP) m68k_write_memory_8(thousandSeparatorP, thousandSeparator);
      if (decimalSeparatorP) m68k_write_memory_8(decimalSeparatorP, decimalSeparator);
      debug(DEBUG_TRACE, "EmuPalmOS", "LocGetNumberSeparators(%d, %u, %u)", numberFormat, thousandSeparatorP, decimalSeparatorP);
    }
    break;
  }
}
