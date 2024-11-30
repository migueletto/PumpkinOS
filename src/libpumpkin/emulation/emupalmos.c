#include <PalmOS.h>
#include <PceNativeCall.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <GPSLib.h>

#include "sys.h"
#include "pwindow.h"
#include "thread.h"
#include "mutex.h"
#include "vfs.h"
#include "bytes.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "AppRegistry.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "trapnames.h"

#define TRAPS_SIZE 0x40000

static const uint8_t SysFormPointerArrayToStrings_code[] = {
0x4e, 0x56, 0x00, 0x00,
0x48, 0xe7, 0x18, 0x20,
0x24, 0x6e, 0x00, 0x08,
0x38, 0x2e, 0x00, 0x0c,
0x76, 0x00,
0xb4, 0xfc, 0x00, 0x00,
0x67, 0x52,
0x30, 0x04,
0x48, 0xc0,
0xe5, 0x88,
0x2f, 0x00,
0x4e, 0x4f,
0xa0, 0x1e,
0x26, 0x08,
0x58, 0x8f,
0x67, 0x40,
0x2f, 0x03,
0x4e, 0x4f,
0xa0, 0x21,
0x58, 0x8f,
0xb0, 0xfc, 0x00, 0x00,
0x67, 0x32,
0x42, 0x40,
0x42, 0x41,
0xb8, 0x41,
0x6f, 0x24,
0x32, 0x40,
0x4a, 0x31, 0xa8, 0x00,
0x66, 0x16,
0x30, 0x01,
0x48, 0xc0,
0xe5, 0x88,
0x21, 0x8a, 0x08, 0x00,
0x45, 0xf2, 0x98, 0x01,
0x42, 0x40,
0x52, 0x41,
0x60, 0x00, 0xff, 0xe0,
0x52, 0x40,
0x60, 0x00, 0xff, 0xda,
0x2f, 0x03,
0x4e, 0x4f,
0xa0, 0x22,
0x20, 0x43,
0x4c, 0xee, 0x04, 0x18, 0xff, 0xf4,
0x4e, 0x5e,
0x4e, 0x75
};

static const uint8_t FrmDrawForm_code[] = {
0x4e, 0x56, 0x00, 0x00, 0x48, 0xe7, 0x18, 0x20, 0x28, 0x2e, 0x00, 0x08, 0x67, 0x40, 0x42, 0x43,
0x60, 0x00, 0x00, 0x2a, 0x3f, 0x03, 0x2f, 0x04, 0x4e, 0x4f, 0xa1, 0x82, 0x5c, 0x8f, 0x0c, 0x00,
0x00, 0x0c, 0x66, 0x16, 0x20, 0x6a, 0x00, 0x10, 0xb0, 0xfc, 0x00, 0x00, 0x67, 0x0c, 0x42, 0xa7,
0x42, 0x67, 0x2f, 0x0a, 0x4e, 0x90, 0x4f, 0xef, 0x00, 0x0a, 0x52, 0x43, 0x3f, 0x03, 0x2f, 0x04,
0x4e, 0x4f, 0xa5, 0x02, 0x24, 0x48, 0x5c, 0x8f, 0xb4, 0xfc, 0x00, 0x00, 0x66, 0xc6, 0x4c, 0xee,
0x04, 0x18, 0xff, 0xf4, 0x4e, 0x5e, 0x4e, 0x75,
};

static const uint8_t SysBinarySearch_code[] = {
0x4e, 0x56, 0x00, 0x00, 0x48, 0xe7, 0x1f, 0x38, 0x28, 0x6e, 0x00, 0x10, 0x26, 0x6e, 0x00, 0x14,
0x24, 0x6e, 0x00, 0x1c, 0x42, 0x07, 0x4a, 0xae, 0x00, 0x08, 0x67, 0x50, 0xb8, 0xfc, 0x00, 0x00,
0x67, 0x4a, 0xb6, 0xfc, 0x00, 0x00, 0x67, 0x44, 0x42, 0x43, 0xb6, 0x6e, 0x00, 0x0c, 0x64, 0x3c,
0x7c, 0x00, 0x42, 0x45, 0x78, 0x00, 0x2f, 0x2e, 0x00, 0x18, 0x3c, 0x05, 0x20, 0x6e, 0x00, 0x08,
0x48, 0x70, 0x68, 0x00, 0x2f, 0x0b, 0x4e, 0x94, 0x4f, 0xef, 0x00, 0x0c, 0x4a, 0x40, 0x66, 0x0c,
0xb4, 0xfc, 0x00, 0x00, 0x67, 0x04, 0x38, 0x03, 0x24, 0x84, 0x7e, 0x01, 0xda, 0x6e, 0x00, 0x0e,
0x52, 0x43, 0xb6, 0x6e, 0x00, 0x0c, 0x64, 0x04, 0x4a, 0x07, 0x67, 0xca, 0x30, 0x07, 0x02, 0x40,
0x00, 0x01, 0x4c, 0xee, 0x1c, 0xf8, 0xff, 0xe0, 0x4e, 0x5e, 0x4e, 0x75
};

static const uint8_t SysLibLoad_code[] = {
0x4e, 0x56, 0xff, 0xc8, 0x48, 0xe7, 0x1f, 0x30, 0x2e, 0x2e, 0x00, 0x08, 0x2c, 0x2e, 0x00, 0x0c,
0x26, 0x6e, 0x00, 0x10, 0x36, 0x3c, 0x05, 0x0a, 0x48, 0x6e, 0xff, 0xcc, 0x48, 0x6e, 0xff, 0xca,
0x42, 0x27, 0x2f, 0x06, 0x2f, 0x07, 0x48, 0x6e, 0xff, 0xe0, 0x1f, 0x3c, 0x00, 0x01, 0x4e, 0x4f,
0xa0, 0x78, 0x4f, 0xef, 0x00, 0x18, 0x4a, 0x40, 0x66, 0x00, 0x00, 0xb6, 0x3f, 0x3c, 0x00, 0x01,
0x2f, 0x2e, 0xff, 0xcc, 0x3f, 0x2e, 0xff, 0xca, 0x4e, 0x4f, 0xa0, 0x49, 0x2a, 0x08, 0x50, 0x8f,
0x67, 0x00, 0x00, 0x9e, 0x42, 0x67, 0x2f, 0x3c, 0x6c, 0x69, 0x62, 0x72, 0x4e, 0x4f, 0xa0, 0x60,
0x28, 0x08, 0x5c, 0x8f, 0x67, 0x00, 0x00, 0x84, 0x2f, 0x04, 0x4e, 0x4f, 0xa0, 0x21, 0x24, 0x48,
0x58, 0x8f, 0xb4, 0xfc, 0x00, 0x00, 0x67, 0x6a, 0x2f, 0x0b, 0x2f, 0x06, 0x2f, 0x07, 0x4e, 0x4f,
0xa5, 0x03, 0x4f, 0xef, 0x00, 0x0c, 0x4a, 0x00, 0x67, 0x06, 0x42, 0x43, 0x60, 0x00, 0x00, 0x4c,
0x42, 0x27, 0x48, 0x78, 0x00, 0x10, 0x76, 0xd0, 0xd6, 0x8e, 0x2f, 0x03, 0x4e, 0x4f, 0xa0, 0x27,
0x2f, 0x03, 0x3f, 0x13, 0x4e, 0x92, 0x36, 0x00, 0x4f, 0xef, 0x00, 0x10, 0x66, 0x24, 0x2f, 0x2e,
0xff, 0xd4, 0x2f, 0x2e, 0xff, 0xd0, 0x2f, 0x04, 0x4e, 0x4f, 0xa0, 0x2d, 0x2e, 0x80, 0x2f, 0x0a,
0x2f, 0x2e, 0xff, 0xcc, 0x3f, 0x13, 0x4e, 0x4f, 0xa5, 0x04, 0x36, 0x00, 0x4f, 0xef, 0x00, 0x16,
0x67, 0x08, 0x3f, 0x13, 0x4e, 0x4f, 0xa5, 0x05, 0x54, 0x8f, 0x2f, 0x04, 0x4e, 0x4f, 0xa0, 0x22,
0x58, 0x8f, 0x2f, 0x04, 0x4e, 0x4f, 0xa0, 0x61, 0x58, 0x8f, 0x2f, 0x05, 0x4e, 0x4f, 0xa0, 0x4a,
0x30, 0x03, 0x4c, 0xee, 0x0c, 0xf8, 0xff, 0xac, 0x4e, 0x5e, 0x4e, 0x75,
};

static int debug_on;

void emupalmos_finish(int f) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  state->m68k_state.finish = f;
  state->finish = f;
}

int emupalmos_finished(void) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  return state->finish;
}

void emupalmos_panic(char *msg, int code) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  UInt32 creator;

  debug(DEBUG_ERROR, "EmuPalmOS", "panic: %s", msg);
  state->panic = xstrdup(msg);
  emupalmos_finish(1);

  creator = pumpkin_get_app_creator();
  pumpkin_set_compat(creator, appCompatCrash, code);
  pumpkin_crash_log(creator, code, msg);
}

void *emupalmos_trap_sel_in(uint32_t address, uint16_t trap, uint16_t sel, int arg) {
  char name[64], argument[64], selector[64], buf[256], *s;
  uint8_t *ram = pumpkin_heap_base();
  uint32_t size = pumpkin_heap_size();
  uint16_t sl;

  if (address > size-4) {
    m68k_pulse_halt();

    s = trapName(trap, &sl, 0);
    if (s) {
      sys_snprintf(name, sizeof(name)-1, "%s", s);
    } else {
      sys_snprintf(name, sizeof(name)-1, "trap %04X", trap);
    }

    if (sel != 0xFFFF) {
      sys_snprintf(selector, sizeof(selector)-1, " selector %d", sel);
    } else {
      selector[0] = 0;
    }

    if (arg >= 0) {
      sys_snprintf(argument, sizeof(argument)-1, " argument %d", arg);
    } else {
      argument[0] = 0;
    }

    sys_snprintf(buf, sizeof(buf)-1, "Invalid address 0x%08X for %s%s%s.", address, name, selector, argument);
    emupalmos_panic(buf, EMUPALMOS_INVALID_ADDRESS);
    return NULL;
  }

  return address ? ram + address : NULL;
}

void *emupalmos_trap_in(uint32_t address, uint16_t trap, int arg) {
  return emupalmos_trap_sel_in(address, trap, -1, arg);
}

uint32_t emupalmos_trap_out(void *address) {
  uint8_t *addr = (uint8_t *)address;
  char buf[256];
  uint8_t *ram = pumpkin_heap_base();
  uint32_t size = pumpkin_heap_size();

  if (address == NULL) return 0;

  if (addr < ram || addr > ram+size-4) {
    sys_snprintf(buf, sizeof(buf)-1, "Invalid address %p.", address);
    emupalmos_panic(buf, EMUPALMOS_INVALID_ADDRESS);
    return 0;
  }

  return addr - ram;
}

static uint32_t monitor_start = 0, monitor_end = 0;

void emupalmos_monitor(uint32_t addr, uint32_t size) {
  monitor_start = addr;
  monitor_end = addr + size;
  debug(DEBUG_INFO, "EmuPalmOS", "monitor access from 0x%08X to 0x%08X (%d bytes)", monitor_start, monitor_end-1, size);
}

static int emupalmos_check_address(uint32_t address, int size, int read) {
  uint32_t hsize = pumpkin_heap_size();
  char buf[256];

  if (monitor_start > 0 && address >= monitor_start && address < monitor_end) {
    debug(DEBUG_INFO, "EmuPalmOS", "monitored %s access %s 0x%08X (offset 0x%04X, size %d)",
      read ? "Read" : "Write", read ? "from" : "to", monitor_start, address - monitor_start, size);
  }

  if (address > hsize-size) {
    m68k_pulse_halt();
    sys_snprintf(buf, sizeof(buf)-1, "%s %d bytes(s) %s invalid 68K address 0x%08X", read ? "Read" : "Write", size, read ? "from" : "to", address);
    debug(DEBUG_ERROR, "EmuPalmOS", "%s", buf);
    emupalmos_panic(buf, EMUPALMOS_INVALID_ADDRESS);
    return 0;
  }

  return 1;
}

uint8_t cpu_read_byte(uint32_t address) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint8_t *ram;
  uint32_t value;

  if (state->read_byte) return state->read_byte(address);

  if (address >= 0xFFFFF000) {
    debug(DEBUG_INFO, "EmuPalmOS", "read 8 bits from register 0x%08X", address);
    value = 0;
  } else {
    if (!emupalmos_check_address(address, 1, 1)) return 0;
    WinLegacyGetAddr(&state->screenStart, &state->screenEnd);
    if (address >= state->screenStart && address < state->screenEnd) {
      value = WinLegacyRead(address - state->screenStart);
      debug(DEBUG_TRACE, "EmuPalmOS", "direct screen read  byte 0x%08X = 0x%02X", address, value);
    } else {
      ram = pumpkin_heap_base();
      value = READ_BYTE(ram, address);
    }
  }

  return value;
}

