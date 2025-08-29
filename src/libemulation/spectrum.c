#include "sys.h"
#include "vfs.h"
#include "ptr.h"
#include "filter.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "cas.h"
#include "rom.h"
#include "spectrumcas.h"
#include "pwindow.h"
#include "computer.h"
#include "z80.h"
#include "spectrum.h"
#include "debug.h"
#include "xalloc.h"

#define Z80_CLOCK     3500000
#define Z80_FPS       50
#define Z80_PERIOD    Z80_CLOCK/Z80_FPS

#define ROM_SIZE      0x4000
#define RAM_SIZE      0x10000
#define VRAM_SIZE     0x1B00

#define NUM_COLORS    16

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 192

typedef struct {
  z80_t *z;

  uint8_t ram[RAM_SIZE];
  uint8_t video[VRAM_SIZE];
  int borderColor;
  int updateBorder;
  int flashCount;
  int flashStatus;

  int ptr;
  surface_t *surface;
  uint8_t matrix[8];
  uint8_t modeE_state;
  uint8_t modeE_key;
  int finish;

  tape_t tape;
  int cas;

  vfs_session_t *session;
  uint32_t color[NUM_COLORS];
} spectrum_data_t;

static const uint8_t colors[] = {
  //  R G B
  0x00, 0x00, 0x00,
  0x00, 0x00, 0xbf,
  0xbf, 0x00, 0x00,
  0xbf, 0x00, 0xbf,
  0x00, 0xbf, 0x00,
  0x00, 0xbf, 0xbf,
  0xbf, 0xbf, 0x00,
  0xbf, 0xbf, 0xbf,
  0x00, 0x00, 0x00,
  0x00, 0x00, 0xff,
  0xff, 0x00, 0x00,
  0xff, 0x00, 0xff,
  0x00, 0xff, 0x00,
  0x00, 0xff, 0xff,
  0xff, 0xff, 0x00,
  0xff, 0xff, 0xff
};

static const uint32_t spectrumVideoAddr[] = {
  0x4000, 0x4100, 0x4200, 0x4300, 0x4400, 0x4500, 0x4600, 0x4700,
  0x4020, 0x4120, 0x4220, 0x4320, 0x4420, 0x4520, 0x4620, 0x4720,
  0x4040, 0x4140, 0x4240, 0x4340, 0x4440, 0x4540, 0x4640, 0x4740,
  0x4060, 0x4160, 0x4260, 0x4360, 0x4460, 0x4560, 0x4660, 0x4760,
  0x4080, 0x4180, 0x4280, 0x4380, 0x4480, 0x4580, 0x4680, 0x4780,
  0x40A0, 0x41A0, 0x42A0, 0x43A0, 0x44A0, 0x45A0, 0x46A0, 0x47A0,
  0x40C0, 0x41C0, 0x42C0, 0x43C0, 0x44C0, 0x45C0, 0x46C0, 0x47C0,
  0x40E0, 0x41E0, 0x42E0, 0x43E0, 0x44E0, 0x45E0, 0x46E0, 0x47E0,
  0x4800, 0x4900, 0x4A00, 0x4B00, 0x4C00, 0x4D00, 0x4E00, 0x4F00,
  0x4820, 0x4920, 0x4A20, 0x4B20, 0x4C20, 0x4D20, 0x4E20, 0x4F20,
  0x4840, 0x4940, 0x4A40, 0x4B40, 0x4C40, 0x4D40, 0x4E40, 0x4F40,
  0x4860, 0x4960, 0x4A60, 0x4B60, 0x4C60, 0x4D60, 0x4E60, 0x4F60,
  0x4880, 0x4980, 0x4A80, 0x4B80, 0x4C80, 0x4D80, 0x4E80, 0x4F80,
  0x48A0, 0x49A0, 0x4AA0, 0x4BA0, 0x4CA0, 0x4DA0, 0x4EA0, 0x4FA0,
  0x48C0, 0x49C0, 0x4AC0, 0x4BC0, 0x4CC0, 0x4DC0, 0x4EC0, 0x4FC0,
  0x48E0, 0x49E0, 0x4AE0, 0x4BE0, 0x4CE0, 0x4DE0, 0x4EE0, 0x4FE0,
  0x5000, 0x5100, 0x5200, 0x5300, 0x5400, 0x5500, 0x5600, 0x5700,
  0x5020, 0x5120, 0x5220, 0x5320, 0x5420, 0x5520, 0x5620, 0x5720,
  0x5040, 0x5140, 0x5240, 0x5340, 0x5440, 0x5540, 0x5640, 0x5740,
  0x5060, 0x5160, 0x5260, 0x5360, 0x5460, 0x5560, 0x5660, 0x5760,
  0x5080, 0x5180, 0x5280, 0x5380, 0x5480, 0x5580, 0x5680, 0x5780,
  0x50A0, 0x51A0, 0x52A0, 0x53A0, 0x54A0, 0x55A0, 0x56A0, 0x57A0,
  0x50C0, 0x51C0, 0x52C0, 0x53C0, 0x54C0, 0x55C0, 0x56C0, 0x57C0,
  0x50E0, 0x51E0, 0x52E0, 0x53E0, 0x54E0, 0x55E0, 0x56E0, 0x57E0
};

