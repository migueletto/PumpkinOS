#include "sys.h"
#include "vfs.h"
#include "ptr.h"
#include "filter.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "rom.h"
#include "computer.h"
#include "trs80.h"
#include "z80.h"
#include "disk.h"
#include "wd1793.h"
#include "debug.h"
#include "xalloc.h"

#define Z80_CLOCK     2027520
#define Z80_FPS       50
#define Z80_PERIOD    Z80_CLOCK/Z80_FPS

#define ROM_SIZE      0x4000
#define RAM_SIZE      0xC000
#define VRAM_SIZE     0x0400
#define CHR_SIZE      0x0400

#define IRQ_M4_RTC    0x04

#define NMI_INTRQ_BIT    0x80  // FDC chip INTRQ line
#define NMI_MOTOROFF_BIT 0x40  // FDC motor timed out (stopped)
#define NMI_RESET_BIT    0x20  // User pressed Reset button

#define SHIFT_LINE    0x80
#define SHIFT_COLUMN  0x01

#define PROGRAM_BAS   "/z80/program.bas"

#define SCREEN_WIDTH  (64*8)
#define SCREEN_HEIGHT (16*12)

#define MAX_PROG      65536

typedef struct {
  uint8_t key, line, column, mods;
} keymap_t;

typedef struct {
  z80_t *z;
  uint8_t rom[ROM_SIZE];
  uint8_t ram[RAM_SIZE];
  uint8_t vram[VRAM_SIZE];
  uint8_t chr[CHR_SIZE];
  disk_t *d;
  wd1793_t *wd;

  vfs_session_t *session;
  char program[MAX_PROG];
  uint32_t progindex, progsize;

  uint8_t irq_mask;
  uint8_t irq_latch;
  uint8_t nmi_mask;
  uint8_t nmi_latch;
  uint8_t do_nmi;
  uint8_t trs80RegLoad;
  uint8_t trs80PortEC;
  keymap_t keymap[256];
  uint8_t key;
  int finish;

  int lastx, lasty;
  uint32_t color[2];
  int ptr;
  surface_t *surface;
  uint32_t c0, c1;
  int dirty;
} trs80_data_t;