uint16_t cpu_read_word(uint32_t address) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint32_t size = pumpkin_heap_size();
  uint8_t *ram;
  uint32_t value;

  if (state->read_word) return state->read_word(address);

  if ((address & 1) == 0 && address >= size && address < (size + TRAPS_SIZE)) {
    debug(DEBUG_TRACE, "EmuPalmOS", "returning RTS for address 0x%08X", address);
    return 0x4E75; // RTS
  }
  if (address >= 0xFFFFF000) {
    debug(DEBUG_INFO, "EmuPalmOS", "read 16 bits from register 0x%08X", address);
    value = 0;
  } else {
    if (!emupalmos_check_address(address, 2, 1)) return 0;
    WinLegacyGetAddr(&state->screenStart, &state->screenEnd);
    if (address >= state->screenStart && address < state->screenEnd - 1) {
      value = WinLegacyRead(address - state->screenStart);
      value <<= 8;
      value |= WinLegacyRead(address - state->screenStart + 1);
      debug(DEBUG_TRACE, "EmuPalmOS", "direct screen read  word 0x%08X = 0x%04X", address, value);
    } else {
      ram = pumpkin_heap_base();
      value = READ_WORD(ram, address);
    }
  }

  return value;
}

uint32_t cpu_read_long(uint32_t address) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint32_t b, value = 0;
  uint8_t *ram;

  if (state->read_long) return state->read_long(address);

  if (address >= 0xFFFFF000) {
    if (state->m68k_state.s_m68ki_cpu.palmos) {
      switch (address) {
        case 0xFFFFFA00: // LSSA, 32 bits, LCD screen starting address register
          pumpkin_set_osversion(10, 1);
          WinLegacyGetAddr(&state->screenStart, &state->screenEnd);
          value = state->screenStart;
          debug(DEBUG_INFO, "EmuPalmOS", "read LSSA (LCD screen starting address register): 0x%08X", value);
          break;
        default:
          debug(DEBUG_INFO, "EmuPalmOS", "read 32 bits from register 0x%08X", address);
          value = 0;
          break;
      }
    }
  } else {
    if (!emupalmos_check_address(address, 4, 1)) return 0;
    WinLegacyGetAddr(&state->screenStart, &state->screenEnd);
    if (address >= state->screenStart && address < state->screenEnd - 3) {
      value = WinLegacyRead(address - state->screenStart);
      value <<= 24;
      b = WinLegacyRead(address - state->screenStart + 1);
      b <<= 16;
      value |= b;
      b = WinLegacyRead(address - state->screenStart + 2);
      b <<= 8;
      value |= b;
      value |= WinLegacyRead(address - state->screenStart + 3);
      debug(DEBUG_TRACE, "EmuPalmOS", "direct screen read  long 0x%08X = 0x%08X", address, value);
    } else {
      ram = pumpkin_heap_base();
      value = READ_LONG(ram, address);
    }
  }

  return value;
}

void cpu_write_byte(uint32_t address, uint8_t value) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint8_t *ram;

  if (state->write_byte) return state->write_byte(address, value);

  if (address >= 0xFFFFF000) {
    debug(DEBUG_INFO, "EmuPalmOS", "write 8 bits 0x%02X to register 0x%08X", value, address);
  } else {
    if (!emupalmos_check_address(address, 1, 0)) return;
    WinLegacyGetAddr(&state->screenStart, &state->screenEnd);
    if (address >= state->screenStart && address < state->screenEnd) {
      debug(DEBUG_TRACE, "EmuPalmOS", "direct screen write byte 0x%08X = 0x%02X", address, value);
      WinLegacyWrite(address - state->screenStart, value);
    } else {
      ram = pumpkin_heap_base();
      WRITE_BYTE(ram, address, value);
    }
  }
}

void cpu_write_word(uint32_t address, uint16_t value) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint8_t *ram;

  if (state->write_word) return state->write_word(address, value);

  if (address >= 0xFFFFF000) {
    debug(DEBUG_INFO, "EmuPalmOS", "write 16 bits 0x%04X to register 0x%08X", value, address);
  } else {
    if (!emupalmos_check_address(address, 2, 0)) return;
    WinLegacyGetAddr(&state->screenStart, &state->screenEnd);
    if (address >= state->screenStart && address < state->screenEnd - 1) {
      debug(DEBUG_TRACE, "EmuPalmOS", "direct screen write word 0x%08X = 0x%04X", address, value);
      WinLegacyWrite(address - state->screenStart,     (value >> 8) & 0xff);
      WinLegacyWrite(address - state->screenStart + 1,  value       & 0xff);
    } else {
      ram = pumpkin_heap_base();
      WRITE_WORD(ram, address, value);
    }
  }
}

void cpu_write_long(uint32_t address, uint32_t value) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint8_t *ram;

  if (state->write_long) return state->write_long(address, value);

  if (address >= 0xFFFFF000) {
    debug(DEBUG_INFO, "EmuPalmOS", "write 32 bits 0x%08X to register 0x%08X", value, address);
  } else {
    if (!emupalmos_check_address(address, 4, 0)) return;
    WinLegacyGetAddr(&state->screenStart, &state->screenEnd);
    if (address >= state->screenStart && address < state->screenEnd - 3) {
      debug(DEBUG_TRACE, "EmuPalmOS", "direct screen write long 0x%08X = 0x%08X", address, value);
      WinLegacyWrite(address - state->screenStart,     (value >> 24) & 0xff);
      WinLegacyWrite(address - state->screenStart + 1, (value >> 16) & 0xff);
      WinLegacyWrite(address - state->screenStart + 2, (value >>  8) & 0xff);
      WinLegacyWrite(address - state->screenStart + 3,  value        & 0xff);
    } else {
      ram = pumpkin_heap_base();
      WRITE_LONG(ram, address, value);
    }
  }
}

void cpu_pulse_reset(void) {
}

void decode_datetime(uint32_t dateTimeP, DateTimeType *dateTime) {
  if (dateTimeP && dateTime) {
    MemSet(dateTime, sizeof(DateTimeType), 0);
    dateTime->second  = m68k_read_memory_16(dateTimeP +  0);
    dateTime->minute  = m68k_read_memory_16(dateTimeP +  2);
    dateTime->hour    = m68k_read_memory_16(dateTimeP +  4);
    dateTime->day     = m68k_read_memory_16(dateTimeP +  6);
    dateTime->month   = m68k_read_memory_16(dateTimeP +  8);
    dateTime->year    = m68k_read_memory_16(dateTimeP + 10);
    dateTime->weekDay = m68k_read_memory_16(dateTimeP + 12);
  }
}

void encode_datetime(uint32_t dateTimeP, DateTimeType *dateTime) {
  if (dateTimeP && dateTime) {
    m68k_write_memory_16(dateTimeP +  0, dateTime->second);
    m68k_write_memory_16(dateTimeP +  2, dateTime->minute);
    m68k_write_memory_16(dateTimeP +  4, dateTime->hour);
    m68k_write_memory_16(dateTimeP +  6, dateTime->day);
    m68k_write_memory_16(dateTimeP +  8, dateTime->month);
    m68k_write_memory_16(dateTimeP + 10, dateTime->year);
    m68k_write_memory_16(dateTimeP + 12, dateTime->weekDay);
  }
}

void decode_appinfo(uint32_t appInfoP, AppInfoType *appInfo) {
  UInt16 i, j, k;

  if (appInfoP && appInfo) {
    appInfo->renamedCategories = m68k_read_memory_16(appInfoP + 0);

    for (i = 0, k = 2; i < dmRecNumCategories; i++) {
      for (j = 0; j < dmCategoryLength; j++, k++) {
        appInfo->categoryLabels[i][j] = m68k_read_memory_8(appInfoP + k);
      }
    }
  }
}

void encode_appinfo(uint32_t appInfoP, AppInfoType *appInfo) {
  UInt16 i, j, k;

  if (appInfoP && appInfo) {
    m68k_write_memory_16(appInfoP + 0, appInfo->renamedCategories);

    for (i = 0, k = 2; i < dmRecNumCategories; i++) {
      for (j = 0; j < dmCategoryLength; j++, k++) {
        m68k_write_memory_8(appInfoP + k, appInfo->categoryLabels[i][j]);
      }
    }
  }
}

void decode_event(uint32_t eventP, EventType *event) {
  uint8_t *ram = pumpkin_heap_base();
  uint32_t a, i;
  char buf[256];

  MemSet(event, sizeof(EventType), 0);

  event->eType    = m68k_read_memory_16(eventP + 0);
  event->penDown  = m68k_read_memory_8 (eventP + 2);
  event->tapCount = m68k_read_memory_8 (eventP + 3);
  event->screenX  = m68k_read_memory_16(eventP + 4);
  event->screenY  = m68k_read_memory_16(eventP + 6);

  switch (event->eType) {
    case penDownEvent:
    case penMoveEvent:
    case nilEvent:
    case appStopEvent:
    case winDisplayChangedEvent:
    case appRaiseEvent:
      break;
    case keyDownEvent:
      event->data.keyDown.chr = m68k_read_memory_16(eventP + 8);
      event->data.keyDown.keyCode = m68k_read_memory_16(eventP + 10);
      event->data.keyDown.modifiers = m68k_read_memory_16(eventP + 12);
      break;
    case modKeyDownEvent:
    case modKeyUpEvent:
      event->data.keyDown.modifiers = m68k_read_memory_16(eventP + 12);
      break;
    case penUpEvent:
      event->data.penUp.start.x = m68k_read_memory_16(eventP + 8);
      event->data.penUp.start.y = m68k_read_memory_16(eventP + 10);
      event->data.penUp.end.x = m68k_read_memory_16(eventP + 12);
      event->data.penUp.end.y = m68k_read_memory_16(eventP + 14);
      break;
    case frmLoadEvent:
      event->data.frmLoad.formID = m68k_read_memory_16(eventP + 8);
      break;
    case frmOpenEvent:
      event->data.frmOpen.formID = m68k_read_memory_16(eventP + 8);
      break;
    case frmUpdateEvent:
      event->data.frmUpdate.formID = m68k_read_memory_16(eventP + 8);
      event->data.frmUpdate.updateCode = m68k_read_memory_16(eventP + 10);
      break;
    case frmCloseEvent:
      event->data.frmClose.formID = m68k_read_memory_16(eventP + 8);
      break;
    case frmTitleEnterEvent:
      event->data.frmTitleEnter.formID = m68k_read_memory_16(eventP + 8);
      break;
    case frmTitleSelectEvent:
      event->data.frmTitleSelect.formID = m68k_read_memory_16(eventP + 8);
      break;
    case menuEvent:
      event->data.menu.itemID = m68k_read_memory_16(eventP + 8);
      break;
    case fldEnterEvent:
      event->data.fldEnter.fieldID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.fldEnter.pField = a ? (FieldType *)(ram + a) : NULL;
      break;
    case fldChangedEvent:
      event->data.fldChanged.fieldID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.fldChanged.pField = a ? (FieldType *)(ram + a) : NULL;
      break;
    case ctlEnterEvent:
      event->data.ctlEnter.controlID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.ctlEnter.pControl = a ? (ControlType *)(ram + a) : NULL;
      break;
    case ctlExitEvent:
      event->data.ctlExit.controlID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.ctlExit.pControl = a ? (ControlType *)(ram + a) : NULL;
      break;
    case ctlSelectEvent:
      event->data.ctlSelect.controlID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.ctlSelect.pControl = a ? (ControlType *)(ram + a) : NULL;
      event->data.ctlSelect.on = m68k_read_memory_8(eventP + 14);
      event->data.ctlSelect.reserved1 = 0;
      event->data.ctlSelect.value = m68k_read_memory_16(eventP + 16);
      break;
    case ctlRepeatEvent:
      event->data.ctlRepeat.controlID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.ctlRepeat.pControl = a ? (ControlType *)(ram + a) : NULL;
      event->data.ctlRepeat.time = m68k_read_memory_32(eventP + 14);
      event->data.ctlRepeat.value = m68k_read_memory_16(eventP + 18);
      break;
    case lstEnterEvent:
      event->data.lstEnter.listID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.lstEnter.pList = a ? (ListType *)(ram + a) : NULL;
      event->data.lstEnter.selection = m68k_read_memory_16(eventP + 14);
      break;
    case lstExitEvent:
      event->data.lstExit.listID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.lstExit.pList = a ? (ListType *)(ram + a) : NULL;
      break;
    case lstSelectEvent:
      event->data.lstSelect.listID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.lstSelect.pList = a ? (ListType *)(ram + a) : NULL;
      event->data.lstSelect.selection = m68k_read_memory_16(eventP + 14);
      break;
    case popSelectEvent:
      event->data.popSelect.controlID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.popSelect.controlP = a ? (ControlType *)(ram + a) : NULL;
      event->data.popSelect.listID = m68k_read_memory_16(eventP + 14);
      a = m68k_read_memory_32(eventP + 16);
      event->data.popSelect.listP = a ? (ListType *)(ram + a) : NULL;
      event->data.popSelect.selection = m68k_read_memory_16(eventP + 20);
      event->data.popSelect.priorSelection = m68k_read_memory_16(eventP + 22);
      break;
    case sclEnterEvent:
      event->data.sclEnter.scrollBarID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.sclEnter.pScrollBar = a ? (ScrollBarType *)(ram + a) : NULL;
      break;
    case sclExitEvent:
      event->data.sclExit.scrollBarID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.sclExit.pScrollBar = a ? (ScrollBarType *)(ram + a) : NULL;
      event->data.sclExit.value = m68k_read_memory_16(eventP + 14);
      event->data.sclExit.newValue = m68k_read_memory_16(eventP + 16);
      break;
    case sclRepeatEvent:
      event->data.sclRepeat.scrollBarID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.sclRepeat.pScrollBar = a ? (ScrollBarType *)(ram + a) : NULL;
      event->data.sclRepeat.value = m68k_read_memory_16(eventP + 14);
      event->data.sclRepeat.newValue = m68k_read_memory_16(eventP + 16);
      event->data.sclRepeat.time = m68k_read_memory_32(eventP + 18);
      break;
    case frmGadgetEnterEvent:
      event->data.gadgetEnter.gadgetID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.gadgetEnter.gadgetP = a ? (FormGadgetType *)(ram + a) : NULL;
      break;
    case frmGadgetMiscEvent:
      event->data.gadgetMisc.gadgetID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.gadgetMisc.gadgetP = a ? (FormGadgetType *)(ram + a) : NULL;
      event->data.gadgetMisc.selector = m68k_read_memory_16(eventP + 14);
      a = m68k_read_memory_32(eventP + 16);
      event->data.gadgetMisc.dataP = a ? (ram + a) : NULL;
      break;
    case tblEnterEvent:
      event->data.tblEnter.tableID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.tblEnter.pTable = a ? (TableType *)(ram + a) : NULL;
      event->data.tblEnter.row = m68k_read_memory_16(eventP + 14);
      event->data.tblEnter.column = m68k_read_memory_16(eventP + 16);
      break;
    case tblExitEvent:
      event->data.tblExit.tableID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.tblExit.pTable = a ? (TableType *)(ram + a) : NULL;
      event->data.tblExit.row = m68k_read_memory_16(eventP + 14);
      event->data.tblExit.column = m68k_read_memory_16(eventP + 16);
      break;
    case tblSelectEvent:
      event->data.tblSelect.tableID = m68k_read_memory_16(eventP + 8);
      a = m68k_read_memory_32(eventP + 10);
      event->data.tblSelect.pTable = a ? (TableType *)(ram + a) : NULL;
      event->data.tblSelect.row = m68k_read_memory_16(eventP + 14);
      event->data.tblSelect.column = m68k_read_memory_16(eventP + 16);
      break;
    case winEnterEvent:
      a = m68k_read_memory_32(eventP + 8);
      event->data.winEnter.enterWindow = a ? (WinHandle)(ram + a) : NULL;
      a = m68k_read_memory_32(eventP + 12);
      event->data.winEnter.exitWindow = a ? (WinHandle)(ram + a) : NULL;
      break;
    case winExitEvent:
      a = m68k_read_memory_32(eventP + 8);
      event->data.winExit.enterWindow = a ? (WinHandle)(ram + a) : NULL;
      a = m68k_read_memory_32(eventP + 12);
      event->data.winExit.exitWindow = a ? (WinHandle)(ram + a) : NULL;
      break;
    default:
      if (event->eType < firstUserEvent || event->eType > lastUserEvent) {
        sys_snprintf(buf, sizeof(buf)-1, "decode event %s (%d) incomplete", EvtGetEventName(event->eType), event->eType);
        emupalmos_panic(buf, EMUPALMOS_UNDECODED_EVENT);
      } else {
        // 68K PalmOS EventType is 24 bytes long
        for (i = 0; i < 24; i++) {
          event->data.buffer[i] = m68k_read_memory_8 (eventP + 8 + i);
        }
      }
      break;
  }
}

