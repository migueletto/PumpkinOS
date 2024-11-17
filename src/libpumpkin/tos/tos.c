#include <PalmOS.h>
#include <VFSMgr.h>

#include "thread.h"
#include "bytes.h"
#include "rgb.h"
#include "plibc.h"
#include "unzip.h"
#include "scancodes.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "tos.h"
#include "gemdos.h"
#include "gemdos_proto.h"
#include "bios_proto.h"
#include "xbios_proto.h"
#include "xbios_proto.h"
#include "debug.h"

#define headerSize       28

#define sysvarsSize    2048
#define kbdvSize         64
#define lineaSize      1024
#define screenSize    32000
#define lowMemSize     (sysvarsSize + kbdvSize + lineaSize + screenSize)
#define basePageSize    256
#define stackSize      4096

#define ioStart 0x00FF8000
#define ioEnd   0x01000000

#define maxPath 256

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


// 0x00000000 - 0x009FFFFF: RAM
// 0x00A00000 – 0x00DEFFFF: RAM
// 0x00DF0000 – 0x00DFFFFF: RAM
// 0x00E00000 – 0x00EFFFFF: ROM
// 0x00F00000 - 0x00F0003F: IDE controller
// 0x00F00040 – 0x00F9FFFF: unassigned
// 0x00FA0000 – 0x00FBFFFF: cartridge ROM
// 0x00FC0000 – 0x00FEFFFF: OS ROM
// 0x00FF0000 – 0x00FF7FFF: unassigned
// 0x00FF8000 - 0x00FFF9FF: I/O
// 0x00FFFA00 - 0x00FFFA3F: I/O 68091 (peripheral)
// 0x00FFFA40 - 0x00FFFA7F: I/O 68881 (math)
// 0x00FFFA80 - 0x00FFFBFF: I/O 68901 (peripheral)
// 0x00FFFC00 - 0x00FFFC1F: I/O 6850 (keyboard, MIDI)
// 0x00FFFC20 - 0x00FFFC3F: I/O RTC
// 0x00FFFC40 – 0x00FFFFFF: unassigned
// 0x01000000 – 0x01FFFFFF: RAM (expansion)
// 0x02000000 – 0xFDFFFFFF: reserved
// 0xFE000000 – 0xFEFEFFFF: VME
// 0xFEFF0000 – 0xFEFFFFFF: VME
// 0xFF000000 – 0xFFFFFFFF: shadow of 0x00000000 – 0x00FFFFFF

static const uint16_t palette[] = {
  7, 7, 7,
  7, 0, 0,
  0, 7, 0,
  7, 7, 0,
  0, 0, 7,
  7, 0, 7,
  0, 7, 7,
  5, 5, 5,
  3, 3, 3,
  7, 3, 3,
  3, 7, 3,
  7, 7, 3,
  3, 3, 7,
  7, 3, 7,
  3, 7, 7,
  0, 0, 0
};

uint32_t tos_read_byte(tos_data_t *data, uint32_t address) {
  EventType event;
  Err err;
  uint8_t value = 0;

  if ((address & 0xFF000000) == 0xFF000000) {
    address &= 0x00FFFFFF;
  }
  
  if (address >= ioStart && address < ioEnd) {
    switch (address) {
      case 0x00FFFA09: // MFP-ST Interrupt Enable Register B
        value = 0x40; // keyboard/MIDI
        break;
      case 0x00FFFC00: // Keyboard ACIA Control
        // bit 0: receiver full
        // bit 1: transmitter empty

        value = 0x02;

        while (EvtEventAvail()) {
          EvtGetEvent(&event, 0);
          if (SysHandleEvent(&event)) continue;
          if (MenuHandleEvent(NULL, &event, &err)) continue;
          if (event.eType == appStopEvent) {
            emupalmos_finish(1);
          }
        }

        if (data->ikbd_state && data->ikbd_cmd != 0x80) {
          value |= 0x01;
        }

        if (data->KeyQueueReadIndex != data->KeyQueueWriteIndex) {
          value |= 0x01;
        }
        break;
      case 0x00FFFC02: // keyboard ACIA data
        if (data->KeyQueueReadIndex != data->KeyQueueWriteIndex && (data->ikbd_cmd == 0 || data->ikbd_state <= 1)) {
          uint32_t code, scancode;
          tos_get_key(data, &code);
          scancode = (code >> 16) & 0xFF;
          if (scancode) {
            value = scancode;
            if ((scancode & 0x7F) == SCAN_LEFTSHIFT) {
              data->shift = scancode & 0x80 ? 0 : 1;
            }
            if ((scancode & 0x7F) == SCAN_CONTROL) {
              data->ctrl = scancode & 0x80 ? 0 : 1;
            }
            debug(DEBUG_TRACE, "TOS", "scancode 0x%02X", value);
          }
        } else if (data->ikbd_cmd) {
          Int16 x, y;
          Boolean penDown, right;

          switch (data->ikbd_cmd) {
            case 0x0D:
              // Interrogate mouse position.
              // Returns:  0xF7  ; absolute mouse position header 
              //   BUTTONS
              //     0000dcba
              //     where a is right button down since last interrogation
              //     b is right button up since last
              //     c is left button down since last
              //     d is left button up since last
              //   XMSB      ; X coordinate
              //   XLSB
              //   YMSB      ; Y coordinate
              //   YLSB

              switch (data->ikbd_state) {
                case 1: // header
                  EvtGetPenEx(&x, &y, &penDown, &right);
                  switch (data->screen_res) {
                    case 0: x >>= 1; y >>= 1; break;
                    case 1: y >>= 1; break;
                  }
                  value = 0xF7;
                  data->ikbd_x = x;
                  data->ikbd_y = y;
                  if (penDown) {
                    data->ikbd_button = right ? 2 : 1;
                  } else {
                    data->ikbd_button = 0;
                  }
                  data->ikbd_state = 2;
                  break;
                case 2: // buttons
                  if (data->ikbd_button) {
                    value = 0x04;
                  } else {
                    value = 0x08;
                  }
                  data->ikbd_state = 3;
                  break;
                case 3: // MSB X
                  value = data->ikbd_x >> 8;
                  data->ikbd_state = 4;
                  break;
                case 4: // LSB X
                  value = data->ikbd_x & 0xff;
                  data->ikbd_state = 5;
                  break;
                case 5: // MSB Y
                  value = data->ikbd_y >> 8;
                  data->ikbd_state = 6;
                  break;
                case 6: // LSB Y
                  value = data->ikbd_y & 0xff;
                  data->ikbd_state = 0;
                  data->ikbd_cmd = 0;
                  break;
                default:
                  value = 0;
                  data->ikbd_state = 0;
                  data->ikbd_cmd = 0;
                  break;
              }
              break;
            default:
              debug(DEBUG_ERROR, "TOS", "unknown ikbd command 0x%02X", data->ikbd_cmd);
              data->ikbd_cmd = 0;
              value = 0;
              break;
          }
        }
        break;
      case 0x00FFFC04: // MIDI ACIA Control
        value = 0;
        break;
      case 0x00FFFC06: // MIDI ACIA data
        value = 0;
        break;
      default:
        value = 0;
        break;
    }
    //debug(DEBUG_INFO, "TOS", "read 0x%02X from IO register 0x%08X", value, address);

  } else if (address < data->memorySize) {
    value = data->memory[address];
    if (address < sysvarsSize) {
      debug(DEBUG_INFO, "TOS", "read 0x%02X from sysvar 0x%08X", value, address);
    }

  } else {
    //debug(DEBUG_ERROR, "TOS", "read from unmapped address 0x%08X", address);
  }

  return value;
}