static keymap_t kmap[] = {
    { '@', 0x01, 0x01, 0 },
    { 'a', 0x01, 0x02, 0 },
    { 'b', 0x01, 0x04, 0 },
    { 'c', 0x01, 0x08, 0 },
    { 'd', 0x01, 0x10, 0 },
    { 'e', 0x01, 0x20, 0 },
    { 'f', 0x01, 0x40, 0 },
    { 'g', 0x01, 0x80, 0 },
    
    { 'h', 0x02, 0x01, 0 },
    { 'i', 0x02, 0x02, 0 },
    { 'j', 0x02, 0x04, 0 },
    { 'k', 0x02, 0x08, 0 },
    { 'l', 0x02, 0x10, 0 },
    { 'm', 0x02, 0x20, 0 },
    { 'n', 0x02, 0x40, 0 },
    { 'o', 0x02, 0x80, 0 },
    
    { 'p', 0x04, 0x01, 0 },
    { 'q', 0x04, 0x02, 0 },
    { 'r', 0x04, 0x04, 0 },
    { 's', 0x04, 0x08, 0 },
    { 't', 0x04, 0x10, 0 },
    { 'u', 0x04, 0x20, 0 },
    { 'v', 0x04, 0x40, 0 },
    { 'w', 0x04, 0x80, 0 },
    
    { 'x', 0x08, 0x01, 0 },
    { 'y', 0x08, 0x02, 0 },
    { 'z', 0x08, 0x04, 0 },
    
    { '0', 0x10, 0x01, 0 },
    { '1', 0x10, 0x02, 0 },
    { '2', 0x10, 0x04, 0 },
    { '3', 0x10, 0x08, 0 },
    { '4', 0x10, 0x10, 0 },
    { '5', 0x10, 0x20, 0 },
    { '6', 0x10, 0x40, 0 },
    { '7', 0x10, 0x80, 0 },
    
    { '!', 0x10, 0x02, WINDOW_MOD_SHIFT },
    { '"', 0x10, 0x04, WINDOW_MOD_SHIFT },
    { '#', 0x10, 0x08, WINDOW_MOD_SHIFT },
    { '$', 0x10, 0x10, WINDOW_MOD_SHIFT },
    { '%', 0x10, 0x20, WINDOW_MOD_SHIFT },
    { '&', 0x10, 0x40, WINDOW_MOD_SHIFT },
    { '\'', 0x10, 0x80, WINDOW_MOD_SHIFT },
    
    { '8', 0x20, 0x01, 0 },
    { '9', 0x20, 0x02, 0 },
    { ':', 0x20, 0x04, 0 },
    { ';', 0x20, 0x08, 0 },
    { ',', 0x20, 0x10, 0 },
    { '-', 0x20, 0x20, 0 },
    { '.', 0x20, 0x40, 0 },
    { '/', 0x20, 0x80, 0 },
    
    { '(', 0x20, 0x01, WINDOW_MOD_SHIFT },
    { ')', 0x20, 0x02, WINDOW_MOD_SHIFT },
    { '*', 0x20, 0x04, WINDOW_MOD_SHIFT },
    { '+', 0x20, 0x08, WINDOW_MOD_SHIFT },
    { '<', 0x20, 0x10, WINDOW_MOD_SHIFT },
    { '=', 0x20, 0x20, WINDOW_MOD_SHIFT },
    { '>', 0x20, 0x40, WINDOW_MOD_SHIFT },
    { '?', 0x20, 0x80, WINDOW_MOD_SHIFT },
    
    { 13,  0x40, 0x01, 0 },       // return
    { 12,  0x40, 0x02, 0 },       // clear
    { 3,   0x40, 0x04, 0 },       // break
    { WINDOW_KEY_UP, 0x40, 0x08, 0 },    // up
    { WINDOW_KEY_DOWN, 0x40, 0x10, 0 },  // down
    { 8,   0x40, 0x20, 0 },       // left
    { WINDOW_KEY_LEFT, 0x40, 0x20, 0 },  // left
    { WINDOW_KEY_RIGHT, 0x40, 0x40, 0 }, // right
    { ' ', 0x40, 0x80, 0 },

    { 0, 0, 0, 0 }
};

static surface_t *lock_surface(trs80_data_t *trs80) {
  if (trs80->surface) return trs80->surface;
  if (trs80->ptr > 0) return ptr_lock(trs80->ptr, TAG_SURFACE);
  return NULL;
}

static void unlock_surface(trs80_data_t *trs80) {
  if (trs80->ptr > 0) ptr_unlock(trs80->ptr, TAG_SURFACE);
}

static void init_kmap(keymap_t *in, keymap_t *out) {
  int i;

  for (i = 0; in[i].key; i++) {
    out[in[i].key] = in[i];
  }

  for (i = 0; i < 256; i++) {
    if (out[i].key == 0) {
      out[i].line = 0xFF;
      out[i].column = 0xFF;
      out[i].mods = 0;
    }
  }
}

static uint8_t read_keyboard(trs80_data_t *trs80, uint8_t line) {
  keymap_t *map;
  uint8_t b = 0;

  if (trs80->key) {
    map = &trs80->keymap[trs80->key];

    if (map->line != 0xFF) {
      if ((line & map->line)) {
        b |= map->column;
      }
      if ((line & SHIFT_LINE) && (map->mods & WINDOW_MOD_SHIFT)) {
        b |= SHIFT_COLUMN;
      }
    }
  }

  return b;
}

