#include "sys.h"
#include "vfs.h"
#include "ptr.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "pwindow.h"
#include "filter.h"
#include "rom.h"
#include "computer.h"
#include "m6502.h"
#include "apple2.h"
#include "debug.h"
#include "xalloc.h"

#define APPLE2_CLOCK   (14318180 / 14)
#define APPLE2_FPS     60
#define APPLE2_PERIOD  APPLE2_CLOCK/APPLE2_FPS

#define MEM_SIZE      64*1024
#define NUM_BANKS     16

#define SCREEN_WIDTH  560
#define SCREEN_HEIGHT 384

#define CHAR_WIDTH    8
#define CHAR_HEIGHT   8

#define CHR_SIZE      96*CHAR_HEIGHT

#define NUM_SLOTS     8
#define DISK_SLOT     6
#define SLOT_SIZE     0x100
#define NUM_DRIVES    2
#define TRACK_LENGTH  0x1A00

#define JOY_INTERVAL  (2816.0 / 255)

#define NUM_COLORS    16

typedef struct {
  m6502_t *cpu;
  uint8_t mem[MEM_SIZE];
  uint8_t chr[CHR_SIZE];
  uint8_t slot[NUM_SLOTS][SLOT_SIZE];
  uint8_t auxMem[NUM_BANKS][MEM_SIZE];
  int auxBank;

  uint64_t key_t;
  uint8_t key;
  int finish;
  int boot;

  vfs_session_t *session;
  int ptr;
  surface_t *surface;
  uint32_t c[16];
  uint32_t oc[16];
  int dirty;

  int flashCount, flashStatus;
  int sw_text, sw_mixed, sw_hires, sw_page2;
  int sw_80store, sw_ramrd, sw_ramwrt, sw_intcxrom, sw_altzp, sw_slotc3rom, sw_80col, sw_altcharset;
  int sw_an0, sw_an1, sw_an2, sw_an3;
  int sw_lcramwrt, sw_lcramrd, sw_lcbank2;
  uint8_t hcolor[4096];
  uint32_t appleEvenHiresColor[8192];
  uint32_t appleOddHiresColor[8192];

  int diskCard;
  int drive;
  int motor[NUM_DRIVES];
  int spinDelay[NUM_DRIVES];
  int changeTrack[NUM_DRIVES];
  int track[NUM_DRIVES];
  int phase[NUM_DRIVES];
  int driveDirty[NUM_DRIVES];
  int numTracks[NUM_DRIVES];
  int dptr[NUM_DRIVES];
  int writeMode;
  uint64_t lastByte[NUM_DRIVES];
  uint64_t lastRW[NUM_DRIVES];
  uint8_t *source[NUM_DRIVES];
  uint8_t buf[NUM_DRIVES][TRACK_LENGTH];
  int buflen[NUM_DRIVES];
  uint8_t latch;

  int joystickButton[2];
  int joystickX;
  int joystickY;
  uint64_t joystickReset;
} apple2_data_t;

static uint8_t colors[] = {
  0x00, 0x00, 0x00, // Black
  0xD0, 0x00, 0x30, // Dark Red
  0x00, 0x00, 0x90, // Dark Blue
  0xD0, 0x20, 0xD0, // Purple
  0x00, 0x70, 0x20, // Dark Green
  0x50, 0x50, 0x50, // Dark Grey
  0x20, 0x20, 0xF0, // Medium Blue
  0x60, 0xA0, 0xF0, // Light Blue
  0x80, 0x50, 0x00, // Brown
  0xF0, 0x60, 0x00, // Orange
  0xA0, 0xA0, 0xA0, // Light Grey
  0xF0, 0x90, 0x80, // Pink
  0x10, 0xD0, 0x00, // Light Green
  0xF0, 0xF0, 0x00, // Yellow
  0x40, 0xF0, 0x90, // Aquamarine
  0xF0, 0xF0, 0xF0, // White
};

static int appleLowColor[] = {
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0
};

static uint16_t appleTextAddr[] = {
  0x0000, 0x0080, 0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380,
  0x0028, 0x00A8, 0x0128, 0x01A8, 0x0228, 0x02A8, 0x0328, 0x03A8,
  0x0050, 0x00D0, 0x0150, 0x01D0, 0x0250, 0x02D0, 0x0350, 0x03D0
};

static uint16_t appleLoresAddr[] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 
  0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 
  0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 
  0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 
  0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 
  0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 
  0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 
  0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 
  0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 
  0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 
  0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 
  0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 
  0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 
  0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 
  0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 
  0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 
  0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 
  0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 
  0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 
  0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 
  0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 
  0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 
  0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0
};

static uint16_t appleHiresAddr[] = {
  0x0000, 0x0400, 0x0800, 0x0C00, 0x1000, 0x1400, 0x1800, 0x1C00, 
  0x0080, 0x0480, 0x0880, 0x0C80, 0x1080, 0x1480, 0x1880, 0x1C80, 
  0x0100, 0x0500, 0x0900, 0x0D00, 0x1100, 0x1500, 0x1900, 0x1D00, 
  0x0180, 0x0580, 0x0980, 0x0D80, 0x1180, 0x1580, 0x1980, 0x1D80, 
  0x0200, 0x0600, 0x0A00, 0x0E00, 0x1200, 0x1600, 0x1A00, 0x1E00, 
  0x0280, 0x0680, 0x0A80, 0x0E80, 0x1280, 0x1680, 0x1A80, 0x1E80, 
  0x0300, 0x0700, 0x0B00, 0x0F00, 0x1300, 0x1700, 0x1B00, 0x1F00, 
  0x0380, 0x0780, 0x0B80, 0x0F80, 0x1380, 0x1780, 0x1B80, 0x1F80, 
  0x0028, 0x0428, 0x0828, 0x0C28, 0x1028, 0x1428, 0x1828, 0x1C28, 
  0x00A8, 0x04A8, 0x08A8, 0x0CA8, 0x10A8, 0x14A8, 0x18A8, 0x1CA8, 
  0x0128, 0x0528, 0x0928, 0x0D28, 0x1128, 0x1528, 0x1928, 0x1D28, 
  0x01A8, 0x05A8, 0x09A8, 0x0DA8, 0x11A8, 0x15A8, 0x19A8, 0x1DA8, 
  0x0228, 0x0628, 0x0A28, 0x0E28, 0x1228, 0x1628, 0x1A28, 0x1E28, 
  0x02A8, 0x06A8, 0x0AA8, 0x0EA8, 0x12A8, 0x16A8, 0x1AA8, 0x1EA8, 
  0x0328, 0x0728, 0x0B28, 0x0F28, 0x1328, 0x1728, 0x1B28, 0x1F28, 
  0x03A8, 0x07A8, 0x0BA8, 0x0FA8, 0x13A8, 0x17A8, 0x1BA8, 0x1FA8, 
  0x0050, 0x0450, 0x0850, 0x0C50, 0x1050, 0x1450, 0x1850, 0x1C50, 
  0x00D0, 0x04D0, 0x08D0, 0x0CD0, 0x10D0, 0x14D0, 0x18D0, 0x1CD0, 
  0x0150, 0x0550, 0x0950, 0x0D50, 0x1150, 0x1550, 0x1950, 0x1D50, 
  0x01D0, 0x05D0, 0x09D0, 0x0DD0, 0x11D0, 0x15D0, 0x19D0, 0x1DD0, 
  0x0250, 0x0650, 0x0A50, 0x0E50, 0x1250, 0x1650, 0x1A50, 0x1E50, 
  0x02D0, 0x06D0, 0x0AD0, 0x0ED0, 0x12D0, 0x16D0, 0x1AD0, 0x1ED0, 
  0x0350, 0x0750, 0x0B50, 0x0F50, 0x1350, 0x1750, 0x1B50, 0x1F50, 
  0x03D0, 0x07D0, 0x0BD0, 0x0FD0, 0x13D0, 0x17D0, 0x1BD0, 0x1FD0
};