static const uint32_t spectrumVideoCAddr[] = {
  0x5800, 0x5800, 0x5800, 0x5800, 0x5800, 0x5800, 0x5800, 0x5800,
  0x5820, 0x5820, 0x5820, 0x5820, 0x5820, 0x5820, 0x5820, 0x5820,
  0x5840, 0x5840, 0x5840, 0x5840, 0x5840, 0x5840, 0x5840, 0x5840,
  0x5860, 0x5860, 0x5860, 0x5860, 0x5860, 0x5860, 0x5860, 0x5860,
  0x5880, 0x5880, 0x5880, 0x5880, 0x5880, 0x5880, 0x5880, 0x5880,
  0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0,
  0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0,
  0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0,
  0x5900, 0x5900, 0x5900, 0x5900, 0x5900, 0x5900, 0x5900, 0x5900,
  0x5920, 0x5920, 0x5920, 0x5920, 0x5920, 0x5920, 0x5920, 0x5920,
  0x5940, 0x5940, 0x5940, 0x5940, 0x5940, 0x5940, 0x5940, 0x5940,
  0x5960, 0x5960, 0x5960, 0x5960, 0x5960, 0x5960, 0x5960, 0x5960,
  0x5980, 0x5980, 0x5980, 0x5980, 0x5980, 0x5980, 0x5980, 0x5980,
  0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0,
  0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0,
  0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0,
  0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00,
  0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20,
  0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40,
  0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60,
  0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80,
  0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0,
  0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0,
  0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0
};

static const uint8_t spectrumInk[] = {
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15,
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15,
  8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15
};

static const uint8_t spectrumPaper[] = {
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,
  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13,
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,
  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13,
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,
  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13,
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,
  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13,
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15
};

static surface_t *lock_surface(spectrum_data_t *spectrum) {
  if (spectrum->surface) return spectrum->surface;
  if (spectrum->ptr > 0) return ptr_lock(spectrum->ptr, TAG_SURFACE);
  return NULL;
}

static void unlock_surface(spectrum_data_t *spectrum) {
  if (spectrum->ptr > 0) ptr_unlock(spectrum->ptr, TAG_SURFACE);
}

static uint8_t spectrum_getb(computer_t *c, uint16_t addr) {
  spectrum_data_t *spectrum;

  spectrum = (spectrum_data_t *)c->data;

  return spectrum->ram[addr];
}

static void render(spectrum_data_t *spectrum, surface_t *surface) {
  uint16_t addr, caddr;
  uint32_t c1, c2, color;
  int x, y, dirty = 0;

  if ((spectrum->flashCount & 1) == 0) {
    if (spectrum->flashCount == 24) {
      for (addr = 0x1800; !dirty && addr < 0x1B00; addr++) {
        if ((spectrum->ram[ROM_SIZE + addr] & 0x80) != 0 || spectrum->ram[ROM_SIZE + addr] != spectrum->video[addr]) {
          dirty = 1;
        }
      }
      for (addr = 0x0000; !dirty && addr < 0x1800; addr++) {
        if (spectrum->ram[ROM_SIZE + addr] != spectrum->video[addr]) {
          dirty = 1;
        }
      }
    } else {
      for (addr = 0x0000; !dirty && addr < 0x1B00; addr++) {
        if (spectrum->ram[ROM_SIZE + addr] != spectrum->video[addr]) {
          dirty = 1;
        }
      }
    }
  }

  if (dirty) {
    y = (surface->height - SCREEN_HEIGHT*2) / 2;

    for (int i = 0; i < 192; i++, y += 2) {
      addr = spectrumVideoAddr[i];
      caddr = spectrumVideoCAddr[i];
      x = (surface->width - SCREEN_WIDTH*2) / 2;

      for (int j = 0; j < 32; j++, addr++) {           
        uint8_t p = spectrum->ram[addr];
        uint8_t c = spectrum->ram[caddr + j];
        
        uint8_t pv = spectrum->video[addr - ROM_SIZE];
        uint8_t cv = spectrum->video[caddr - ROM_SIZE + j];
            
        if ((c & 0x80) == 0 && p == pv && c == cv) {
          //continue;
        }
     
        uint8_t ink = spectrumInk[c];
        uint8_t paper = spectrumPaper[c];
            
        if ((c & 0x80) != 0 && spectrum->flashStatus) {
          uint8_t aux = ink;
          ink = paper; 
          paper = aux | 0x08;
        }
        c1 = spectrum->color[ink];
        c2 = spectrum->color[paper];
      
        for (int q = 0; q < 16; q++, x++) {
          color = (p & 0x80) != 0 ? c1 : c2;
          surface->setpixel(surface->data, x, y, color);
          surface->setpixel(surface->data, x, y+1, color);
          if (q & 1) p <<= 1;
        }
      }
    }

    for (int i = 0; i < 0x1B00; i++) {
      spectrum->video[i] = spectrum->ram[ROM_SIZE + i];
    }

    surface_update(surface, 0, surface->height);
  }
    
  spectrum->flashCount++;

  if (spectrum->flashCount == 25) {
    spectrum->flashCount = 0;
    spectrum->flashStatus = !spectrum->flashStatus;
  }
}

