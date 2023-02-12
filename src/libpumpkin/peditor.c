#include <PalmOS.h>
#include <VFSMgr.h>
#include <time.h>

#include "editor.h"
#include "pterm.h"
#include "peditor.h"
#include "pumpkin.h"

static void *vfs_editor_fopen(void *data, char *name, int writing) {
  FileRef f = NULL;
  Err err = vfsErrBadName;

  if (name) {
    if (writing) {
      if (VFSFileOpen(1, name, vfsModeWrite, &f) != errNone) {
        err = VFSFileCreate(1, name);
      }
      err = VFSFileOpen(1, name, vfsModeWrite, &f);
    } else {
      err = VFSFileOpen(1, name, vfsModeRead, &f);
    }
  }

  return err == errNone ? f : NULL;
}

static int vfs_editor_fclose(void *data, void *_f) {
  FileRef f;
  int r = -1;

  if (_f) {
    f = (FileRef)_f;
    r = (VFSFileClose(f) == errNone) ? 0 : -1;
  }

  return r;
}

static int vfs_editor_fread(void *data, void *_f, void *buf, int n) {
  FileRef f;
  uint32_t nread;
  int r = -1;

  if (_f && buf) {
    f = (FileRef)_f;
    if (VFSFileRead(f, n, buf, &nread) == errNone) {
      r = nread;
    }
  }

  return r;
}

static int vfs_editor_fwrite(void *data, void *_f, void *buf, int n) {
  FileRef f;
  uint32_t nwritten;
  int r = -1;

  if (_f && buf) {
    f = (FileRef)_f;
    if (VFSFileWrite(f, n, buf, &nwritten) == errNone) {
      r = nwritten;
    }
  }

  return r;
}

static int vfs_editor_fsize(void *data, void *_f) {
  FileRef f;
  uint32_t size;
  int r = -1;

  if (_f) {
    f = (FileRef)_f;
    if (VFSFileSize(f, &size) == errNone) {
      r = size;
    }
  }

  return r;
}

static int vfs_editor_seek(void *data, void *_f, int orig, int offset) {
  FileRef f;
  FileOrigin origin;
  Boolean ok = true;
  int r = -1;

  if (_f) {
    f = (FileRef)_f;
    switch (orig) {
      case  0: origin = vfsOriginBeginning; break;
      case -1: origin = vfsOriginCurrent; break;
      case  1: origin = vfsOriginEnd; break;
      default: ok = false; break;
    }
    if (ok && VFSFileSeek(f, origin, offset) == errNone) {
      r = 0;
    }
  }

  return r;
}

static int pterm_editor_cursor(void *data, int col, int row) {
  pterm_t *t = (pterm_t *)data;
  uint8_t cols, rows;

  pterm_getsize(t, &cols, &rows);

  if (col >= 0 && col < cols) {
    pterm_setx(t, col);
  }

  if (row >= 0 && row < rows) {
    pterm_sety(t, row);
  }

  return 0;
}

static int pterm_editor_cls(void *data) {
  pterm_t *t = (pterm_t *)data;

  pterm_cls(t);

  return 0;
}

static int pterm_editor_clreol(void *data) {
  pterm_t *t = (pterm_t *)data;

  pterm_clreol(t);

  return 0;
}

static int pterm_editor_peek(void *data, int ms) {
  return EvtPumpEvents(ms < 0 ? evtWaitForever : ms*1000) == 0 ? 0 : 1;
}

static int pterm_editor_read(void *data, uint16_t *c) {
  pterm_t *t = (pterm_t *)data;
  EventType event;
  int r = 0;

  pterm_cursor_blink(t);
  EvtGetEvent(&event, SysTicksPerSecond() / 2);

  if (!SysHandleEvent(&event)) {
    switch (event.eType) {
      case keyDownEvent:
        *c = event.data.keyDown.chr;
        r = 1;
        break;
      case appStopEvent:
        r = -1;
        break;
      default:
        break;
    }
  }

  return r;
}

static int pterm_editor_write(void *data, char *buf, int len) {
  pterm_t *t = (pterm_t *)data;

  pterm_send(t, (uint8_t *)buf, len);

  return 0;
}

static int pterm_editor_color(void *data, uint32_t fg, uint32_t bg) {
  pterm_t *t = (pterm_t *)data;

  if (fg != -1) pterm_setfg(t, fg);
  if (bg != -1) pterm_setbg(t, bg);

  return 0;
}

static int pterm_editor_window(void *data, int *ncols, int *nrows) {
  pterm_t *t = (pterm_t *)data;
  uint8_t cols, rows;

  pterm_getsize(t, &cols, &rows);
  if (ncols) *ncols = cols;
  if (nrows) *nrows = rows;

  return 0;
}

int pumpkin_editor_init(editor_t *e, pterm_t *t) {
  int r = -1;

  if (e) {
    e->data = t;
    e->cursor = pterm_editor_cursor;
    e->cls = pterm_editor_cls;
    e->clreol = pterm_editor_clreol;
    e->peek = pterm_editor_peek;
    e->read = pterm_editor_read;
    e->write = pterm_editor_write;
    e->color = pterm_editor_color;
    e->window = pterm_editor_window;

    e->fopen = vfs_editor_fopen;
    e->fclose = vfs_editor_fclose;
    e->fread = vfs_editor_fread;
    e->fwrite = vfs_editor_fwrite;
    e->fsize = vfs_editor_fsize;
    e->seek = vfs_editor_seek;
    r = 0;
  }

  return r;
}