static surface_t *lock_surface(apple2_data_t *apple2) {
  if (apple2->surface) return apple2->surface;
  if (apple2->ptr > 0) return ptr_lock(apple2->ptr, TAG_SURFACE);
  return NULL;
}

static void unlock_surface(apple2_data_t *apple2) {
  if (apple2->ptr > 0) ptr_unlock(apple2->ptr, TAG_SURFACE);
}

/*
  MEMORY MANAGEMENT SOFT SWITCHES
   $C000   W       80STOREOFF      Allow page2 to switch video page1 page2
   $C001   W       80STOREON       Allow page2 to switch main & aux video memory
   $C002   W       RAMRDOFF        Read enable main memory from $0200-$BFFF
   $C003   W       RAMDRON         Read enable aux memory from $0200-$BFFF
   $C004   W       RAMWRTOFF       Write enable main memory from $0200-$BFFF
   $C005   W       RAMWRTON        Write enable aux memory from $0200-$BFFF
   $C006   W       INTCXROMOFF     Enable slot ROM from $C100-$CFFF
   $C007   W       INTCXROMON      Enable main ROM from $C100-$CFFF
   $C008   W       ALZTPOFF        Enable main memory from $0000-$01FF & avl BSR
   $C009   W       ALTZPON         Enable aux memory from $0000-$01FF & avl BSR
   $C00A   W       SLOTC3ROMOFF    Enable main ROM from $C300-$C3FF
   $C00B   W       SLOTC3ROMON     Enable slot ROM from $C300-$C3FF

  VIDEO SOFT SWITCHES
   $C00C   W       80COLOFF        Turn off 80 column display
   $C00D   W       80COLON         Turn on 80 column display
   $C00E   W       ALTCHARSETOFF   Turn off alternate characters
   $C00F   W       ALTCHARSETON    Turn on alternate characters
   $C050   R/W     TEXTOFF         Select graphics mode
   $C051   R/W     TEXTON          Select text mode
   $C052   R/W     MIXEDOFF        Use full screen for graphics
   $C053   R/W     MIXEDON         Use graphics with 4 lines of text
   $C054   R/W     PAGE2OFF        Select panel display (or main video memory)
   $C055   R/W     PAGE2ON         Select page2 display (or aux video memory)
   $C056   R/W     HIRESOFF        Select low resolution graphics
   $C057   R/W     HIRESON         Select high resolution graphics

  SOFT SWITCH STATUS FLAGS
   $C010   R7      AKD             1=key pressed   0=keys free    (clears strobe)
   $C011   R7      BSRBANK2        1=bank2 available    0=bank1 available
   $C012   R7      BSRREADRAM      1=BSR active for read   0=$D000-$FFFF active
   $C013   R7      RAMRD           0=main $0200-$BFFF active reads  1=aux active
   $C014   R7      RAMWRT          0=main $0200-$BFFF active writes 1=aux writes
   $C015   R7      INTCXROM        1=main $C100-$CFFF ROM active   0=slot active
   $C016   R7      ALTZP           1=aux $0000-$1FF+auxBSR    0=main available
   $C017   R7      SLOTC3ROM       1=slot $C3 ROM active   0=main $C3 ROM active
   $C018   R7      80STORE         1=page2 switches main/aux   0=page2 video
   $C019   R7      VERTBLANK       1=vertical retrace on   0=vertical retrace off
   $C01A   R7      TEXT            1=text mode is active   0=graphics mode active
   $C01B   R7      MIXED           1=mixed graphics & text    0=full screen
   $C01C   R7      PAGE2           1=video page2 selected or aux
   $C01D   R7      HIRES           1=high resolution graphics   0=low resolution
   $C01E   R7      ALTCHARSET      1=alt character set on    0=alt char set off
   $C01F   R7      80COL           1=80 col display on     0=80 col display off
   */

static uint8_t c00xRead(apple2_data_t *apple2, uint16_t addr) {
  return apple2->key;
}

static uint8_t c01xRead(apple2_data_t *apple2, uint16_t addr) {
  uint8_t b = 0;

  switch (addr) {
    case 0x00: // KBDSTRB
      apple2->key &= 0x7F;
      b = apple2->key ? 0x80 : 0x00;
      break;
    case 0x01:
      b = apple2->sw_lcbank2 ? 0x80 : 0x00;
      break;
    case 0x02:
      b = apple2->sw_lcramrd ? 0x80 : 0x00;
      break;
    case 0x03:
      b = apple2->sw_ramrd ? 0x80 : 0x00;
      break;
    case 0x04:
      b = apple2->sw_ramwrt ? 0x80 : 0x00;
      break;
    case 0x05:
      b = apple2->sw_intcxrom? 0x80 : 0x00;
      break;
    case 0x06:
      b = apple2->sw_altzp ? 0x80 : 0x00;
      break;
    case 0x07:
      b = apple2->sw_slotc3rom ? 0x80 : 0x00;
      break;
    case 0x08:
      b = apple2->sw_80store ? 0x80 : 0x00;
      break;
    case 0x09:
      // XXX is this correct ?
      //int currCycles = cpu.getCallbackCycles();
      //int vbl = (currCycles > vblCycles) && (currCycles - vblCycles) < 22;
      //b = vbl ? 0x80 : 0x00;
      break;
    case 0x0A: // RDTEXT
      b = apple2->sw_text ? 0x80 : 0x00;
      break;
    case 0x0B: // RDMIXED
      b = apple2->sw_mixed ? 0x80 : 0x00;
      break;
    case 0x0C: // RDPAGE2
      b = apple2->sw_page2 ? 0x80 : 0x00;
      break;
    case 0x0D: // RDHIRES
      b = apple2->sw_hires ? 0x80 : 0x00;
      break;
    case 0x0E:
      b = apple2->sw_altcharset ? 0x80 : 0x00;
      break;
    case 0x0F:
      b = apple2->sw_80col ? 0x80 : 0x00;
      break;
    }

    return b;
  }

