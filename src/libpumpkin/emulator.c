#include <PalmOS.h>

#include "thread.h"
#include "vfs.h"
#include "filter.h"
#include "computer.h"
#include "surface.h"
#include "emulator.h"
#include "bytes.h"
#include "debug.h"

#define MAX_ROMS   4
#define MAX_EVENTS 32

typedef struct {
  int ev, arg1, arg2;
} evt_t;

typedef struct {
  vfs_session_t *session;
  surface_t *surface;
  UInt16 numRoms;
  MemHandle hrom[MAX_ROMS];
  UInt8 *rom[MAX_ROMS];
  UInt16 numRams;
  MemHandle hram[MAX_ROMS];
  UInt8 *ram[MAX_ROMS];
  uint32_t keyMask, modMask, buttonMask;
  uint64_t extKeyMask[2];
  int x, y;
  evt_t events[MAX_EVENTS];
  uint32_t nevt, ievt, oevt;
} emulator_data_t;

typedef struct vfs_fpriv_t {
  uint32_t size, pos;
  uint8_t *buf;
} vfs_fpriv_t;

typedef computer_t *(*computer_init)(vfs_session_t *session);

static int fake_checktype(char *path, void *data) {
  return VFS_FILE;
}

static vfs_fpriv_t *fake_open(char *path, int mode, void *_data) {
  emulator_data_t *data = (emulator_data_t *)_data;
  vfs_fpriv_t *f = NULL;
  int id;

  if (sys_sscanf(path, "/pROM.%d", &id) == 1) {
    if (id >= 0 && id < data->numRoms && data->rom[id]) {
      if ((f = sys_calloc(1, sizeof(vfs_fpriv_t))) != NULL) {
        f->size = MemHandleSize(data->hrom[id]);
        f->buf = data->rom[id];
        f->pos = 0;
      }
    }
  } else if (sys_sscanf(path, "/pRAM.%d", &id) == 1) {
    if (id > 0 && id <= data->numRams && data->ram[id-1]) {
      if ((f = sys_calloc(1, sizeof(vfs_fpriv_t))) != NULL) {
        f->size = MemHandleSize(data->hram[id-1]);
        f->buf = data->ram[id-1];
        f->pos = 0;
      }
    }
  }

  debug(DEBUG_INFO, "Emulator", "open name=\"%s\" f=%p", path, f);
  return f;
}

static int fake_read(vfs_fpriv_t *f, uint8_t *buf, uint32_t len) {
  int r = -1;

  if (f && buf) {
    if (f->pos + len > f->size) {
      len = f->size - f->pos;
    }
    if (len) {
      debug(DEBUG_INFO, "Emulator", "read f=%p pos=%u size=%u len=%u", f, f->pos, f->size, len);
      sys_memcpy(buf, f->buf + f->pos, len);
      f->pos += len;
    }
    r = len;
  }

  return r;
}

static int fake_write(vfs_fpriv_t *f, uint8_t *buf, uint32_t len) {
  debug(DEBUG_INFO, "Emulator", "write f=%p len=%u", f, len);
  return -1;
}

static int fake_close(vfs_fpriv_t *f) {
  int r = -1;

  debug(DEBUG_INFO, "Emulator", "close f=%p", f);
  if (f) {
    sys_free(f);
  }

  return r;
}

static uint32_t fake_seek(vfs_fpriv_t *f, uint32_t pos, int fromend) {
  debug(DEBUG_INFO, "Emulator", "seek %p pos %u whence %d", f, pos, fromend);
  return -1;
}