void encode_gadget(uint32_t gadgetP, FormGadgetType *gadget) {
  if (gadgetP && gadget) {
    m68k_write_memory_16(gadgetP +  0, gadget->id);
    m68k_write_memory_16(gadgetP +  2, 0); // XXX attr
    m68k_write_memory_16(gadgetP +  4, gadget->rect.topLeft.x);
    m68k_write_memory_16(gadgetP +  6, gadget->rect.topLeft.y);
    m68k_write_memory_16(gadgetP +  8, gadget->rect.extent.x);
    m68k_write_memory_16(gadgetP + 10, gadget->rect.extent.y);
    m68k_write_memory_32(gadgetP + 12, gadget->m68k_data);
    m68k_write_memory_32(gadgetP + 16, gadget->m68k_handler);
  }
}

void encode_event(uint32_t eventP, EventType *event) {
  uint8_t *ram = pumpkin_heap_base();
  uint32_t a, i;
  char buf[256];

  m68k_write_memory_16(eventP + 0, event->eType);
  m68k_write_memory_8 (eventP + 2, event->penDown);
  m68k_write_memory_8 (eventP + 3, event->tapCount);
  m68k_write_memory_16(eventP + 4, event->screenX);
  m68k_write_memory_16(eventP + 6, event->screenY);

  switch (event->eType) {
    case penDownEvent:
    case penMoveEvent:
    case nilEvent:
    case appStopEvent:
    case winDisplayChangedEvent:
    case appRaiseEvent:
      break;
    case keyDownEvent:
      m68k_write_memory_16(eventP +  8, event->data.keyDown.chr);
      m68k_write_memory_16(eventP + 10, event->data.keyDown.keyCode);
      m68k_write_memory_16(eventP + 12, event->data.keyDown.modifiers);
      break;
    case modKeyDownEvent:
    case modKeyUpEvent:
      m68k_write_memory_16(eventP + 12, event->data.keyDown.modifiers);
      break;
    case penUpEvent:
      m68k_write_memory_16(eventP +  8, event->data.penUp.start.x);
      m68k_write_memory_16(eventP + 10, event->data.penUp.start.y);
      m68k_write_memory_16(eventP + 12, event->data.penUp.end.x);
      m68k_write_memory_16(eventP + 14, event->data.penUp.end.y);
      break;
    case frmLoadEvent:
      m68k_write_memory_16(eventP + 8, event->data.frmLoad.formID);
      break;
    case frmOpenEvent:
      m68k_write_memory_16(eventP + 8, event->data.frmOpen.formID);
      break;
    case frmUpdateEvent:
      m68k_write_memory_16(eventP +  8, event->data.frmUpdate.formID);
      m68k_write_memory_16(eventP + 10, event->data.frmUpdate.updateCode);
      break;
    case frmCloseEvent:
      m68k_write_memory_16(eventP + 8, event->data.frmClose.formID);
      break;
    case frmTitleEnterEvent:
      m68k_write_memory_16(eventP + 8, event->data.frmTitleEnter.formID);
      break;
    case frmTitleSelectEvent:
      m68k_write_memory_16(eventP + 8, event->data.frmTitleSelect.formID);
      break;
    case menuEvent:
      m68k_write_memory_16(eventP + 8, event->data.menu.itemID);
      break;
    case fldEnterEvent:
      m68k_write_memory_16(eventP +  8, event->data.fldEnter.fieldID);
      m68k_write_memory_32(eventP + 10, event->data.fldEnter.pField ? (uint32_t)((uint8_t *)event->data.fldEnter.pField - ram) : 0);
      break;
    case fldChangedEvent:
      m68k_write_memory_16(eventP +  8, event->data.fldChanged.fieldID);
      m68k_write_memory_32(eventP + 10, event->data.fldChanged.pField ? (uint32_t)((uint8_t *)event->data.fldChanged.pField - ram) : 0);
      break;
    case ctlEnterEvent:
      m68k_write_memory_16(eventP +  8, event->data.ctlEnter.controlID);
      m68k_write_memory_32(eventP + 10, event->data.ctlEnter.pControl ? (uint32_t)((uint8_t *)event->data.ctlEnter.pControl - ram) : 0);
      break;
    case ctlExitEvent:
      m68k_write_memory_16(eventP +  8, event->data.ctlExit.controlID);
      m68k_write_memory_32(eventP + 10, event->data.ctlExit.pControl ? (uint32_t)((uint8_t *)event->data.ctlExit.pControl - ram) : 0);
      break;
    case ctlSelectEvent:
      m68k_write_memory_16(eventP +  8, event->data.ctlSelect.controlID);
      m68k_write_memory_32(eventP + 10, event->data.ctlSelect.pControl ? (uint32_t)((uint8_t *)event->data.ctlSelect.pControl - ram) : 0);
      m68k_write_memory_8 (eventP + 14, event->data.ctlSelect.on ? 1 : 0);
      m68k_write_memory_8 (eventP + 15, 0); // reserved
      m68k_write_memory_16(eventP + 16, event->data.ctlSelect.value);
      break;
    case ctlRepeatEvent:
      m68k_write_memory_16(eventP +  8, event->data.ctlRepeat.controlID);
      m68k_write_memory_32(eventP + 10, event->data.ctlRepeat.pControl ? (uint32_t)((uint8_t *)event->data.ctlRepeat.pControl - ram) : 0);
      m68k_write_memory_32(eventP + 14, event->data.ctlRepeat.time);
      m68k_write_memory_16(eventP + 18, event->data.ctlRepeat.value);
      break;
    case lstEnterEvent:
      m68k_write_memory_16(eventP +  8, event->data.lstEnter.listID);
      m68k_write_memory_32(eventP + 10, event->data.lstEnter.pList ? (uint32_t)((uint8_t *)event->data.lstEnter.pList - ram) : 0);
      m68k_write_memory_16(eventP + 14, event->data.lstEnter.selection);
      break;
    case lstExitEvent:
      m68k_write_memory_16(eventP +  8, event->data.lstExit.listID);
      m68k_write_memory_32(eventP + 10, event->data.lstExit.pList ? (uint32_t)((uint8_t *)event->data.lstExit.pList - ram) : 0);
      break;
    case lstSelectEvent:
      m68k_write_memory_16(eventP +  8, event->data.lstSelect.listID);
      m68k_write_memory_32(eventP + 10, event->data.lstSelect.pList ? (uint32_t)((uint8_t *)event->data.lstSelect.pList - ram) : 0);
      m68k_write_memory_16(eventP + 14, event->data.lstSelect.selection);
      break;
    case popSelectEvent:
      m68k_write_memory_16(eventP +  8, event->data.popSelect.controlID);
      m68k_write_memory_32(eventP + 10, event->data.popSelect.controlP ? (uint32_t)((uint8_t *)event->data.popSelect.controlP - ram) : 0);
      m68k_write_memory_16(eventP + 14, event->data.popSelect.listID);
      m68k_write_memory_32(eventP + 16, event->data.popSelect.listP ? (uint32_t)((uint8_t *)event->data.popSelect.listP - ram) : 0);
      m68k_write_memory_16(eventP + 20, event->data.popSelect.selection);
      m68k_write_memory_16(eventP + 22, event->data.popSelect.priorSelection);
      break;
    case sclEnterEvent:
      m68k_write_memory_16(eventP +  8, event->data.sclEnter.scrollBarID);
      m68k_write_memory_32(eventP + 10, event->data.sclEnter.pScrollBar ? (uint32_t)((uint8_t *)event->data.sclEnter.pScrollBar - ram) : 0);
      break;
    case sclExitEvent:
      m68k_write_memory_16(eventP +  8, event->data.sclExit.scrollBarID);
      m68k_write_memory_32(eventP + 10, event->data.sclExit.pScrollBar ? (uint32_t)((uint8_t *)event->data.sclExit.pScrollBar - ram) : 0);
      m68k_write_memory_16(eventP + 14, event->data.sclExit.value);
      m68k_write_memory_16(eventP + 16, event->data.sclExit.newValue);
      break;
    case sclRepeatEvent:
      m68k_write_memory_16(eventP +  8, event->data.sclRepeat.scrollBarID);
      m68k_write_memory_32(eventP + 10, event->data.sclRepeat.pScrollBar ? (uint32_t)((uint8_t *)event->data.sclRepeat.pScrollBar - ram) : 0);
      m68k_write_memory_16(eventP + 14, event->data.sclRepeat.value);
      m68k_write_memory_16(eventP + 16, event->data.sclRepeat.newValue);
      m68k_write_memory_32(eventP + 18, event->data.sclRepeat.time);
      break;
    case frmGadgetEnterEvent:
      m68k_write_memory_16(eventP +  8, event->data.gadgetEnter.gadgetID);
      a = event->data.gadgetEnter.gadgetP ? (uint32_t)((uint8_t *)event->data.gadgetEnter.gadgetP - ram) : 0;
      m68k_write_memory_32(eventP + 10, a);
      encode_gadget(a, event->data.gadgetEnter.gadgetP);
      break;
    case frmGadgetMiscEvent:
      m68k_write_memory_16(eventP +  8, event->data.gadgetMisc.gadgetID);
      a = event->data.gadgetMisc.gadgetP ? (uint32_t)((uint8_t *)event->data.gadgetMisc.gadgetP - ram) : 0;
      m68k_write_memory_32(eventP + 10, a);
      m68k_write_memory_16(eventP + 14, event->data.gadgetMisc.selector);
      m68k_write_memory_32(eventP + 16, event->data.gadgetMisc.dataP ? (uint32_t)((uint8_t *)event->data.gadgetMisc.dataP - ram) : 0);
      encode_gadget(a, event->data.gadgetMisc.gadgetP);
      break;
    case tblEnterEvent:
      m68k_write_memory_16(eventP +  8, event->data.tblEnter.tableID);
      a = event->data.tblEnter.pTable ? (uint32_t)((uint8_t *)event->data.tblEnter.pTable - ram) : 0;
      m68k_write_memory_32(eventP + 10, a);
      m68k_write_memory_16(eventP + 14, event->data.tblEnter.row);
      m68k_write_memory_16(eventP + 16, event->data.tblEnter.column);
      break;
    case tblExitEvent:
      m68k_write_memory_16(eventP +  8, event->data.tblExit.tableID);
      a = event->data.tblExit.pTable ? (uint32_t)((uint8_t *)event->data.tblExit.pTable - ram) : 0;
      m68k_write_memory_32(eventP + 10, a);
      m68k_write_memory_16(eventP + 14, event->data.tblExit.row);
      m68k_write_memory_16(eventP + 16, event->data.tblExit.column);
      break;
    case tblSelectEvent:
      m68k_write_memory_16(eventP +  8, event->data.tblSelect.tableID);
      a = event->data.tblSelect.pTable ? (uint32_t)((uint8_t *)event->data.tblSelect.pTable - ram) : 0;
      m68k_write_memory_32(eventP + 10, a);
      m68k_write_memory_16(eventP + 14, event->data.tblSelect.row);
      m68k_write_memory_16(eventP + 16, event->data.tblSelect.column);
      break;
    case winEnterEvent:
      a = event->data.winEnter.enterWindow ? (uint32_t)((uint8_t *)event->data.winEnter.enterWindow - ram) : 0;
      m68k_write_memory_32(eventP + 8, a);
      a = event->data.winEnter.exitWindow ? (uint32_t)((uint8_t *)event->data.winEnter.exitWindow - ram) : 0;
      m68k_write_memory_32(eventP + 12, a);
      break;
    case winExitEvent:
      a = event->data.winExit.enterWindow ? (uint32_t)((uint8_t *)event->data.winExit.enterWindow - ram) : 0;
      m68k_write_memory_32(eventP + 8, a);
      a = event->data.winExit.exitWindow ? (uint32_t)((uint8_t *)event->data.winExit.exitWindow - ram) : 0;
      m68k_write_memory_32(eventP + 12, a);
      break;
    default:
      if (event->eType < firstUserEvent || event->eType > lastUserEvent) {
        sys_snprintf(buf, sizeof(buf)-1, "encode event %s (%d) incomplete", EvtGetEventName(event->eType), event->eType);
        emupalmos_panic(buf, EMUPALMOS_UNDECODED_EVENT);
      } else {
        for (i = 0; i < 24; i++) {
          m68k_write_memory_8 (eventP + 8 + i, event->data.buffer[i]);
        }
      }
      break;
  }
}