static uint8_t c02xRead(apple2_data_t *apple2, uint16_t addr) {
  uint8_t b = 0;

  switch (addr) {
    case 0x00: // C020: toggle Cassette Tape Output
      //waveOutput(tapeLevel);
      //tapeLevel = 255 - tapeLevel;
      break;
  }

  return b;
}

static uint8_t c03xRead(apple2_data_t *apple2, uint16_t addr) {
  if (addr == 0) {
    //simpleSound.sound(soundLevel, cpu.getCallbackCycles());
    //soundLevel = 0x40 - soundLevel;
  }
  return 0;
}

static uint8_t c04xRead(apple2_data_t *apple2, uint16_t addr) {
  return 0;
}

static uint8_t c05xRead(apple2_data_t *apple2, uint16_t addr) {
  int value = (addr & 1) != 0;

  switch (addr >> 1) {
    case 0: apple2->sw_text  = value; break;
    case 1: apple2->sw_mixed = value; break;
    case 2: apple2->sw_page2 = value; break;
    case 3: apple2->sw_hires = value; break;
    case 4: apple2->sw_an0 = !value; break;
    case 5: apple2->sw_an1 = !value; break;
    case 6: apple2->sw_an2 = !value; break;
    case 7: apple2->sw_an3 = !value; break;
  }

  return 0;
}

static uint8_t c06xRead(apple2_data_t *apple2, uint16_t addr) {
  int pos, active;
  uint8_t b = 0;

  switch (addr) {
    case 0x00:
      // C060: read Cassette Input (bit 7)
      //if (waveInput(casOsd)) {
      //  b |= 0x80;
      //}
      break;
    case 0x01:
    case 0x02:
      // Joystick button
      b = apple2->joystickButton[addr-1] ? 0x80 : 0x00;
      break;
    case 0x04:
    case 0x05:
      // XXX 6 and 7 too ?
      // Joystick position
      pos = (addr & 1) == 1 ? apple2->joystickY : apple2->joystickX;
      active = m6502_getcycles(apple2->cpu) <= (apple2->joystickReset + (pos * JOY_INTERVAL));
      b = active ? 0x80: 0x00;
      break;
    }

  return b;
}

static uint8_t c07xRead(apple2_data_t *apple2, uint16_t addr) {
  uint8_t b = 0;

  switch (addr) {
    case 0x00:
      // Joystick reset position
      apple2->joystickReset = m6502_getcycles(apple2->cpu);
      b = 0x80;
      break;
    case 0x7F:
      b = apple2->sw_an3 ? 0x80 : 0x00;
      break;
  }

  return b;
}

static uint8_t c08xRead(apple2_data_t *apple2, uint16_t addr) {
  apple2->sw_lcramwrt = (addr & 1) != 0;
  apple2->sw_lcramrd  = ((addr & 2) >> 1) == (addr & 1);
  apple2->sw_lcbank2  = (addr < 8);
  //System.out.printf("Language Card: RAM read = %s, RAM write = %s, BANK2 = %s\n", sw_lcramrd, sw_lcramwrt, sw_lcbank2);
  return 0;
}

#define SPIN_DELAY 20000
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static void apple2_checkSpinning(apple2_data_t *apple2) {
  if (apple2->motor[apple2->drive]) {
    apple2->spinDelay[apple2->drive] = SPIN_DELAY;
  }
}

static void apple2_update(apple2_data_t *apple2, uint32_t cycles) {
  uint32_t spinDecr = cycles >> 6;

  for (int i = 0; i < NUM_DRIVES; i++) {
    if (apple2->spinDelay[i] > 0 && !apple2->motor[i]) {
      apple2->spinDelay[i] -= MIN(apple2->spinDelay[i], spinDecr);
      if (apple2->spinDelay[i] == 0) {
        debug(DEBUG_INFO, "APPLE2", "drive %d motor OFF", i);
        apple2->lastRW[i] = 0;
      }
    }
  }
}

static int apple2_stepper(apple2_data_t *apple2, int addr) {
  if (apple2->source[apple2->drive]) {
    if ((addr & 1) != 0) {
      int direction = 0;
      int phase = (addr >> 1) & 3;
      if (phase == ((apple2->phase[apple2->drive] + 1) & 3)) {
        direction = 1;
      }
      if (phase == ((apple2->phase[apple2->drive] + 3) & 3)) {
        direction = -1;
      }
      if (direction != 0) {
        apple2->phase[apple2->drive] = MAX(0, MIN(79, apple2->phase[apple2->drive] + direction));
        if ((apple2->phase[apple2->drive] & 1) == 0) {
          if (apple2->phase[apple2->drive] == 0 && direction == -1) {
          }
          int track = MIN(apple2->numTracks[apple2->drive] - 1, apple2->phase[apple2->drive] >> 1);
          if (track != apple2->track[apple2->drive]) {
            if (apple2->driveDirty[apple2->drive]) {
              sys_memcpy(&apple2->source[apple2->drive][apple2->track[apple2->drive] * TRACK_LENGTH], apple2->buf[apple2->drive], TRACK_LENGTH);
              apple2->driveDirty[apple2->drive] = 0;
            }
            apple2->track[apple2->drive] = track;
            apple2->changeTrack[apple2->drive] = 1;
          }
        }
      }
    }
  }

  return addr == 0 ? 0xFF : 0x00;
}

