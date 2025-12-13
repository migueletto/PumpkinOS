#include <PalmOS.h>

#include <unicorn/unicorn.h>

#include "sys.h"
#include "pumpkin.h"
#include "logtrap.h"
#include "armemu.h"
#include "armp.h"
#include "emupalmosinc.h"
#include "disasm.h"
#include "debug.h"

#define unicornArmPluginId 'ucAr'

static arm_plugin_t ucarm;

struct arm_emu_t {
  uc_engine *uc;
  uc_hook trace;
  uint32_t call68KAddr;
  call68KFunc_f f;
  uint8_t *buf;
  uint32_t size;
  int disasm;
};

static uint32_t ucarmGetReg(arm_emu_t *arm, uint32_t reg) {
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

static void ucarmSetReg(arm_emu_t *arm, uint32_t reg, uint32_t value) {
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

static void ucarmHookCode(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
  arm_emu_t *arm = (arm_emu_t *)user_data;
  uint32_t addr = (uint32_t)address;
  uint32_t r, r0, r1, r2, r3, r12, sp, lr;
  size_t mode;
  char buf[256];

  if (addr == arm->call68KAddr) {
    debug(DEBUG_TRACE, "ARM", "call68KAddr");
    r0 = ucarmGetReg(arm, 0);
    r1 = ucarmGetReg(arm, 1);
    r2 = ucarmGetReg(arm, 2);
    r3 = ucarmGetReg(arm, 3);
    r = arm->f(r0, r1, r2, r3);
    ucarmSetReg(arm, 0, r);

    // PC <-- LR
    lr = ucarmGetReg(arm, 14);
    ucarmSetReg(arm, 15, lr);
    return;
  }

  if (addr >= 0x04000000) {
    // native ARM syscall emulation: the address identifies which syscall is being called,
    // beforeno matter which instruction is contained in that address.
    uint32_t group, function;
    group = (addr & 0x00F00000) >> 20;
    function = addr & 0xFFFF;
    r0 = ucarmGetReg(arm, 0);
    r1 = ucarmGetReg(arm, 1);
    r2 = ucarmGetReg(arm, 2);
    r3 = ucarmGetReg(arm, 3);
    sp = ucarmGetReg(arm, 13);
    lr = ucarmGetReg(arm, 14);
    ucarmSetReg(arm, 0, emupalmos_arm_syscall(group, function, r0, r1, r2, r3, sp));
    ucarmSetReg(arm, 14, lr);
    ucarmSetReg(arm, 15, lr); // return from subroutine
    return;
  }

  if (arm->disasm) {
    r0 = ucarmGetReg(arm, 0);
    r1 = ucarmGetReg(arm, 1);
    r2 = ucarmGetReg(arm, 2);
    r3 = ucarmGetReg(arm, 3);
    r12 = ucarmGetReg(arm, 12);
    lr = ucarmGetReg(arm, 14);
    sys_snprintf(buf, sizeof(buf)-1, "(r0=%08X r1=%08X r2=%08X r3=%08X r12=%08X LR=%08X)", r0, r1, r2, r3, r12, lr);

    uc_query(arm->uc, UC_QUERY_MODE, &mode);
    if (mode == UC_MODE_THUMB) {
      if (size == 4) {
        uint32_t instr = *(uint32_t *)(arm->buf + addr);
        debug(DEBUG_TRACE, "ARM", "%08X: %08X %s", addr, instr, buf);
      } else {
        uint16_t instr = *(uint16_t *)(arm->buf + addr);
        disasm(addr, instr, buf, 1);
      }
    } else {
      if (size == 4) {
        uint32_t instr = *(uint32_t *)(arm->buf + addr);
        disasm(addr, instr, buf, 0);
      } else {
        debug(DEBUG_ERROR, "ARM", "%08X: wrong size %u %s", addr, size, buf);
      }
    }
  }
}

static bool ucarmHookFetchUnmapped(uc_engine *uc, uc_mem_type type, uint64_t address, int size, int64_t value, void *user_data) {
  uint32_t addr = (uint32_t)address;
  debug(DEBUG_ERROR, "ARM", "access to unmapped address 0x%08X", addr);
  return false;
}

static arm_emu_t *ucarmInit(uint8_t *buf, uint32_t size) {
  uint32_t version;
  arm_emu_t *arm;
  uc_err err;

  if ((arm = sys_calloc(1, sizeof(arm_emu_t))) != NULL) {
    if ((err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &arm->uc)) == 0) {
      version = uc_version(NULL, NULL);
      debug(DEBUG_INFO, "ARM", "unicorn version 0x%08X", version);
      uc_ctl_set_cpu_model(arm->uc, UC_CPU_ARM_PXA255);
      uc_hook_add(arm->uc, &arm->trace, UC_HOOK_CODE, ucarmHookCode, arm, 0, arm->size - 1);
      uc_hook_add(arm->uc, &arm->trace, UC_HOOK_MEM_UNMAPPED, ucarmHookFetchUnmapped, arm, 1, 0);

      // main memory
      err = uc_mem_map_ptr(arm->uc, 0, size, UC_PROT_ALL, buf);
      if (err) debug(DEBUG_ERROR, "ARM", "uc_mem_map_ptr error: %s", uc_strerror(err));

      // virtual ARM syscall memory
      err = uc_mem_map(arm->uc, 0x04100000, 0x00210000, UC_PROT_ALL);
      if (err) debug(DEBUG_ERROR, "ARM", "uc_mem_map error: %s", uc_strerror(err));

      arm->buf = buf;
      arm->size = size;
    } else {
      debug(DEBUG_ERROR, "ARM", "uc_open error: %s", uc_strerror(err));
      sys_free(arm);
      arm = NULL;
    }
  }

  return arm;
}

static void ucarmFinish(arm_emu_t *arm) {
  if (arm) {
    uc_close(arm->uc);
    sys_free(arm);
  }
}

static void ucarmDisasm(arm_emu_t *arm, int disasm) {
  arm->disasm = disasm;
}

static int ucarmRun(arm_emu_t *arm, uint32_t n, uint32_t call68KAddr, call68KFunc_f f, uint32_t returnAddr) {
  uint32_t pc;
  uc_err err;

  //arm->buf32[returnAddr >> 2] = 0xe1a00000; // nop at return address
  arm->call68KAddr = call68KAddr;
  arm->f = f;

  pc = ucarmGetReg(arm, 15);
  err = uc_emu_start(arm->uc, pc, returnAddr, 0, 0);
  if (err) debug(DEBUG_ERROR, "ARM", "uc_emu_start error: %s", uc_strerror(err));

  return pc == returnAddr || err ? -1 : 0;
}

static void *PluginMain(void *p) {
  ucarm.armInit = ucarmInit;
  ucarm.armFinish = ucarmFinish;
  ucarm.armGetReg = ucarmGetReg;
  ucarm.armSetReg = ucarmSetReg;
  ucarm.armRun = ucarmRun;
  ucarm.armDisasm = ucarmDisasm;

  return &ucarm;
}

pluginMainF PluginInit(UInt32 *type, UInt32 *id) {
  *type = armPluginType;
  *id = unicornArmPluginId;

  return PluginMain;
}