static uint8_t trs80_getb(computer_t *c, uint16_t addr) {
  trs80_data_t *trs80;
  uint8_t b;

  trs80 = (trs80_data_t *)c->data;

  // 0000 - 2FFF : ROM
  // 3000 - 37FF : ROM on Model III
  // 3800 - 38FF : keyboard matrix
  // 3900 - 3BFF : unused
  // 3C00 - 3FFF : video RAM
  // 4000 - FFFF : RAM
  
  if (addr < 0x3C00) {
    if (addr >= 0x3800 && addr < 0x3900) {
      b = read_keyboard(trs80, addr & 0xFF);
    } else {
      switch (addr) {
        case 0x37EC: //b = wd1793.read(WD1793.WD1793_STATUS); break;
        case 0x37ED: //b = wd1793.read(WD1793.WD1793_TRACK);  break;
        case 0x37EE: //b = wd1793.read(WD1793.WD1793_SECTOR); break;
        case 0x37EF: //b = wd1793.read(WD1793.WD1793_DATA);   break;
          //System.out.printf("read 0x%04X\n", addr);
        default:
          b = trs80->rom[addr];
          break;
      }
    }
  } else if (addr < 0x4000) {
    b = trs80->vram[addr - 0x3C00];
  } else {
    b = trs80->ram[addr - 0x4000];
  }

  return b;
}

static void render(trs80_data_t *trs80, surface_t *surface) {
  int addr, x0, y0, x, y;
  int caddr, ext, i, j;
  uint8_t code, mask;
  uint32_t c;

  x0 = (surface->width - SCREEN_WIDTH) / 2;
  y0 = (surface->height - SCREEN_HEIGHT*2) / 2;

  for (addr = 0; addr < 1024; addr++) {
    x = x0 + ((addr & 0x3F) << 3);
    y = y0 + (addr >> 6) * 24;
        
    code = trs80->vram[addr];
    caddr = (code & 0x7F) << 3;
    ext = (code & 0x80) != 0;

    for (i = 0; i < 12; i++, y += 2) {
      if (ext) {
        mask = 1 << ((i & 0x0C) >> 1);
        c = (code & mask) ? 1 : 0;
        c = trs80->color[c];
        for (j = 0; j < 4; j++) {
          surface->setpixel(surface->data, x + j, y, c);
          surface->setpixel(surface->data, x + j, y + 1, c);
        }
        mask <<= 1;
        c = (code & mask) ? 1 : 0;
        c = trs80->color[c];
        for (; j < 8; j++) {
          surface->setpixel(surface->data, x + j, y, c);
          surface->setpixel(surface->data, x + j, y + 1, c);
        }
            
      } else {
        mask = (i < 8) ? trs80->chr[caddr++] : 0;
        
        for (j = 0; j < 8; j++) {
          c = (mask & 0x80) ? 1 : 0;
          c = trs80->color[c];
          surface->setpixel(surface->data, x + j, y, c);
          surface->setpixel(surface->data, x + j, y + 1, c);
          mask <<= 1;
        }
      }
    }
  }

  surface_update(surface);
}

static void set_vram(trs80_data_t *trs80, uint16_t vaddr, uint8_t b) {
  debug(DEBUG_TRACE, "TRS80", "set vram 0x%02X at vaddr %d", b, vaddr);

  if (trs80->vram[vaddr] != b) {
    trs80->vram[vaddr] = b;
    trs80->dirty = 1;
  }
}

static void trs80_putb(computer_t *c, uint16_t addr, uint8_t b) {
  trs80_data_t *trs80;

  trs80 = (trs80_data_t *)c->data;

  if (addr >= 0x4000) {
    trs80->ram[addr - 0x4000] = b;

  } else if (addr >= 0x3C00) {
    set_vram(trs80, addr - 0x3C00, b);
    
  } else {
    if (addr >= 0x37E0 && addr < 0x37F0) {
      //System.out.printf("write 0x%04X 0x%02X\n", addr, b);
    }
  }
}