static void apple2_disk_write(apple2_data_t *apple2) {
  if (apple2->buflen[apple2->drive]) {
    if ((apple2->latch & 0x80) != 0) {
      uint64_t diff = apple2->lastRW[apple2->drive] - apple2->lastByte[apple2->drive];
      if (diff >= 32) {
        if (apple2->lastByte[apple2->drive] == 0) {
          apple2->lastByte[apple2->drive] = apple2->lastRW[apple2->drive];
        } else {
          if (diff >= 64) {
            // estava causando erro #8 no dos
            //apple2->dptr[apple2->drive] = (apple2->dptr[apple2->drive] + (int)(diff - 32) / 32) % apple2->buflen[apple2->drive];
          }
          apple2->buf[apple2->drive][apple2->dptr[apple2->drive]] = apple2->latch;
          apple2->dptr[apple2->drive] = (apple2->dptr[apple2->drive] + 1) % apple2->buflen[apple2->drive];
          apple2->lastByte[apple2->drive] = apple2->lastRW[apple2->drive];
          apple2->driveDirty[apple2->drive] = 1;
        }
      }
    }
  }
}

static uint8_t apple2_disk_read(apple2_data_t *apple2) {
  uint8_t b = 0;

  if (apple2->buflen[apple2->drive]) {
    uint64_t diff = apple2->lastRW[apple2->drive] - apple2->lastByte[apple2->drive];
    if (diff >= 32) {
      if (apple2->lastByte[apple2->drive] == 0) {
        apple2->lastByte[apple2->drive] = apple2->lastRW[apple2->drive];
      } else {
        b = apple2->buf[apple2->drive][apple2->dptr[apple2->drive]];
        apple2->dptr[apple2->drive] = (apple2->dptr[apple2->drive] + 1) % apple2->buflen[apple2->drive];
        apple2->lastByte[apple2->drive] = apple2->lastRW[apple2->drive];
      }
    }
  }

  return b;
}

static uint8_t diskRead(apple2_data_t *apple2, uint16_t addr) {
  int motor;
  uint8_t b = 0;

  switch (addr) {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
      b = apple2_stepper(apple2, addr);
      break;
    case 0x8:
    case 0x9:
      motor = (addr & 1) != 0;
      if (!apple2->motor[apple2->drive] && apple2->spinDelay[apple2->drive] == 0 && motor) {
        apple2->dptr[apple2->drive] = 0;
        debug(DEBUG_INFO, "APPLE2", "drive %d motor ON", apple2->drive);
      }
      apple2->motor[apple2->drive] = motor;
      apple2_checkSpinning(apple2);
      break;
    case 0xA:
    case 0xB:
      apple2->drive = addr & 1;
      apple2->spinDelay[1-apple2->drive] = 0;
      apple2_checkSpinning(apple2);
      break;
    case 0xC:
      if (apple2->changeTrack[apple2->drive] && apple2->source[apple2->drive] && apple2->track[apple2->drive] >= 0) {
        sys_memcpy(apple2->buf[apple2->drive], &apple2->source[apple2->drive][apple2->track[apple2->drive] * TRACK_LENGTH], TRACK_LENGTH);
        apple2->changeTrack[apple2->drive] = 0;
        debug(DEBUG_INFO, "APPLE2", "read drive %d track %d", apple2->drive, apple2->track[apple2->drive]);
      }
      if (apple2->writeMode) {
        // SHIFT
        apple2_disk_write(apple2);
      } else {
        // READ
        b = apple2_disk_read(apple2);
      }
      break;
    case 0xD:
      if (apple2->writeMode) {
        // LOAD
        b = apple2->latch;
      } else {
        // CHECK WRITE PROTECT
      }
      break;
    case 0xE:
    case 0xF:
      apple2->writeMode = addr & 1;
      break;
  }

  return b;
}

static uint8_t slotRead(apple2_data_t *apple2, int slot, uint16_t addr) {
  uint32_t t;
  int i;
  uint8_t b = 0;

  //if (apple2->slots[slot] != null) {
  //  diskII.setCycles(cpu.getTotalCycles());
  //  b = apple2->slots[slot].ioRead(addr);
  //}

  switch (slot) {
    case DISK_SLOT:
      t = m6502_getcycles(apple2->cpu);
      for (i = 0; i < NUM_DRIVES; i++) {
        apple2->lastRW[i] = t;
      }
      b = diskRead(apple2, addr);
      break;
  }

  return b;
}

static uint8_t apple2_in(apple2_data_t *apple2, uint8_t addr) {
  uint8_t b = 0;

  if (addr < 0x90) {
    switch (addr & 0xF0) {
      case 0x00: b = c00xRead(apple2, addr & 0x0F); break;
      case 0x10: b = c01xRead(apple2, addr & 0x0F); break;
      case 0x20: b = c02xRead(apple2, addr & 0x0F); break;
      case 0x30: b = c03xRead(apple2, addr & 0x0F); break;
      case 0x40: b = c04xRead(apple2, addr & 0x0F); break;
      case 0x50: b = c05xRead(apple2, addr & 0x0F); break;
      case 0x60: b = c06xRead(apple2, addr & 0x0F); break;
      case 0x70: b = c07xRead(apple2, addr & 0x0F); break;
      case 0x80: b = c08xRead(apple2, addr & 0x0F); break;
    }
  } else {
    int slot = (addr - 0x80) >> 4;
    b = slotRead(apple2, slot, addr & 0x0F);
  }

  return b;
}

static void diskWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
  switch (addr) {
    case 0xD:
      if (apple2->writeMode) {
        // LOAD
        apple2->latch = b;
      }
      break;
    default:
      diskRead(apple2, addr);    
      break;
  }
}

static void slotWrite(apple2_data_t *apple2, int slot, uint16_t addr, uint8_t b) {
  uint32_t t;
  int i;

  //if (slots[slot] != null) {
  //  diskII.setCycles(cpu.getTotalCycles());
  //  slots[slot].ioWrite(addr, b);
  //}

  switch (slot) {
    case DISK_SLOT:
      t = m6502_getcycles(apple2->cpu);
      for (i = 0; i < NUM_DRIVES; i++) {
        apple2->lastRW[i] = t;
      }
      diskWrite(apple2, addr, b);
      break;
  }
}

static void c00xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
  int value = (addr & 1) != 0;

  switch (addr >> 1) {
    case 0: apple2->sw_80store = value; break;
    case 1: apple2->sw_ramrd = value; break;
    case 2: apple2->sw_ramwrt = value; break;
    case 3:
      if (apple2->sw_intcxrom && !value) {
        //extRom = null;
      }
      apple2->sw_intcxrom = value;
      break;
    case 4: apple2->sw_altzp = value; break;
    case 5: apple2->sw_slotc3rom = value; break;
    case 6: apple2->sw_80col = value; break;
    case 7: apple2->sw_altcharset = value; break;
  }
}

