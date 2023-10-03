#include "sys.h"
#include "script.h"
#include "thread.h"
#include "vfs.h"
#include "ptr.h"
#include "filter.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "computer.h"
#include "disk.h"
#include "cz80.h"
#include "trs80.h"
#include "spectrum.h"
#include "zx81.h"
#include "apple2.h"
#include "coco.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_COMPUTER  "COMPUTER"

typedef computer_t *(*computer_init)(vfs_session_t *session);

static void computer_destructor(void *p) {
  computer_t *c;

  if (p) {
    debug(DEBUG_INFO, "EMU", "destroying computer");
    c = (computer_t *)p;
    c->close(c);
  }

  vfs_refresh();
}

static int libemulation_create(int pe) {
  vfs_session_t *session;
  computer_init cinit;
  computer_t *c;
  script_int_t fullscreen, surface;
  int ptr = -1;
  char *machine = NULL;

  if (script_get_string(pe,  0, &machine) == 0 &&
      script_get_integer(pe, 1, &fullscreen) == 0 &&
      script_get_integer(pe, 2, &surface) == 0) {

    if ((session = vfs_open_session()) != NULL) {
      if ((cinit = script_get_pointer(pe, machine)) != NULL) {
        if ((c = cinit(session)) != NULL) {
          if (surface > 0 && c->set_surface) {
            c->set_surface(c, surface, NULL);
          }
          c->tag = TAG_COMPUTER;
          ptr = ptr_new(c, computer_destructor);
        }
      }
    }
  }

  if (machine) xfree(machine);

  return ptr != -1 ? script_push_integer(pe, ptr) : -1;
}

static int libemulation_disk(int pe) {
  script_int_t ptr, drive, skip, tracks, heads, sectors, sectorlen, sector0;
  unsigned int disk_skip, disk_tracks, disk_heads, disk_sectors, disk_sectorlen, disk_sector0;
  computer_t *c;
  char *disk = NULL;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &drive) == 0 &&
      script_get_integer(pe, 2, &skip) == 0 &&
      script_get_integer(pe, 3, &tracks) == 0 &&
      script_get_integer(pe, 4, &heads) == 0 &&
      script_get_integer(pe, 5, &sectors) == 0 &&
      script_get_integer(pe, 6, &sectorlen) == 0 &&
      script_get_integer(pe, 7, &sector0) == 0 &&
      script_get_string(pe,  8, &disk) == 0) {

    disk_skip = skip;
    disk_tracks = tracks;
    disk_heads = heads;
    disk_sectors = sectors;
    disk_sectorlen = sectorlen;
    disk_sector0 = sector0;

    if ((c = ptr_lock(ptr, TAG_COMPUTER)) != NULL) {
      if (c->disk) {
        if (c->disk(c, drive, disk_skip, disk_tracks, disk_heads, disk_sectors, disk_sectorlen, disk_sector0, disk) == 0) {
          debug(DEBUG_INFO, "EMU", "using \"%s\" in drive %d", disk, drive);
          r = 0;
        } else {
          debug(DEBUG_ERROR, "EMU", "error using \"%s\" in drive %d", disk, drive);
        }
      }
      ptr_unlock(ptr, TAG_COMPUTER);
    }
  }

  return script_push_boolean(pe, r == 0);
}

static int libemulation_rom(int pe) {
  script_int_t ptr, num, size;
  computer_t *c;
  char *name = NULL;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &num) == 0 &&
      script_get_integer(pe, 2, &size) == 0 &&
      script_get_string(pe,  3, &name) == 0) {

    if ((c = ptr_lock(ptr, TAG_COMPUTER)) != NULL) {
      if (c->rom) {
        if (c->rom(c, num, size, name) == 0) {
          debug(DEBUG_INFO, "EMU", "using \"%s\"", name);
          r = 0;
        } else {
          debug(DEBUG_ERROR, "EMU", "error using \"%s\"", name);
        }
      }
      ptr_unlock(ptr, TAG_COMPUTER);
    }
  }

  if (name) xfree(name);

  return script_push_boolean(pe, r == 0);
}

static int libemulation_option(int pe) {
  script_int_t ptr;
  computer_t *c;
  char *name = NULL;
  char *value = NULL;
  void *p;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_string(pe,  1, &name) == 0 &&
      script_get_string(pe,  2, &value) == 0) {

    if ((c = ptr_lock(ptr, TAG_COMPUTER)) != NULL) {
      if (c->option) {
        if (!sys_strcmp(name, "_pointer")) {
          p = script_get_pointer(pe, value);
          r = c->option(c, value, p);
        } else {
          r = c->option(c, name, value);
        }
        if (r == 0) {
          debug(DEBUG_INFO, "EMU", "option \"%s\" = \"%s\"", name, value);
        } else {
          debug(DEBUG_ERROR, "EMU", "error setting option \"%s\" = \"%s\"", name, value);
        }
      }
      ptr_unlock(ptr, TAG_COMPUTER);
    }
  }

  if (name) xfree(name);
  if (value) xfree(value);

  return script_push_boolean(pe, r == 0);
}

static int libemulation_run(int pe) {
  script_int_t ptr;
  computer_t *c;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if ((c = ptr_lock(ptr, TAG_COMPUTER)) != NULL) {
      debug(DEBUG_INFO, "EMU", "emulation started");
      for (; !thread_must_end();) {
        if (c->run(c, 0) != 0) break;
      }
      debug(DEBUG_INFO, "EMU", "emulation finished");
      ptr_unlock(ptr, TAG_COMPUTER);
      r = ptr_free(ptr, TAG_COMPUTER);
    }
  }

  return script_push_boolean(pe, r == 0);
}

int libemulation_init(int pe, script_ref_t obj) {
  script_set_pointer(pe, "cz80",     cz80_init);
  script_set_pointer(pe, "trs80",    trs80_init);
  script_set_pointer(pe, "zx81",     zx81_init);
  script_set_pointer(pe, "spectrum", spectrum_init);
  script_set_pointer(pe, "apple2",   apple2_init);
  script_set_pointer(pe, "coco",     coco_init);

  script_add_function(pe, obj, "create", libemulation_create);
  script_add_function(pe, obj, "disk",   libemulation_disk);
  script_add_function(pe, obj, "rom",    libemulation_rom);
  script_add_function(pe, obj, "option", libemulation_option);
  script_add_function(pe, obj, "run",    libemulation_run);

  return 0;
}