void decode_rgb(uint32_t rgbP, RGBColorType *rgb) {
  if (rgbP && rgb) {
    rgb->index = m68k_read_memory_8(rgbP);
    rgb->r = m68k_read_memory_8(rgbP+1);
    rgb->g = m68k_read_memory_8(rgbP+2);
    rgb->b = m68k_read_memory_8(rgbP+3);
  }
}

void encode_rgb(uint32_t rgbP, RGBColorType *rgb) {
  if (rgbP && rgb) {
    m68k_write_memory_8(rgbP,     rgb->index);
    m68k_write_memory_8(rgbP + 1, rgb->r);
    m68k_write_memory_8(rgbP + 2, rgb->g);
    m68k_write_memory_8(rgbP + 3, rgb->b);
  }
}

void decode_rectangle(uint32_t rP, RectangleType *rect) {
  if (rP && rect) {
    rect->topLeft.x = m68k_read_memory_16(rP);
    rect->topLeft.y = m68k_read_memory_16(rP + 2);
    rect->extent.x  = m68k_read_memory_16(rP + 4);
    rect->extent.y  = m68k_read_memory_16(rP + 6);
  }
}

void encode_rectangle(uint32_t rP, RectangleType *rect) {
  if (rP && rect) {
    m68k_write_memory_16(rP,     rect->topLeft.x);
    m68k_write_memory_16(rP + 2, rect->topLeft.y);
    m68k_write_memory_16(rP + 4, rect->extent.x);
    m68k_write_memory_16(rP + 6, rect->extent.y);
  }
}

void decode_point(uint32_t pP, PointType *point) {
  if (pP && point) {
    point->x = m68k_read_memory_16(pP);
    point->y = m68k_read_memory_16(pP + 2);
  }
}

void encode_point(uint32_t pP, PointType *point) {
  if (pP && point) {
    m68k_write_memory_16(pP,     point->x);
    m68k_write_memory_16(pP + 2, point->y);
  }
}

void encode_deviceinfo(uint32_t deviceInfoP, DeviceInfoType *deviceInfo) {
  if (deviceInfoP && deviceInfo) {
    uint8_t *ram = pumpkin_heap_base();
    m68k_write_memory_32(deviceInfoP +  0, deviceInfo->serDevCreator);
    m68k_write_memory_32(deviceInfoP +  4, deviceInfo->serDevFtrInfo);
    m68k_write_memory_32(deviceInfoP +  8, deviceInfo->serDevMaxBaudRate);
    m68k_write_memory_32(deviceInfoP + 12, deviceInfo->serDevHandshakeBaud);
    m68k_write_memory_32(deviceInfoP + 16, deviceInfo->serDevPortInfoStr ? ((uint8_t *)deviceInfo->serDevPortInfoStr - ram) : 0);
  }
}

void decode_NetSocketAddrType(uint32_t addrP, NetSocketAddrType *a) {
  if (addrP && a) {
    NetSocketAddrINType *addr = (NetSocketAddrINType *)a;
    addr->family = m68k_read_memory_16(addrP + 0);
    addr->port   = _NetSwap16(m68k_read_memory_16(addrP + 2));
    addr->addr   = m68k_read_memory_32(addrP + 4);
  }
}

void encode_NetSocketAddrType(uint32_t addrP, NetSocketAddrType *a) {
  if (addrP && a) {
    NetSocketAddrINType *addr = (NetSocketAddrINType *)a;
    m68k_write_memory_32(addrP + 0, addr->family);
    m68k_write_memory_32(addrP + 2, _NetSwap16(addr->port));
    m68k_write_memory_32(addrP + 4, addr->addr);
  }
}

/*
typedef struct {
  Char *    nameP;         // official name of host
  Char **   nameAliasesP;  // array of alias's for the name
  UInt16    addrType;      // address type of return addresses
  UInt16    addrLen;       // the length, in bytes, of the addresse
                           // Note this denotes length of a address, not # of addresses.
  UInt8 **  addrListP;     // array of ptrs to addresses in HBO
} NetHostInfoType;

typedef struct {
  NetHostInfoType hostInfo;         // high level results of call are here

  // The following fields contain the variable length data that hostInfo points to
  Char  name[netDNSMaxDomainName+1];      // hostInfo->name
  Char *aliasList[netDNSMaxAliases+1];    // +1 for 0 termination.
  Char  aliases[netDNSMaxAliases][netDNSMaxDomainName+1];

  NetIPAddr *addressList[netDNSMaxAddresses];
  NetIPAddr address[netDNSMaxAddresses];

} NetHostInfoBufType;
*/

void encode_NetHostInfoBufType(uint32_t bufP, NetHostInfoBufType *buf) {
  uint32_t i, j, offset, offset0;

  if (bufP && buf) {
    // hostInfo
    m68k_write_memory_32(bufP +  0, bufP + 16);
    m68k_write_memory_16(bufP +  8, buf->hostInfo.addrType);
    m68k_write_memory_16(bufP + 10, buf->hostInfo.addrLen);

    offset = 16;
    // name
    for (i = 0; i < netDNSMaxDomainName+1; i++, offset++) {
      m68k_write_memory_8(bufP + offset, buf->name[i]);
    }

    m68k_write_memory_32(bufP + 4, bufP + offset);

    // aliases
    offset0 = offset;
    for (i = 0; i < netDNSMaxAliases; i++, offset += 4) {
      m68k_write_memory_32(bufP + offset, bufP + offset0 + (netDNSMaxAliases+1)*4 + i*(netDNSMaxDomainName+1));
    }
    m68k_write_memory_32(bufP + offset, 0);
    offset += 4;
    for (i = 0; i < netDNSMaxAliases; i++) {
      for (j = 0; j < netDNSMaxDomainName+1; j++, offset++) {
        m68k_write_memory_8(bufP + offset, buf->aliases[i][j]);
      }
    }

    m68k_write_memory_32(bufP + 12, bufP + offset);

    // addresses
    offset0 = offset;
    for (i = 0; i < netDNSMaxAddresses; i++, offset += 4) {
      m68k_write_memory_32(bufP + offset, bufP + offset0 + netDNSMaxAddresses*4 + i*4);
    }
    for (i = 0; i < netDNSMaxAddresses; i++, offset += 4) {
      m68k_write_memory_32(bufP + offset, buf->address[i]);
    }
  }
}

/*
typedef struct {
  Char name[netConfigNameSize]; // name of configuration
} NetConfigNameType;
*/

void decode_NetConfigNameType(uint32_t nameArrayP, NetConfigNameType *nameArray) {
  uint32_t i;

  if (nameArrayP && nameArray) {
    for (i = 0; i < netConfigNameSize; i++) {
      nameArray->name[i] = m68k_read_memory_8(nameArrayP + i);
    }
  }
}

void encode_NetConfigNameType(uint32_t nameArrayP, NetConfigNameType *nameArray) {
  uint32_t i;

  if (nameArrayP && nameArray) {
    for (i = 0; i < netConfigNameSize; i++) {
      m68k_write_memory_8(nameArrayP + i, nameArray->name[i]);
    }
  }
}

void decode_FileInfoType(uint32_t fileInfoP, FileInfoType *fileInfo) {
  if (fileInfoP && fileInfo) {
    uint8_t *ram = pumpkin_heap_base();
    uint32_t a = m68k_read_memory_32(fileInfoP + 4);
    fileInfo->nameP = a ? (char *)ram + a : NULL;
    fileInfo->nameBufLen = m68k_read_memory_16(fileInfoP + 8);
  }
}

void encode_FileInfoType(uint32_t fileInfoP, FileInfoType *fileInfo) {
  uint32_t i;

  if (fileInfoP && fileInfo) {
    m68k_write_memory_32(fileInfoP + 0, fileInfo->attributes);
    if (fileInfo->nameP) {
      uint32_t a = m68k_read_memory_32(fileInfoP + 4);
      for (i = 0; i < fileInfo->nameBufLen && fileInfo->nameP[i]; i++) {
        m68k_write_memory_8(a + i, fileInfo->nameP[i]);
      }
      if (i < fileInfo->nameBufLen) {
        m68k_write_memory_8(a + i, 0);
      }
    }
  }
}

void decode_smfoptions(uint32_t selP, SndSmfOptionsType *options) {
  if (selP && options) {
    options->dwStartMilliSec = m68k_read_memory_32(selP);
    options->dwEndMilliSec = m68k_read_memory_32(selP + 4);
    options->amplitude = m68k_read_memory_16(selP + 8);
    options->interruptible = m68k_read_memory_8(selP + 10);
  }
}

/*
typedef struct VolumeInfoTag {
        UInt32  attributes;                     // read-only etc.
        UInt32  fsType;                         // Filesystem type for this volume (defined below)
        UInt32  fsCreator;                      // Creator code of filesystem driver for this volume.  For use with VFSCustomControl().
        UInt32  mountClass;                     // mount class that mounted this volume

        // For slot based filesystems: (mountClass = vfsMountClass_SlotDriver)
        UInt16  slotLibRefNum;          // Library on which the volume is mounted
        UInt16  slotRefNum;                     // ExpMgr slot number of card containing volume
        UInt32  mediaType;                      // Type of card media (mediaMemoryStick, mediaCompactFlash, etc...)
        UInt32  reserved;                       // reserved for future use (other mountclasses may need more space)
} VolumeInfoType;
*/

void encode_VolumeInfoType(uint32_t volInfoP, VolumeInfoType *volInfo) {
  if (volInfoP && volInfo) {
    m68k_write_memory_32(volInfoP +  0, volInfo->attributes);
    m68k_write_memory_32(volInfoP +  4, volInfo->fsType);
    m68k_write_memory_32(volInfoP +  8, volInfo->fsCreator);
    m68k_write_memory_32(volInfoP + 12, volInfo->mountClass);
    m68k_write_memory_16(volInfoP + 16, volInfo->slotLibRefNum);
    m68k_write_memory_16(volInfoP + 18, volInfo->slotRefNum);
    m68k_write_memory_32(volInfoP + 20, volInfo->mediaType);
    m68k_write_memory_32(volInfoP + 24, volInfo->reserved);
  }
}