static void trs80_out(computer_t *c, uint16_t port, uint16_t b) {
  trs80_data_t *trs80;
  int drive, head;

  trs80 = (trs80_data_t *)c->data;
  debug(DEBUG_TRACE, "TRS80", "out 0x%04X, 0x%02X", port, b);

  switch (port & 0xFF) {
      case 0xE0:
      case 0xE1:
      case 0xE2:
      case 0xE3:
        // Interrupt settings - which devices are allowed to interrupt - bits align with read of E0
        // d6 Enable Rec Err
        // d5 Enable Rec Data
        // d4 Enable Xmit Emp
        // d3 Enable I/O int
        // d2 Enable RT int
        // d1 C fall Int
        // d0 C Rise Int */

        debug(DEBUG_INFO, "TRS80", "irq_mask = 0x%02X", b);
        trs80->irq_mask = b;
        break;
      case 0xE4:
      case 0xE5:
      case 0xE6:
      case 0xE7:
        // Disk to NMI interface
        // d7 1 = enable disk INTRQ to generate NMI
        // d6 1 = enable disk Motor Timeout to generate NMI

        debug(DEBUG_INFO, "TRS80", "nmi_mask = 0x%02X (motor=%d, fdc=%d)", b, (b & NMI_MOTOROFF_BIT) ? 1 : 0, (b & NMI_INTRQ_BIT) ? 1 : 0);
        trs80->nmi_mask = b;
        if ((trs80->nmi_latch & trs80->nmi_mask) != 0) {
          debug(DEBUG_INFO, "TRS80", "nmi_mask change caused nmi");
          trs80->do_nmi = 1;
        }
        break;
      case 0xE8:
        // d1 when '1' enables control register load (see below)
        trs80->trs80RegLoad = b & 0x02;
        break;
      case 0xE9:
        // UART set baud rate. Rx = bits 0..3, Tx = bits 4..7
        // 00h    50
        // 11h    75
        // 22h    110
        // 33h    134.5
        // 44h    150
        // 55h    300
        // 66h    600
        // 77h    1200
        // 88h    1800
        // 99h    2000
        // AAh    2400
        // BBh    3600
        // CCh    4800
        // DDh    7200
        // EEh    9600
        // FFh    19200
        break;
      case 0xEA:
        if (trs80->trs80RegLoad != 0) {
          // d2..d0 not emulated
          // d7 Even Parity Enable ('1'=even, '0'=odd)
          // d6='1',d5='1' for 8 bits
          // d6='0',d5='1' for 7 bits
          // d6='1',d5='0' for 6 bits
          // d6='0',d5='0' for 5 bits
          // d4 Stop Bit Select ('1'=two stop bits, '0'=one stop bit)
          // d3 Parity Inhibit ('1'=disable; No parity, '0'=parity enabled)
          // d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
          // d1 Request-to-Send (RTS), pin 4
          // d0 Data-Terminal-Ready (DTR), pin 20 
        } else {
          // d7,d6 Not used
          // d5 Secondary Unassigned, pin 18
          // d4 Secondary Transmit Data, pin 14
          // d3 Secondary Request-to-Send, pin 19
          // d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
          // d1 Data-Terminal-Ready (DTR), pin 20
          // d0 Request-to-Send (RTS), pin 4
        }
        break;
      case 0xEB:
        break;
      case 0xEC:
      case 0xED:
      case 0xEE:
      case 0xEF:
        // Hardware settings
        // d6 CPU fast (1=4MHz, 0=2MHz)
        // d5 1=Enable Video Wait
        // d4 1=Enable External I/O bus
        // d3 1=Enable Alternate Character Set
        // d2 Mode Select (0=64 chars, 1=32chars)
        // d1 Cassette Motor (1=On)
        trs80->trs80PortEC = b & 0x7E;
        break;
      case 0xF0:
        wd1793_write(trs80->wd, WD1793_COMMAND, b);
        if (b == 0x82) {
          //debug(DEBUG_INFO, "TRS80", "nmiMask = 0x%02X (motor=%d, fdc=%d) LDOS", 0x80, (0x80&0x40) ? 1 : 0, (0x80&0x80) ? 1 : 0);
          //trs80->nmiMask = 0x80; // XXX gambiarra para LDOS chegar na tela "Date ?"
        }
        break;
      case 0xF1:
        wd1793_write(trs80->wd, WD1793_TRACK, b);
        break;
      case 0xF2:
        wd1793_write(trs80->wd, WD1793_SECTOR, b);
        break;
      case 0xF3:
        wd1793_write(trs80->wd, WD1793_DATA, b);
        break;
      case 0xF4:
        // Selection of drive and parameters
        // A write also causes the selected drive motor to turn on for about 3 seconds.
        // When the motor turns off, the drive is deselected.
        //   d7 1=MFM, 0=FM
        //   d6 1=Wait
        //   d5 1=Write Precompensation enabled
        //   d4 0=Side 0, 1=Side 1
        //   d3 1=select drive 3
        //   d2 1=select drive 2
        //   d1 1=select drive 1
        //   d0 1=select drive 0
        
        drive = -1;
        
        if ((b & 1) != 0) drive = 0;
        else if ((b & 2) != 0) drive = 1;
        else if ((b & 4) != 0) drive = 2;
        else if ((b & 8) != 0) drive = 3;

        head = (b & 16) != 0 ? 1 : 0;

        if (drive != -1) {
          wd1793_set_drive(trs80->wd, drive);
          wd1793_set_head(trs80->wd, head);
        }
        break;
      case 0xF8:
      case 0xF9:
      case 0xFA:
      case 0xFB:
        break;
      case 0xFC:
      case 0xFD:
      case 0xFE:
      case 0xFF:
        // d1, d0 Cassette output
        break;
  }
}