static void spectrum_putb(computer_t *c, uint16_t addr, uint8_t b) {
  spectrum_data_t *spectrum;

  spectrum = (spectrum_data_t *)c->data;

  if (addr >= ROM_SIZE) {
    spectrum->ram[addr] = b;
  }
}

static void spectrum_out(computer_t *c, uint16_t port, uint16_t b) {
  spectrum_data_t *spectrum;
  uint8_t bcolor;

  spectrum = (spectrum_data_t *)c->data;
  debug(DEBUG_TRACE, "SPECTRUM", "out 0x%04X, 0x%02X", port, b);

  if ((port & 1) == 0) {
    bcolor = b & 0x07;
    if (spectrum->borderColor != bcolor) {
      spectrum->borderColor = bcolor;
      spectrum->updateBorder = 1;
    }
    //waveOutput((b & 0x08) != 0 ? 0 : 255);
  }
}

static uint16_t spectrum_in(computer_t *c, uint16_t port) {
  spectrum_data_t *spectrum;
  uint8_t b = 0;

  spectrum = (spectrum_data_t *)c->data;
  debug(DEBUG_TRACE, "SPECTRUM", "in 0x%04X", port);

  switch (port & 0xFF) {
    case 0xFE:
      switch (port >> 8) {
        case 0xFE: b = spectrum->matrix[0]; break;
        case 0xFD: b = spectrum->matrix[1]; break;
        case 0xFB: b = spectrum->matrix[2]; break;
        case 0xF7: b = spectrum->matrix[3]; break;
        case 0xEF: b = spectrum->matrix[4]; break;
        case 0xDF: b = spectrum->matrix[5]; break;
        case 0xBF: b = spectrum->matrix[6]; break;
        case 0x7F: b = spectrum->matrix[7]; break;
        default: b = 0xFF; break;
      }

/*
      if (spectrum_casInput(&spectrum->tape, z80_get_event_count(spectrum->z), Z80_CLOCK)) {
        b &= ~0x40;
      }

      if (spectrum->cas && casFinished(&spectrum->tape)) {
        spectrum->cas = 0;
      }
*/
      break;
  }

  return b;
}

static int spectrum_disk(computer_t *c, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  return -1;
}

static int spectrum_rom(computer_t *c, int num, uint32_t size, char *name) {
  spectrum_data_t *spectrum;
  int r = -1;

  spectrum = (spectrum_data_t *)c->data;

  if (num == 0) {
    r = load_rom(spectrum->session, name, size, spectrum->ram) != -1 ? 0 : -1;
  }

  return r;
}

static int spectrum_run(computer_t *c, uint32_t us) {
  spectrum_data_t *spectrum;

  spectrum = (spectrum_data_t *)c->data;
  if (spectrum->finish) return -1;

  return z80_loop(spectrum->z, (us * Z80_CLOCK) / 1000000);
}

static int spectrum_close(computer_t *c) {
  spectrum_data_t *spectrum;
  int r = -1;

  spectrum = (spectrum_data_t *)c->data;

  if (spectrum) {
    xfree(spectrum);
    r = 0;
  }
  xfree(c);

  return r;
}

#define K(m,bit) if (ev == WINDOW_KEYDOWN) spectrum->matrix[m] &= bit; else spectrum->matrix[m] |= (bit^0xFF)
#define K_SSHIFT(m,bit) K(m,bit); K(7,0xFD)
#define K_CSHIFT(m,bit) K(m,bit); K(0,0xFE)