void decode_notify(uint32_t notifyP, SysNotifyParamType *notify, int free_details) {
  char buf1[8], buf2[8];

  if (notifyP && notify) {
    uint8_t *ram = pumpkin_heap_base();
    notify->notifyType = m68k_read_memory_32(notifyP + 0);
    notify->broadcaster = m68k_read_memory_32(notifyP + 4);
    uint32_t notifyDetailsP = m68k_read_memory_32(notifyP + 8);
    uint32_t userDataP = m68k_read_memory_32(notifyP + 12);
    notify->userDataP = userDataP ? ram + userDataP : NULL;
    notify->handled = m68k_read_memory_8(notifyP + 16);

    pumpkin_id2s(notify->notifyType, buf1);
    if (notify->broadcaster) {
      pumpkin_id2s(notify->broadcaster, buf2);
    } else {
      sys_strcpy(buf2, "<none>");
    }
    debug(DEBUG_TRACE, "PALMOS", "SysNotifyParamType '%s' from '%s' details 0x%08X handled %d", buf1, buf2, notifyDetailsP, notify->handled);

    if (notifyDetailsP) {
      switch (notify->notifyType) {
        case sysNotifyDisplayChangeEvent: {
          SysNotifyDisplayChangeDetailsType *displayChange = (SysNotifyDisplayChangeDetailsType *)notify->notifyDetailsP;
          displayChange->oldDepth = m68k_read_memory_32(notifyDetailsP);
          displayChange->newDepth = m68k_read_memory_32(notifyDetailsP + 4);
        }
        break;
        case sysNotifyGPSDataEvent: {
          UInt16 *data = (UInt16 *)notify->notifyDetailsP;
          *data = m68k_read_memory_16(notifyDetailsP);
        }
        break;
        default:
        break;
      }

      if (free_details) {
        void *d = ram + notifyDetailsP;
        pumpkin_heap_free(d, "notifyDetails");
      }
    }
  }
}

void encode_notify(uint32_t notifyP, SysNotifyParamType *notify, int alloc_details) {
  if (notifyP && notify) {
    uint8_t *ram = pumpkin_heap_base();
    uint32_t notifyDetailsP = 0;
    uint8_t *notifyDetails;

    if (notify->notifyDetailsP) {
      switch (notify->notifyType) {
        case sysNotifyDisplayChangeEvent: {
          SysNotifyDisplayChangeDetailsType *displayChange = (SysNotifyDisplayChangeDetailsType *)notify->notifyDetailsP;
          if (alloc_details) {
            notifyDetails = pumpkin_heap_alloc(2*sizeof(uint32_t), "notifyDetails");
            notifyDetailsP = notifyDetails - ram;
          } else {
            notifyDetailsP = m68k_read_memory_32(notifyP + 8);
          }
          m68k_write_memory_32(notifyDetailsP + 0, displayChange->oldDepth);
          m68k_write_memory_32(notifyDetailsP + 4, displayChange->newDepth);
        }
        break;
        case sysNotifyGPSDataEvent: {
          UInt16 *data = (UInt16 *)notify->notifyDetailsP;
          notifyDetails = pumpkin_heap_alloc(sizeof(uint16_t), "notifyDetails");
          notifyDetailsP = notifyDetails - ram;
          m68k_write_memory_16(notifyDetailsP + 0, *data);
        }
        break;
        default:
        break;
      }
    }

    m68k_write_memory_32(notifyP +  0, notify->notifyType);
    m68k_write_memory_32(notifyP +  4, notify->broadcaster);
    m68k_write_memory_32(notifyP +  8, notifyDetailsP);
    m68k_write_memory_32(notifyP + 12, notify->userDataP ? ((uint8_t *)notify->userDataP - ram) : 0);
    m68k_write_memory_8(notifyP +  16, notify->handled);
  }
}

void encode_locale(uint32_t localeP, LmLocaleType *locale) {
  if (localeP && locale) {
    m68k_write_memory_16(localeP +  0, locale->language);
    m68k_write_memory_16(localeP +  2, locale->country);
  }
}

static int cpu_instr_callback(unsigned int pc);

// unsigned long Call68KFuncType(const void *emulStateP, unsigned long trapOrFunction, const void *argsOnStackP, unsigned long argsSizeAndwantA0)
static uint32_t call68K_func(uint32_t emulStateP, uint32_t trapOrFunction, uint32_t argsOnStackP, uint32_t argsSizeAndwantA0) {
  uint8_t *ram = pumpkin_heap_base();
  //void *emulState;
  void *argsOnStack;
  m68ki_cpu_core old_cpu, aux_cpu;
  uint32_t argsSize, wantA0, a4, a5, sp, r = 0;
  m68k_state_t *m68k_state;

  debug(DEBUG_TRACE, "EmuPalmOS", "call68K_func(0x%08X, 0x%08X, 0x%08X, 0x%08x)", emulStateP, trapOrFunction, argsOnStackP, argsSizeAndwantA0);

  // emulStateP: Pointer to the PACE emulation state. Supply the pointer that was passed to your ARM function by PACE.
  //emulState = emulStateP ? ram + emulStateP : NULL;

  // argsOnStackP: Native (little-endian) pointer to a block of memory to be copied to the 68K stack prior to the function call. This memory normally contains the arguments for the 68K function being called. Call68KFuncType pops these values from the 68K stack before returning.
  argsOnStack = argsOnStackP ? ram + argsOnStackP : NULL;

  // argsSizeAndwantA0: The number of bytes, in little-endian format, from argsOnStackP that are to be copied to the 68K emulator stack. If the function or trap returns its result in 68K register A0 (as when the result is a pointer type), you must OR the byte count with kPceNativeWantA0.
  argsSize = argsSizeAndwantA0 & ~kPceNativeWantA0;
  wantA0 = (argsSizeAndwantA0 & kPceNativeWantA0) ? 1 : 0;
  debug(DEBUG_TRACE, "EmuPalmOS", "call68K_func argsSize %d, wantA0 %d", argsSize, wantA0);

  // trapOrFunction: The trap number AND’ed with kPceNativeTrapNoMask, or a pointer to the function to call. Any value less than kPceNativeTrapNoMask is treated as a trap number.
  if (trapOrFunction < kPceNativeTrapNoMask) {
    debug(DEBUG_TRACE, "EmuPalmOS", "call68K_func trap 0x%04X", 0xA000 | trapOrFunction);
    sp = m68k_get_reg(NULL, M68K_REG_SP);
    sp -= argsSize;
    xmemcpy(ram + sp, argsOnStack, argsSize);
    m68k_set_reg(M68K_REG_SP, sp);

    palmos_systrap(0xA000 | trapOrFunction);
    r = m68k_get_reg(NULL, wantA0 ? M68K_REG_A0 : M68K_REG_D0);
    m68k_set_reg(M68K_REG_SP, sp + argsSize);

  } else {
    debug(DEBUG_TRACE, "EmuPalmOS", "call68K_func function 0x%08X", trapOrFunction);
    sp = m68k_get_reg(NULL, M68K_REG_SP);
    sp -= argsSize;
    xmemcpy(ram + sp, argsOnStack, argsSize);

    // return address: 0x00000000 so that m68k_execute aborts the loop
    sp -= 4;
    m68k_write_memory_32(sp, 0);

    a4 = m68k_get_reg(NULL, M68K_REG_A4);
    a5 = m68k_get_reg(NULL, M68K_REG_A5);
    m68k_get_context(&old_cpu);

    MemSet(&aux_cpu, sizeof(m68ki_cpu_core), 0);
    aux_cpu.palmos = 1;
    m68k_set_context(&aux_cpu);
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68020);
    m68k_pulse_reset();
    m68k_set_reg(M68K_REG_PC, trapOrFunction);
    m68k_set_reg(M68K_REG_SP, sp);
    m68k_set_reg(M68K_REG_A4, a4);
    m68k_set_reg(M68K_REG_A5, a5);
    m68k_set_instr_hook_callback(cpu_instr_callback);

    m68k_state = m68k_get_state();
    for (; !emupalmos_finished() && !thread_must_end();) {
      if (m68k_execute(m68k_state, 100000) == -1) break;
    }
    r = m68k_get_reg(NULL, wantA0 ? M68K_REG_A0 : M68K_REG_D0);
    debug(DEBUG_TRACE, "EmuPalmOS", "call68K_func function 0x%08X returned %d (0x%08X)", trapOrFunction, r, r);

    m68k_set_context(&old_cpu);
  }

  return r;
}

Int16 CallCompareFunction(UInt32 comparF, void *e1, void *e2, Int32 other) {
  uint32_t a, argsSize;
  uint8_t *p;
  Int16 r = 0;

  argsSize = sizeof(uint32_t) * 3;

  if ((p = pumpkin_heap_alloc(argsSize, "CallCompareFunction")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, (uint8_t *)e1 - ram);
    m68k_write_memory_32(a + 4, (uint8_t *)e2 - ram);
    m68k_write_memory_32(a + 8, other);
    debug(DEBUG_TRACE, "EmuPalmOS", "CallCompareFunction 0x%08X 0x%08X 0x%08X %d ...", comparF, (uint32_t)((uint8_t *)e1 - ram), (uint32_t)((uint8_t *)e2 - ram), other);
    r = call68K_func(0, comparF, a, argsSize) & 0xFFFF;
    debug(DEBUG_TRACE, "EmuPalmOS", "CallCompareFunction returned %d", r);
    pumpkin_heap_free(p, "CallCompareFunction");
  }

  return r;
}

Boolean CallFormHandler(UInt32 addr, EventType *eventP) {
  uint32_t a, argsSize, eventOffset;
  uint8_t *p;
  Boolean handled = false;

  debug(DEBUG_TRACE, "EmuPalmOS", "CallFormHandler addr 0x%08X event %d", addr, eventP->eType);
  argsSize = sizeof(uint32_t);
  eventOffset = argsSize;

  if ((p = pumpkin_heap_alloc(argsSize + sizeof(EventType), "CallForm")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, a + eventOffset);
    encode_event(a + eventOffset, eventP);
    handled = (call68K_func(0, addr, a, argsSize) & 0xFF) != 0x00;
    pumpkin_heap_free(p, "CallForm");
  }
  debug(DEBUG_TRACE, "EmuPalmOS", "CallFormHandler handled %d", handled);

  return handled;
}

Boolean CallGadgetHandler(UInt32 addr, FormGadgetTypeInCallback *gadgetP, UInt8 cmd, EventType *eventP) {
  uint32_t a, argsSize, gadgetOffset, eventOffset;
  uint8_t *p;
  Boolean handled = false;

  argsSize = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t);
  gadgetOffset = argsSize;
  eventOffset = argsSize + sizeof(FormGadgetTypeInCallback);

  if ((p = pumpkin_heap_alloc(argsSize + sizeof(FormGadgetTypeInCallback) + sizeof(EventType), "CallGadget")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, a + gadgetOffset);
    m68k_write_memory_16(a + sizeof(uint32_t), cmd);
    m68k_write_memory_32(a + sizeof(uint32_t) + sizeof(uint16_t), eventP ? a + eventOffset : 0);
    encode_gadget(a + gadgetOffset, (FormGadgetType *)gadgetP);
    if (eventP) encode_event(a + eventOffset, eventP);
    handled = call68K_func(0, addr, a, argsSize);
    pumpkin_heap_free(p, "CallGadget");
  }

  return handled;
}

