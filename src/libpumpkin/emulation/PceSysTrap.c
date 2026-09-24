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
#include "bytes.h"
#include "debug.h"

void palmos_PceSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapPceNativeCall: {
      // UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
#ifdef ARMEMU
      uint32_t nativeFuncP = ARG32;
      uint32_t userDataP = ARG32;
      emupalmos_trap_in(nativeFuncP, trap, 0);
      emupalmos_trap_in(userDataP, trap, 1);
      UInt32 res = arm_native_call_pce(nativeFuncP, userDataP);
      debug(DEBUG_TRACE, "EmuPalmOS", "PceNativeCall(0x%08X, 0x%08X): %d", nativeFuncP, userDataP, res);
      m68k_set_reg(M68K_REG_A0, res);
      m68k_set_reg(M68K_REG_D0, res);
#endif
    }
    break;

    // XXX this trap does not exist in PalmOS, but app Palmkedex calls it.
    // It performs the equivalent of PceNativeCall(), but parameters are stored
    // right after the trap instruction.
    // Don't know why it is doing that, but here is the code to handle it.
    case 0xA7FF: {
      uint32_t nativeFuncP, userDataP, pc, addr;
      uint8_t *ram = pumpkin_heap_base();
      pc = m68k_get_reg(NULL, M68K_REG_PC);
      get4b(&addr, ram, sp);
      get4l(&nativeFuncP, ram, pc);
      get4l(&userDataP, ram, pc + 4);
      emupalmos_trap_in(nativeFuncP, trap, 0);
      emupalmos_trap_in(userDataP, trap, 1);
      UInt32 res = arm_native_call_pce(nativeFuncP, userDataP);
      debug(DEBUG_TRACE, "EmuPalmOS", "A7FF(0x%08X, 0x%08X): %d", nativeFuncP, userDataP, res);
      m68k_set_reg(M68K_REG_A0, res);
      m68k_set_reg(M68K_REG_D0, res);
      m68k_set_reg(M68K_REG_SP, sp + 4);
      m68k_set_reg(M68K_REG_PC, addr);
      fake_cpu_instr_callback(pc);
    }
    break;
  }
}