static vfs_callback_t fake_callback = {
  NULL,
  NULL,
  NULL,
  fake_checktype,
  NULL,
  NULL,
  NULL,
  NULL,
  fake_open,
  NULL,
  fake_read,
  fake_write,
  fake_close,
  fake_seek,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static void put_evt(emulator_data_t *data, int ev, int arg1, int arg2) {
  if (data->nevt < MAX_EVENTS) {
    data->events[data->ievt].ev = ev;
    data->events[data->ievt].arg1 = arg1;
    data->events[data->ievt].arg2 = arg2;
    data->ievt++;
    if (data->ievt == MAX_EVENTS) data->ievt = 0;
    data->nevt++;
  }
}

static int get_evt(emulator_data_t *data, int *arg1, int *arg2) {
  int ev = 0;

  if (data->nevt > 0) {
    ev = data->events[data->oevt].ev;
    *arg1 = data->events[data->oevt].arg1;
    *arg2 = data->events[data->oevt].arg2;
    data->oevt++;
    if (data->oevt == MAX_EVENTS) data->oevt = 0;
    data->nevt--;
  }

  return ev;
}

static void process_normal_keys(emulator_data_t *data, uint32_t keyMask) {
  uint32_t diff;
  int arg1, arg2;

  diff = data->keyMask ^ keyMask;

  if (diff) {
    if (diff & keyBitPageDown) {
      if (keyMask & keyBitPageDown) {
        arg2 = data->surface->height - 1;
      } else {
        arg2 = data->surface->height / 2 + data->surface->height / 4;
      }
      arg1 = data->surface->width / 2;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }
    if (diff & keyBitPageUp) {
      if (keyMask & keyBitPageUp) {
        arg2 = 0;
      } else { 
        arg2 = data->surface->height / 2 - data->surface->height / 4;
      }
      arg1 = data->surface->width / 2;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }
    if (diff & keyBitLeft) {
      if (keyMask & keyBitLeft) {
        arg1 = 0;
      } else {
        arg1 = data->surface->width / 2 - data->surface->width / 4;
      }
      arg2 = data->surface->height / 2;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }
    if (diff & keyBitRight) {
      if (keyMask & keyBitRight) {
        arg1 = data->surface->width - 1;
      } else {
        arg1 = data->surface->width / 2 + data->surface->width / 4;
      }
      arg2 = data->surface->height / 2;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }

  } else {
    if (keyMask & keyBitPageDown) {
      arg1 = data->surface->width / 2;
      arg2 = data->surface->height - 1;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }
    if (keyMask & keyBitPageUp) {
      arg1 = data->surface->width / 2;
      arg2 = 0;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }
    if (keyMask & keyBitLeft) {
      arg1 = 0;
      arg2 = data->surface->height / 2;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }
    if (keyMask & keyBitRight) {
      arg1 = data->surface->width - 1;
      arg2 = data->surface->height / 2;
      put_evt(data, WINDOW_MOTION, arg1, arg2);
    }
  }

  data->keyMask = keyMask;
}

static void process_ext_keys(emulator_data_t *data, uint64_t *extKeyMask) {
  uint64_t diff, mask;
  int offset, i, j;

  for (j = 0, offset = 0; j < 2; j++, offset += 64) {
    mask = extKeyMask[j];
    diff = data->extKeyMask[j] ^ mask;
    if (diff) {
      for (i = 0; i < 64; i++) {
        if (diff & 1) {
          put_evt(data, (mask & 1) ? WINDOW_KEYDOWN : WINDOW_KEYUP, offset + i, 0);
          break;
        }
        mask >>= 1;
        diff >>= 1;
      }
    }
    data->extKeyMask[j] = extKeyMask[j];
  }
}

static void process_mod_keys(emulator_data_t *data, uint32_t modMask) {
  uint32_t diff;

  diff = data->modMask ^ modMask;
  if (diff) {
    if (diff & WINDOW_MOD_CTRL) {
      put_evt(data, (modMask & WINDOW_MOD_CTRL) ? WINDOW_BUTTONDOWN : WINDOW_BUTTONUP, 1, 0);
    }
  }

  data->modMask = modMask;
}

static int emulator_surface_event(void *_data, uint32_t us, int *arg1, int *arg2) {
  emulator_data_t *data = (emulator_data_t *)_data;
  EventType event;
  uint32_t keyMask, modMask, buttonMask;
  uint64_t extKeyMask[2];
  int x, y, ev;

  EvtGetEventUs(&event, us);
  SysHandleEvent(&event);

  if (event.eType == appStopEvent) {
    ev = -1;
  } else {
    pumpkin_status(&x, &y, &keyMask, &modMask, &buttonMask, extKeyMask);
    process_normal_keys(data, keyMask);
    process_ext_keys(data, extKeyMask);
    process_mod_keys(data, modMask);

    if (data->buttonMask != buttonMask) {
      put_evt(data, (buttonMask & 1) ? WINDOW_BUTTONDOWN : WINDOW_BUTTONUP, 1, 0);
      data->buttonMask = buttonMask;
    }

    if (data->x != x || data->y != y) {
      put_evt(data, WINDOW_MOTION, x, y);
      data->x = x;
      data->y = y;
    }

    ev = get_evt(data, arg1, arg2);
  }

  return ev;
}

static void emulator_surface_update(void *_data, int y0, int y1) {
  emulator_data_t *data = (emulator_data_t *)_data;
  void *buf;
  int len;
  
  buf = data->surface->getbuffer(data->surface->data, &len);
  pumpkin_screen_copy(buf, y0, y1);
}

UInt32 EmulatorMain(void) {
  emulator_data_t data;
  MemHandle h, hram0;
  computer_init cinit;
  computer_t *c;
  UInt32 size, width, height;
  UInt8 *ram0;
  UInt16 load_addr, exec_addr;
  void *lib;
  char *s, name[64];
  int first_load;
  UInt32 i, r = -1;

  MemSet(&data, sizeof(emulator_data_t), 0);

  if ((lib = sys_lib_load("libemulation", &first_load)) != NULL) {
    if ((h = DmGet1Resource('pEMU', 1)) != NULL) {
      if ((s = MemHandleLock(h)) != NULL) {
        sys_snprintf(name, sizeof(name)-1, "%s_init", s);
        if ((cinit = sys_lib_defsymbol(lib, name, 1)) != NULL) {
          if (vfs_map("fake", "/emulator", &data, &fake_callback, 0) == 0) {
            if ((data.session = vfs_open_session()) != NULL) {
              if ((c = cinit(data.session)) != NULL && c->set_surface != NULL) {
                WinScreenGetAttribute(winScreenWidth, &width);
                WinScreenGetAttribute(winScreenHeight, &height);

                if ((data.surface = surface_create(width, height, pumpkin_get_encoding())) != NULL) {
                  data.surface->event = emulator_surface_event;
                  data.surface->update = emulator_surface_update;
                  data.surface->udata = &data;
                  c->set_surface(c, 0, data.surface);

                  for (data.numRoms = 0; data.numRoms < MAX_ROMS; data.numRoms++) {
                    data.hrom[data.numRoms] = DmGet1Resource('pROM', data.numRoms);
                    if (data.hrom[data.numRoms] == NULL) break;
                    data.rom[data.numRoms] = MemHandleLock(data.hrom[data.numRoms]);
                  }

                  for (i = 0; i < data.numRoms; i++) {
                    size = MemHandleSize(data.hrom[i]);
                    sys_snprintf(name, sizeof(name)-1, "/emulator/pROM.%u", i);
                    c->rom(c, i, size, name);
                    MemHandleUnlock(data.hrom[i]);
                    DmReleaseResource(data.hrom[i]);
                  }

                  if ((hram0 = DmGet1Resource('pRAM', 0)) != NULL) {
                    size = MemHandleSize(hram0);
                    if (size > 0 && (size % 4) == 0 && (ram0 = MemHandleLock(hram0)) != NULL) {
                      for (data.numRams = 0; data.numRams < MAX_ROMS; data.numRams++) {
                        data.hram[data.numRams] = DmGet1Resource('pRAM', data.numRams+1);
                        if (data.hram[data.numRams] == NULL) break;
                        data.ram[data.numRams] = MemHandleLock(data.hram[data.numRams]);
                      }
                      for (i = 0; i < data.numRams; i++) {
                        size = MemHandleSize(data.hram[i]);
                        sys_snprintf(name, sizeof(name)-1, "/emulator/pRAM.%u", i+1);
                        get2b(&load_addr, ram0, i*4);
                        get2b(&exec_addr, ram0, i*4 + 2);
                        c->ram(c, load_addr, size, exec_addr, name);
                        MemHandleUnlock(data.hram[i]);
                        DmReleaseResource(data.hram[i]);
                      }
                      MemHandleUnlock(hram0);
                    }
                    DmReleaseResource(hram0);
                  }

                  for (; !thread_must_end();) {
                    if (c->run(c, 10000) != 0) break;
                  }
                  surface_destroy(data.surface);
                }
              }
              vfs_close_session(data.session);
            }
          }
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
  }

  return r;
}