void CallListDrawItem(UInt32 addr, Int16 i, RectangleType *rect, char **text) {
  uint32_t a, argsSize, rectOffset;
  uint8_t *p;

  argsSize = sizeof(int16_t) + sizeof(uint32_t) + sizeof(uint32_t);
  rectOffset = argsSize;

  if ((p = pumpkin_heap_alloc(argsSize + sizeof(RectangleType), "CallList")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_16(a, i);
    m68k_write_memory_32(a + sizeof(int16_t), a + rectOffset);
    m68k_write_memory_32(a + sizeof(int16_t) + sizeof(uint32_t), text ? (uint8_t *)&text[0] - ram : 0);
    encode_rectangle(a + rectOffset, rect);
    call68K_func(0, addr, a, argsSize);
    pumpkin_heap_free(p, "CallList");
  }
}

void CallTableDrawItem(UInt32 addr, TableType *tableP, Int16 row, Int16 column, RectangleType *rect) {
  uint32_t a, argsSize, rectOffset;
  uint8_t *p;

  debug(DEBUG_TRACE, "EmuPalmOS", "CallTableDrawItem(0x%08X, %p, %d, %d, [%d,%d,%d,%d])", addr, tableP, row, column, rect->topLeft.x, rect->topLeft.y, rect->extent.x, rect->extent.y);
  argsSize = sizeof(uint32_t) + sizeof(int16_t) + sizeof(int16_t) + sizeof(uint32_t);
  rectOffset = argsSize;

  if ((p = pumpkin_heap_alloc(argsSize + sizeof(RectangleType), "CallTable")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, (uint8_t *)tableP - ram);
    m68k_write_memory_16(a + sizeof(uint32_t), row);
    m68k_write_memory_16(a + sizeof(uint32_t) + sizeof(int16_t), column);
    m68k_write_memory_32(a + sizeof(uint32_t) + sizeof(int16_t) + sizeof(int16_t), a + rectOffset);
    encode_rectangle(a + rectOffset, rect);
    call68K_func(0, addr, a, argsSize);
    pumpkin_heap_free(p, "CallTable");
  }
}

Boolean CallTableSaveData(UInt32 addr, TableType *tableP, Int16 row, Int16 column) {
  uint32_t a, argsSize;
  uint8_t *p;
  Boolean r = false;

  debug(DEBUG_TRACE, "EmuPalmOS", "CallTableSaveData(0x%08X, %p, %d, %d)", addr, tableP, row, column);
  argsSize = sizeof(uint32_t) + sizeof(int16_t) + sizeof(int16_t);

  if ((p = pumpkin_heap_alloc(argsSize, "CallTable")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, (uint8_t *)tableP - ram);
    m68k_write_memory_16(a + sizeof(uint32_t), row);
    m68k_write_memory_16(a + sizeof(uint32_t) + sizeof(int16_t), column);
    r = call68K_func(0, addr, a, argsSize);
    pumpkin_heap_free(p, "CallTable");
  }

  return r;
}

Err CallTableLoadData(UInt32 addr, TableType *tableP, Int16 row, Int16 column, Boolean editable, MemHandle *dataH, Int16 *dataOffset, Int16 *dataSize, FieldPtr fld) {
  uint32_t a, argsSize, handleOffset, dOffset, d;
  uint16_t w;
  uint8_t *p;
  Err err = errNone;

  debug(DEBUG_TRACE, "EmuPalmOS", "CallTableLoadData(0x%08X, %p, %d, %d)", addr, tableP, row, column);
  argsSize = sizeof(uint32_t) + 3*sizeof(int16_t) + 4*sizeof(uint32_t);
  handleOffset = argsSize;
  dOffset = handleOffset + sizeof(MemHandle);

  if ((p = pumpkin_heap_alloc(argsSize +  sizeof(MemHandle) + 2*sizeof(Int16), "CallTable")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, (uint8_t *)tableP - ram);
    m68k_write_memory_16(a + sizeof(uint32_t), row);
    m68k_write_memory_16(a + sizeof(uint32_t) + sizeof(int16_t), column);
    m68k_write_memory_16(a + sizeof(uint32_t) + 2*sizeof(int16_t), editable);
    m68k_write_memory_32(a + sizeof(uint32_t) + 3*sizeof(int16_t), a + handleOffset);
    m68k_write_memory_32(a + sizeof(uint32_t) + 3*sizeof(int16_t) + sizeof(uint32_t), a + dOffset);
    m68k_write_memory_32(a + sizeof(uint32_t) + 3*sizeof(int16_t) + 2*sizeof(uint32_t), a + dOffset + 2);
    m68k_write_memory_32(a + sizeof(uint32_t) + 3*sizeof(int16_t) + 3*sizeof(uint32_t), (uint8_t *)fld - ram);
    err = call68K_func(0, addr, a, argsSize);
    get4b(&d, p, handleOffset);
    *dataH = ram + d;
    get2b(&w, p, dOffset);
    *dataOffset = w;
    get2b(&w, p, dOffset + 2);
    *dataSize = w;
    pumpkin_heap_free(p, "CallTable");
  }

  return err;
}

Int16 CallDmCompare(UInt32 addr, UInt32 rec1, UInt32 rec2, Int16 other, UInt32 rec1SortInfo, UInt32 rec2SortInfo, UInt32 appInfoH) {
  uint32_t a, argsSize;
  uint8_t *p;
  Int16 r = 0;

  debug(DEBUG_TRACE, "EmuPalmOS", "CallDmCompare(0x%08X, 0x%08X, 0x%08X, %d, 0x%08X, 0x%08X, 0x%08X", addr, rec1, rec2, other, rec1SortInfo, rec2SortInfo, appInfoH);
  argsSize = 5*sizeof(uint32_t) + sizeof(int16_t);

  if ((p = pumpkin_heap_alloc(argsSize, "CallDmCompare")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, rec1);
    m68k_write_memory_32(a +  4, rec2);
    m68k_write_memory_16(a +  8, other);
    m68k_write_memory_32(a + 10, rec1SortInfo);
    m68k_write_memory_32(a + 14, rec2SortInfo);
    m68k_write_memory_32(a + 18, appInfoH);
    r = call68K_func(0, addr, a, argsSize);
    pumpkin_heap_free(p, "CallDmCompare");
  }

  return r;
}

/*
typedef struct SysNotifyParamType {
UInt32 notifyType;
UInt32 broadcaster;
void * notifyDetailsP;
void * userDataP;
Boolean handled;
UInt8 reserved2;
} SysNotifyParamType;
*/

Err CallNotifyProc(UInt32 addr, SysNotifyParamType *notify) {
  uint32_t a, argsSize, notifyOffset;
  uint8_t *p;
  Err err = dmErrInvalidParam;

  argsSize = sizeof(uint32_t);
  notifyOffset = argsSize;

  if ((p = pumpkin_heap_alloc(argsSize + 4*sizeof(uint32_t) + sizeof(uint16_t), "CallNotify")) != NULL) {
    uint8_t *ram = pumpkin_heap_base();
    a = p - ram;
    m68k_write_memory_32(a, a + notifyOffset);
    encode_notify(a + notifyOffset, notify, 1);
    err = call68K_func(0, addr, a, argsSize);
    decode_notify(a + notifyOffset, notify, 1);
    pumpkin_heap_free(p, "CallNotify");
  }

  return err;
}

#ifdef ARMEMU
uint32_t arm_native_call(uint32_t code, uint32_t data, uint32_t userData) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint8_t *stack, *call68KAddr, *returnAddr;
  uint32_t stackAddr, callAddr, retAddr, sysAddr;
  uint8_t *ram = pumpkin_heap_base();

  // r9 = x
  // [x] = y
  // [y + 2052] = sysAddr + 16

  // LDR r9, [r9]
  // LDR r9, [r9, #2052]
  // [r9 + 2052] = sysAddr + 16
  // r9 = sysAddr + 16
  // LDR r12, [r9, #-8]

  // the ARM syscall master table: four 32-bits addresses,
  // each pointing to a syscall function table
  sysAddr = (uint8_t *)state->systable - ram;

  returnAddr = pumpkin_heap_alloc(0x8, "returnAddr");
  retAddr = returnAddr - ram;

  call68KAddr = pumpkin_heap_alloc(0x8, "call68kAddr");
  callAddr = call68KAddr - ram;

  stack = pumpkin_heap_alloc(stackSize, "stack");
  stackAddr = stack - ram;

  if (data) {
    armSetReg(state->arm,  9, sysAddr + 16); // register r9 points to the end of the syscall master table
    put4l(sysAddr, ram, sysAddr + 16);
    put4l(sysAddr + 16, ram, sysAddr + 2052);
  } else {
    armSetReg(state->arm,  9, sysAddr + 16); // register r9 points to the end of the syscall master table
  }
  armSetReg(state->arm, 15, code);    // PC
  armSetReg(state->arm, 14, retAddr); // LR
  armSetReg(state->arm, 13, stackAddr + stackSize); // SP
  debug(DEBUG_TRACE, "EmuPalmOS", "arm_native_call(0x%08X, 0x%08X) stack 0x%08X begin", code, userData, stackAddr + stackSize);

  // unsigned long NativeFuncType(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP)
  // The first four registers r0-r3 (a1-a4) are used to pass argument values into a subroutine and to return a result value from a function
  armSetReg(state->arm, 0, 0); // emulStateP == NULL
  armSetReg(state->arm, 1, userData);
  armSetReg(state->arm, 2, callAddr);

  for (; !emupalmos_finished();) {
    if (armRun(state->arm, 1000, callAddr, call68K_func, retAddr, 0)) break;
  }

  pumpkin_heap_free(returnAddr, "returnAddr");
  pumpkin_heap_free(call68KAddr, "call68kAddr");
  pumpkin_heap_free(stack, "stack");

  debug(DEBUG_TRACE, "EmuPalmOS", "arm_native_call(0x%08X, 0x%08X) end", code, userData);
  return armGetReg(state->arm, 0);
}
#endif

static void make_hex(char *buf, unsigned int pc, unsigned int length) {
  char *ptr = buf;

  for (; length > 0; length -= 2) {
    sys_sprintf(ptr, "%04x", cpu_read_word(pc));
    pc += 2;
    ptr += 4;
    if (length > 2) *ptr++ = ' ';
  }
}

/*
static void print_regs(void) {
  char buf[1024], aux[16];
  uint32_t d[8], a[8];
  int i;

  d[0] = m68k_get_reg(NULL, M68K_REG_D0);
  d[1] = m68k_get_reg(NULL, M68K_REG_D1);
  d[2] = m68k_get_reg(NULL, M68K_REG_D2);
  d[3] = m68k_get_reg(NULL, M68K_REG_D3);
  d[4] = m68k_get_reg(NULL, M68K_REG_D4);
  d[5] = m68k_get_reg(NULL, M68K_REG_D5);
  d[6] = m68k_get_reg(NULL, M68K_REG_D6);
  d[7] = m68k_get_reg(NULL, M68K_REG_D7);
  a[0] = m68k_get_reg(NULL, M68K_REG_A0);
  a[1] = m68k_get_reg(NULL, M68K_REG_A1);
  a[2] = m68k_get_reg(NULL, M68K_REG_A2);
  a[3] = m68k_get_reg(NULL, M68K_REG_A3);
  a[4] = m68k_get_reg(NULL, M68K_REG_A4);
  a[5] = m68k_get_reg(NULL, M68K_REG_A5);
  a[6] = m68k_get_reg(NULL, M68K_REG_A6);
  a[7] = m68k_get_reg(NULL, M68K_REG_A7);

  buf[0] = 0;
  for (i = 0; i < 8; i++) {
    sys_sprintf(aux, " D%d=%08X", i, d[i]);
    sys_strcat(buf, aux);
  }
  debug(DEBUG_INFO, "EmuPalmOS", "%s", buf);

  buf[0] = 0;
  for (i = 0; i < 8; i++) {
    sys_sprintf(aux, " A%d=%08X", i, a[i]);
    sys_strcat(buf, aux);
  }
  debug(DEBUG_INFO, "EmuPalmOS", "%s", buf);
}
*/

//static int aaa = 0;

static int cpu_instr_callback(unsigned int pc) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  uint32_t size = pumpkin_heap_size();
  uint32_t instr_size, d[8], a0, a1, a2, a3, a4, a5, a6, a7;
  uint16_t trap, selector;
  char buf[128], buf2[128], *s;
  int i;

  trapHook(pc, state);

  if ((pc & 1) == 0 && pc >= size && pc < (size + TRAPS_SIZE)) {
    trap = (pc - size) >> 2;
    if ((s = trapName(trap, &selector, 0)) != NULL) {
      debug(DEBUG_TRACE, "EmuPalmOS", "direct call to trap %s (pc 0x%08X)", s, pc);
      uint32_t a7 = m68k_get_reg(NULL, M68K_REG_A7);
      m68k_set_reg(M68K_REG_A7, a7+4);
      palmos_systrap(0xA000 | trap);
      m68k_set_reg(M68K_REG_A7, a7);
      debug(DEBUG_TRACE, "EmuPalmOS", "returned from trap %s (pc 0x%08X)", s, pc);
      return 0;
    }
    sys_snprintf(buf, sizeof(buf)-1, "trap 0x%04X unknown (pc 0x%08X)", trap, pc);
    emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
    return -1;
  }

//if (pc == 0x0006BED8) aaa = 1;
//else if (pc == 0x0006BEDA) aaa = 0;

  if (debug_on) {
    instr_size = m68k_disassemble(buf, pc, M68K_CPU_TYPE_68020);
    make_hex(buf2, pc, instr_size);
    for (i = 0; i <= M68K_REG_D7; i++) {
      d[i] = m68k_get_reg(NULL, M68K_REG_D0 + i);
    }
    a0 = m68k_get_reg(NULL, M68K_REG_A0);
    a1 = m68k_get_reg(NULL, M68K_REG_A1);
    a2 = m68k_get_reg(NULL, M68K_REG_A2);
    a3 = m68k_get_reg(NULL, M68K_REG_A3);
    a4 = m68k_get_reg(NULL, M68K_REG_A4);
    a5 = m68k_get_reg(NULL, M68K_REG_A5);
    a6 = m68k_get_reg(NULL, M68K_REG_A6);
    a7 = m68k_get_reg(NULL, M68K_REG_A7);
    debug(DEBUG_INFO, "M68K", "A0=0x%08X,A1=0x%08X,A2=0x%08X,A3=0x%08X,A4=0x%08X,A5=0x%08X,A6=0x%08X,A7=0x%08X",
      a0, a1, a2, a3, a4, a5, a6, a7);
    debug(DEBUG_INFO, "M68K", "%08X: %-20s: %s (%d,%d,%d,%d,%d,%d,%d,%d)",
      pc, buf2, buf, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
  }

  return 0;
}