static uint8_t trs80_getclock(uint8_t cmd) {
  sys_tm_t *tm, ttm;
  uint64_t t;

  t = sys_time();
  sys_localtime(&t, &ttm);
  tm = &ttm;

  debug(DEBUG_INFO, "TRS80", "get clock cmd %d", cmd);

  switch (cmd) {
    case 0xC: // year (high)
      return (tm->tm_year / 10) % 10;
    case 0xB: // year (low)
      return (tm->tm_year % 10);
    case 0xA: // month (high)
      return ((tm->tm_mon + 1) / 10);
    case 0x9: // month (low)
      return ((tm->tm_mon + 1) % 10);
    case 0x8: // date (high) and leap year (bit 2)
      return ((tm->tm_mday / 10) | ((tm->tm_year % 4) ? 0 : 4));
    case 0x7: // date (low)
      return (tm->tm_mday % 10);
    case 0x6: // day-of-week
      return tm->tm_wday;
    case 0x5: // hours (high) and PM (bit 2) and 24hr (bit 3)
      return ((tm->tm_hour / 10) | 8);
    case 0x4: // hours (low)
      return (tm->tm_hour % 10);
    case 0x3: // minutes (high)
      return (tm->tm_min / 10);
    case 0x2: // minutes (low)
      return (tm->tm_min % 10);
    case 0x1: // seconds (high)
      return (tm->tm_sec / 10);
    case 0x0: // seconds (low)
      return (tm->tm_sec % 10);
  }

  return 0;
}

/*
  Portas usadas uando da boot com o LDOS:
  E0
  E4
  EC
  F0
  F1
  F3
*/