static void c01xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
  if (addr == 0) {
    apple2->key &= 0x7F;
  }
}

static void c02xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
}

static void c03xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
  c03xRead(apple2, addr);
}

static void c04xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
}

static void c05xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
  c05xRead(apple2, addr);
}

static void c06xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
}

static void c07xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
  switch (addr) {
    case 0x01: // extended memory card set page
    case 0x03: // Ramworks III set page
      apple2->auxBank = (b < NUM_BANKS) ? b : -1;
      break;
    default:
      c07xRead(apple2, addr);
      break;
  }
}

static void c08xWrite(apple2_data_t *apple2, uint16_t addr, uint8_t b) {
  c08xRead(apple2, addr);
}

static void apple2_out(apple2_data_t *apple2, uint8_t addr, uint8_t b) {
  if (addr < 0x90) {
    switch (addr & 0xF0) {
      case 0x00: c00xWrite(apple2, addr & 0x0F, b); break;
      case 0x10: c01xWrite(apple2, addr & 0x0F, b); break;
      case 0x20: c02xWrite(apple2, addr & 0x0F, b); break;
      case 0x30: c03xWrite(apple2, addr & 0x0F, b); break;
      case 0x40: c04xWrite(apple2, addr & 0x0F, b); break;
      case 0x50: c05xWrite(apple2, addr & 0x0F, b); break;
      case 0x60: c06xWrite(apple2, addr & 0x0F, b); break;
      case 0x70: c07xWrite(apple2, addr & 0x0F, b); break;
      case 0x80: c08xWrite(apple2, addr & 0x0F, b); break;
    }
  } else {
    int slot = (addr - 0x80) >> 4;
    slotWrite(apple2, slot, addr & 0x0F, b);
  }
}

static uint8_t apple2_getb(computer_t *c, uint16_t addr) {
  apple2_data_t *apple2;

  apple2 = (apple2_data_t *)c->data;

  if (addr >= 0xC000 && addr < 0xC100) {
    return apple2_in(apple2, addr & 0xFF);
  }

  if (addr >= 0xC100 && addr < 0xC800) {
    int slot = (addr & 0x0F00) >> 8;
    return apple2->slot[slot][addr & 0xFF];
  }

  return apple2->mem[addr];
}

static void apple2_putb(computer_t *c, uint16_t addr, uint8_t b) {
  apple2_data_t *apple2;
  uint16_t start;

  apple2 = (apple2_data_t *)c->data;

  if (addr < 0xC000) {
    apple2->mem[addr] = b;

    if (!apple2->sw_hires || apple2->sw_mixed) {
      start = (apple2->sw_page2 && (!apple2->sw_80col || !apple2->sw_80store)) ? 0x800 : 0x400;
      if (addr >= start && addr < (start + 0x400)) apple2->dirty = 1;
    }

    if (apple2->sw_hires) {
      start = (apple2->sw_page2 && (!apple2->sw_80col || !apple2->sw_80store)) ? 0x4000 : 0x2000;
      if (addr >= start && addr < (start + 0x2000)) apple2->dirty = 1;
    }

    return;
  }

  if (addr < 0xC100) apple2_out(apple2, addr & 0xFF, b);
}

/*
$00..$1F Inverse  Uppercase Letters (aka glyphs of ASCII $40..$5F)
$20..$3F Inverse  Symbols/Numbers   (aka glyphs of ASCII $20..$3F)
$40..$5F Flashing Uppercase Letters
$60..$7F Flashing Symbols/Numbers
$80..$9F Normal   Uppercase Letters (make ASCII control codes show up as letters)
$A0..$BF Normal   Symbols/Numbers   (like ASCII + $80)
$C0..$DF Normal   Uppercase Letters (like ASCII + $80)
$E0..$FF Normal   Symbols/Numbers
*/

static void textLine(apple2_data_t *apple2, surface_t *surface, uint16_t base, int i0, int i1) {
  for (int i = i0; i < i1; i++) {
    uint16_t addr = appleTextAddr[i];
    for (int j = 0; j < 40; j++) {
      uint8_t c = apple2->mem[base + addr + j];
      if (!apple2->sw_altcharset && !apple2->sw_80col && !apple2->flashStatus && c >= 0x40 && c < 0x80) {
        c += 0x40;
      }
      int inv = c < 0x80;
      uint32_t fg, bg;
      if (inv) {
        fg = apple2->c[0];
        bg = apple2->c[15];
      } else {
        fg = apple2->c[15];
        bg = apple2->c[0];
      }
      int chr = c & 0x3F;
      int idx = chr << 3;

      int x0 = (surface->width - SCREEN_WIDTH) / 2 + j * (CHAR_WIDTH-1)*2;
      int y0 = i * CHAR_HEIGHT*2;
      for (int y = 0; y < CHAR_HEIGHT*2; y++) {
        uint8_t b = apple2->chr[idx];
        if (y & 1) idx++;
        for (int x = 0; x < (CHAR_WIDTH-1)*2; x++) {
          surface->setpixel(surface->data, x0 + x, y0 + y, (b & 0x80) ? fg : bg);
          if (x & 1) b <<= 1;
        }
      }
    }
  }
}

static void doubleTextLine(apple2_data_t *apple2, surface_t *surface, uint16_t base, int i0, int i1) {
}

static void textScreen(apple2_data_t *apple2, surface_t *surface) {
  uint16_t start = apple2->sw_page2 && (!apple2->sw_80col || !apple2->sw_80store) ? 0x800 : 0x400;
    
  if (apple2->sw_80col) {
    doubleTextLine(apple2, surface, start, 0, 24);
  } else {
    textLine(apple2, surface, start, 0, 24);
  }
}