void tos_write_byte(tos_data_t *data, uint32_t address, uint8_t value) {
  if ((address & 0xFF000000) == 0xFF000000) {
    address &= 0x00FFFFFF;
  }

  if (address >= ioStart && address < ioEnd) {
    //debug(DEBUG_INFO, "TOS", "write 0x%02X to IO register 0x%08X", value, address);

    switch (address) {
      case 0x00FFFC02: // keyboard ACIA data
        if (data->ikbd_cmd != 0x80) {
          data->ikbd_cmd = value;
          data->ikbd_state = 1;
        } else {
          data->ikbd_cmd = 0;
        }
        break;
    }

  } else if (address < data->memorySize) {
    if (address < sysvarsSize) {
      debug(DEBUG_INFO, "TOS", "write 0x%02X to sysvar 0x%08X", value, address);
    }
    data->memory[address] = value;
  } else {
    //debug(DEBUG_ERROR, "TOS", "write 0x%02X to unmapped address 0x%08X", value, address);
  }
}

static uint8_t read_byte(uint32_t address) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  return tos_read_byte(data, address);
}

static uint16_t read_word(uint32_t address) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint16_t w = 0;

  if ((address & 1) == 0) {
    w = (tos_read_byte(data, address) << 8) | tos_read_byte(data, address + 1);
  } else {
    debug(DEBUG_ERROR, "TOS", "read_word unaligned address 0x%08X", address);
  }

  return w;
}

static uint32_t read_long(uint32_t address) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint32_t l = 0;

  if ((address & 1) == 0) {
    l = (tos_read_byte(data, address    ) << 24) |
        (tos_read_byte(data, address + 1) << 16) |
        (tos_read_byte(data, address + 2) <<  8) |
         tos_read_byte(data, address + 3);
  } else {
    debug(DEBUG_ERROR, "TOS", "read_long unaligned address 0x%08X", address);
  }

  return l;
}

static void write_screen_color(uint16_t *screen, uint32_t offset, uint16_t color, int le) {
  if (le) {
    put2l(color, (uint8_t *)screen, offset * 2);
  } else {
    put2b(color, (uint8_t *)screen, offset * 2);
  }
}

void tos_write_screen(uint16_t *screen, uint32_t offset, uint16_t value, uint8_t *m, uint16_t *palette, int res, int le, int dbl) {
  uint32_t i, k, x, y, plane, index, factor;
  uint16_t b, b2, b3, b4, color;

  put2b(value, m, offset);

  switch (res) {
    case 0: // 320x200 (4 planes)
      plane = (offset >> 1) & 0x03;
      switch (plane) {
        case 0:
          b = value;
          get2b(&b2, m, offset + 2);
          get2b(&b3, m, offset + 4);
          get2b(&b4, m, offset + 6);
          break;
        case 1:
          get2b(&b,  m, offset - 2);
          b2 = value;
          get2b(&b3, m, offset + 2);
          get2b(&b4, m, offset + 4);
          break;
        case 2:
          get2b(&b,  m, offset - 4);
          get2b(&b2, m, offset - 2);
          b3 = value;
          get2b(&b4, m, offset + 2);
          break;
        case 3:
          get2b(&b,  m, offset - 6);
          get2b(&b2, m, offset - 4);
          get2b(&b3, m, offset - 2);
          b4 = value;
          break;
      }
      offset >>= 3;
      y = offset / 20;        // 0 .. 199
      x = (offset % 20) * 16; // 0 .. 304
      factor = dbl ? 2 : 1;
      x *= factor;
      y *= factor;
      offset = y * 320 * factor + x;
      for (i = 0, k = 15*factor; i < 16; i++, k -= factor) {
        index = (b & 1) | ((b2 & 1) << 1) | ((b3 & 1) << 2) | ((b4 & 1) << 3);
        color = palette[index];
        write_screen_color(screen, offset + k, color, le);
        if (dbl) {
          write_screen_color(screen, offset + k + 1, color, le);
          write_screen_color(screen, offset + 640 + k, color, le);
          write_screen_color(screen, offset + 640 + k + 1, color, le);
        }
        b  >>= 1;
        b2 >>= 1;
        b3 >>= 1;
        b4 >>= 1;
      }
      break;
    case 1: // 640x200 (2 planes)
      plane = (offset >> 1) & 0x01;
      switch (plane) {
        case 0:
          b = value;
          get2b(&b2, m, offset + 2);
          break;
        case 1:
          get2b(&b, m, offset - 2);
          b2 = value;
          break;
      }
      offset >>= 2;
      y = offset / 40;        // 0 .. 199
      x = (offset % 40) * 16; // 0 .. 624
      factor = dbl ? 2 : 1;
      y *= factor;
      offset = y * 640 + x;
      for (i = 0, k = 30; i < 16; i++, k--) {
        index = (b & 1) | ((b2 & 1) << 1);
        color = palette[index];
        write_screen_color(screen, offset + k, color, le);
        if (dbl) write_screen_color(screen, offset + 640 + k, color, le);
        b  >>= 1;
        b2 >>= 1;
      }
      break;
    case 2: // 640x400 (1 plane)
      b = value;
      offset >>= 1;
      y = offset / 40;        // 0 .. 199
      x = (offset % 40) * 16; // 0 .. 624
      for (i = 0, k = 30; i < 16; i++, k--) {
        color = palette[b & 1];
        write_screen_color(screen, offset + k, color, le);
        b >>= 1;
      }
      break;
  }
}