static uint16_t trs80_in(computer_t *c, uint16_t port) {
  trs80_data_t *trs80;
  uint8_t b = 0;

  trs80 = (trs80_data_t *)c->data;
  debug(DEBUG_TRACE, "TRS80", "in 0x%04X", port);
  port &= 0xFF;

  if ((port >= 0x70 && port <= 0x7C) || (port >= 0xB0 && port <= 0xBC)) {
    return trs80_getclock(port & 0x0F);
  }

  switch (port) {
      case 0xE0:
      case 0xE1:
      case 0xE2:
      case 0xE3:
        // d6 RS232 Error (Any of {FE, PE, OR} errors has occured)
        // d5 RS232 Rcv (DAV indicates a char ready to be picked up from uart)
        // d4 RS232 Xmit (TBMT indicates ready to accept another char from cpu)
        // d3 I/O Bus
        // d2 RTC
        // d1 Cass 1500 baud Falling
        // d0 Cass 1500 baud Rising
        b = ~trs80->irq_latch;
        break;
      case 0xE4:
        // d7 status of FDC INTREQ (0 = true)
        // d6 status of Motor Timeout (0 = true)
        // d5 status of Reset signal (0 = true - this will reboot the computer)
        b = ~trs80->nmi_latch;
        break;
      case 0xE8:
        // d7 Clear-to-Send (CTS), Pin 5
        // d6 Data-Set-Ready (DSR), pin 6
        // d5 Carrier Detect (CD), pin 8
        // d4 Ring Indicator (RI), pin 22
        // d3,d2,d0 Not used
        // d1 UART Receiver Input, pin 20 (pin 20 is also DTR)
        b = 0;
        break;
      case 0xE9:
        // power-on UART settings
        // d2,d1,d0: not used (0)
        // d7,d3: parity, 01=none, 00=odd, 10=even
        // d4: stop bits, 1=2 stop bits, 0=1 stop bit
        // d6,d5: word size, 00=5 bits, 01=6 bits, 10=7 bits, 11=8 bits
        break;
      case 0xEA:
        // UART Status Register
        // d7 Data Received ('1'=condition true)
        // d6 Transmitter Holding Register empty ('1'=condition true)
        // d5 Overrun Error ('1'=condition true)
        // d4 Framing Error ('1'=condition true)
        // d3 Parity Error ('1'=condition true)
        // d2..d0 Not used
        break;
      case 0xEB:
        // UART received data 
        break;
      case 0xEC:
      case 0xED:
      case 0xEE:
      case 0xEF:
        b = 0xFF;
        break;
      case 0xF0:
        b = wd1793_read(trs80->wd, WD1793_STATUS);
        break;
      case 0xF1:
        b = wd1793_read(trs80->wd, WD1793_TRACK);
        break;
      case 0xF2:
        b = wd1793_read(trs80->wd, WD1793_SECTOR);
        break;
      case 0xF3:
//if (z80_getpc(trs80->z) >= 0x4000) z80_debug(trs80->z, 1);
        b = wd1793_read(trs80->wd, WD1793_DATA);
        break;
      case 0xF4:
        break;
      case 0xF8:
      case 0xF9:
      case 0xFA:
      case 0xFB:
        // Bit 7 - 1 = Busy; 0 = Not Busy
        // Bit 6 - 1 = Out of Paper; 0 = Paper
        // Bit 5 - 1 = Printer selected; 0 = Printer not selected
        // Bit 4 - 1 = No Fault; 0 = Fault
        // Bits 3..0 - Not used
        break;
      case 0xFC:
      case 0xFD:
      case 0xFE:
      case 0xFF:
        // Return of cassette data stream from tape
        // d7 Low-speed data
        // d6..d1 info from write of port EC
        // d0 High-speed data
        b = 0xFF;
  }

  return b;
}

static int trs80_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  trs80_data_t *trs80;
  int r = -1;

  trs80 = (trs80_data_t *)c->data;

  if (disk_validate(name, &skip, &tracks, &heads, &sectors, &sectorlen, &sector0, trs80->session) != -1) {
    r = wd1793_insert(trs80->wd, drive, skip, tracks, heads, sectors, sectorlen, sector0, name);
  }

  return r;
}

static int trs80_rom(computer_t *c, int num, uint32_t size, char *name) {
  trs80_data_t *trs80;
  int r = -1;

  trs80 = (trs80_data_t *)c->data;

  switch (num) {
    case 0:
      r = load_rom(trs80->session, name, size, trs80->chr) != -1 ? 0 : -1;
      break;
    case 1:
      r = load_rom(trs80->session, name, size, trs80->rom) != -1 ? 0 : -1;
      break;
  }

  return r;
}

static int trs80_run(computer_t *c, uint32_t us) {
  trs80_data_t *trs80;

  trs80 = (trs80_data_t *)c->data;
  if (trs80->finish) return -1;

  return z80_loop(trs80->z, (us * Z80_CLOCK) / 1000000);
}

static int trs80_close(computer_t *c) {
  trs80_data_t *trs80;
  int r = -1;

  trs80 = (trs80_data_t *)c->data;

  if (trs80) {
    wd1793_close(trs80->wd);
    xfree(trs80);
    r = 0;
  }
  xfree(c);

  return r;
}

static void load_program(trs80_data_t *trs80) {
  int n;

  if ((n = load_rom(trs80->session, PROGRAM_BAS, MAX_PROG, (uint8_t *)trs80->program)) != -1) {
    trs80->progsize = n;
    trs80->progindex = 0;
  }
}