#define K_KEY(m,k,bit) case k: K(m,bit); break
#define K_KEY_SSHIFT(m,k,bit) case k: K_SSHIFT(m,bit); break
#define K_KEY_CSHIFT(m,k,bit) case k: K_CSHIFT(m,bit); break
#define K_CSSHIFT case 5: K(7,0xFD); K(0,0xFE); break

#define K_KEY_CSSHIFT(k,k2) case k: spectrum->modeE_key = k2; K(7,0xFD); K(0,0xFE); spectrum->modeE_state++; break

#define K_LINE(m,a,b,c,d,e) \
  K_KEY(m,a,0xFE); \
  K_KEY(m,b,0xFD); \
  K_KEY(m,c,0xFB); \
  K_KEY(m,d,0xF7); \
  K_KEY(m,e,0xEF)

#define K_LINE_SSHIFT(m,a,b,c,d,e) \
  K_KEY_SSHIFT(m,a,0xFE); \
  K_KEY_SSHIFT(m,b,0xFD); \
  K_KEY_SSHIFT(m,c,0xFB); \
  K_KEY_SSHIFT(m,d,0xF7); \
  K_KEY_SSHIFT(m,e,0xEF)

#define K_LINE_CSHIFT(m,a,b,c,d,e) \
  K_KEY_CSHIFT(m,a,0xFE); \
  K_KEY_CSHIFT(m,b,0xFD); \
  K_KEY_CSHIFT(m,c,0xFB); \
  K_KEY_CSHIFT(m,d,0xF7); \
  K_KEY_CSHIFT(m,e,0xEF)

#define K_CS_SHIFT ((spectrum->matrix[0] & 0x02) && (spectrum->matrix[7] & 0x01))

static void spectrum_callback(void *data, uint32_t cycles) {
  spectrum_data_t *spectrum;
  int ev, arg1, arg2;

  spectrum = (spectrum_data_t *)data;

  surface_t *surface = lock_surface(spectrum);
  switch (spectrum->modeE_state) {
    case 0:
    case 1:
      ev = surface_event(surface, 0, &arg1, &arg2);
      break;
    case 2:
      ev = WINDOW_KEYDOWN;
      arg1 = spectrum->modeE_key;
      spectrum->modeE_state++;
      K(7,0xFD);
      break;
    case 3:
      ev = WINDOW_KEYUP;
      arg1 = spectrum->modeE_key;
      spectrum->modeE_state = 0;
      K(7,0xFD);
      break;
  }

  render(spectrum, surface);
  unlock_surface(spectrum);

  switch (ev) {
    case WINDOW_KEYDOWN:
    case WINDOW_KEYUP:
      switch (arg1) {
        K_CSSHIFT; // ctrl-e -> CAPS SHIFT + SYMBOL SHIFT

        K_KEY(0, 'z' ,0xFD);
        K_KEY(0, 'x' ,0xFB);
        K_KEY(0, 'c' ,0xF7);
        K_KEY(0, 'v' ,0xEF);
        K_LINE(1, 'a', 's', 'd', 'f', 'g');
        K_LINE(2, 'q', 'w', 'e', 'r', 't');
        K_LINE(3, '1', '2', '3', '4', '5');
        K_LINE(4, '0', '9', '8', '7', '6');
        K_LINE(5, 'p', 'o', 'i', 'u', 'y');
        K_LINE(6, 13, 'l', 'k', 'j', 'h');
        K_KEY(7, ' ' , 0xFE);
        K_KEY(7, WINDOW_KEY_LALT ,0xFD);
        K_KEY(7, 'm' , 0xFB);
        K_KEY(7, 'n' , 0xF7);
        K_KEY(7, 'b' , 0xEF);

        K_KEY_CSHIFT(0, 'Z', 0xFD);
        K_KEY_CSHIFT(0, 'X', 0xFB);
        K_KEY_CSHIFT(0, 'C', 0xF7);
        K_KEY_CSHIFT(0, 'V', 0xEF);
        K_LINE_CSHIFT(1, 'A', 'S', 'D', 'F', 'G');
        K_LINE_CSHIFT(2, 'Q', 'W', 'E', 'R', 'T');
        K_KEY_CSHIFT(3, WINDOW_KEY_LEFT, 0xEF);
        K_KEY_CSHIFT(4, 8, 0xFE); // backspace -> DELETE
        K_KEY_CSHIFT(4, 7, 0xFD); // ctrl-g -> GRAPHICS
        K_KEY_CSHIFT(4, WINDOW_KEY_RIGHT, 0xFB);
        K_KEY_CSHIFT(4, WINDOW_KEY_UP, 0xF7);
        K_KEY_CSHIFT(4, WINDOW_KEY_DOWN, 0xEF);
        K_LINE_CSHIFT(5, 'P', 'O', 'I', 'U', 'Y');
        K_KEY_CSHIFT(6, 'L', 0xFD);
        K_KEY_CSHIFT(6, 'K', 0xFB);
        K_KEY_CSHIFT(6, 'J', 0xF7);
        K_KEY_CSHIFT(6, 'H', 0xEF);
        K_KEY_CSHIFT(7, 'M', 0xFB);
        K_KEY_CSHIFT(7, 'N', 0xF7);
        K_KEY_CSHIFT(7, 'B', 0xEF);

        K_KEY_SSHIFT(0, ':', 0xFD);
        K_KEY_SSHIFT(0, '?', 0xF7);
        K_KEY_SSHIFT(0, '/', 0xEF);
        K_KEY_SSHIFT(2, '<', 0xF7);
        K_KEY_SSHIFT(2, '>', 0xEF);
        K_LINE_SSHIFT(3, '!', '@', '#', '$', '%');
        K_LINE_SSHIFT(4, '_', ')', '(', '\'', '&');
        K_KEY_SSHIFT(5, '"', 0xFE);
        K_KEY_SSHIFT(5, ';', 0xFD);
        K_KEY_SSHIFT(6, '=', 0xFD);
        K_KEY_SSHIFT(6, '+', 0xFB);
        K_KEY_SSHIFT(6, '-', 0xF7);
        K_KEY_SSHIFT(6, '^', 0xEF);
        K_KEY_SSHIFT(7, '.', 0xFB);
        K_KEY_SSHIFT(7, ',', 0xF7);
        K_KEY_SSHIFT(7, '*', 0xEF);

        K_KEY_CSSHIFT('[', 'y');
        K_KEY_CSSHIFT(']', 'u');
        K_KEY_CSSHIFT('{', 'f');
        K_KEY_CSSHIFT('}', 'g');
        K_KEY_CSSHIFT('|', 's');
        K_KEY_CSSHIFT('`', 'd');
        K_KEY_CSSHIFT('~', 'a');
      }
      break;
    case -1:
      spectrum->finish = 1;
      return;
  }

  z80_irq(spectrum->z);
}

