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

void palmos_ClipboardSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapClipboardAddItem: {
      // void ClipboardAddItem(const ClipboardFormatType format, const void *ptr, UInt16 length)
      uint8_t format = ARG8;
      uint32_t ptrP = ARG32;
      uint16_t length = ARG16;
      void *ptr = emupalmos_trap_in(ptrP, trap, 1);
      ClipboardAddItem(format, ptr, length);
      debug(DEBUG_TRACE, "EmuPalmOS", "ClipboardAddItem(%d, 0x%08X, %d)", format, ptrP, length);
    }
    break;
    case sysTrapClipboardGetItem: {
      // MemHandle ClipboardGetItem(const ClipboardFormatType format, UInt16 *length)
      uint8_t format = ARG8;
      uint32_t lengthP = ARG32;
      emupalmos_trap_in(lengthP, trap, 1);
      UInt16 length;
      MemHandle h = ClipboardGetItem(format, &length);
      uint32_t r = emupalmos_trap_out(h);
      debug(DEBUG_TRACE, "EmuPalmOS", "ClipboardGetItem(%d, 0x%08X): 0x%08X", format, lengthP, r);
      m68k_set_reg(M68K_REG_A0, r);
    }
    break;
  }
}
