#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include "armemu.h"
#include "armem.h"
#include "RAM.h"
#include "CPU.h"
#include "soc_IC.h"
#include "emupalmosinc.h"
#include "debug.h"
#include "xalloc.h"

#define CPUID_PXA255 0x69052D06ul

#define ROM_BASE 0x40000000ul

struct arm_emu_t {
  struct ArmMem *mem;
  struct ArmRam *ram;
  struct ArmCpu *cpu;
  struct SocIc *ic;
};

arm_emu_t *armInit(uint8_t *buf, uint32_t size) {
  arm_emu_t *arm;

  if ((arm = xcalloc(1, sizeof(arm_emu_t))) != NULL) {
    arm->mem = memInit();
    arm->cpu = cpuInit(ROM_BASE, arm->mem, true, false, CPUID_PXA255, 0x0B16A16AUL);
    arm->ram = ramInit(arm->mem, 0, size, (uint32_t *)buf);
    arm->ic = socIcInit(arm->cpu, arm->mem, 0);
  }

  return arm;
}

void armFinish(arm_emu_t *arm) {
  if (arm) {
    socIcDeinit(arm->ic);
    ramDeinit(arm->ram);
    cpuDeinit(arm->cpu);
    memDeinit(arm->mem);
    free(arm);
  }
}

uint32_t armGetReg(arm_emu_t *arm, uint32_t reg) {
  return cpuReg(arm->cpu, reg);
}

void armSetReg(arm_emu_t *arm, uint32_t reg, uint32_t value) {
 cpuSetReg(arm->cpu, reg, value);
}

int armRun(arm_emu_t *arm, uint32_t n, uint32_t call68KAddr, call68KFunc_f f, uint32_t returnAddr) {
  uint32_t i, r, pc, a0, a1, a2, a3;

  for (i = 0; i < n && !emupalmos_finished(); i++) {
    pc = armGetReg(arm, 15);
    if (pc == returnAddr) {
      debug(DEBUG_TRACE, "EmuPalmOS", "armRun return address");
      return -1;
    }

    if (pc == call68KAddr) {
      a0 = armGetReg(arm, 0);
      a1 = armGetReg(arm, 1);
      a2 = armGetReg(arm, 2);
      a3 = armGetReg(arm, 3);
      r = f(a0, a1, a2, a3);
      armSetReg(arm, 0, r);

      // PC <-- LR
      a0 = armGetReg(arm, 14);
      armSetReg(arm, 15, a0);

    } else {
      cpuCycle(arm->cpu);
    }
  }

  return 0;
}
