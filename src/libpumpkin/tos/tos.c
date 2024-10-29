#include <PalmOS.h>
#include <VFSMgr.h>

#include "thread.h"
#include "bytes.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "debug.h"

#define headerSize      28
#define basePageSize   256
#define heapSize      65536

//  uint8_t ph_branch[2];  branch to start of program (0x601a)
//  uint8_t ph_tlen[4];    .text length
//  uint8_t ph_dlen[4];    .data length
//  uint8_t ph_blen[4];    .bss length
//  uint8_t ph_slen[4];    length of symbol table
//  uint8_t ph_magic[4];
//  uint8_t ph_flags[4];   Atari special flags
//  uint8_t ph_abs[2];     has to be 0, otherwise no relocation takes place

// 00000000  60 1a 00 00 0f 28 00 00  00 28 00 00 00 44 00 00  |`....(...(...D..|
// 00000010  00 00 00 00 00 00 00 00  00 07 00 00 60 08 56 42  |............`.VB|
// 00000020  43 43 20 30 2e 39 2c 6f  00 04 49 f9 00 00 8f 28  |CC 0.9,o..I....(|

// base page format:
// 0x0000: base address of TPA
// 0x0004: end address of TPA + 1
// 0x0008: base address of text
// 0x000C: length of text
// 0x0010: base address of data
// 0x0014: length of data
// 0x0018: base address of bss
// 0x001C: length of bss
// 0x0020: DTA address pointer
// 0x0024: parent's base page pointer
// 0x0028: reserved
// 0x002C: pointer to env string
// 0x0080: command line image

static void make_hex(char *buf, unsigned int pc, unsigned int length) {
  char *ptr = buf;

  for (; length > 0; length -= 2) {
    sys_sprintf(ptr, "%04x", cpu_read_word(pc));
    pc += 2;
    ptr += 4;
    if (length > 2) *ptr++ = ' ';
  }
}