static emu_state_t *emupalmos_new(void) {
  emu_state_t *state;

  if ((state = xcalloc(1, sizeof(emu_state_t))) != NULL) {
#ifdef ARMEMU
    uint8_t *ram = pumpkin_heap_base();
    uint32_t size = pumpkin_heap_size();
    uint32_t *dalFunctions, *bootFunctions, *uiFunctions;
    uint32_t i;

    state->arm = armInit(ram, size);

    // fill the ARM syscall tables for DAL, BOOT and UI functions
    state->systable = pumpkin_heap_alloc(4096, "sysTable");
    dalFunctions  = pumpkin_heap_alloc(0x1000, "dalFuntions");
    bootFunctions = pumpkin_heap_alloc(0x1000, "bootFuntions");
    uiFunctions   = pumpkin_heap_alloc(0x1000, "uiFuntions");
    state->systable[1] = (uint8_t *)dalFunctions  - ram;
    state->systable[2] = (uint8_t *)bootFunctions - ram;
    state->systable[3] = (uint8_t *)uiFunctions   - ram;

    // each syscall is identified by a special address of the form 0x04G0NNNN,
    // where G is the group (1, 2, or 3) and NNNN is the function "number" (multiple of 4).
    // if the PC reaches such address, emupalmos_arm_syscall will be called.
    for (i = 0; i < 0x1000 / 4; i++) {
      dalFunctions[i]  = 0x04000000 | 0x00100000 | (i << 2); // group 1: DAL
      bootFunctions[i] = 0x04000000 | 0x00200000 | (i << 2); // group 2: BOOT
      uiFunctions[i]   = 0x04000000 | 0x00300000 | (i << 2); // group 3: UI
    }
#endif
  }

  return state;
}

static void emupalmos_destroy(emu_state_t *state) {
  if (state) {
#ifdef ARMEMU
    uint8_t *ram = pumpkin_heap_base();
    armFinish(state->arm);
    pumpkin_heap_free(ram + state->systable[1], "dalFunctions");
    pumpkin_heap_free(ram + state->systable[2], "bootFunctions");
    pumpkin_heap_free(ram + state->systable[3], "uiFunctions");
    pumpkin_heap_free(state->systable, "sysTable");
#endif
    xfree(state);
  }
}

int emupalmos_debug_on(void) {
  return debug_on;
}

void emupalmos_debug(int on) {
  debug_on = 1;
}

int emupalmos_init(void) {
  m68k_init_once();
  allTrapsInit();
  debug_on = debug_getsyslevel("M68K") == DEBUG_TRACE;

  return 0;
}

#define strArg(a) (a ? ((char *)(ram + a)) : "")

uint32_t emupalmos_arm_syscall(uint32_t group, uint32_t function, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) {
  uint8_t *ram = pumpkin_heap_base();
  uint8_t *p;
  char st[8], st2[8];
  UInt32 value;
  MemHandle h;

  switch (group) {
    case 1: // DAL
      r0 = 0;
      break;
    case 2: // BOOT
      switch (function) {
        case 0x210:
          debug(DEBUG_TRACE, "ARM", "DmQueryRecord(0x%08X, %u)", r0, r1);
          h = DmQueryRecord((DmOpenRef)(ram + r0), r1);
          r0 = h ? (uint8_t *)h - ram : 0;
          break;
        case 0x318:
          // Err ExgGetDefaultApplication(UInt32 *creatorIDP, UInt16 id, const Char *dataTypeP)
          debug(DEBUG_TRACE, "ARM", "ExgGetDefaultApplication(0x%08X, 0x%04X, \"%s\") not implemented", r0, r1, strArg(r2));
          r0 = 0xffff;
          break;
        case 0x348:
          // Err ExgRegisterDatatype(UInt32 creatorID, UInt16 id, const Char *dataTypesP, const Char *descriptionsP, UInt16 flags)
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "ExgRegisterDatatype('%s', 0x%04X, %p, %p, ___) not implemented", st, r1, strArg(r2), strArg(r3));
          r0 = 0xffff;
          break;
        case 0x354:
          // Err ExgSetDefaultApplication(UInt32 creatorID, UInt16 id, const Char *dataTypeP)
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "ExgSetDefaultApplication('%s', 0x%04X, \"%s\") not implemented", st, r1, strArg(r2));
          r0 = 0xffff;
          break;
        case 0x424:
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "FtrGet('%s', %u, 0x%08X)", st, r1, r2);
          r0 = FtrGet(r0, r1, &value);
          if (r0 == 0 && r2) {
            put4l(value, ram, r2);
          }
          break;
        case 0x4DC:
          debug(DEBUG_TRACE, "ARM", "MemHandleFree(0x%08X)", r0);
          r0 = MemHandleFree((MemHandle)(ram + r0));
          break;
        case 0x4E4:
          debug(DEBUG_TRACE, "ARM", "MemHandleLock(0x%08X)", r0);
          p = MemHandleLock((MemHandle)(ram + r0));
          r0 = p ? (uint8_t *)p - ram : 0;
          break;
        case 0x508:
          debug(DEBUG_TRACE, "ARM", "MemHandleUnlock(0x%08X)", r0);
          r0 = MemHandleUnlock((MemHandle)(ram + r0));
          break;
        case 0x880:
          pumpkin_id2s(r0, st);
          pumpkin_id2s(r1, st2);
          debug(DEBUG_TRACE, "ARM", "SysLoadModule('%s', '%s', %d) not implemented", st, st2, r2);
          r0 = 0;
          break;
        case 0xB18:
          debug(DEBUG_TRACE, "ARM", "WinDrawChars(0x%08X, %d, %d, %d)", r0, r1, r2, r3);
          WinDrawChars((char *)(ram + r0), r1, r2, r3);
          break;
      }
      break;
    case 3: // UI
      r0 = 0;
      break;
    default:
      debug(DEBUG_ERROR, "ARM", "invalid group %u", group);
      r0 = 0;
      break;
  }

  return r0;
}

static void palmos_systrap_init(emu_state_t *state) {
  uint8_t *native, *addr;
  uint32_t szfunc, nativeSize;

  nativeSize = sizeof(SysFormPointerArrayToStrings_code) + sizeof(FrmDrawForm_code) + sizeof(SysBinarySearch_code) + sizeof(SysLibLoad_code);
  state->hNative = MemHandleNew(nativeSize);
  native = MemHandleLock(state->hNative);
  addr = native;

  szfunc = sizeof(SysFormPointerArrayToStrings_code);
  MemMove(addr, SysFormPointerArrayToStrings_code, szfunc);
  state->SysFormPointerArrayToStrings_addr = emupalmos_trap_out(addr);
  addr += szfunc;

  szfunc = sizeof(FrmDrawForm_code);
  MemMove(addr, FrmDrawForm_code, szfunc);
  state->FrmDrawForm_addr = emupalmos_trap_out(addr);
  addr += szfunc;

  szfunc = sizeof(SysBinarySearch_code);
  MemMove(addr, SysBinarySearch_code, szfunc);
  state->SysBinarySearch_addr = emupalmos_trap_out(addr);
  addr += szfunc;

  szfunc = sizeof(SysLibLoad_code);
  MemMove(addr, SysLibLoad_code, szfunc);
  state->SysLibLoad_addr = emupalmos_trap_out(addr);
  addr += szfunc;
}

static void palmos_systrap_finish(emu_state_t *state) {
  if (state && state->hNative) {
    MemHandleUnlock(state->hNative);
    MemHandleFree(state->hNative);
  }
}

emu_state_t *emupalmos_install(void) {
  emu_state_t *oldState, *state;

  state = emupalmos_new();
  oldState = pumpkin_get_local_storage(emu_key);
  pumpkin_set_local_storage(emu_key, state);

  return oldState;
}

void emupalmos_deinstall(emu_state_t *oldState) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  pumpkin_set_local_storage(emu_key, oldState);
  palmos_systrap_finish(state);
  emupalmos_destroy(state);
}

void emupalmos_memory_hooks(
  uint8_t (*read_byte)(uint32_t address),
  uint16_t (*read_word)(uint32_t address),
  uint32_t (*read_long)(uint32_t address),
  void (*write_byte)(uint32_t address, uint8_t value),
  void (*write_word)(uint32_t address, uint16_t value),
  void (*write_long)(uint32_t address, uint32_t value)) {

  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  state->read_byte = read_byte;
  state->read_word = read_word;
  state->read_long = read_long;
  state->write_byte = write_byte;
  state->write_word = write_word;
  state->write_long = write_long;
}

static uint8_t *getParamBlock(uint16_t launchCode, void *param, uint8_t *ram) {
  uint8_t *p = NULL;
  uint32_t a;

  switch (launchCode) {
    case sysAppLaunchCmdNotify:
      p = MemPtrNew(sizeof(SysNotifyParamType));
      a = p - ram;
      encode_notify(a, param, 1);
      break;
  }

  return p;
}

static void freeParamBlock(uint16_t launchCode, void *param, uint8_t *p, uint8_t *ram) {
  uint32_t a;

  switch (launchCode) {
    case sysAppLaunchCmdNotify:
      a = p - ram;
      decode_notify(a, param, 1);
      MemPtrFree(p);
      break;
  }
}