static int spectrum_set_surface(struct computer_t *c, int ptr, surface_t *surface) {
  spectrum_data_t *spectrum;
  int i, j;

  spectrum = (spectrum_data_t *)c->data;
  spectrum->surface = surface;
  spectrum->ptr = ptr;

  if (ptr) {
    surface = ptr_lock(ptr, TAG_SURFACE);
  }

  if (surface->encoding == SURFACE_ENCODING_PALETTE) {
    for (i = 0, j = 0; i < NUM_COLORS; i++) {
      surface_palette(surface, i, colors[j], colors[j+1], colors[j+2]);
      spectrum->color[i] = i;
      j += 3;
    }
  } else {
    for (i = 0, j = 0; i < NUM_COLORS; i++) {
      spectrum->color[i] = surface_color_rgb(surface->encoding, NULL, 0, colors[j], colors[j+1], colors[j+2], 0xFF);
      j += 3;
    }
  }
  surface->setarea(surface->data, 0, 0, surface->width-1, surface->height-1, spectrum->color[0]);

  if (ptr) {
    ptr_unlock(ptr, TAG_SURFACE);
  }

  return 0;
}

computer_t *spectrum_init(vfs_session_t *session) {
  spectrum_data_t *spectrum;
  computer_t *c = NULL;
  int i;

  if ((spectrum = xcalloc(1, sizeof(spectrum_data_t))) != NULL) {
    if ((c = xcalloc(1, sizeof(computer_t))) != NULL) {
      spectrum->z = z80_open(Z80_PERIOD, 1, spectrum_callback, spectrum, c);
      spectrum->borderColor = 0;
      spectrum->updateBorder = 1;
      spectrum->session = session;
      for (i = 0; i < 8; i++) spectrum->matrix[i] = 0xFF;

      c->set_surface = spectrum_set_surface;
      c->disk = spectrum_disk;
      c->rom = spectrum_rom;
      c->run = spectrum_run;
      c->close = spectrum_close;
      c->getb = spectrum_getb;
      c->getop = spectrum_getb;
      c->putb = spectrum_putb;
      c->out = spectrum_out;
      c->in = spectrum_in;
      c->data = spectrum;

    } else {
      xfree(spectrum);
    }
  }

  return c;
}