static void write_byte(uint32_t address, uint8_t value) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  if (address >= data->physbase && address < data->physbase + screenSize) {
    uint16_t w;
    if ((address & 1) == 0) {
      w = value;
      w <<= 8;
      w |= data->memory[address + 1];
    } else {
      address--;
      w = data->memory[address];
      w <<= 8;
      w |= value;
    }
    tos_write_screen(data->screen, address - data->physbase, w, &data->memory[data->physbase], data->pumpkin_color, data->screen_res, 1, 1);
    data->screen_updated = 1;
  } else {
    tos_write_byte(data, address, value);
  }
}

static void write_word(uint32_t address, uint16_t value) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  if ((address & 1) == 0) {
    if (address >= data->physbase && address < data->physbase + screenSize) {
      tos_write_screen(data->screen, address - data->physbase, value, &data->memory[data->physbase], data->pumpkin_color, data->screen_res, 1, 1);
      data->screen_updated = 1;
    } else {
      tos_write_byte(data, address    , value >> 8);
      tos_write_byte(data, address + 1, value & 0xFF);

      if ((address & 0xFF000000) == 0xFF000000) {
        address &= 0x00FFFFFF;
      }
      if (address >= 0x00FF8240 && address <= 0x00FF825E) {
        uint16_t colornum = (address - 0x00FF8240) >> 1;
        uint16_t rgb = tos_convert_color(value);
        data->tos_color[colornum] = value;
        data->pumpkin_color[colornum] = rgb;
        data->screen_updated = 1;
        debug(DEBUG_TRACE, "TOS", "palette register %u color 0x%04X rgb565 0x%04X", colornum, value, rgb);
      }
    }
  } else {
    debug(DEBUG_ERROR, "TOS", "write_word unaligned address 0x%08X", address);
  }
}

static void write_long(uint32_t address, uint32_t value) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  if ((address & 1) == 0) {
    if (address >= data->physbase && address < data->physbase + screenSize) {
      tos_write_screen(data->screen, address - data->physbase    , value >> 16, &data->memory[data->physbase], data->pumpkin_color, data->screen_res, 1, 1);
      tos_write_screen(data->screen, address - data->physbase + 2, value & 0xffff, &data->memory[data->physbase], data->pumpkin_color, data->screen_res, 1, 1);
      data->screen_updated = 1;
    } else {
      tos_write_byte(data, address    ,  value >> 24);
      tos_write_byte(data, address + 1, (value >> 16) & 0xFF);
      tos_write_byte(data, address + 2, (value >>  8) & 0xFF);
      tos_write_byte(data, address + 3,  value        & 0xFF);
    }
  } else {
    debug(DEBUG_ERROR, "TOS", "write_word unaligned address 0x%08X", address);
  }
}