uint32_t emupalmos_main(uint16_t launchCode, void *param, uint16_t flags) {
  MemHandle hAmdc0, hAmdd0, hCode0, hCode1, hCodeN, hData0, hData, hStack, hSysAppInfo;
  uint8_t *amdc0, *amdd0, *code0, *code1, *data0, *data, *stack, *sysAppInfo, *paramBlock;
  uint32_t pc, a5, a7, st;
  uint32_t sysAppInfoStart, stackStart, codeStart, codeSize, dataStart, data0Size, dataSize, aboveSize;
  uint8_t *p8, *start, b;
  uint32_t i, j, k, k0, n, m, xr, count, creator;
  uint32_t paramBlockStart;
  int32_t code1_xrefs, data0_xrefs, offset, value;
  uint8_t *ram = pumpkin_heap_base();
  m68ki_cpu_core main_cpu;
  emu_state_t *oldState, *state;

  ram = pumpkin_heap_base();

#ifdef ARMEMU
  // amdc 0 segment
  hAmdc0 = DmGet1Resource('amdc', 0);
  amdc0 = hAmdc0 ? MemHandleLock(hAmdc0) : NULL;

  // amdd 0 segment
  hAmdd0 = DmGet1Resource('amdd', 0);
  amdd0 = hAmdd0 ? MemHandleLock(hAmdd0) : NULL;

  if (amdc0 && amdd0) {
    state = emupalmos_new();
    oldState = pumpkin_get_local_storage(emu_key);
    pumpkin_set_local_storage(emu_key, state);

    paramBlock = getParamBlock(launchCode, param, ram);
    paramBlockStart = paramBlock ? paramBlock - ram : 0;

    hSysAppInfo = MemHandleNew(sizeof(SysAppInfoType));
    sysAppInfo = MemHandleLock(hSysAppInfo);
    MemSet(sysAppInfo, MemHandleSize(hSysAppInfo), 0);
    sysAppInfoStart = sysAppInfo - ram;
    put2l(launchCode, sysAppInfo, 0);
    put4l(paramBlockStart, sysAppInfo, 2); // cmdPBP
    put2l(flags | sysAppLaunchFlagNewGlobals | sysAppLaunchFlagDataRelocated, sysAppInfo, 6); // launch flags
    put4l((uint8_t *)hAmdc0 - ram, sysAppInfo, 12); // codeH

    state->sysAppInfoStart = sysAppInfoStart;
    palmos_systrap_init(state);

    creator = pumpkin_get_app_creator();
    pumpkin_set_compat(creator, appCompatOk, 0);
    emupalmos_finish(0);

    arm_native_call(amdc0 - ram, amdd0 - ram, 0);

    MemHandleUnlock(hSysAppInfo);
    MemHandleFree(hSysAppInfo);

    pumpkin_set_local_storage(emu_key, oldState);
    palmos_systrap_finish(state);
    emupalmos_destroy(state);

    MemHandleUnlock(hAmdd0);
    DmReleaseResource(hAmdd0);
    MemHandleUnlock(hAmdc0);
    DmReleaseResource(hAmdc0);

    return 0;
  }
#endif

  // code 0 segment
  hCode0 = DmGet1Resource('code', 0);
  code0 = hCode0 ? MemHandleLock(hCode0) : NULL;

  // code 1 segment
  hCode1 = DmGet1Resource('code', 1);
  code1 = hCode1 ? MemHandleLock(hCode1) : NULL;

  // data 0 segment
  hData0 = DmGet1Resource('data', 0);
  data0 = hData0 ? MemHandleLock(hData0) : NULL;

  if (code1) {
      state = emupalmos_new();
      oldState = pumpkin_get_local_storage(emu_key);
      pumpkin_set_local_storage(emu_key, state);

      codeStart = code1 - ram;
      codeSize = MemHandleSize(hCode1);

      paramBlock = getParamBlock(launchCode, param, ram);
      paramBlockStart = paramBlock ? paramBlock - ram : 0;

      hSysAppInfo = MemHandleNew(sizeof(SysAppInfoType));
      sysAppInfo = MemHandleLock(hSysAppInfo);
      MemSet(sysAppInfo, MemHandleSize(hSysAppInfo), 0);
      sysAppInfoStart = sysAppInfo - ram;
      m68k_write_memory_16(sysAppInfoStart, launchCode);
      m68k_write_memory_32(sysAppInfoStart +  2, paramBlockStart); // cmdPBP
      m68k_write_memory_16(sysAppInfoStart +  6, flags | sysAppLaunchFlagNewGlobals | sysAppLaunchFlagDataRelocated); // launch flags
      m68k_write_memory_32(sysAppInfoStart + 12, (uint8_t *)hCode1 - ram); // codeH
      debug(DEBUG_TRACE, "EmuPalmOS", "sysAppInfoStart 0x%08X", sysAppInfoStart);
      debug(DEBUG_TRACE, "EmuPalmOS", "cmdPBP %p 0x%08X", paramBlock, paramBlockStart);

      if (code0) {
        debug(DEBUG_TRACE, "EmuPalmOS", "code0 begin");
        debug_bytes(DEBUG_TRACE, "EmuPalmOS", code0, MemHandleSize(hCode0));
        debug(DEBUG_TRACE, "EmuPalmOS", "code0 end");
        get4b(&aboveSize, code0, 0);
        get4b(&dataSize, code0, 4);
        hData = MemHandleNew(dataSize + aboveSize);
        debug(DEBUG_INFO, "EmuPalmOS", "dataSize %d aboveSize %d", dataSize, aboveSize);
        data = MemHandleLock(hData);
        MemSet(data, dataSize + aboveSize, 0);
        dataStart = data - ram;
      } else {
        aboveSize = 0;
        dataSize = 0;
        dataStart = 0;
        hData = 0;
        data = NULL;
      }

      data0_xrefs = 0;
      code1_xrefs = 0;

      if (data0) {
        data0Size = MemHandleSize(hData0);
        debug(DEBUG_TRACE, "EmuPalmOS", "data0 begin");
        debug_bytes(DEBUG_TRACE, "EmuPalmOS", data0, data0Size);
        debug(DEBUG_TRACE, "EmuPalmOS", "data0 end");

        i = get4b((uint32_t *)&code1_xrefs, data0, 0);

        for (m = 0; m < 3 && !state->panic; m++) {
          i += get4b((uint32_t *)&offset, data0, i);
          st = dataSize + offset;
          debug(DEBUG_INFO, "EmuPalmOS", "decoding data chain %d", m);
          debug(DEBUG_INFO, "EmuPalmOS", "data A5 offset %d (0x%04X)", offset, offset);
          debug(DEBUG_INFO, "EmuPalmOS", "data start %d (0x%04X)", st, st);
          start = &data[st];
          p8 = data0;

          for (k = 0; !state->panic;) {
            b = p8[i++];
            if (b == 0x00) {
              debug(DEBUG_TRACE, "EmuPalmOS", "data chain %d end %d (0x%04X) i 0x%04X", m, st+k, st+k, i);
              break;
            }
            if ((b & 0x80) == 0x80) {
              n = b & 0x7F;
              debug(DEBUG_TRACE, "EmuPalmOS", "%3d 0x%02X 0x80 block %3d bytes (0x%08X to 0x%08X)",
                i-1, b, n+1, dataStart + st + k, dataStart + st + k + n);
              for (j = 0, k0 = k; j < n+1; j++) {
                start[k++] = p8[i++];
              }
              debug_bytes(DEBUG_TRACE, "EmuPalmOS", &start[k0], n+1);
            } else if ((b & 0xC0) == 0x40) {
              n = b & 0x3F;
              debug(DEBUG_TRACE, "EmuPalmOS", "%3d 0x%02X 0x40 block %3d bytes (0x%08X to 0x%08X = 0x00)",
                i-1, b, n+1, dataStart + st + k, dataStart + st + k + n);
              for (j = 0; j < n+1; j++) {
                start[k++] = 0x00;
              }
            } else if ((b & 0xE0) == 0x20) {
              n = b & 0x1F;
              b = p8[i++];
              debug(DEBUG_TRACE, "EmuPalmOS", "%3d 0x%02X 0x20 block %3d bytes (0x%08X to 0x%08X = 0x%02X)",
                i-1, b, n+2, dataStart + st + k, dataStart + st + k + n+1, b);
              for (j = 0; j < n+2; j++) {
                start[k++] = b;
              }
            } else if ((b & 0xF0) == 0x10) {
              n = b & 0x0F;
              debug(DEBUG_TRACE, "EmuPalmOS", "%3d 0x%02X 0x10 block %3d bytes (0x%08X to 0x%08X = 0xFF)",
                i-1, b, n+1, dataStart + st + k, dataStart + st + k + n);
              for (j = 0; j < n+1; j++) {
                start[k++] = 0xFF;
              }
            } else if (b == 0x01) {
              debug(DEBUG_TRACE, "EmuPalmOS", "0x01 block");
              start[k++] = 0x00;
              start[k++] = 0x00;
              start[k++] = 0x00;
              start[k++] = 0x00;
              start[k++] = 0xFF;
              start[k++] = 0xFF;
              start[k++] = p8[i++];
              start[k++] = p8[i++];
            } else if (b == 0x02) {
              debug(DEBUG_TRACE, "EmuPalmOS", "0x01 block");
              start[k++] = 0x00;
              start[k++] = 0x00;
              start[k++] = 0x00;
              start[k++] = 0x00;
              start[k++] = 0xFF;
              start[k++] = p8[i++];
              start[k++] = p8[i++];
              start[k++] = p8[i++];
            } else if (b == 0x03) {
              debug(DEBUG_TRACE, "EmuPalmOS", "0x01 block");
              start[k++] = 0xA9;
              start[k++] = 0xF0;
              start[k++] = 0x00;
              start[k++] = 0x00;
              start[k++] = p8[i++];
              start[k++] = p8[i++];
              start[k++] = 0x00;
              start[k++] = p8[i++];
            } else if (b == 0x04) {
              debug(DEBUG_TRACE, "EmuPalmOS", "0x01 block");
              start[k++] = 0xA9;
              start[k++] = 0xF0;
              start[k++] = 0x00;
              start[k++] = p8[i++];
              start[k++] = p8[i++];
              start[k++] = p8[i++];
              start[k++] = 0x00;
              start[k++] = p8[i++];
            }
          }
        }

        debug(DEBUG_TRACE, "EmuPalmOS", "data begin");
        debug_bytes(DEBUG_TRACE, "EmuPalmOS", data, dataSize);
        debug(DEBUG_TRACE, "EmuPalmOS", "data end");

        data0_xrefs = i;
        if (data0_xrefs < data0Size - 12) {
          debug(DEBUG_INFO, "EmuPalmOS", "data xrefs at 0x%04X", data0_xrefs);

          for (m = 0; m < 3 && !state->panic; m++) {
            i += get4b(&count, data0, i);
            debug(DEBUG_INFO, "EmuPalmOS", "decoding %d xrefs for chain %d at 0x%04X", count, m, i);
            uint32_t segment, relocbase;
            if (m == 0) {
              // chain 0 base is at the end of data segment
              relocbase = dataStart + dataSize;
            } else {
              // chain 1 base is at the beginning of code segment
              relocbase = codeStart;
            }
            segment = dataStart + dataSize;
            offset = 0;

            for (xr = 0; xr < count && !state->panic; xr++) {
              b = data0[i++];
              if (b & 0x80) {
                // relative 8 bits signed delta
                int8_t d = b;
                d <<= 1;
                offset += d;
                get4b((uint32_t *)&value, ram, segment + offset);
                debug(DEBUG_TRACE, "EmuPalmOS", " 8-bits data xref %2d at 0x%04X: %5d 0x%08X 0x%08X", xr, i-1, d, segment + offset, value);
              } else {
                if (b & 0x40) {
                  // relative 15 bits unsigned delta
                  int16_t w = b;
                  w <<= 8;
                  w |= data0[i++];
                  w <<= 2;
                  w >>= 1;
                  offset += w;
                  get4b((uint32_t *)&value, ram, segment + offset);
                  debug(DEBUG_TRACE, "EmuPalmOS", "16-bits data xref %2d at 0x%04X: %5d 0x%08X 0x%08X", xr, i-2, w, segment + offset, value);
                } else {
                  // absolute 31 bits offset
                  int32_t l = b;
                  l <<= 8;
                  l |= data0[i++];
                  l <<= 8;
                  l |= data0[i++];
                  l <<= 8;
                  l |= data0[i++];
                  l <<= 2;
                  l >>= 1;
                  offset = l;
                  get4b((uint32_t *)&value, ram, segment + offset);
                  debug(DEBUG_TRACE, "EmuPalmOS", "31-bits data xref %2d at 0x%04X: %5d 0x%08X 0x%08X", xr, i-4, l, segment + offset, value);
                }
              }
              if (m < 2) put4b(value + relocbase, ram, segment + offset);
            }
          }
        } else {
          debug(DEBUG_INFO, "EmuPalmOS", "no data xrefs");
        }

        code1_xrefs = i;
        if (code1_xrefs < data0Size - 12) {
          debug(DEBUG_INFO, "EmuPalmOS", "code xrefs at 0x%04X", code1_xrefs);

          for (m = 0; m < 3 && !state->panic; m++) {
            i += get4b(&count, data0, i);
            debug(DEBUG_INFO, "EmuPalmOS", "decoding %d xrefs for chain %d at 0x%04X", count, m, i);
            if (count) {
              //emupalmos_panic("Unsupported code xrefs.", EMUPALMOS_INVALID_XREF);
              break;
            }
          }
        } else {
          debug(DEBUG_INFO, "EmuPalmOS", "no code xrefs");
        }

        debug(DEBUG_TRACE, "EmuPalmOS", "data begin (fixed)");
        debug_bytes(DEBUG_TRACE, "EmuPalmOS", data, dataSize);
        debug(DEBUG_TRACE, "EmuPalmOS", "data end");
      }

      hStack = MemHandleNew(stackSize);
      stack = MemHandleLock(hStack);
      MemSet(stack, stackSize, 0);
      stackStart = stack - ram;

      pc = codeStart;
      a5 = dataStart + dataSize; // On PalmOS, register a5 points to the end of the global data
      a7 = stackStart + stackSize - 16;
      state->stackStart = stackStart;
      state->sysAppInfoStart = sysAppInfoStart;

      MemSet(&main_cpu, sizeof(m68ki_cpu_core), 0);
      main_cpu.palmos = 1;
      m68k_set_context(&main_cpu);
      m68k_init();
      m68k_set_cpu_type(M68K_CPU_TYPE_68020);
      m68k_pulse_reset();
      m68k_set_reg(M68K_REG_PC, pc);
      m68k_set_reg(M68K_REG_SP, a7);
      m68k_set_reg(M68K_REG_A5, a5);
      m68k_set_instr_hook_callback(cpu_instr_callback);

      palmos_systrap_init(state);

      debug(DEBUG_INFO, "EmuPalmOS", "code  segment from 0x%08X to 0x%08X size 0x%04X", codeStart,  codeStart  + codeSize  - 1, codeSize);
      debug(DEBUG_INFO, "EmuPalmOS", "stack segment from 0x%08X to 0x%08X size 0x%04X", stackStart, stackStart + stackSize - 1, stackSize);
      if (dataSize+aboveSize) debug(DEBUG_INFO, "EmuPalmOS", "data  segment from 0x%08X to 0x%08X size 0x%04X", dataStart,  dataStart  + dataSize + aboveSize - 1, dataSize+ aboveSize );

      if (!state->panic) {
        creator = pumpkin_get_app_creator();
        pumpkin_set_compat(creator, appCompatOk, 0);
        emupalmos_finish(0);
        for (; !emupalmos_finished() && !thread_must_end();) {
          m68k_execute(&state->m68k_state, 100000);
        }
      }

      if (state->panic) {
        SysFatalAlert(state->panic);
        xfree(state->panic);
        pumpkin_forward_msg(0, MSG_KEY, WINDOW_KEY_CUSTOM, vchrAppCrashed, 0);
      }

      if (paramBlock) {
        freeParamBlock(launchCode, param, paramBlock, ram);
      }

      pumpkin_set_local_storage(emu_key, oldState);
      palmos_systrap_finish(state);
      emupalmos_destroy(state);

      if (hData) MemHandleUnlock(hData);
      if (hData) MemHandleFree(hData);
      MemHandleUnlock(hStack);
      MemHandleFree(hStack);
      MemHandleUnlock(hSysAppInfo);
      MemHandleFree(hSysAppInfo);
  }

  if (hData0) MemHandleUnlock(hData0);
  if (hData0) DmReleaseResource(hData0);
  if (hCode0) MemHandleUnlock(hCode0);
  if (hCode0) DmReleaseResource(hCode0);
  if (hCode1) MemHandleUnlock(hCode1);
  if (hCode1) DmReleaseResource(hCode1);

  // prc-tools crt calls DmGet1Resource twice for each code resource >= 2,
  // but calls DmReleaseResource just once. So we have to account for the missing DmReleaseResource here.
  for (i = 2;; i++) {
    if ((hCodeN = DmGet1Resource('code', i)) == NULL) break;
    DmReleaseResource(hCodeN);
    DmReleaseResource(hCodeN);
  }

  return 0;
}

uint8_t *emupalmos_ram(void) {
  return pumpkin_heap_base();
}

emu_state_t *m68k_get_emu_state(void) {
  return pumpkin_get_local_storage(emu_key);
}

m68k_state_t *m68k_get_state(void) {
  emu_state_t *state = pumpkin_get_local_storage(emu_key);
  return &state->m68k_state;
}