static void normalHiresScreen(apple2_data_t *apple2, surface_t *surface) {
  uint16_t start  = (apple2->sw_page2 && (!apple2->sw_80col || !apple2->sw_80store)) ? 0x4000 : 0x2000;
  uint16_t tstart = (apple2->sw_page2 && (!apple2->sw_80col || !apple2->sw_80store)) ? 0x0800 : 0x0400;
  int i1 = apple2->sw_mixed ? (20 << 3) : (24 << 3);
  int y = 0;

  for (int i = 0; i < i1; i++) {
    uint16_t addr = appleHiresAddr[i];
    uint16_t laddr = start + addr;
    uint8_t c1 = 0;
    uint8_t c2 = apple2->mem[laddr];
    uint8_t c3 = apple2->mem[laddr + 1];
    int x = 0;

    for (int j = 0; j < 40; j += 2) {
      uint32_t c = ((c1 & 0x40) >> 6) | (c2 << 1) | ((c3 & 0x01) << 9);
      uint32_t offset = c << 3;

      for (int k = 0; k < CHAR_WIDTH-1; k++, x += 2, offset++) {
        uint32_t ec = apple2->appleEvenHiresColor[offset];
        surface->setarea(surface->data, x, y, x+1, y+1, ec);
      }

      laddr++;
      c1 = c2;
      c2 = c3;
      c3 = apple2->mem[laddr + 1];
      c = ((c1 & 0x40) >> 6) | (c2 << 1) | ((c3 & 0x01) << 9);

      offset = c << 3;
      for (int k = 0; k < CHAR_WIDTH-1; k++, x += 2, offset++) {
        uint32_t ec = apple2->appleOddHiresColor[offset];
        surface->setarea(surface->data, x, y, x+1, y+1, ec);
      }

      laddr++;
      c1 = c2;
      c2 = c3;
      c3 = apple2->mem[laddr + 1];
    }
    y += 2;
  }

  if (apple2->sw_mixed) {
    if (apple2->sw_80col) {
      doubleTextLine(apple2, surface, tstart, 20, 24);
    } else {
      textLine(apple2, surface, tstart, 20, 24);
    }
  }
}

static void doubleHighresScreen(apple2_data_t *apple2, surface_t *surface) {
  uint32_t start = 0x2000;
  int i1 = 24 << 3;
  int c[7];
  int y = 0;

  //int[] code = {0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};
  uint8_t code[] = {0x0, 0x1, 0x8, 0x9, 0x4, 0x5, 0xC, 0xD, 0x2, 0x3, 0xA, 0xB, 0x6, 0x7, 0xE, 0xF};

  for (int i = 0; i < i1; i++) {
    int addr = appleHiresAddr[i];
    int laddr = start + addr;

    for (int j = 0; j < 40; j += 2) {
      uint8_t b1 = apple2->auxMem[apple2->auxBank][laddr + j];
      uint8_t b2 = apple2->mem[laddr + j];
      uint8_t b3 = apple2->auxMem[apple2->auxBank][laddr + j + 1];
      uint8_t b4 = apple2->mem[laddr + j + 1];
      int x = 0;

      if (1) {
        c[0] =   b1       & 0x0F;
        c[1] = ((b1 >> 4) & 0x07) | ((b2 & 0x01) << 3);
        c[2] =  (b2 >> 1) & 0x0F;
        c[3] = ((b2 >> 5) & 0x03) | ((b3 & 0x03) << 2);
        c[4] =  (b3 >> 2) & 0x0F;
        c[5] = ((b3 >> 6) & 0x01) | ((b4 & 0x07) << 1);
        c[6] =  (b4 >> 3) & 0x0F;
      } else {
        c[0] =  (b1 >> 3) & 0x0F;
        c[1] = ((b1 << 1) & 0x0E) | ((b2 >> 6) & 0x01);
        c[2] =  (b2 >> 2) & 0x0F;
        c[3] = ((b2 << 2) & 0x0C) | ((b3 >> 5) & 0x03);
        c[4] =  (b2 >> 1) & 0x0F;
        c[5] = ((b3 << 3) & 0x08) | ((b4 >> 4) & 0x07);
        c[6] =  (b4 >> 0) & 0x0F;
      }

      for (int m = 0; m < 7; m++) c[m] = code[c[m]];

      for (int m = 0; m < 7; m++) {
        for (int k = 0; k < 4; k++) {
          int ec = (c[m] & (1 << k)) != 0 ? apple2->c[c[m]] : 0;
          surface->setpixel(surface->data, x, y, ec);
          surface->setpixel(surface->data, x, y+1, ec);
        }
        x += 4;
      }
    }
    y++;
  }
}

static void hiresScreen(apple2_data_t *apple2, surface_t *surface) {
  if (apple2->sw_80col && apple2->sw_an3) {
    doubleHighresScreen(apple2, surface);
  } else {
    normalHiresScreen(apple2, surface);
  }
}

static void loresScreen(apple2_data_t *apple2, surface_t *surface) {
  uint16_t start = (apple2->sw_page2 && (!apple2->sw_80col || !apple2->sw_80store)) ? 0x800 : 0x400;
  int i1 = apple2->sw_mixed ? (20 << 3) : (24 << 3);
  int y = 0;

  for (int i = 0; i < i1; i++, y += 2) {
    uint16_t addr = appleLoresAddr[i];
    int x = 0;
    for (int j = 0; j < 40; j++) {
      uint8_t c = apple2->mem[start + addr + j];
      uint32_t b = apple2->c[appleLowColor[i] ? c & 0x0F : c >> 4];
      for (int k = 0; k < (CHAR_WIDTH-1)*2; k++, x++) {
        surface->setpixel(surface->data, x, y, b);
        surface->setpixel(surface->data, x, y+1, b);
      }
    }
  }

  if (apple2->sw_mixed) {
    if (apple2->sw_80col) {
      doubleTextLine(apple2, surface, start, 20, 24);
    } else {
      textLine(apple2, surface, start, 20, 24);
    }
  }
}

static void render(apple2_data_t *apple2, surface_t *surface) {
  if (apple2->sw_text) {
    textScreen(apple2, surface);
  } else if (apple2->sw_hires) {
    hiresScreen(apple2, surface);
  } else {
    loresScreen(apple2, surface);
  }

  surface_update(surface, 0, surface->height);
}

static int apple2_rom(computer_t *c, int num, uint32_t size, char *name) {
  apple2_data_t *apple2;
  int r = -1;

  apple2 = (apple2_data_t *)c->data;

  switch (num) {
    case 0:
      if (size == CHR_SIZE) {
        r = load_rom(apple2->session, name, size, apple2->chr) != -1 ? 0 : -1;
      }
      break;
    case 1:
      if (size <= 0x3000) {
        r = load_rom(apple2->session, name, size, &apple2->mem[0xD000]) != -1 ? 0 : -1;
      }
      break;
    case 2:
      if (size == SLOT_SIZE) {
        r = load_rom(apple2->session, name, size, apple2->slot[DISK_SLOT]) != -1 ? 0 : -1;
      }
      break;
  }

  return r;
}