static void trs80_callback(void *data, uint32_t cycles) {
  trs80_data_t *trs80;
  int arg1, arg2;

  trs80 = (trs80_data_t *)data;
  surface_t *surface = lock_surface(trs80);

  if (trs80->dirty) {
    render(trs80, surface);
    trs80->dirty = 0;
  }

  switch (surface_event(surface, 0, &arg1, &arg2)) {
    case WINDOW_KEYDOWN:
      trs80->key = arg1;
      break;
    case WINDOW_KEYUP:
      trs80->key = 0;
      break;
    case WINDOW_BUTTON:
      if (!trs80->progsize) {
        load_program(trs80);
      }
      break;
    case -1:
      trs80->finish = 1;
      break;
  }
  unlock_surface(trs80);

  if ((trs80->irq_mask & IRQ_M4_RTC) != 0) {
    trs80->irq_latch |= IRQ_M4_RTC;
    z80_irq(trs80->z);
  }
}

static void wdDataRequest(int r, void *cdata) {
}
  
static void wdInterruptRequest(int r, void *cdata) {
  trs80_data_t *trs80;

  trs80 = (trs80_data_t *)cdata;

  if (r) {
    trs80->nmi_latch |= NMI_INTRQ_BIT;
  } else {
    trs80->nmi_latch &= ~NMI_INTRQ_BIT;
  }
  debug(DEBUG_INFO, "TRS80", "nmi_latch = %d", r);

  if (trs80->do_nmi || (trs80->nmi_latch & trs80->nmi_mask) != 0) {
    debug(DEBUG_INFO, "TRS80", "z80_nmi");
    z80_nmi(trs80->z, 1);
    trs80->do_nmi = 0;
  }
}

static int trs80_set_surface(struct computer_t *c, int ptr, surface_t *surface) {
  trs80_data_t *trs80;

  trs80 = (trs80_data_t *)c->data;
  trs80->surface = surface;
  trs80->ptr = ptr;

  if (ptr) {
    surface = ptr_lock(ptr, TAG_SURFACE);
  }

  if (surface->encoding == SURFACE_ENCODING_PALETTE) {
    surface_palette(surface, 0, 0x00, 0x00, 0x00);
    surface_palette(surface, 1, 0x00, 0xFF, 0x00);
    trs80->color[0] = 0;
    trs80->color[1] = 1;
  } else {
    trs80->color[0] = surface_color_rgb(surface->encoding, NULL, 0, 0x00, 0x00, 0x00, 0xFF);
    trs80->color[1] = surface_color_rgb(surface->encoding, NULL, 0, 0x00, 0xFF, 0x00, 0xFF);
  }
  surface->setarea(surface->data, 0, 0, surface->width-1, surface->height-1, trs80->color[0]);

  if (ptr) {
    ptr_unlock(ptr, TAG_SURFACE);
  }

  return 0;
}

computer_t *trs80_init(vfs_session_t *session) {
  trs80_data_t *trs80;
  computer_t *c = NULL;

  if ((trs80 = xcalloc(1, sizeof(trs80_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      trs80->session = session;
      trs80->d = disk_init(c, 4, 0, session);
      trs80->wd = wd1793_init(trs80->d, trs80);
      wd1793_set_dr(trs80->wd, wdDataRequest);
      wd1793_set_ir(trs80->wd, wdInterruptRequest);
      trs80->z = z80_open(Z80_PERIOD, 1, trs80_callback, trs80, c);
      trs80->dirty = 1;

      init_kmap(kmap, trs80->keymap);
      sys_memset(trs80->vram, ' ', VRAM_SIZE);

      c->set_surface = trs80_set_surface;
      c->disk = trs80_disk;
      c->rom = trs80_rom;
      c->run = trs80_run;
      c->close = trs80_close;
      c->getb = trs80_getb;
      c->getop = trs80_getb;
      c->putb = trs80_putb;
      c->out = trs80_out;
      c->in = trs80_in;
      c->data = trs80;

    } else {
      xfree(trs80);
    }
  }

  return c;
}
