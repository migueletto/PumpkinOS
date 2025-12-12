#include "sys.h"
#include "armemu.h"
#include "logtrap.h"
#include "emupalmosinc.h"
#include "debug.h"

#define ARM_UARM
//#define ARM_UNICORN

#ifdef ARM_UARM

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

arm_emu_t *armInit(uint8_t *buf, uint32_t size) {
  arm_emu_t *arm;

  if ((arm = sys_calloc(1, sizeof(arm_emu_t))) != NULL) {
    arm->mem = memInit();
    arm->cpu = cpuInit(ROM_BASE, arm->mem, 1, 0, CPUID, 0x0B16A16AUL);
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
    sys_free(arm);
  }
}

uint32_t armGetReg(arm_emu_t *arm, uint32_t reg) {
  return cpuReg(arm->cpu, reg);
}

void armSetReg(arm_emu_t *arm, uint32_t reg, uint32_t value) {
 cpuSetReg(arm->cpu, reg, value);
}

void armDisasm(arm_emu_t *arm, int disasm) {
  cpuDisasm(arm->cpu, disasm);
}

int armRun(arm_emu_t *arm, uint32_t n, uint32_t call68KAddr, call68KFunc_f f, uint32_t returnAddr) {
  uint32_t i, r, pc, a0, a1, a2, a3;

  for (i = 0; i < n && !emupalmos_finished(); i++) {
    pc = armGetReg(arm, 15);
    if (pc == returnAddr) {
      debug(DEBUG_TRACE, "ARM", "armRun return address 0x%08X", returnAddr);
      return -1;
    }

    if (pc == call68KAddr) {
      debug(DEBUG_TRACE, "ARM", "call68KAddr");
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

#endif

#ifdef ARM_UNICORN

#include <unicorn/unicorn.h>
#include "disasm.h"

struct arm_emu_t {
  uc_engine *uc;
  uc_hook trace;
  uint32_t call68KAddr;
  call68KFunc_f f;
  uint8_t *buf;
  uint32_t size;
  int first;
  int disasm;
};

arm_emu_t *armInit(uint8_t *buf, uint32_t size) {
  arm_emu_t *arm;
  uc_err err;

  if ((arm = sys_calloc(1, sizeof(arm_emu_t))) != NULL) {
    if ((err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &arm->uc)) == 0) {
      uc_ctl_set_cpu_model(arm->uc, UC_CPU_ARM_PXA255);
      err = uc_mem_map(arm->uc, 0, size, UC_PROT_ALL);
      debug(DEBUG_TRACE, "ARM", "uc_mem_map %d", err);
      arm->buf = buf;
      arm->size = size;
      arm->first = 1;
    } else {
      debug(DEBUG_ERROR, "ARM", "uc_open %d", err);
      sys_free(arm);
      arm = NULL;
    }
  }

  return arm;
}

void armFinish(arm_emu_t *arm) {
  if (arm) {
    uc_close(arm->uc);
    sys_free(arm);
  }
}

uint32_t armGetReg(arm_emu_t *arm, uint32_t reg) {
  int r;

  if (reg < 13) {
    reg = UC_ARM_REG_R0 + reg;
  } else switch (reg) {
    case 13: reg = UC_ARM_REG_R13; break;
    case 14: reg = UC_ARM_REG_R14; break;
    case 15: reg = UC_ARM_REG_R15; break;
  }
  uc_reg_read(arm->uc, reg, &r);

  return r;
}

void armSetReg(arm_emu_t *arm, uint32_t reg, uint32_t value) {
  int r = value;

  if (reg < 13) {
    reg = UC_ARM_REG_R0 + reg;
  } else switch (reg) {
    case 13: reg = UC_ARM_REG_R13; break;
    case 14: reg = UC_ARM_REG_R14; break;
    case 15: reg = UC_ARM_REG_R15; break;
  }

  uc_reg_write(arm->uc, reg, &r);
}

void armDisasm(arm_emu_t *arm, int disasm) {
  arm->disasm = disasm;
}

static void armHook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
  arm_emu_t *arm = (arm_emu_t *)user_data;
  uint32_t addr = (uint32_t)address;
  uint32_t a0, a1, a2, a3, r;

  if (addr == arm->call68KAddr) {
    debug(DEBUG_TRACE, "ARM", "call68KAddr");
    a0 = armGetReg(arm, 0);
    a1 = armGetReg(arm, 1);
    a2 = armGetReg(arm, 2);
    a3 = armGetReg(arm, 3);
    r = arm->f(a0, a1, a2, a3);
    armSetReg(arm, 0, r);

    // PC <-- LR
    a0 = armGetReg(arm, 14);
    armSetReg(arm, 15, a0);
    return;
  }

  if (arm->disasm) {
    if (size == 4) {
      uint32_t r0 = armGetReg(arm, 0);
      uint32_t r1 = armGetReg(arm, 1);
      uint32_t r2 = armGetReg(arm, 2);
      uint32_t r3 = armGetReg(arm, 3);
      uint32_t r12 = armGetReg(arm, 12);
      uint32_t r14 = armGetReg(arm, 14);
      uint32_t instr = *(uint32_t *)(arm->buf + addr);
      char buf[256];
      sys_snprintf(buf, sizeof(buf)-1, "(r0=%08X r1=%08X r2=%08X r3=%08X r12=%08X LR=%08X)", r0, r1, r2, r3, r12, r14);
      disasm(addr, instr, buf);
    } else {
      uint16_t instr = *(uint16_t *)(arm->buf + addr);
      debug(DEBUG_TRACE, "ARM", "trace 0x%08X %u 0x%04X", addr, size, instr);
    }
  }
}

int armRun(arm_emu_t *arm, uint32_t n, uint32_t call68KAddr, call68KFunc_f f, uint32_t returnAddr) {
  uint32_t pc, *buf32;
  uc_err err;

  if (arm->first) {
    err = uc_mem_write(arm->uc, 0, arm->buf, arm->size);
    err = uc_hook_add(arm->uc, &arm->trace, UC_HOOK_CODE, armHook, arm, 0, arm->size - 1);
    buf32 = (uint32_t *)arm->buf;
    buf32[returnAddr >> 2] = 0xe1a00000; // nop at return address
    arm->call68KAddr = call68KAddr;
    arm->f = f;
    arm->first = 0;
  }

  pc = armGetReg(arm, 15);
  debug(DEBUG_TRACE, "ARM", "uc_emu_start begin 0x%08X", pc);
  err = uc_emu_start(arm->uc, pc, returnAddr, 0, 0);
  if (err) debug(DEBUG_ERROR, "ARM", "uc_emu_start err=%d", err);
  pc = armGetReg(arm, 15);
  debug(DEBUG_TRACE, "ARM", "uc_emu_start end 0x%08X", pc);

  return pc == returnAddr ? -1 : 0;
}

#endif