static int cpu_instr_callback(unsigned int pc) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint32_t instr_size, d[8], a[8];
  char buf[128], buf2[128];
  int i;

  if (data->debug_m68k) {
    instr_size = m68k_disassemble(buf, pc, M68K_CPU_TYPE_68000);
    m68k_make_hex(buf2, pc, instr_size);
    for (i = 0; i < 8; i++) {
      d[i] = m68k_get_reg(NULL, M68K_REG_D0 + i);
      a[i] = m68k_get_reg(NULL, M68K_REG_A0 + i);
    }
    debug(DEBUG_TRACE, "M68K", "A0=0x%08X,A1=0x%08X,A2=0x%08X,A3=0x%08X,A4=0x%08X,A5=0x%08X,A6=0x%08X,A7=0x%08X",
      a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
    debug(DEBUG_TRACE, "M68K", "%08X: %-20s: %s (%d,%d,%d,%d,%d,%d,%d,%d)",
      pc, buf2, buf, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
  }

  return 0;
}

static void screen_refresh(tos_data_t *data) {
  if (data->screen_updated) {
    uint64_t t = sys_get_clock();

    if ((t - data->last_refresh) >= 50000) {
      pumpkin_screen_copy(data->screen, 0, 400);
      data->screen_updated = 0;
      data->last_refresh = t;
    }
  }
}

static void tos_put_key(tos_data_t *data, int down, uint32_t scancode, uint32_t key) {
  if (!down) scancode |= 0x80;
  uint32_t code = key | (scancode << 16);
  data->KeyQueue[data->KeyQueueWriteIndex] = code;
  data->KeyQueueWriteIndex++;
  data->KeyQueueWriteIndex %= KEYQUEUE_SIZE;
  debug(1, "XXX", "put_key %s 0x%02X '%c' 0x%04X", down ? "down" : "up", scancode, key, code);
}

int tos_has_key(tos_data_t *data) {
  return data->KeyQueueReadIndex != data->KeyQueueWriteIndex;
}

int tos_get_key(tos_data_t *data, uint32_t *code) {
  int r = 0;

  if (data->KeyQueueReadIndex != data->KeyQueueWriteIndex) {
    *code = data->KeyQueue[data->KeyQueueReadIndex];
    data->KeyQueueReadIndex++;
    data->KeyQueueReadIndex %= KEYQUEUE_SIZE;
    debug(1, "XXX", "get_key 0x%04X'", *code);
    r = 1;
  }

  return r;
}

static void process_normal_keys(tos_data_t *data, uint32_t oldMask, uint32_t newMask) {
  uint32_t diff;

  diff = oldMask ^ newMask;
  if (diff) {
    if (diff & keyBitPageDown) {
      tos_put_key(data, newMask & keyBitPageDown ? 1 : 0, SCAN_DOWNARROW, 0);
    }
    if (diff & keyBitPageUp) {
      tos_put_key(data, newMask & keyBitPageUp ? 1 : 0, SCAN_UPARROW, 0);
    }
    if (diff & keyBitLeft) {
      tos_put_key(data, newMask & keyBitLeft ? 1 : 0, SCAN_LEFTARROW, 0);
    }
    if (diff & keyBitRight) {
      tos_put_key(data, newMask & keyBitRight ? 1 : 0, SCAN_RIGHTARROW, 0);
    }
  }
}

static void process_mod_keys(tos_data_t *data, uint32_t oldMask, uint32_t newMask) {
  uint32_t diff;

  diff = oldMask ^ newMask;
  if (diff) {
/*
    if (diff & WINDOW_MOD_CTRL) {
      data->ctrl = newMask & WINDOW_MOD_CTRL ? 1 : 0;
      tos_put_key(data, data->ctrl, SCAN_CONTROL, 0);
    }
    if (diff & WINDOW_MOD_SHIFT) {
      data->shift = newMask & WINDOW_MOD_SHIFT ? 1 : 0;
      tos_put_key(data, data->shift, SCAN_LEFTSHIFT, 0);
    }
*/
  }
}

static void process_ext_keys(tos_data_t *data, uint64_t oldMask, uint64_t newMask, uint16_t offset) {
  uint8_t key, scancode, shift, down, *e;
  uint64_t diff;
  uint16_t i;

  diff = oldMask ^ newMask;
  if (diff) {
    for (i = 0; i < 64; i++) {
      if (diff & 1) {
        key = offset + i;
        e = data->key2scan[key];
        if (e) {
          shift = e[0];
          scancode = e[1];
          down = newMask & 1;
          if (down) {
            if (shift) tos_put_key(data, 1, SCAN_LEFTSHIFT, 0);
            tos_put_key(data, 1, scancode, key);
          } else {
            tos_put_key(data, 0, scancode, key);
            if (shift) tos_put_key(data, 0, SCAN_LEFTSHIFT, 0);
          }
        }
      }
      newMask >>= 1;
      diff >>= 1;
    }
  }
}

static void process_keys(tos_data_t *data) {
  uint32_t keyMask, modMask;
  uint64_t extKeyMask[2];

  pumpkin_status(NULL, NULL, &keyMask, &modMask, NULL, extKeyMask);

  process_normal_keys(data, data->keyMask, keyMask);
  data->keyMask = keyMask;

  process_mod_keys(data, data->modMask, modMask);
  data->modMask = modMask;

  process_ext_keys(data, data->extKeyMask[0], extKeyMask[0], 0);
  data->extKeyMask[0] = extKeyMask[0];

  process_ext_keys(data, data->extKeyMask[1], extKeyMask[1], 64);
  data->extKeyMask[1] = extKeyMask[1];
}

static int tos_pterm_draw(uint8_t col, uint8_t row, uint8_t code, uint32_t fg, uint32_t bg, uint8_t attr, void *_data) {
  tos_data_t *data = (tos_data_t *)_data;
  uint32_t x, y;
    
  if (col < data->ncols && row < data->nrows) {
    x = col * data->fwidth;
    y = row * data->fheight;
    WinPaintChar(code, x, y);
  }

  return 0;
}

static int tos_pterm_erase(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, uint32_t bg, uint8_t attr, void *_data) {
  tos_data_t *data = (tos_data_t *)_data;
  RectangleType rect;

  if (col1 < data->ncols && col2 <= data->ncols && row1 < data->nrows && row2 <= data->nrows && col2 >= col1 && row2 >= row1) {
    RctSetRectangle(&rect, col1 * data->fwidth, row1 * data->fheight, (col2 - col1) * data->fwidth, (row2 - row1) * data->fheight);
    WinEraseRectangle(&rect, 0);
  }

  return 0;
}

static int tos_pterm_scroll(uint8_t row1, uint8_t row2, int16_t dir, void *_data) {
  tos_data_t *data = (tos_data_t *)_data;
  RectangleType rect, vacated;

  if (row1 < data->nrows && row2 <= data->nrows && row2 >= row1) {
    RctSetRectangle(&rect, 0, row1 * data->fheight, data->ncols * data->fwidth, (row2 - row1) * data->fheight);
    WinScrollRectangle(&rect, dir < 0 ? winUp : winDown, data->fheight, &vacated);
  }

  return 0;
}

static int tos_getchar(void *_data) {
  tos_data_t *data = (tos_data_t *)_data;
  uint32_t code;
  int c = 0;

  if (tos_get_key(data, &code)) {
    if ((code & 0x80) == 0x00) {
      c = code & 0x7F;
    }
  }

  return c;
}

static int tos_haschar(void *_data) {
  tos_data_t *data = (tos_data_t *)_data;
  return tos_has_key(data);
}

static void tos_putchar(void *_data, char c) {
  tos_data_t *data = (tos_data_t *)_data;
  uint8_t b = c;

  pterm_cursor(data->t, 0);
  pterm_send(data->t, &b, 1);
}

static void tos_setcolor(void *_data, uint32_t fg, uint32_t bg) {
}

static int tos_main_memory(UInt16 volRefNumA, UInt16 volRefNumB, uint8_t *tos, uint32_t tosSize, int argc, char *argv[]) {
  MemHandle hMemory, hIo;
  uint8_t *memory, *reloc;
  uint16_t jump, aflags;
  uint32_t offset, textSize, dataSize, bssSize, symSize, relocSize, reserved, pflags, memorySize;
  uint32_t pc, a7, basePageStart, stackStart, textStart, dataStart, bssStart, *relocBase, value, rem;
  m68ki_cpu_core main_cpu;
  emu_state_t *state, *oldState;
  tos_data_t data;
  int i, j, k, r = -1;

  if (tos && tosSize > headerSize) {
    offset = get2b(&jump, tos, 0);

    if (jump == 0x601a) {
      offset += get4b(&textSize, tos, offset);
      offset += get4b(&dataSize, tos, offset);
      offset += get4b(&bssSize, tos, offset);
      offset += get4b(&symSize, tos, offset);
      offset += get4b(&reserved, tos, offset);
      offset += get4b(&pflags, tos, offset);
      offset += get2b(&aflags, tos, offset);
      relocSize = tosSize - (headerSize + textSize + dataSize + symSize);

      debug(DEBUG_INFO, "TOS", "header:");
      debug_bytes(DEBUG_INFO, "TOS", tos, headerSize);
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
      state->extra = &data;

      if ((textSize & 1)) textSize++;
      if ((dataSize & 1)) dataSize++;
      if ((bssSize  & 1)) bssSize++;

      rem = (textSize + dataSize + bssSize) & 0x0F;
      if (rem != 0) {
        bssSize += 16 - rem;
      }

      memorySize = 0x100000; // 1M
      hMemory = MemHandleNew(memorySize + 16);
      memory = MemHandleLock(hMemory);
      MemSet(memory, memorySize, 0);

      rem = ((uint64_t)(memory)) & 0x0F;
      if (rem != 0) {
        memory += 16 - rem;
      }

      MemMove(memory + lowMemSize + basePageSize, &tos[offset], textSize);
      if (dataSize > 0) {
        MemMove(memory + lowMemSize + basePageSize + textSize, &tos[offset + textSize], dataSize);
      }

      basePageStart = lowMemSize;
      textStart     = lowMemSize + basePageSize;
      dataStart     = lowMemSize + basePageSize + textSize;
      bssStart      = lowMemSize + basePageSize + textSize + dataSize;
      stackStart    = lowMemSize + basePageSize + textSize + dataSize + bssSize;

      MemSet(&data, sizeof(tos_data_t), 0);
      data.debug_m68k = debug_getsyslevel("M68K") == DEBUG_TRACE;
      data.memory = memory;
      data.memorySize = memorySize;
      data.basePageStart = basePageStart;
      data.kbdvbase  = sysvarsSize;
      data.lineaVars = sysvarsSize + kbdvSize;
      data.physbase  = sysvarsSize + kbdvSize + lineaSize;
      data.logbase   = data.physbase;
      data.screen = sys_malloc(640 * 400 * 2);
      data.screen_res = 0;
      data.font = tos8x8Font;
      FntSetFont(data.font);

      for (i = 0; scan_codes[i+1]; i += 3) {
        data.key2scan[scan_codes[i+2]] = (uint8_t *)&scan_codes[i];
      }

      for (i = 0, j = 0; i < 16; i++, j += 3) {
        data.tos_color[i] = (palette[j] << 8) | (palette[j+1] << 4) | palette[j+2];
        //data.tos_color[i] = (palette[j] << 9) | (palette[j+1] << 5) | (palette[j+2] << 1) | 0x0111;
        data.pumpkin_color[i] = rgb565(
          palette[j  ] ? ((palette[j  ] << 5) | 0x1F) : 0,
          palette[j+1] ? ((palette[j+1] << 5) | 0x1F) : 0,
          palette[j+2] ? ((palette[j+2] << 5) | 0x1F) : 0);
        debug(DEBUG_TRACE, "TOS", "init palette %d r%u g%u b%u rgb565 0x%04X", i, palette[j], palette[j+1], palette[j+2], data.pumpkin_color[i]);
      }

      data.ioSize = ioEnd - ioStart;
      hIo = MemHandleNew(data.ioSize);
      data.io = MemHandleLock(hIo);
      MemSet(data.io, data.ioSize, 0);

      write_long(0x0000042E, data.memorySize); // phystop: physical top of ST compatible RAM

      write_long(0x000005A0, 0x00000700); // _p_cookies: pointer to the system Cookie Jar
      write_long(0x00000700, '_VDO');
      //write_long(0x00000704, 0x10000); // Deluxe Paint checks if this value is 0x10000 (STe)
      write_long(0x00000704, 0);
      write_long(0x00000708, 0);
      write_long(0x0000070C, 0);

      write_long(basePageStart + 0x0000, basePageStart);
      write_long(basePageStart + 0x0004, memorySize);
      write_long(basePageStart + 0x0008, textStart);
      write_long(basePageStart + 0x000C, textSize);
      write_long(basePageStart + 0x0010, dataStart);
      write_long(basePageStart + 0x0014, dataSize);
      write_long(basePageStart + 0x0018, bssStart);
      write_long(basePageStart + 0x001C, bssSize);
      write_long(basePageStart + 0x0020, basePageStart + 0x0080);
      write_long(basePageStart + 0x0024, basePageStart);
      write_long(basePageStart + 0x0028, 0);
      write_long(basePageStart + 0x002C, basePageStart + 0x0028); // pointer to env string -> reserved

      for (i = 1, k = 0; i < argc && k < 128; i++) {
        if (i > 1) {
          write_byte(basePageStart + 0x0081 + k, ' ');
          k++;
        }
        for (j = 0; argv[i][j] && k < 128; j++, k++) {
          write_byte(basePageStart + 0x0081 + k, argv[i][j]);
        }
      }
      write_byte(basePageStart + 0x0080, k);

      debug(DEBUG_INFO, "TOS", "sysvars : 0x%08X (%u bytes)", 0, sysvarsSize);
      debug(DEBUG_INFO, "TOS", "kbdvbase: 0x%08X (%u bytes)", data.kbdvbase, kbdvSize);
      debug(DEBUG_INFO, "TOS", "line-a  : 0x%08X (%u bytes)", data.lineaVars, lineaSize);
      debug(DEBUG_INFO, "TOS", "screen  : 0x%08X (%u bytes)", data.physbase, screenSize);

      debug(DEBUG_INFO, "TOS", "basePage: 0x%08X", basePageStart);
      debug_bytes_offset(DEBUG_INFO, "TOS", memory + basePageStart, basePageSize, basePageStart);

      debug(DEBUG_INFO, "TOS", "text: 0x%08X (%u bytes)", textStart, textSize);
      debug_bytes_offset(DEBUG_INFO, "TOS", memory + textStart, textSize, textStart);

      if (dataSize > 0) {
        debug(DEBUG_INFO, "TOS", "data: 0x%08X (%u bytes)", dataStart, dataSize);
        debug_bytes_offset(DEBUG_INFO, "TOS", memory + dataStart, dataSize, dataStart);
      }

      if (bssSize > 0) {
        debug(DEBUG_INFO, "TOS", "bss: 0x%08X (%u bytes)", bssStart, bssSize);
      }

      debug(DEBUG_INFO, "TOS", "stack: 0x%08X (%u bytes)", stackStart, stackSize);

      if (relocSize > 0) {
        relocBase = (uint32_t *)(tos + headerSize + textSize + dataSize + symSize);
        get4b(&offset, (uint8_t *)relocBase, 0);
        if (offset) {
          debug(DEBUG_INFO, "TOS", "reloc:");
          debug_bytes(DEBUG_INFO, "TOS", (uint8_t *)relocBase, relocSize);

          value = read_long(textStart + offset);
          debug(DEBUG_INFO, "TOS", "reloc offset 0x00 0x%08X 0x%08X -> 0x%08X", offset, value, value + textStart);
          value += textStart;
          write_long(textStart + offset, value);

          for (reloc = ((uint8_t *)relocBase) + 4; *reloc; reloc++) {
            if (*reloc == 1) {
              offset += 254;
              continue;
            }
            offset += *reloc;

            value = read_long(textStart + offset);
            debug(DEBUG_INFO, "TOS", "reloc offset 0x%02X 0x%08X 0x%08X -> 0x%08X", *reloc, offset, value, value + textStart);
            value += textStart;
            write_long(textStart + offset, value);
          }
        }
      }

      data.a = volRefNumA;
      data.b = volRefNumB;
      data.c = VFSAddVolume("/app_card/TOS/C/");
      Dsetdrv(0);

      data.t = pterm_init(80, 25, 1);
      data.cb.draw = tos_pterm_draw;
      data.cb.erase = tos_pterm_erase;
      data.cb.scroll = tos_pterm_scroll;
      data.cb.data = &data;
      pterm_callback(data.t, &data.cb);

      pumpkin_setio(tos_getchar, tos_haschar, tos_putchar, tos_setcolor, &data);

      pc = textStart;
      a7 = stackStart + stackSize - 16;
      state->stackStart = stackStart;

      write_long(a7 + 4, basePageStart);
      write_long(a7, 0); // return address

      MemSet(&main_cpu, sizeof(m68ki_cpu_core), 0);
      main_cpu.tos = 1;
      m68k_set_context(&main_cpu);
      m68k_init();
      m68k_set_cpu_type(M68K_CPU_TYPE_68000);
      m68k_pulse_reset();
      m68k_set_reg(M68K_REG_PC, pc);
      m68k_set_reg(M68K_REG_SP, a7);
      m68k_set_instr_hook_callback(cpu_instr_callback);

      emupalmos_memory_hooks(
        read_byte, read_word, read_long,
        write_byte, write_word, write_long
      );

      if (data.debug_m68k) {
        debug(DEBUG_INFO, "TOS", "text disassembly begin");
        m68k_disassemble_range(textStart, dataStart, M68K_CPU_TYPE_68000);
        debug(DEBUG_INFO, "TOS", "text disassembly end");
      }

      for (; !emupalmos_finished() && !thread_must_end();) {
        if (m68k_get_reg(NULL, M68K_REG_PC) == 0) break;
        m68k_execute(&state->m68k_state, 100000);
        screen_refresh(&data);
        process_keys(&data);
      }
      r = 0;

      emupalmos_deinstall(oldState);
      MemHandleFree(hMemory);
      MemHandleFree(hIo);
      sys_free(data.screen);
    } else {
      debug(DEBUG_ERROR, "TOS", "0x601a signature not found");
      debug_bytes(DEBUG_INFO, "TOS", tos, headerSize);
    }
  } else {
    debug(DEBUG_ERROR, "TOS", "invalid TOS pointer (%p) or size (%d)", tos, tosSize);
  }

  return r;
}

static UInt16 tos_map_drive(char drive, char *dir, Boolean *created) {
  char vfsd[64];

  StrNPrintF(vfsd, sizeof(vfsd) - 1, "/app_tos/%c/%s/", drive, dir);
  *created = !VFSVolumeExists(vfsd);

  return VFSAddVolume(vfsd);
}

static int tos_main(UInt16 volRefNumA, UInt16 volRefNumB, char *program, int argc, char *argv[]) {
  UInt32 tosSize, numBytesRead;
  FileRef fileRef;
  uint8_t *tos;
  int r = -1;

  if (program && program[0]) {
    if (VFSFileOpen(volRefNumA, program, vfsModeRead, &fileRef) == errNone) {
      if (VFSFileSize(fileRef, &tosSize) == errNone) {
        if ((tos = MemPtrNew(tosSize)) != NULL) {
          if (VFSFileRead(fileRef, tosSize, tos, &numBytesRead) == errNone && numBytesRead == tosSize) {
            r = tos_main_memory(volRefNumA, volRefNumB, tos, tosSize, argc, argv);
          } else {
            debug(DEBUG_ERROR, "TOS", "could not read file \"%s\"", program);
          }
          MemPtrFree(tos);
        }
      } else {
        debug(DEBUG_ERROR, "TOS", "could not obtain file size for \"%s\"", program);
      }
      VFSFileClose(fileRef);
    } else {
      debug(DEBUG_ERROR, "TOS", "could not open \"%s\"", program);
    }
  }

  return r;
}

static int tos_main_zip(char *dirA, char *dirB, char *program, int argc, char *argv[]) {
  UInt16 volRefNumA, volRefNumB;
  Boolean createdA, createdB;
  int r = -1;

  if (dirA && dirA[0] && program && program[0]) {
    volRefNumA = tos_map_drive('A', dirA, &createdA);

    if (volRefNumA != 0xffff) {
      if (createdA) {
        debug(DEBUG_INFO, "TOS", "extracting contents for drive A");
        r = pumpkin_unzip_resource(zipRsc, 1, volRefNumA, "");
      } else {
        r = 0;
      }

      if (r == 0) {
        volRefNumB = (dirB && dirB[0]) ? tos_map_drive('B', dirB, &createdB) : 0xffff;
        if (volRefNumB != 0xffff && createdB) {
          debug(DEBUG_INFO, "TOS", "extracting contents for drive B");
          pumpkin_unzip_resource(zipRsc, 2, volRefNumB, "");
        }
        r = tos_main(volRefNumA, volRefNumB, program, argc, argv);
      }
    }
  }

  return r;
}

UInt32 TOSMain(void) {
  MemHandle hDir[2], hPrg;
  char *sDir[2], *sPrg, *argv[1];
  UInt32 drive, r = -1;

  for (drive = 0; drive < 2; drive++) {
    hDir[drive] = DmGet1Resource('tDir', drive + 1);
    sDir[drive] = hDir[drive] ? MemHandleLock(hDir[drive]) : NULL;
  }

  hPrg = DmGet1Resource('tPrg', 1);
  sPrg = hPrg ? MemHandleLock(hPrg) : NULL;

  if (sDir[0] && sPrg) {
    plibc_init();
    plibc_dup(2);
    argv[0] = sPrg;
    r = tos_main_zip(sDir[0], sDir[1], sPrg, 1, argv);
    plibc_close(3);
    plibc_finish();
  }

  for (drive = 0; drive < 2; drive++) {
    if (sDir[drive]) MemHandleUnlock(hDir[drive]);
    if (hDir[drive]) DmReleaseResource(hDir[drive]);
  }

  if (sPrg) MemHandleUnlock(hPrg);
  if (hPrg) DmReleaseResource(hPrg);

  return r;
}

static uint32_t tos_gemdos_systrap(void) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint8_t *memory = data->memory;
  uint32_t sp, opcode, r = 0;
  uint16_t idx;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;
  opcode = ARG16;

  switch (opcode) {
#include "gemdos_case.c"
    case 72: { // void *Malloc(int32_t number)
        int32_t number = ARG32;
        uint32_t addr = 0;
        if (number == -1) {
          // get size of the largest available memory block
          addr = 256*1024; // XXX
        } else if (data->heap && number > 0) {
          uint8_t *p = heap_alloc(data->heap, number);
          addr = p ? (p - memory) : 0;
        }
        m68k_set_reg(M68K_REG_D0, addr);
        debug(DEBUG_INFO, "TOS", "GEMDOS Malloc(%d): 0x%08X", number, addr);
      }
      break;
    case 260: { // int32_t Fcntl(int16_t fh, int32_t arg, int16_t cmd)
        int16_t fh = ARG16;
        uint32_t arg = ARG32;
        int32_t cmd = ARG16;
        int32_t res = -1;
        switch (cmd) {
          case 0x5406: // TIOCGPGRP: returns via the parameter arg a pointer to the process group ID of the terminal
            write_long(arg, 1);
            res = 0;
            break;
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_INFO, "TOS", "GEMDOS Fcntl(%d, %d, 0x%04X): %d", fh, arg, cmd, res);
      }
      break;
    default:
      debug(DEBUG_ERROR, "TOS", "unmapped GEMDOS opcode %d", opcode);
      emupalmos_finish(1);
      break;
  }

  return r;
}

