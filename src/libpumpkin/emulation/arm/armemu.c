#include "sys.h"
#include "armemu.h"
#include "logtrap.h"
#include "emupalmosinc.h"
#include "debug.h"

#include "armem.h"
#include "RAM.h"
#include "CPU.h"
#include "soc_IC.h"

#define CPUID 0x69052d00ul // PXA255

#define ROM_BASE 0x40000000ul

struct arm_emu_t {
  struct ArmMem *mem;
  struct ArmRam *ram;
  struct ArmCpu *cpu;
  struct SocIc *ic;
};

arm_emu_t *uarmInit(uint8_t *buf, uint32_t size) {
  arm_emu_t *arm;

  if ((arm = sys_calloc(1, sizeof(arm_emu_t))) != NULL) {
    arm->mem = memInit();
    arm->cpu = cpuInit(ROM_BASE, arm->mem, 1, 0, CPUID, 0x0B16A16AUL);
    arm->ram = ramInit(arm->mem, 0, size, (uint32_t *)buf);
    arm->ic = socIcInit(arm->cpu, arm->mem, 0);
    cpuDisasm(arm->cpu, 1);
  }

  return arm;
}

void uarmFinish(arm_emu_t *arm) {
  if (arm) {
    socIcDeinit(arm->ic);
    ramDeinit(arm->ram);
    cpuDeinit(arm->cpu);
    memDeinit(arm->mem);
    sys_free(arm);
  }
}

uint32_t uarmGetReg(arm_emu_t *arm, uint32_t reg) {
  return cpuReg(arm->cpu, reg);
}

void uarmSetReg(arm_emu_t *arm, uint32_t reg, uint32_t value) {
 cpuSetReg(arm->cpu, reg, value);
}

void uarmDisasm(arm_emu_t *arm, int disasm) {
  cpuDisasm(arm->cpu, disasm);
}

int uarmRun(arm_emu_t *arm, uint32_t n, uint32_t call68KAddr, call68KFunc_f f, uint32_t returnAddr) {
  uint32_t i, r, pc, a0, a1, a2, a3;

  for (i = 0; i < n && !emupalmos_finished(); i++) {
    pc = uarmGetReg(arm, 15);
    if (pc == returnAddr) {
      debug(DEBUG_TRACE, "ARM", "armRun return address 0x%08X", returnAddr);
      return -1;
    }

    if (pc == call68KAddr) {
      debug(DEBUG_TRACE, "ARM", "call68KAddr");
      a0 = uarmGetReg(arm, 0);
      a1 = uarmGetReg(arm, 1);
      a2 = uarmGetReg(arm, 2);
      a3 = uarmGetReg(arm, 3);
      r = f(a0, a1, a2, a3);
      uarmSetReg(arm, 0, r);

      // PC <-- LR
      a0 = uarmGetReg(arm, 14);
      uarmSetReg(arm, 15, a0);

    } else {
      cpuCycle(arm->cpu);
    }
  }

  return 0;
}