static int cpu_instr_callback(unsigned int pc) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint32_t size = pumpkin_heap_size();
  uint32_t instr_size, d[8], a[8];
  char buf[128], buf2[128];
  int i;

  if (emupalmos_debug_on()) {
    instr_size = m68k_disassemble(buf, pc, M68K_CPU_TYPE_68020);
    make_hex(buf2, pc, instr_size);
    for (i = 0; i < 8; i++) {
      d[i] = m68k_get_reg(NULL, M68K_REG_D0 + i);
      a[i] = m68k_get_reg(NULL, M68K_REG_A0 + i);
    }
    debug(DEBUG_INFO, "M68K", "A0=0x%08X,A1=0x%08X,A2=0x%08X,A3=0x%08X,A4=0x%08X,A5=0x%08X,A6=0x%08X,A7=0x%08X",
      a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
    debug(DEBUG_INFO, "M68K", "%08X: %-20s: %s (%d,%d,%d,%d,%d,%d,%d,%d)",
      pc, buf2, buf, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
  }

  return 0;
}

uint32_t tos_main(uint16_t launchCode, void *param, uint16_t flags) {
  MemHandle hTos, hBlock;
  uint8_t *ram, *tos, *block, *reloc;
  uint16_t jump, aflags;
  uint32_t offset, textSize, dataSize, bssSize, symSize, relocSize, reserved, pflags, blockSize, tosSize;
  uint32_t pc, a7, basePageStart, stackStart, textStart, dataStart, bssStart, *relocBase, value;
  m68ki_cpu_core main_cpu;
  emu_state_t *state, *oldState;

  if ((hTos = DmGet1Resource('dtos', 1)) != NULL) {
    if ((tos = MemHandleLock(hTos)) != NULL) {
      tosSize = MemHandleSize(hTos);
      offset = get2b(&jump, tos, 0);
      debug_bytes(DEBUG_INFO, "TOS", tos, tosSize);

      if (jump == 0x601a) {
        offset += get4b(&textSize, tos, offset);
        offset += get4b(&dataSize, tos, offset);
        offset += get4b(&bssSize, tos, offset);
        offset += get4b(&symSize, tos, offset);
        offset += get4b(&reserved, tos, offset);
        offset += get4b(&pflags, tos, offset);
        offset += get2b(&aflags, tos, offset);
        relocSize = tosSize - (headerSize + textSize + dataSize + symSize);
        debug(DEBUG_INFO, "TOS", "tos   size %d (0x%04X)", tosSize, tosSize);
        debug(DEBUG_INFO, "TOS", "text  size %d (0x%04X)", textSize, textSize);
        debug(DEBUG_INFO, "TOS", "data  size %d (0x%04X)", dataSize, dataSize);
        debug(DEBUG_INFO, "TOS", "bss   size %d (0x%04X)", bssSize, bssSize);
        debug(DEBUG_INFO, "TOS", "sym   size %d (0x%04X)", symSize, symSize);
        debug(DEBUG_INFO, "TOS", "reloc size %d (0x%04X)", relocSize, relocSize);
        debug(DEBUG_INFO, "TOS", "pflags 0x%08X", pflags);
        debug(DEBUG_INFO, "TOS", "aflags 0x%04X", aflags);

        oldState = emupalmos_install();
        state = pumpkin_get_local_storage(emu_key);

        ram = pumpkin_heap_base();

        blockSize = basePageSize + textSize + dataSize + bssSize + heapSize + stackSize;
        hBlock = MemHandleNew(blockSize);
        block = MemHandleLock(hBlock);
        MemSet(block, blockSize, 0);
        MemMove(block + basePageSize, &tos[offset], textSize);
        MemMove(block + basePageSize + textSize, &tos[offset + textSize], dataSize);

        basePageStart = block - ram;
        textStart  = basePageStart + basePageSize;
        dataStart  = basePageStart + basePageSize + textSize;
        bssStart   = basePageStart + basePageSize + textSize + dataSize;
        stackStart = basePageStart + basePageSize + textSize + dataSize + bssSize + heapSize;

        m68k_write_memory_32(basePageStart + 0x0000, basePageStart); // XXX
        m68k_write_memory_32(basePageStart + 0x0004, basePageStart + blockSize); // XXX
        m68k_write_memory_32(basePageStart + 0x0008, textStart);
        m68k_write_memory_32(basePageStart + 0x000C, textSize);
        m68k_write_memory_32(basePageStart + 0x0010, dataStart);
        m68k_write_memory_32(basePageStart + 0x0014, dataSize);
        m68k_write_memory_32(basePageStart + 0x0018, bssStart);
        m68k_write_memory_32(basePageStart + 0x001C, bssSize);

        debug(DEBUG_INFO, "TOS", "basePage:");
        debug_bytes(DEBUG_INFO, "TOS", block, basePageSize);

        debug(DEBUG_INFO, "TOS", "text:");
        debug_bytes(DEBUG_INFO, "TOS", block + basePageSize, textSize);

        if (dataSize > 0) {
          debug(DEBUG_INFO, "TOS", "data:");
          debug_bytes(DEBUG_INFO, "TOS", block + basePageSize + textSize, dataSize);
        }

        if (relocSize > 0) {
          relocBase = (uint32_t *)(tos + headerSize + textSize + dataSize + symSize);
          get4b(&offset, (uint8_t *)relocBase, 0);
          if (offset) {
            debug(DEBUG_INFO, "TOS", "reloc:");
            debug_bytes(DEBUG_INFO, "TOS", (uint8_t *)relocBase, relocSize);

            value = m68k_read_memory_32(textStart + offset);
            debug(DEBUG_INFO, "TOS", "reloc offset 0x00 0x%08X 0x%08X -> 0x%08X", offset, value, value + textStart);
            value += textStart;
            m68k_write_memory_32(textStart + offset, value);

            for (reloc = ((uint8_t *)relocBase) + 4; *reloc; reloc++) {
              if (*reloc == 1) {
                offset += 254;
                continue;
              }
              offset += *reloc;

              value = m68k_read_memory_32(textStart + offset);
              debug(DEBUG_INFO, "TOS", "reloc offset 0x%02X 0x%08X 0x%08X -> 0x%08X", *reloc, offset, value, value + textStart);
              value += textStart;
              m68k_write_memory_32(textStart + offset, value);
            }
          }
        }
  
        pc = textStart;
        a7 = stackStart + stackSize - 16;
        state->stackStart = stackStart;

        m68k_write_memory_32(a7 + 4, basePageStart);
        m68k_write_memory_32(a7, 0); // return address

        MemSet(&main_cpu, sizeof(m68ki_cpu_core), 0);
        main_cpu.trapf = 1;
        m68k_set_context(&main_cpu);
        m68k_init();
        m68k_set_cpu_type(M68K_CPU_TYPE_68020);
        m68k_pulse_reset();
        m68k_set_reg(M68K_REG_PC, pc);
        m68k_set_reg(M68K_REG_SP, a7);
        m68k_set_instr_hook_callback(cpu_instr_callback);

        emupalmos_debug(1);
        for (; !emupalmos_finished() && !thread_must_end();) {
          m68k_execute(&state->m68k_state, 100000);
        }

        emupalmos_deinstall(oldState);
        MemHandleFree(hBlock);
      }
      MemHandleUnlock(hTos);
    }
    DmReleaseResource(hTos);
  }

  return 0;
}

static uint32_t tos_gemdos_systrap(void) {
  emu_state_t *state = m68k_get_emu_state();
  uint32_t sp, call, r = 0;
  uint16_t idx;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;
  call = ARG16;

  switch (call) {
    case 0: // Pterm0
      debug(DEBUG_INFO, "TOS", "GEMDOS Pterm0");
      emupalmos_finish(1);
      break;
    case 64: { // int32_t Fwrite(int16_t handle, int32_t count, void *buf)
        int16_t handle = ARG16;
        int32_t count = ARG32;
        uint32_t buf = ARG32;
        int32_t res = count;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_INFO, "TOS", "GEMDOS Fwrite(%d, %d, 0x%08X): %d", handle, count, buf, res);
      }
      break;
    case 72: { // void *Malloc(int32_t number)
        int32_t number = ARG32;
        uint32_t addr = 0;
        if (number == -1) {
          // get size of the largest available memory block
        } else {
        }
        m68k_set_reg(M68K_REG_D0, addr);
        debug(DEBUG_INFO, "TOS", "GEMDOS Malloc(%d): 0x%08X", number, addr);
      }
      break;
    case 73: { // int32_t Mfree(void *block)
        uint32_t block = ARG32;
        int32_t res = 0;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_INFO, "TOS", "GEMDOS Mfree(0x%08X): %u", block, res);
      }
      break;
    case 74: { // int32_t Mshrink(void *block, int32_t newsiz)
        uint16_t dummy = ARG16;
        uint32_t block = ARG32;
        int32_t newsiz = ARG32;
        int32_t res = 0;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_INFO, "TOS", "GEMDOS Mshrink(0x%08X, %d): %d", block, newsiz, res);
      }
      break;
    case 76: { // void Pterm(uint16_t retcode)
        uint16_t retcode = ARG16;
        debug(DEBUG_INFO, "TOS", "GEMDOS Pterm(%u)", retcode);
        emupalmos_finish(1);
      }
      break;
    case 260: { // int32_t Fcntl(int16_t fh, int32_t arg, int16_t cmd)
        int16_t fh = ARG16;
        int32_t arg = ARG16;
        int32_t cmd = ARG16;
        int32_t res = 0;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_INFO, "TOS", "GEMDOS Fcntl(%d, %d, %d): %d", fh, arg, cmd, res);
      }
      break;
    default:
      debug(DEBUG_ERROR, "TOS", "unmapped GEMDOS call %d", call);
      break;
  }

  return r;
}

static uint32_t tos_bios_systrap(void) {
  emu_state_t *state = m68k_get_emu_state();
  uint32_t sp, call, r = 0;
  uint16_t idx;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;
  call = ARG16;

  switch (call) {
    case 2: { // Bconin
        uint16_t fd = ARG16;
        m68k_set_reg(M68K_REG_D0, 0);
        debug(DEBUG_INFO, "TOS", "Bconin %d", fd);
      }
      break;
    case 3: { // Bconout
        uint16_t fd = ARG16;
        uint16_t ch = ARG16;
        debug(DEBUG_INFO, "TOS", "Bconout %d '%c'", fd, ch);
      }
      break;
    default:
      debug(DEBUG_ERROR, "TOS", "unmapped BIOS call %d", call);
      break;
  }

  return r;
}

uint32_t tos_systrap(uint8_t type) {
  uint32_t r = 0;

  switch (type) {
    case 0x01:
      r = tos_gemdos_systrap();
      break;
    case 0x0d:
      r = tos_bios_systrap();
      break;
    default:
      debug(DEBUG_ERROR, "TOS", "unmapped call type %d", type);
      break;
  }

  return r;
}