static uint32_t tos_bios_systrap(void) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint8_t *memory = data->memory;
  uint32_t sp, opcode, r = 0;
  uint16_t idx;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;
  opcode = ARG16;

  switch (opcode) {
#include "bios_case.c"
    default:
      debug(DEBUG_ERROR, "TOS", "unmapped BIOS opcode %d", opcode);
      emupalmos_finish(1);
      break;
  }

  return r;
}

uint16_t tos_convert_color(uint16_t color) {
  uint16_t red, green, blue;

  red   = (color & 0x0700) >> 8;
  green = (color & 0x0070) >> 4;
  blue  =  color & 0x0007;

  return rgb565(red ? ((red << 5) | 0x1F) : 0, green ? ((green << 5) | 0x1F) : 0, blue ? ((blue << 5) | 0x1F) : 0);
}

uint16_t xtos_convert_color(uint16_t color) {
  uint16_t red, green, blue;

  red   = ((color & 0x0700) >> 7) | ((color & 0x0800) >> 11);
  green = ((color & 0x0070) >> 3) | ((color & 0x0080) >> 7);
  blue  = ((color & 0x0007) << 1) | ((color & 0x0008) >> 3);

  return rgb565(red ? ((red << 4) | 0x0F) : 0, green ? ((green << 4) | 0x0F) : 0, blue ? ((blue << 4) | 0x0F) : 0);
}