static int apple2_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  apple2_data_t *apple2;
  vfs_file_t *f;
  int r = -1;

  apple2 = (apple2_data_t *)c->data;

  if (drive >= 0 && drive < NUM_DRIVES && skip == 0 && tracks >= 35 && heads == 1 && sectors == 16 && sectorlen == 256 && sector0) {
    apple2->source[drive] = xcalloc(1, tracks * TRACK_LENGTH);

    if (apple2->source[drive] && (f = vfs_open(apple2->session, name, VFS_READ)) != NULL) {
      if (vfs_read(f, apple2->source[drive], tracks * TRACK_LENGTH) == tracks * TRACK_LENGTH) {
        apple2->numTracks[drive] = tracks;
        apple2->changeTrack[drive] = 1;
        apple2->buflen[drive] = TRACK_LENGTH;
        r = 0;
      }
      vfs_close(f);
    }
  }

  return r;
}

static int apple2_run(computer_t *c, uint32_t us) {
  apple2_data_t *apple2;

  apple2 = (apple2_data_t *)c->data;
  if (apple2->boot) {
    sys_memset(&apple2->mem[0x0000], 0x00, 0xC000);
    m6502_reset(apple2->cpu);
    apple2->boot = 0;
  }
  if (apple2->finish) return -1;

  return m6502_loop(apple2->cpu, (us * APPLE2_CLOCK) / 1000000);
}

static int apple2_close(computer_t *c) {
  apple2_data_t *apple2;
  int i, r = -1;

  apple2 = (apple2_data_t *)c->data;

  if (apple2) {
    for (i = 0; i < NUM_DRIVES; i++) {
      if (apple2->source[i]) {
        xfree(apple2->source[i]);
      }
    }
    xfree(apple2);
    r = 0;
  }
  xfree(c);

  return r;
}

static void apple2_callback(void *data, uint32_t cycles) {
  apple2_data_t *apple2;
  int arg1, arg2;

  apple2 = (apple2_data_t *)data;
  surface_t *surface = lock_surface(apple2);

  switch (surface_event(apple2->surface, 0, &arg1, &arg2)) {
    case WINDOW_KEYDOWN:
      if (arg1 == 18) {
        apple2->boot = 1;
      } else if (arg1 == 26) {
        apple2->finish = 1;
      } else {
        if (arg1 >= 'a' && arg1 <= 'z') {
          arg1 -= 32;
        }
        apple2->key = arg1 | 0x80;
      }
      break;
    case WINDOW_BUTTON:
      //apple2->joystickButton[0] = (buttons & 1) ? 1 : 0;
      //apple2->joystickButton[1] = (buttons & 2) ? 1 : 0;
      break;
    case WINDOW_MOTION:
      //apple2->joystickX = (int)(key  * 256.0 / SCREEN_WIDTH);
      //apple2->joystickY = (int)(mods * 256.0 / SCREEN_HEIGHT);
      break;
    case -1:
      apple2->finish = 1;
      break;
  }

  apple2->flashCount++;

  if (apple2->flashCount >= APPLE2_FPS / 4) {
    apple2->flashCount = 0;
    apple2->flashStatus = !apple2->flashStatus;
    apple2->dirty = 1;
  }

  if (apple2->dirty) {
    render(apple2, surface);
    apple2->dirty = 0;
  }

  unlock_surface(apple2);

  apple2_update(apple2, cycles);
}

#define black       0
#define purple      3
#define light_blue  7
#define orange      9
#define light_green 12
#define white       15

