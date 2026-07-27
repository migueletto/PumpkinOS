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

void palmos_GrfSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapGrfGetState: {
      // Err GrfGetState(Boolean *capsLockP, Boolean *numLockP, UInt16 *tempShiftP, Boolean *autoShiftedP)
      uint32_t capsLockP = ARG32;
      uint32_t numLockP = ARG32;
      uint32_t tempShiftP = ARG32;
      uint32_t autoShiftedP = ARG32;
      emupalmos_trap_in(capsLockP, trap, 0);
      emupalmos_trap_in(numLockP, trap, 1);
      emupalmos_trap_in(tempShiftP, trap, 2);
      emupalmos_trap_in(autoShiftedP, trap, 3);
      Boolean capsLock, numLock, autoShifted;
      UInt16 tempShift;
      Err err = GrfGetState(&capsLock, &numLock, &tempShift, &autoShifted);
      debug(DEBUG_TRACE, "EmuPalmOS", "GrfGetState(%d, %d, %d, %d): %d", capsLock, numLock, tempShift, autoShifted, err);
      if (capsLockP) m68k_write_memory_8(capsLockP, capsLock);
      if (numLockP) m68k_write_memory_8(numLockP, numLock);
      if (tempShiftP) m68k_write_memory_16(tempShiftP, tempShift);
      if (autoShiftedP) m68k_write_memory_8(autoShiftedP, autoShifted);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapGrfSetState: {
      // Err GrfSetState(Boolean capsLock, Boolean numLock, Boolean upperShift)
      uint8_t capsLock = ARG8;
      uint8_t numLock = ARG8;
      uint8_t upperShift = ARG8;
      Err err = GrfSetState(capsLock, numLock, upperShift);
      debug(DEBUG_TRACE, "EmuPalmOS", "GrfSetState(%d, %d, %d): %d", capsLock, numLock, upperShift, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
  }
}