static uint32_t tos_xbios_systrap(void) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint8_t *memory = data->memory;
  uint32_t sp, opcode, r = 0;
  uint16_t idx;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;
  opcode = ARG16;

  switch (opcode) {
#include "xbios_case.c"
    default:
      debug(DEBUG_ERROR, "TOS", "unmapped XBIOS opcode %d", opcode);
      emupalmos_finish(1);
      break;
  }

  return r;
}

static uint32_t tos_gem_systrap(void) {
  uint32_t d0, addr, r = 0;
  int16_t i, selector;
  vdi_pb_t vdi_pb;
  aes_pb_t aes_pb;

  d0 = m68k_get_reg(NULL, M68K_REG_D0);
  selector = d0 & 0xFFFF;

  if (selector == -2) {
    // app is checking if GDOS is installed
    // leaving d0 as is means GDOS is not installed
    debug(DEBUG_INFO, "TOS", "GDOS check");

  } else if (selector == 115) {
    // app is calling a VDI opcode
    addr = m68k_get_reg(NULL, M68K_REG_D1);
    vdi_pb.pcontrol = read_long(addr + 0);
    vdi_pb.pintin   = read_long(addr + 4);
    vdi_pb.pptsin   = read_long(addr + 8);
    vdi_pb.pintout  = read_long(addr + 12);
    vdi_pb.pptsout  = read_long(addr + 16);

    vdi_pb.opcode      = read_word(vdi_pb.pcontrol + 0);
    vdi_pb.ptsin_len   = read_word(vdi_pb.pcontrol + 2);
    vdi_pb.intin_len   = read_word(vdi_pb.pcontrol + 6);
    vdi_pb.sub_opcode  = read_word(vdi_pb.pcontrol + 10);
    vdi_pb.workstation = read_word(vdi_pb.pcontrol + 12);

    debug(DEBUG_INFO, "TOS", "VDI opcode %d.%d", vdi_pb.opcode, vdi_pb.sub_opcode);
    vdi_call(&vdi_pb);

    write_word(vdi_pb.pcontrol + 4, vdi_pb.ptsout_len);
    write_word(vdi_pb.pcontrol + 8, vdi_pb.intout_len);

  } else if (selector == 200) {
    // app is calling an AES opcode
    addr = m68k_get_reg(NULL, M68K_REG_D1);
    aes_pb.pcontrol = read_long(addr + 0);
    aes_pb.pglobal  = read_long(addr + 4);
    aes_pb.pintin   = read_long(addr + 8);
    aes_pb.pintout  = read_long(addr + 12);
    aes_pb.padrin   = read_long(addr + 16);
    aes_pb.padrout  = read_long(addr + 20);

    aes_pb.opcode     = read_word(aes_pb.pcontrol + 0);
    aes_pb.intin_len  = read_word(aes_pb.pcontrol + 2);
    aes_pb.adrin_len  = read_word(aes_pb.pcontrol + 6);

    for (i = 0; i < AES_GLOBAL_LEN; i++) {
      aes_pb.global[i] = read_word(aes_pb.pglobal + i*2);
    }

    for (i = 0; i < aes_pb.intin_len && i < AES_INTIN_LEN; i++) {
      aes_pb.intin[i] = read_word(aes_pb.pintin + i*2);
    }

    for (i = 0; i < aes_pb.adrin_len && i < AES_ADRIN_LEN; i++) {
      aes_pb.adrin[i] = read_long(aes_pb.padrin + i*4);
    }

    if (aes_call(&aes_pb) == -1) {
      emupalmos_finish(1);
    }

    write_word(aes_pb.pcontrol + 4, aes_pb.intout_len);
    write_word(aes_pb.pcontrol + 8, aes_pb.adrout_len);

    for (i = 0; i < aes_pb.adrout_len && i < AES_INTOUT_LEN; i++) {
      write_long(aes_pb.padrout + i*4, aes_pb.adrout[i]);
    }

    for (i = 0; i < aes_pb.intout_len && i < AES_ADROUT_LEN; i++) {
      write_word(aes_pb.pintout + i*2, aes_pb.intout[i]);
    }

    for (i = 0; i < AES_GLOBAL_LEN; i++) {
      write_word(aes_pb.pglobal + i*2, aes_pb.global[i]);
    }

  } else {
    debug(DEBUG_ERROR, "TOS", "invalid GEM selector %d", selector);
  }

  return r;
}