static void buildHColor(apple2_data_t *apple2) {
  uint32_t ba, b0, b1, b2, b3, b4, b5, b6, bd, c, nb1, nb2, b, index;
  int i, cs;
    
  index = 0;
    
  for (i = 0; i < 1024; i++) {
    ba =  i & 0x0001;
    b0 = (i & 0x0002) >> 1;
    b1 = (i & 0x0004) >> 2;
    b2 = (i & 0x0008) >> 3;
    b3 = (i & 0x0010) >> 4;
    b4 = (i & 0x0020) >> 5;
    b5 = (i & 0x0040) >> 6;
    b6 = (i & 0x0080) >> 7;
    cs = ((i & 0x0100) >> 8) == 1;
    bd = (i & 0x0200) >> 9;
    c = 0;

    // bit 0

    if (ba == 0 && b0 == 0 && b1 == 0) c = black;
    if (ba == 1 && b0 == 0 && b1 == 0) c = black;
    if (ba == 0 && b0 == 1 && b1 == 0) c = cs ? light_blue : purple;
    if (ba == 1 && b0 == 1 && b1 == 0) c = white;
    if (ba == 0 && b0 == 0 && b1 == 1) c = black;
    if (ba == 1 && b0 == 0 && b1 == 1) c = cs ? orange : light_green;
    if (ba == 0 && b0 == 1 && b1 == 1) c = white;
    if (ba == 1 && b0 == 1 && b1 == 1) c = white;
    nb1 = c;

    // bit 1

    if (b0 == 0 && b1 == 0 && b2 == 0) c = black;
    if (b0 == 1 && b1 == 0 && b2 == 0) c = black;
    if (b0 == 0 && b1 == 1 && b2 == 0) c = cs ? orange : light_green;
    if (b0 == 1 && b1 == 1 && b2 == 0) c = white;
    if (b0 == 0 && b1 == 0 && b2 == 1) c = black;
    if (b0 == 1 && b1 == 0 && b2 == 1) c = cs ? light_blue : purple;
    if (b0 == 0 && b1 == 1 && b2 == 1) c = white;
    if (b0 == 1 && b1 == 1 && b2 == 1) c = white;
    nb2 = c;
    b = (nb1 << 4) | nb2;
    apple2->hcolor[index++] = b;

    // bit 2

    if (b1 == 0 && b2 == 0 && b3 == 0) c = black;
    if (b1 == 1 && b2 == 0 && b3 == 0) c = black;
    if (b1 == 0 && b2 == 1 && b3 == 0) c = cs ? light_blue : purple;
    if (b1 == 1 && b2 == 1 && b3 == 0) c = white;
    if (b1 == 0 && b2 == 0 && b3 == 1) c = black;
    if (b1 == 1 && b2 == 0 && b3 == 1) c = cs ? orange : light_green;
    if (b1 == 0 && b2 == 1 && b3 == 1) c = white;
    if (b1 == 1 && b2 == 1 && b3 == 1) c = white;
    nb1 = c;

    // bit 3

    if (b2 == 0 && b3 == 0 && b4 == 0) c = black;
    if (b2 == 1 && b3 == 0 && b4 == 0) c = black;
    if (b2 == 0 && b3 == 1 && b4 == 0) c = cs ? orange : light_green;
    if (b2 == 1 && b3 == 1 && b4 == 0) c = white;
    if (b2 == 0 && b3 == 0 && b4 == 1) c = black;
    if (b2 == 1 && b3 == 0 && b4 == 1) c = cs ? light_blue : purple;
    if (b2 == 0 && b3 == 1 && b4 == 1) c = white;
    if (b2 == 1 && b3 == 1 && b4 == 1) c = white;
    nb2 = c;
    b = (nb1 << 4) | nb2;
    apple2->hcolor[index++] = b;

    // bit 4

    if (b3 == 0 && b4 == 0 && b5 == 0) c = black;
    if (b3 == 1 && b4 == 0 && b5 == 0) c = black;
    if (b3 == 0 && b4 == 1 && b5 == 0) c = cs ? light_blue : purple;
    if (b3 == 1 && b4 == 1 && b5 == 0) c = white;
    if (b3 == 0 && b4 == 0 && b5 == 1) c = black;
    if (b3 == 1 && b4 == 0 && b5 == 1) c = cs ? orange : light_green;
    if (b3 == 0 && b4 == 1 && b5 == 1) c = white;
    if (b3 == 1 && b4 == 1 && b5 == 1) c = white;
    nb1 = c;

    // bit 5

    if (b4 == 0 && b5 == 0 && b6 == 0) c = black;
    if (b4 == 1 && b5 == 0 && b6 == 0) c = black;
    if (b4 == 0 && b5 == 1 && b6 == 0) c = cs ? orange : light_green;
    if (b4 == 1 && b5 == 1 && b6 == 0) c = white;
    if (b4 == 0 && b5 == 0 && b6 == 1) c = black;
    if (b4 == 1 && b5 == 0 && b6 == 1) c = cs ? light_blue : purple;
    if (b4 == 0 && b5 == 1 && b6 == 1) c = white;
    if (b4 == 1 && b5 == 1 && b6 == 1) c = white;
    nb2 = c;
    b = (nb1 << 4) | nb2;
    apple2->hcolor[index++] = b;

    // bit 6
    
    if (b5 == 0 && b6 == 0 && bd == 0) c = black;
    if (b5 == 1 && b6 == 0 && bd == 0) c = black;
    if (b5 == 0 && b6 == 1 && bd == 0) c = cs ? light_blue : purple;
    if (b5 == 1 && b6 == 1 && bd == 0) c = white;
    if (b5 == 0 && b6 == 0 && bd == 1) c = black;
    if (b5 == 1 && b6 == 0 && bd == 1) c = cs ? orange : light_green;
    if (b5 == 0 && b6 == 1 && bd == 1) c = white;
    if (b5 == 1 && b6 == 1 && bd == 1) c = white;
    nb1 = c;

    nb2 = 0;
    b = (nb1 << 4) | nb2;
    apple2->hcolor[index++] = b;
  }
}

static void buildHiresColors(apple2_data_t *apple2) {
  int i, j;

  buildHColor(apple2);

  for (i = 0; i < 8192; i++) {
    apple2->appleEvenHiresColor[i] = apple2->c[0];
    apple2->appleOddHiresColor[i]  = apple2->c[0];
  }

  int k = 0;
  int m = 0;
  int low = 1;

  for (i = 0; i < 1024; i++) {
    for (j = 0; j < 8; j++) {
      uint8_t hc = apple2->hcolor[m];
      uint8_t nibble;
      if (low) {
        nibble = hc >> 4;
      } else {
        nibble = hc & 0x0F;
        m++;
      }

      apple2->appleEvenHiresColor[k] = apple2->c[nibble];
      apple2->appleOddHiresColor[k] = apple2->oc[nibble];

      k++;
      low = !low;
    }
  }
}

static int apple2_set_surface(struct computer_t *c, int ptr, surface_t *surface) {
  apple2_data_t *apple2;
  int i, j;

  apple2 = (apple2_data_t *)c->data;
  apple2->surface = surface;
  apple2->ptr = ptr;

  if (ptr) {
    surface = ptr_lock(ptr, TAG_SURFACE);
  }

  if (surface->encoding == SURFACE_ENCODING_PALETTE) {
    for (i = 0, j = 0; i < NUM_COLORS; i++) {
      surface_palette(surface, i, colors[j], colors[j+1], colors[j+2]);
      apple2->c[i] = i;
      apple2->oc[i] = apple2->c[i];
      j += 3;
    }
    surface->setarea(surface->data, 0, 0, surface->width-1, surface->height-1, 0);
  } else {
    for (i = 0, j = 0; i < NUM_COLORS; i++) {
      apple2->c[i] = surface_color_rgb(surface->encoding, NULL, 0, colors[j], colors[j+1], colors[j+2], 0xFF);
      apple2->oc[i] = apple2->c[i];
      j += 3;
    }
    surface->setarea(surface->data, 0, 0, surface->width-1, surface->height-1, apple2->c[0]);
  }
  apple2->oc[3]  = apple2->c[12];
  apple2->oc[12] = apple2->c[3];
  apple2->oc[7]  = apple2->c[9];
  apple2->oc[9]  = apple2->c[7];
  buildHiresColors(apple2);

  if (ptr) {
    ptr_unlock(ptr, TAG_SURFACE);
  }

  return 0;
}

computer_t *apple2_init(vfs_session_t *session) {
  apple2_data_t *apple2;
  computer_t *c = NULL;

  if ((apple2 = xcalloc(1, sizeof(apple2_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      apple2->session = session;
      apple2->cpu = m6502_open(APPLE2_PERIOD, 1, apple2_callback, apple2, c);
      sys_memset(&apple2->mem[0xC000], 0xFF, 0x4000);
      apple2->sw_text = 1;
      apple2->dirty = 1;
      apple2->boot = 1;
      c->set_surface = apple2_set_surface;
      c->disk = apple2_disk;
      c->rom = apple2_rom;
      c->run = apple2_run;
      c->close = apple2_close;
      c->getb = apple2_getb;
      c->putb = apple2_putb;
      c->data = apple2;

    } else {
      xfree(apple2);
    }
  }

  return c;
}
