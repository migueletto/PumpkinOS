#include <PalmOS.h>

#include "thread.h"
#include "vfs.h"
#include "filter.h"
#include "computer.h"
#include "surface.h"
#include "emulator.h"
#include "debug.h"

#define MAX_ROM 4

typedef struct {
  vfs_session_t *session;
  surface_t *surface;
  UInt16 numRoms;
  MemHandle hrom[MAX_ROM];
  UInt8 *rom[MAX_ROM];
  uint32_t keyMask, modMask;
  uint64_t extKeyMask[2];
} emulator_data_t;

typedef struct vfs_fpriv_t {
  emulator_data_t *data;
  uint32_t size, pos;
  int i;
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
        f->data = data;
        f->size = MemHandleSize(data->hrom[id]);
        f->pos = 0;
        f->i = id;
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
      debug(DEBUG_INFO, "Emulator", "read f=%p i=%d pos=%u size=%u len=%u", f, f->i, f->pos, f->size, len);
      sys_memcpy(buf, f->data->rom[f->i] + f->pos, len);
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

static int process_ext_keys(uint64_t oldMask, uint64_t newMask, int offset, int *key) {
  uint64_t diff;
  int i, ev = 0;

  diff = oldMask ^ newMask;
  if (diff) {
    for (i = 0; i < 64; i++) {
      if (diff & 1) {
        debug(DEBUG_TRACE, "Emulator", "key %d %d", offset + i, (newMask & 1) ? 1 : 0);
        *key = offset + i;
        ev = (newMask & 1) ? WINDOW_KEYDOWN : WINDOW_KEYUP;
        break;
      }
      newMask >>= 1;
      diff >>= 1;
    }
  }

  return ev;
}

static int emulator_surface_event(void *_data, uint32_t us, int *arg1, int *arg2) {
  emulator_data_t *data = (emulator_data_t *)_data;
  EventType event;
  uint32_t keyMask, modMask;
  uint64_t extKeyMask[2];
  int ev = 0;

  EvtGetEventUs(&event, us);
  SysHandleEvent(&event);

  if (event.eType == appStopEvent) {
    ev = -1;
  } else {
    pumpkin_status(NULL, NULL, &keyMask, &modMask, NULL, extKeyMask);
    ev = process_ext_keys(data->extKeyMask[0], extKeyMask[0], 0, arg1);
    data->extKeyMask[0] = extKeyMask[0];
    if (ev == 0) {
      ev = process_ext_keys(data->extKeyMask[1], extKeyMask[1], 64, arg1);
    }
    data->extKeyMask[1] = extKeyMask[1];
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
  MemHandle h;
  computer_init cinit;
  computer_t *c;
  UInt32 size, width, height;
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

                  for (data.numRoms = 0; data.numRoms < MAX_ROM; data.numRoms++) {
                    data.hrom[data.numRoms] = DmGet1Resource('pROM', data.numRoms);
                    if (data.hrom[data.numRoms] == NULL) break;
                    data.rom[data.numRoms] = MemHandleLock(data.hrom[data.numRoms]);
                  }

                  for (i = 0; i < data.numRoms; i++) {
                    size = MemHandleSize(data.hrom[i]);
                    sys_snprintf(name, sizeof(name)-1, "/emulator/pROM.%u", i);
                    c->rom(c, i, size, name);
                  }

                  for (; !thread_must_end();) {
                    if (c->run(c, 0) != 0) break;
                  }

                  for (i = 0; i < data.numRoms; i++) {
                    MemHandleUnlock(data.hrom[i]);
                    DmReleaseResource(data.hrom[i]);
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