static uint32_t tos_linea(uint16_t opcode) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint32_t r = 0;

  debug(DEBUG_TRACE, "TOS", "LINEA opcode %d", opcode);

  switch (opcode) {
    case  0: // Initialization
      // output:
      // d0 = pointer to the base address of Line A interface variables
      // a0 = pointer to the base address of Line A interface variables
      // a1 = pointer to the array of pointers to the three system font headers
      // a2 = pointer to array of pointers to the fifteen Line A routines
      m68k_set_reg(M68K_REG_D0, data->lineaVars);
      m68k_set_reg(M68K_REG_A0, data->lineaVars);
      break;
    case  1: // Put pixel
      // input:
      // INTIN[0] = pixel value
      // PTSIN[0] = x coordinate
      // PTSIN[1] = y coordinate
      break;
    case  2: // Get pixel
      // input:
      // PTSIN[0] = x coordinate
      // PTSIN[1] = y coordinate
      // output: d0 = pixel value
      m68k_set_reg(M68K_REG_D0, 0);
      break;
    case  3:
      break;
    case  4:
      break;
    case  5:
      break;
    case  6:
      break;
    case  7:
      break;
    case  8:
      break;
    case  9: // Show mouse
      break;
    case 10: // Hide mouse
      break;
    case 11:
      break;
    case 12:
      break;
    case 13:
      break;
    case 14:
      break;
    case 15:
      break;
  }

  return r;
}

uint32_t tos_systrap(uint16_t type) {
  uint32_t r = 0;

  switch (type) {
    case 1:
      r = tos_gemdos_systrap();
      break;
    case 2:
      r = tos_gem_systrap();
      break;
    case 13:
      r = tos_bios_systrap();
      break;
    case 14:
      r = tos_xbios_systrap();
      break;
    default:
      if (type >= 0xA000 && type <= 0xA00F) {
        r = tos_linea(type & 0x0FFF);
      } else {
        debug(DEBUG_ERROR, "TOS", "invalid syscall trap #%d", type);
        emupalmos_finish(1);
      }
      break;
  }

  return r;
}
