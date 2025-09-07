#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "pfont.h"
#include "graphic.h"
#include "ptr.h"
#include "vfs.h"
#include "util.h"
#include "findargs.h"
#include "filter.h"
#include "shell.h"
#include "telnet.h"
#include "pterm.h"
#include "editor.h"
#include "peditor.h"
#include "edit.h"
#include "pumpkin.h"
#include "storage.h"
#include "debug.h"
#include "xalloc.h"
#include "color.h"
#include "fill.h"
#include "file.h"
#include "deploy.h"
#include "wav.h"
#include "command.h"
#include "plibc.h"
#include "resource.h"

#define AppID    'CmdP'

#define TAG_LDB  "cmd_ldb"
#define TAG_DIR  "cmd_dir"
#define TAG_DB   "cmd_db"
#define TAG_RSRC "cmd_rsrc"

#define MAX_EXTERNAL_CMDS 64
#define MAXCMD 512

#define SCREEN_PEN_DOWN 1
#define SCREEN_PEN_MOVE 2
#define SCREEN_PEN_UP   3

#define COLS 80
#define ROWS 25

typedef struct {
  char *tag;
  DmOpenRef dbRef;
} command_db_t;

typedef struct {
  char *tag;
  UInt32 type;
  UInt16 id;
  MemHandle h;
} command_resource_t;

typedef struct {
  UInt16 font;
  RGBColorType foreground;
  RGBColorType background;
  RGBColorType highlight;
} command_prefs_t;

typedef struct {
  char *tag;
  FileInfoType info;
  FileRef f;
  UInt32 op;
  char buf[MAXCMD];
} command_dir_t;

typedef struct {
  char *tag;
  DmSearchStateType stateInfo;
  UInt32 type, creator;
  Boolean first;
} command_ldb_t;

typedef struct {
  FileRef fileRef;
  UInt32 sampleSize;
} command_play_t;

typedef struct {
  mutex_t *mutex;
  LocalID dbID;
  int (*cmain)(int argc, char *argv[]);
  char name[dmDBNameLength];
  void *lib;
} command_ext_t;

struct command_internal_data_t {
  int pe;
  Int32 wait;
  UInt16 volume;
  UInt16 blink;
  char cmd[MAXCMD];
  UInt16 cmdIndex;
  FontID font;
  UInt16 fwidth, fheight;
  UInt16 ncols, nrows;
  pterm_t *t;
  pterm_callback_t cb;
  char cwd[MAXCMD];
  conn_filter_t *telnet;
  Coord col0, col1, row0, row1;
  Boolean moved, selected, down;
  SndStreamRef sound;
  command_prefs_t prefs;
  FileRef in, out;
  command_ext_t ext_commands[MAX_EXTERNAL_CMDS];
};

static const RGBColorType defaultForeground = { 0, 0xFF, 0xFF, 0xFF };
static const RGBColorType defaultBackground = { 0, 0x13, 0x32, 0x65 };
static const RGBColorType defaultHighlight  = { 0, 0xFF, 0xFF, 0x80 };

static int command_script_tid(int pe);
static int command_script_cls(int pe);
static int command_script_print(int pe);
static int command_script_gets(int pe);
static int command_script_cd(int pe);
static int command_script_opendb(int pe);
static int command_script_readdb(int pe);
static int command_script_closedb(int pe);
static int command_script_opendir(int pe);
static int command_script_readdir(int pe);
static int command_script_closedir(int pe);
static int command_script_launch(int pe);
static int command_script_ps(int pe);
static int command_script_kill(int pe);
static int command_script_cat(int pe);
static int command_script_rm(int pe);
static int command_script_run(int pe);
static int command_script_deploy(int pe);
static int command_script_edit(int pe);
static int command_script_telnet(int pe);
static int command_script_exit(int pe);
static int command_script_pit(int pe);
static int command_script_play(int pe);
static int command_script_crash(int pe);
static int command_script_setio(int pe);

static const command_builtin_t builtinCommands[] = {
  { "tid",      command_script_tid      },
  { "cls",      command_script_cls      },
  { "print",    command_script_print    },
  { "gets",     command_script_gets     },
  { "cd",       command_script_cd       },
  { "opendb",   command_script_opendb   },
  { "readdb",   command_script_readdb   },
  { "closedb",  command_script_closedb  },
  { "opendir",  command_script_opendir  },
  { "readdir",  command_script_readdir  },
  { "closedir", command_script_closedir },
  { "launch",   command_script_launch   },
  { "ps",       command_script_ps       },
  { "kill",     command_script_kill     },
  { "cat",      command_script_cat      },
  { "rm",       command_script_rm       },
  { "run",      command_script_run      },
  { "deploy",   command_script_deploy   },
  { "edit",     command_script_edit     },
  { "telnet",   command_script_telnet   },
  { "exit",     command_script_exit     },
  { "pit",      command_script_pit      },
  { "play",     command_script_play     },
  { "crash",    command_script_crash    },
  { "setio",    command_script_setio    },
  { NULL, NULL }
};

static void command_putc(command_internal_data_t *idata, char c) {
  plibc_putchar(c);
}

static void command_puts(command_internal_data_t *idata, char *s) {
  int i, n;
  char prev;

  if (s) {
    n = StrLen(s);
    for (i = 0, prev = 0; i < n; prev = s[i], i++) {
      if (s[i] == '\n' && prev != '\r') {
        command_putc(idata, '\r');
      }
      command_putc(idata, s[i]);
    }
  }
}

static void command_putchar(void *data, char c) {
  command_internal_data_t *idata = (command_internal_data_t *)data;
  uint8_t b = c;

  pterm_cursor(idata->t, 0);
  pterm_send(idata->t, &b, 1);
}

static void command_setcolor(void *data, uint32_t fg, uint32_t bg) {
  command_internal_data_t *idata = (command_internal_data_t *)data;

  if (fg & 0x80000000) {
    fg = RGBToLong(&idata->prefs.foreground);
  }

  if (bg & 0x80000000) {
    bg = RGBToLong(&idata->prefs.background);
  }

  pterm_setfg(idata->t, fg);
  pterm_setbg(idata->t, bg);
}

static int command_getchar(void *data) {
  command_internal_data_t *idata = (command_internal_data_t *)data;
  EventType event;
  Err err;
  int c = -1;

  for (; !thread_must_end();) {
    EvtGetEvent(&event, idata->wait);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (event.eType == appStopEvent) break;
    if (event.eType == nilEvent) {
      pterm_cursor_blink(idata->t);
      continue;
    }
    if (event.eType == keyDownEvent && !(event.data.keyDown.modifiers & commandKeyMask)) {
      c = event.data.keyDown.chr;
      break;
    }
  }

  return c;
}

static int command_haschar(void *data) {
  return EvtKeyEventAvail();
}

static command_internal_data_t *command_get_data(void) {
  command_data_t *data = pumpkin_get_data();
  return data->idata;
}

static void check_prefix(char *path, char *buf, int n) {
  command_internal_data_t *idata = command_get_data();

  if (path) {
    if (path[0] == '/') {
      StrNCopy(buf, path, n-1);
    } else {
      StrNCopy(buf, idata->cwd, n-1);
      StrNCat(buf, path, n-1-StrLen(buf));
    }
  } else {
    StrNCopy(buf, idata->cwd, n-1);
  }
}

static void command_execute(command_internal_data_t *idata, char *s) {
  char *val;

  if (s && s[0]) {
    val = pumpkin_script_call(idata->pe, "command_eval", s);
    if (val) {
      if (val[0]) {
        command_puts(idata, val);
        command_putc(idata, '\r');
        command_putc(idata, '\n');
      }
      xfree(val);
    }
  }
}

static void command_prompt(command_internal_data_t *idata) {
  command_puts(idata, idata->cwd);
  command_putc(idata, '>');
}

static void command_expand(command_internal_data_t *idata) {
  FileInfoType info;
  FileRef f;
  UInt32 op;
  char buf[MAXCMD];
  char absolute[MAXCMD];
  int i, j, first, last, len;

  if (idata->cmdIndex > 0 && idata->cmd[idata->cmdIndex-1] != '/') {
    idata->cmd[idata->cmdIndex] = 0;

    // find the previous '"' or ' ' in the command buffer
    for (i = idata->cmdIndex-1; i >= 0; i--) {
      if (idata->cmd[i] == '"' || idata->cmd[i] == ' ') break;
    }

    if (i >= 0 && (idata->cmd[i] == '"' || idata->cmd[i] == ' ')) {
      // found '"' or ' ', advance one position
      i++;

      if (idata->cmd[i] == '/') {
        // it is an absolute path, just copy it
        StrNCopy(absolute, &idata->cmd[i], MAXCMD - 1);
      } else {
        // it is a relative path, copy the current directoy
        StrNCopy(absolute, idata->cwd, MAXCMD - 1);
        // and append the command
        StrNCat(absolute, &idata->cmd[i], MAXCMD - StrLen(absolute) - 1);
      }

      // find the last '/' in the absolute path
      first = last = 0;
      for (j = 1; absolute[j]; j++) {
        if (absolute[j] == '/') last = j;
      }

      // set the name of the directory in buf
      MemSet(buf, sizeof(buf), 0);
      if (last == first) {
        StrCopy(buf, "/");
      } else {
        StrNCopy(buf, &absolute[first], last - first);
      }

      // open the directory
      if (VFSFileOpen(idata->volume, buf, vfsModeRead, &f) == errNone) {
        for (op = vfsIteratorStart;;) {
          // read the next entry in the directory
          MemSet(buf, sizeof(buf), 0);
          info.nameP = buf;
          info.nameBufLen = sizeof(buf)-1;
          if (VFSDirEntryEnumerate(f, &op, &info) != errNone) break;

          // if the typed prefix matches the begining of the entry name
          len = StrLen(&absolute[last+1]);
          if (StrNCompare(&absolute[last+1], info.nameP, len) == 0) {
            // fills in the rest of the name
            idata->cmdIndex = StrLen(idata->cmd);
            for (i = len; info.nameP[i]; i++) {
              if (idata->cmdIndex < MAXCMD-1) {
                command_putc(idata, info.nameP[i]);
                idata->cmd[idata->cmdIndex++] = info.nameP[i];
              }
            }
            // if the entry is a directory, append '/'
            if (idata->cmdIndex < MAXCMD-1 && info.attributes & vfsFileAttrDirectory) {
              command_putc(idata, '/');
              idata->cmd[idata->cmdIndex++] = '/';
            }
            idata->cmd[idata->cmdIndex] = 0;
            // exit
            break;
          }
        }
        VFSFileClose(f);
      }
    }
  }
}

static void command_key(command_internal_data_t *idata, UInt8 c, Boolean expand) {
  conn_filter_t *conn, *telnet;

  telnet = idata->telnet;

  if (telnet) {
    if (telnet->write(telnet, &c, 1) == -1) {
      idata->telnet = NULL;
      conn = telnet->next;
      telnet_close(telnet);
      conn_close(conn);
      idata->wait = SysTicksPerSecond() / 4;
    }

  } else {
    if (c == '\t' && !expand) {
      c = ' ';
    }

    switch (c) {
      case '\b':
        if (idata->cmdIndex > 0) {
          command_putc(idata, '\b');
          idata->cmdIndex--;
        }
        break;
      case '\t':
        command_expand(idata);
        break;
      case '\n':
        command_putc(idata, '\r');
        command_putc(idata, '\n');
        idata->cmd[idata->cmdIndex] = 0;
        idata->cmdIndex = 0;
        if (idata->cmd[0]) {
          command_execute(idata, idata->cmd);
        }
        command_prompt(idata);
        break;
      default:
        if (idata->cmdIndex < MAXCMD-1) {
          command_putc(idata, c);
          idata->cmd[idata->cmdIndex++] = c;
        }
        break;
    }
  }
}

static Boolean ctlSelected(FormType *formP, UInt16 id) {
  ControlType *ctl;
  UInt16 index;

  index = FrmGetObjectIndex(formP, id);
  ctl = (ControlType *)FrmGetObjectPtr(formP, index);

  return CtlGetValue(ctl) != 0;
}

static Boolean MenuEvent(UInt16 id) {
  command_internal_data_t *idata = command_get_data();
  MemHandle h;
  UInt16 i, length;
  Coord pos, col, row;
  uint32_t fg, bg;
  uint8_t c;
  char *s, buf[MAXCMD];
  Boolean handled = false;

  switch (id) {
    case aboutCmd:
      AbtShowAboutPumpkin(AppID);
      handled = true;
      break;

    case pasteCmd:
      if ((h = ClipboardGetItem(clipboardText, &length)) != NULL) {
        if (length > 0 && (s = MemHandleLock(h)) != NULL) {
          for (i = 0; i < length; i++) {
            command_key(idata, s[i], false);
          }
          MemHandleUnlock(h);
        }
      }
      handled = true;
      break;

    case copyCmd:
      if (idata->selected) {
        i = 0;
        pos = idata->row0 * idata->ncols + idata->col0;

        if (idata->row0 == idata->row1) {
          for (col = idata->col0; col <= idata->col1; col++, pos++) {
            pterm_getchar(idata->t, pos, &c, &fg, &bg);
            if (i < MAXCMD) buf[i++] = c;
          }
          // remove trailing spaces
          for (; i > 0 && buf[i-1] == ' '; i--);

        } else {
          for (col = idata->col0; col < idata->ncols; col++, pos++) {
            pterm_getchar(idata->t, pos, &c, &fg, &bg);
            if (i < MAXCMD) buf[i++] = c;
          }
          // remove trailing spaces
          for (; i > 0 && buf[i-1] == ' '; i--);
          // add newline
          if (i < MAXCMD) buf[i++] = '\n';

          for (row = idata->row0+1; row < idata->row1; row++) {
            for (col = 0; col < idata->ncols; col++, pos++) {
              pterm_getchar(idata->t, pos, &c, &fg, &bg);
              if (i < MAXCMD) buf[i++] = c;
            }
            // remove trailing spaces
            for (; i > 0 && buf[i-1] == ' '; i--);
            // add newline
            if (i < MAXCMD) buf[i++] = '\n';
          }

          for (col = 0; col <= idata->col1; col++, pos++) {
            pterm_getchar(idata->t, pos, &c, &fg, &bg);
            if (i < MAXCMD) buf[i++] = c;
          }
          // remove trailing spaces
          for (; i > 0 && buf[i-1] == ' '; i--);
        }

        if (i > 0) {
          debug(DEBUG_INFO, "Command", "copying to clipboard \"%.*s\"", i, buf);
          ClipboardAddItem(clipboardText, buf, i);
        }
      }
      handled = true;
      break;

    case prefsCmd:
      FrmPopupForm(PrefsForm);
      handled = true;
      break;

    case forkCmd:
      pumpkin_fork();
      handled = true;
      break;

  }

  return handled;
}

static void command_update_line(command_internal_data_t *idata, Int16 row, Int16 col1, Int16 col2, Boolean selected) {
  RectangleType rect;
  RGBColorType fg_rgb, bg_rgb;
  Int16 pos, col, x, y;
  uint32_t fg, bg;
  uint8_t c;

  y = row * idata->fheight;
  x = col1 * idata->fwidth;
  pos = row * idata->ncols + col1;

  for (col = col1; col <= col2; col++, pos++) {
    pterm_getchar(idata->t, pos, &c, &fg, &bg);
    if (selected) {
      bg_rgb = idata->prefs.highlight;
      fg_rgb.r = 255 - bg_rgb.r;
      fg_rgb.g = 255 - bg_rgb.g;
      fg_rgb.b = 255 - bg_rgb.b;
    } else {
      LongToRGB(bg, &bg_rgb);
      LongToRGB(fg, &fg_rgb);
    }
    WinSetBackColorRGB(&bg_rgb, NULL);
    WinSetTextColorRGB(&fg_rgb, NULL);

    RctSetRectangle(&rect, x, y, idata->fwidth, idata->fheight);
    WinEraseRectangle(&rect, 0);
    WinPaintChar(c, x, y);
    x += idata->fwidth;
  }
}

static void command_draw(command_internal_data_t *idata) {
  RGBColorType oldt, oldb;
  FontID old;
  Int16 row;

  old = FntSetFont(idata->font);
  WinSetBackColorRGB(NULL, &oldb);
  WinSetTextColorRGB(NULL, &oldt);

  for (row = 0; row < idata->nrows; row++) {
    command_update_line(idata, row, 0, idata->ncols-1, false);
  }

  WinSetBackColorRGB(&oldb, NULL);
  WinSetTextColorRGB(&oldt, NULL);
  FntSetFont(old);
}

static void command_update(command_internal_data_t *idata, Int16 row1, Int16 col1, Int16 row2, Int16 col2, Boolean selected) {
  RGBColorType oldt, oldb;
  FontID old;
  Int16 row;

  old = FntSetFont(idata->font);
  WinSetBackColorRGB(NULL, &oldb);
  WinSetTextColorRGB(NULL, &oldt);

  if (row1 == row2) {
    command_update_line(idata, row1, col1, col2, selected);
  } else if (row1 < row2) {
    command_update_line(idata, row1, col1, idata->ncols-1, selected);
    for (row = row1 + 1; row < row2; row++) {
      command_update_line(idata, row, 0, idata->ncols-1, selected);
    }
    command_update_line(idata, row2, 0, col2, selected);
  }

  WinSetBackColorRGB(&oldb, NULL);
  WinSetTextColorRGB(&oldt, NULL);
  FntSetFont(old);
}

void command_drag(command_internal_data_t *idata, Int16 x, Int16 y, Int16 type) {
  UInt32 swidth, sheight;
  Int16 col, row;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);

  if (x >= 0 && x < swidth && y >= 0 && y < sheight) {
    pterm_cursor(idata->t, 0);
    col = x / idata->fwidth;
    row = y / idata->fheight;

    switch (type) {
      case SCREEN_PEN_DOWN:
        idata->moved = false;
        if (idata->selected) {
          idata->selected = false;
          command_draw(idata);
        }
        idata->col0 = col;
        idata->row0 = row;
        idata->col1 = col;
        idata->row1 = row;
        idata->down = true;
        break;
      case SCREEN_PEN_MOVE:
        if (idata->down) {
          if (row != idata->row1 || col != idata->col1) {
            if (row > idata->row1 || (row == idata->row1 && col > idata->col1)) {
              if (idata->row0 > idata->row1 || (idata->row0 == idata->row1 && idata->col0 > idata->col1)) {
                command_update(idata, idata->row1, idata->col1, row, col, false);
              } else {
                command_update(idata, idata->row0, idata->col0, row, col, true);
              }
            } else if (row < idata->row1 || (row == idata->row1 && col < idata->col1)) {
              if (idata->row0 > idata->row1 || (idata->row0 == idata->row1 && idata->col0 > idata->col1)) {
                command_update(idata, row, col, idata->row0, idata->col0, true);
              } else {
                command_update(idata, row, col, idata->row1, idata->col1, false);
              }
            }
            idata->col1 = col;
            idata->row1 = row;
            idata->moved = true;
          }
        }
        break;
      case SCREEN_PEN_UP:
        idata->selected = idata->moved;
        idata->moved = false;
        idata->down = false;
        break;
    }
  }
}

static Boolean MainFormHandleEvent(EventType *event) {
  command_internal_data_t *idata = command_get_data();
  WinHandle wh;
  FormType *formP;
  RectangleType rect;
  RGBColorType oldb;
  UInt32 density, swidth, sheight;
  FontID font, old;
  Coord x, y;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      idata->cmdIndex = 0;

      WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
      formP = FrmGetActiveForm();
      wh = FrmGetWindowHandle(formP);
      RctSetRectangle(&rect, 0, 0, swidth, sheight);
      WinSetBounds(wh, &rect);
      WinSetBackColorRGB(&idata->prefs.background, &oldb);
      WinEraseRectangle(&rect, 0);
      WinSetBackColorRGB(&oldb, NULL);

      pumpkin_script_init(idata->pe, pumpkin_script_engine_id(), 1);
      pterm_cls(idata->t);
      command_puts(idata, "PumpkinOS shell\r\n");
      command_prompt(idata);
      handled = true;
      break;

    case frmUpdateEvent:
      handled = true;
      break;

    case winDisplayChangedEvent:
      WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
      formP = FrmGetActiveForm();
      wh = FrmGetWindowHandle(formP);
      RctSetRectangle(&rect, 0, 0, swidth, sheight);
      WinSetBounds(wh, &rect);

      WinScreenGetAttribute(winScreenDensity, &density);
      font = idata->prefs.font - ((density == kDensityLow) ? 9100 : 9000);
      idata->font = font;
      old = FntSetFont(font);
      idata->fwidth = FntCharWidth('A');
      idata->fheight = FntCharHeight();
      FntSetFont(old);
      pumpkin_set_size(AppID, event->data.winDisplayChanged.newBounds.extent.x, event->data.winDisplayChanged.newBounds.extent.y);
      command_draw(idata);
      handled = true;
      break;

    case nilEvent:
      pterm_cursor_blink(idata->t);
      handled = true;
      break;

    case keyDownEvent:
      if (!(event->data.keyDown.modifiers & commandKeyMask)) {
        command_key(idata, event->data.keyDown.chr, true);
        handled = true;
      }
      break;

    case menuEvent:
      handled = MenuEvent(event->data.menu.itemID);
      break;

    case penDownEvent:
      x = event->screenX;
      y = event->screenY;
      command_drag(idata, x, y, SCREEN_PEN_DOWN);
      break;

    case penMoveEvent:
      x = event->screenX;
      y = event->screenY;
      command_drag(idata, x, y, SCREEN_PEN_MOVE);
      break;

    case penUpEvent:
      x = event->screenX;
      y = event->screenY;
      command_drag(idata, x, y, SCREEN_PEN_UP);
      break;

    default:
      break;
  }

  return handled;
}

static void DrawColor(RGBColorType *rgb, RectangleType *rect) {
  RGBColorType old, aux;
  UInt16 x, y, dx, dy;

  x = rect->topLeft.x;
  y = rect->topLeft.y;
  dx = rect->extent.x;
  dy = rect->extent.y;

  WinSetBackColorRGB(rgb, &old);
  WinEraseRectangle(rect, 0);
  WinSetBackColorRGB(&old, NULL);

  aux.r = 0;
  aux.g = 0;
  aux.b = 0;
  WinSetForeColorRGB(&aux, &old);
  WinDrawLine(x, y, x+dx-1, y);
  WinDrawLine(x+dx-1, y, x+dx-1, y+dy-1);
  WinDrawLine(x+dx-1, y+dy-1, x, y+dy-1);
  WinDrawLine(x, y+dy-1, x, y);
  WinSetForeColorRGB(&old, NULL);
}

static void DrawColorGadget(FormType *formP, UInt16 id, RGBColorType *rgb) {
  FormGadgetTypeInCallback *gad;
  UInt16 index;

  index = FrmGetObjectIndex(formP, id);
  gad = (FormGadgetTypeInCallback *)FrmGetObjectPtr(formP, index);
  DrawColor(rgb, &gad->rect);
}

static Boolean ColorGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  command_internal_data_t *idata = command_get_data();
  RGBColorType rgb;
  EventType *event;
  Boolean ok;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  switch (cmd) {
    case formGadgetDrawCmd:
      switch (gad->id) {
        case fgCtl: DrawColor(&idata->prefs.foreground, &gad->rect); break;
        case bgCtl: DrawColor(&idata->prefs.background, &gad->rect); break;
        case hlCtl: DrawColor(&idata->prefs.highlight, &gad->rect); break;
        default: return true;
      }
      break;
    case formGadgetEraseCmd:
      WinEraseRectangle(&gad->rect, 0);
      break;
    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        switch (gad->id) {
          case fgCtl: rgb = idata->prefs.foreground; break;
          case bgCtl: rgb = idata->prefs.background; break;
          case hlCtl: rgb = idata->prefs.highlight; break;
          default: return true;
        }
        ok = false;
        if (UIPickColor(NULL, &rgb, UIPickColorStartRGB, NULL, NULL)) {
          DrawColor(&rgb, &gad->rect);
          ok = true;
        }
        if (ok) {
          switch (gad->id) {
            case fgCtl: idata->prefs.foreground = rgb; break;
            case bgCtl: idata->prefs.background = rgb; break;
            case hlCtl: idata->prefs.highlight = rgb; break;
          }
        }
      }
      break;
  }

  return true;
}

static Boolean PrefsFormHandleEvent(EventType *event) {
  command_internal_data_t *idata = command_get_data();
  FormType *formP;
  ControlType *ctl;
  FontID old;
  UInt32 color, density;
  UInt16 index, sel, width, height;
  Boolean handled = false;

  WinScreenGetAttribute(winScreenDensity, &density);

  switch (event->eType) {
    case frmOpenEvent:
      formP = FrmGetActiveForm();
      if (density == kDensityDouble) {
        switch (idata->prefs.font) {
          case font6x10Id:  sel = sel6x10;  break;
          case font8x14Id:  sel = sel8x14;  break;
          case font8x16Id:  sel = sel8x16;  break;
          default: sel = sel8x14; break;
        }
        index = FrmGetObjectIndex(formP, sel);
        ctl = (ControlType *)FrmGetObjectPtr(formP, index);
        CtlSetValue(ctl, 1);
        index = FrmGetObjectIndex(formP, lbl6x10);
        FrmHideObject(formP, index);
      } else {
        index = FrmGetObjectIndex(formP, sel6x10);
        FrmHideObject(formP, index);
        index = FrmGetObjectIndex(formP, sel8x14);
        FrmHideObject(formP, index);
        index = FrmGetObjectIndex(formP, sel8x16);
        FrmHideObject(formP, index);
      }
      FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, fgCtl), ColorGadgetCallback);
      FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, bgCtl), ColorGadgetCallback);
      FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, hlCtl), ColorGadgetCallback);
      FrmDrawForm(formP);
      handled = true;
      break;
    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case dflBtn:
          idata->prefs.font = font8x14Id;
          if (density != kDensityDouble) idata->prefs.font = font16x16Id;
          idata->prefs.foreground = defaultForeground;
          idata->prefs.background = defaultBackground;
          idata->prefs.highlight  = defaultHighlight;
          formP = FrmGetActiveForm();
          index = FrmGetObjectIndex(formP, sel8x14);
          ctl = (ControlType *)FrmGetObjectPtr(formP, index);
          CtlSetValue(ctl, 1);
          DrawColorGadget(formP, fgCtl, &idata->prefs.foreground);
          DrawColorGadget(formP, bgCtl, &idata->prefs.background);
          DrawColorGadget(formP, hlCtl, &idata->prefs.highlight);
          break;
        case okBtn:
          formP = FrmGetActiveForm();
          if (density == kDensityDouble) {
            if (ctlSelected(formP, sel6x10)) {
              idata->prefs.font = font6x10Id;
            } else if (ctlSelected(formP, sel8x14)) {
              idata->prefs.font = font8x14Id;
            } else if (ctlSelected(formP, sel8x16)) {
              idata->prefs.font = font8x16Id;
            }
            old = FntSetFont(idata->prefs.font - 9000);
          } else {
            old = FntSetFont(idata->prefs.font - 9100);
          }
          PrefSetAppPreferences(AppID, 1, 1, &idata->prefs, sizeof(command_prefs_t), true);
          color = RGBToLong(&idata->prefs.foreground);
          pterm_setfg(idata->t, color);
          color = RGBToLong(&idata->prefs.background);
          pterm_setbg(idata->t, color);

          width = FntCharWidth('A') * 2;
          height = FntCharHeight() * 2;
          FntSetFont(old);
          pumpkin_change_display(COLS * width, ROWS * height);

          FrmReturnToForm(MainForm);
          break;
        case cancelBtn:
          FrmReturnToForm(MainForm);
          break;
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormType *formP;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      formP = FrmInitForm(form);
      FrmSetActiveForm(formP);

      switch (form) {
        case MainForm:
          FrmSetEventHandler(formP, MainFormHandleEvent);
          break;
        case PrefsForm:
          FrmSetEventHandler(formP, PrefsFormHandleEvent);
          break;
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

Int32 read_file(char *name, char *b, Int32 max) {
  command_internal_data_t *idata = command_get_data();
  UInt32 nread;
  FileRef fr;
  char buf[MAXCMD];
  Int32 n = -1;

  check_prefix(name, buf, MAXCMD);

  if (VFSFileOpen(idata->volume, buf, vfsModeRead, &fr) == errNone) {
    if (VFSFileRead(fr, max, b, &nread) == errNone) {
      n = nread;
    }
    VFSFileClose(fr);
  }

  return n;
}

static int command_script_file(int pe, int run) {
  command_internal_data_t *idata = command_get_data();
  Int32 n, max = 65536;
  char *buf, *name = NULL;

  if (script_get_string(pe, 0, &name) == 0) {
    if ((buf = MemPtrNew(max)) != NULL) {
      MemSet(buf, max, 0);

      if ((n = read_file(name, buf, max-1)) == -1) {
        command_puts(idata, "Error reading file\r\n");

      } else if (n > 0) {
        if (run) {
          if (pumpkin_script_run_string(pe, buf) != 0) {
            command_puts(idata, "Error loading file\r\n");
          }
        } else {
          command_puts(idata, buf);
          if (buf[n-1] != '\n') {
            command_putc(idata, '\r');
            command_putc(idata, '\n');
          }
        }
      }
      MemPtrFree(buf);
    }
  }

  if (name) xfree(name);

  return 0;
}

static int command_script_cat(int pe) {
  return command_script_file(pe, 0);
}

static int command_script_run(int pe) {
  return command_script_file(pe, 1);
}

static int command_script_rm(int pe) {
  command_internal_data_t *idata = command_get_data();
  char *name = NULL;

  if (script_get_string(pe, 0, &name) == 0) {
    VFSFileDelete(idata->volume, name);
  }

  if (name) xfree(name);

  return 0;
}

static int command_script_deploy(int pe) {
  command_internal_data_t *idata = command_get_data();
  UInt32 creator;
  char *name = NULL;
  char *screator = NULL;
  char *script = NULL;

  if (script_get_string(pe, 0, &name) == 0 &&
      script_get_string(pe, 1, &screator) == 0 &&
      script_get_string(pe, 2, &script) == 0) {

    pumpkin_s2id(&creator, screator);
    if (command_app_deploy(name, creator, script) == errNone) {
      command_puts(idata, "Script deployed\r\n");
    } else {
      command_puts(idata, "Error deploying script\r\n");
    }
  }

  if (name) xfree(name);
  if (screator) xfree(screator);
  if (script) xfree(script);

  return 0;
}

static int command_script_kill(int pe) {
  char *name = NULL;

  if (script_get_string(pe, 0, &name) == 0) {
    pumpkin_kill(name);
  }

  if (name) xfree(name);

  return 0;
}

static int command_script_tid(int pe) {
  return script_push_integer(pe, pumpkin_get_taskid());
}

static int command_script_cls(int pe) {
  command_internal_data_t *idata = command_get_data();
  pterm_cls(idata->t);
  pterm_home(idata->t);

  return 0;
}

static char *value_tostring(int pe, script_arg_t *arg) {
  char buf[64], *val;

  switch (arg->type) {
    case SCRIPT_ARG_INTEGER:
      StrPrintF(buf, "%d", arg->value.i);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_REAL:
      StrPrintF(buf, "%f", arg->value.d);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_BOOLEAN:
      StrCopy(buf, arg->value.i ? "true" : "false");
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_STRING:
      val = arg->value.s;
      break;
    case SCRIPT_ARG_LSTRING:
      val = xcalloc(1, arg->value.l.n + 1);
      if (val) {
        MemMove(val, arg->value.l.s, arg->value.l.n);
      }
      xfree(arg->value.l.s);
      break;
    case SCRIPT_ARG_OBJECT:
      StrCopy(buf, "<object>");
      val = xstrdup(buf);
      script_remove_ref(pe, arg->value.r);
      break;
    case SCRIPT_ARG_FUNCTION:
      StrCopy(buf, "<function>");
      val = xstrdup(buf);
      script_remove_ref(pe, arg->value.r);
      break;
    case SCRIPT_ARG_NULL:
      StrCopy(buf, "nil");
      val = xstrdup(buf);
      break;
    default:
      StrPrintF(buf, "type(%c)", arg->type);
      val = xstrdup(buf);
      break;
  }

  return val;
}

static int command_script_print(int pe) {
  command_internal_data_t *idata = command_get_data();
  script_arg_t arg;
  Int16 i;
  Boolean crlf;
  char *s = NULL;

  crlf = false;

  for (i = 0;; i++) {
    if (script_get_named_value(pe, i, SCRIPT_ARG_ANY, NULL, NULL, NULL, 1, &arg) != 0) break;
    s = value_tostring(pe, &arg);
    if (s) {
      if (i > 0) command_putc(idata, '\t');
      command_puts(idata, s);
      xfree(s);
      crlf = true;
    }
  }

  if (crlf) {
    command_putc(idata, '\r');
    command_putc(idata, '\n');
  }

  return 0;
}

static int command_script_gets(int pe) {
  char buf[256];
  int n, r = -1;

  if (plibc_fgets(buf, sizeof(buf)-1, plibc_stdin) != NULL) {
    n = StrLen(buf);
    if (n > 0 && buf[n-1] == '\n') buf[n-1] = 0;
    if (n > 1 && buf[n-2] == '\r') buf[n-2] = 0;
    r = script_push_string(pe, buf);
  }

  return r;
}

static int command_script_launch(int pe) {
  UInt32 result;
  char *name = NULL;

  if (script_get_string(pe, 0, &name) == 0) {
    LocalID dbID = DmFindDatabase(0, name);
    if (dbID) {
      SysAppLaunch(0, dbID, 0, sysAppLaunchCmdNormalLaunch, NULL, &result);
    }
  }

  if (name) xfree(name);

  return 0;
}

static int ps_callback(int i, char *name, int m68k, void *idata) {
  char stype[8], screator[8];
  UInt32 type, creator;
  LocalID dbID;
  Int16 r = -1;

  dbID = DmFindDatabase(0, name);
  if (dbID && DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator) == errNone) {
    pumpkin_id2s(type, stype);
    command_puts(idata, stype);
    command_putc(idata, ' ');
    pumpkin_id2s(creator, screator);
    command_puts(idata, screator);
    command_putc(idata, ' ');
    command_puts(idata, name);
    command_putc(idata, '\r');
    command_putc(idata, '\n');
    r = 0;
  }

  return r;
}

static int command_script_ps(int pe) {
  command_internal_data_t *idata = command_get_data();

  pumpkin_ps(ps_callback, idata);

  return 0;
}

static int command_script_cd(int pe) {
  command_internal_data_t *idata = command_get_data();
  char *dir = NULL;

  if (script_get_string(pe, 0, &dir) != 0) {
    dir = xstrdup("/");
  }

  if (VFSChangeDir(idata->volume, dir) == errNone) {
    VFSCurrentDir(idata->volume, idata->cwd, MAXCMD);
  } else {
    command_puts(idata, "Invalid directory\r\n");
  }

  if (dir) xfree(dir);

  return 0;
}

static void ldb_destructor(void *p) {
  command_ldb_t *d = (command_ldb_t *)p;

  if (d) {
    xfree(d);
  }
}

static int command_script_opendb(int pe) {
  command_ldb_t *d;
  char *type = NULL;
  char *creator = NULL;
  int ptr, r = -1;

  script_opt_string(pe, 0, &type);
  script_opt_string(pe, 1, &creator);

  if ((d = xcalloc(1, sizeof(command_ldb_t))) != NULL) {
    d->tag = TAG_LDB;
    d->first = true;
    if (type && type[0]) {
      pumpkin_s2id(&d->type, type);
    } else {
      d->type = 0;
    }
    if (creator && creator[0]) {
      pumpkin_s2id(&d->creator, creator);
    } else {
      d->creator = 0;
    }
    if ((ptr = ptr_new(d, ldb_destructor)) != -1) {
      r = script_push_integer(pe, ptr);
    } else {
      xfree(d);
    }
  }

  if (type) xfree(type);
  if (creator) xfree(creator);

  return r;
}

static int command_script_readdb(int pe) {
  script_int_t ptr;
  script_ref_t obj;
  command_ldb_t *d;
  LocalID dbID;
  UInt16 cardNo;
  UInt32 type, creator;
  char name[dmDBNameLength], stype[8], screator[8];
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if ((d = ptr_lock(ptr, TAG_LDB)) != NULL) {
      if (DmGetNextDatabaseByTypeCreator(d->first, &d->stateInfo, d->type, d->creator, false, &cardNo, &dbID) == errNone) {
        if (DmDatabaseInfo(cardNo, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator) == errNone) {
          pumpkin_id2s(type, stype);
          pumpkin_id2s(creator, screator);
          obj = pumpkin_script_create_obj(pe, NULL);
          script_add_sconst(pe, obj, "name", name);
          script_add_sconst(pe, obj, "type", stype);
          script_add_sconst(pe, obj, "creator", screator);
          r = script_push_object(pe, obj);
        }
      }
      d->first = false;
      ptr_unlock(ptr, TAG_LDB);
    }
  }

  return r;
}

static int command_script_closedb(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if (ptr_free(ptr, TAG_LDB) == 0) {
      r = script_push_boolean(pe, 1);
    }
  }

  return r;
}

static void dir_destructor(void *p) {
  command_dir_t *d = (command_dir_t *)p;

  if (d) {
    if (d->f) VFSFileClose(d->f);
    xfree(d);
  }
}

static int command_script_opendir(int pe) {
  command_internal_data_t *idata = command_get_data();
  command_dir_t *d;
  char *dir = NULL;
  int ptr, r = -1;

  if (script_get_string(pe, 0, &dir) == 0) {
    if ((d = xcalloc(1, sizeof(command_dir_t))) != NULL) {
      check_prefix(dir, d->buf, MAXCMD);
      if (VFSFileOpen(idata->volume, d->buf, vfsModeRead, &d->f) == errNone) {
        d->tag = TAG_DIR;
        d->op = vfsIteratorStart;
        if ((ptr = ptr_new(d, dir_destructor)) != -1) {
          r = script_push_integer(pe, ptr);
        } else {
          VFSFileClose(d->f);
          xfree(d);
        }
      } else {
        xfree(d);
      }
    }
  }

  if (dir) xfree(dir);

  return r;
}

static int command_script_readdir(int pe) {
  script_int_t ptr;
  script_ref_t obj;
  command_dir_t *d;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if ((d = ptr_lock(ptr, TAG_DIR)) != NULL) {
      d->info.nameP = d->buf;
      d->info.nameBufLen = MAXCMD-1;
      if (VFSDirEntryEnumerate(d->f, &d->op, &d->info) == errNone) {
        obj = pumpkin_script_create_obj(pe, NULL);
        script_add_sconst(pe, obj, "name", d->info.nameP);
        script_add_boolean(pe, obj, "directory", d->info.attributes & vfsFileAttrDirectory ? 1 : 0);
        script_add_boolean(pe, obj, "readOnly", d->info.attributes & vfsFileAttrReadOnly ? 1 : 0);
        r = script_push_object(pe, obj);
      }
      ptr_unlock(ptr, TAG_DIR);
    }
  }

  return r;
}

static int command_script_closedir(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if (ptr_free(ptr, TAG_DIR) == 0) {
      r = script_push_boolean(pe, 1);
    }
  }

  return r;
}

static int command_script_edit(int pe) {
  command_internal_data_t *idata = command_get_data();
  editor_t e;
  char *name = NULL;

  script_opt_string(pe, 0, &name);

  xmemset(&e, 0, sizeof(editor_t));
  pumpkin_editor_init(&e, idata->t);

  if (editor_get_plugin(&e, sysAnyPluginId) == 0 && e.edit) {
    e.edit(&e, name);
    pterm_cls(idata->t);
    pterm_home(idata->t);
    if (e.destroy) e.destroy(&e);
  }

  if (name) xfree(name);

  return 0;
}

static int command_script_telnet(int pe) {
  command_internal_data_t *idata = command_get_data();
  char *host = NULL;
  script_int_t port;
  conn_filter_t *conn, *telnet;
  int fd;

  if (script_get_string(pe, 0, &host) == 0) {
     if (script_opt_integer(pe, 1, &port) != 0) {
       port = 23;
     }

     if ((fd = sys_socket_open_connect(host, port, IP_STREAM)) != -1) {
       if ((conn = conn_filter(fd)) != NULL) {
         if ((telnet = telnet_filter(conn)) != NULL) {
           idata->telnet = telnet;
           pterm_cls(idata->t);
           idata->wait = SysTicksPerSecond() / 10;
         }
       }
     }
  }

  if (host) xfree(host);

  return 0;
}

static int command_script_exit(int pe) {
  EventType event;

  event.eType = appStopEvent;
  EvtAddEventToQueue(&event);

  return 0;
}

static int command_shell_filter_peek(conn_filter_t *filter, uint32_t us) {
  command_internal_data_t *idata = command_get_data();

  idata->blink++;
  if (idata->blink == 50) {
    pterm_cursor_blink(idata->t);
    idata->blink = 0;
  }

  return EvtSysEventAvail(true) ? 1 : EvtPumpEvents(us);
}

static int command_shell_filter_read(conn_filter_t *filter, uint8_t *b) {
  command_internal_data_t *idata = command_get_data();
  EventType event;
  Err err;
  int stop = 0, r = -1;

  do {
    EvtGetEvent(&event, idata->wait);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;

    switch (event.eType) {
      case keyDownEvent:
        if (!(event.data.keyDown.modifiers & commandKeyMask)) {
          *b = event.data.keyDown.chr;
          if (*b == '\n') *b = '\r';
          stop = 1;
          r = 1;
        }
        break;
      case appStopEvent:
        stop = 1;
        break;
      default:
        FrmDispatchEvent(&event);
        break;
    }
  } while (!stop);

  return r;
}

static int command_shell_filter_write(conn_filter_t *filter, uint8_t *b, int n) {
  command_internal_data_t *idata = command_get_data();
  int i;

  for (i = 0; i < n; i++) {
    command_putc(idata, (char)b[i]);
  }

  return n;
}

static int command_script_pit(int pe) {
  shell_provider_t *p;
  conn_filter_t filter;
  int r = -1;

  if ((p = script_get_pointer(pe, SHELL_PROVIDER)) != NULL) {
    xmemset(&filter, 0, sizeof(conn_filter_t));
    filter.peek = command_shell_filter_peek;
    filter.read = command_shell_filter_read;
    filter.write = command_shell_filter_write;
    r = p->run(&filter, pe, 0);
  }

  return r;
}

static Err sndCallback(void *userdata, SndStreamRef sound, void *buffer, UInt32 numberofframes) {
  command_play_t *play = (command_play_t *)userdata;
  UInt32 n, nread;
  Err err = sndErrBadParam;

  if (play && play->fileRef && buffer) {
    n = numberofframes * play->sampleSize;
    sys_memset(buffer, 0, n);
    if (VFSFileRead(play->fileRef, n, buffer, &nread) == errNone && nread > 0) {
      err = errNone;
    } else {
      VFSFileClose(play->fileRef);
      xfree(play);
    }
  }

  return err;
}

static int command_script_play(int pe) {
  command_internal_data_t *idata = command_get_data();
  char *name = NULL;
  FileRef fileRef;
  command_play_t *play;
  SndStreamWidth width;
  SndSampleType type;
  UInt32 rate;
  int r = -1;

  if (script_get_string(pe, 0, &name) == 0) {
    if (VFSFileOpen(idata->volume, name, vfsModeRead, &fileRef) == errNone) {
      if (idata->sound) {
        SndStreamDelete(idata->sound);
        idata->sound = 0;
      }
      if (WavFileHeader(fileRef, &rate, &type, &width)) {
        debug(DEBUG_INFO, "Command", "wav rate %d", rate);
        debug(DEBUG_INFO, "Command", "wav type %d", type);
        debug(DEBUG_INFO, "Command", "wav %s", width == sndMono ? "mono" : "stereo");

        play = xcalloc(1, sizeof(command_play_t));
        play->fileRef = fileRef;
        play->sampleSize = 1;
        if (width == sndStereo) play->sampleSize <<= 1;
        if (type == sndInt16) play->sampleSize <<= 1;
        else if (type == sndInt32 || type == sndFloat) play->sampleSize <<= 2;

        if (SndStreamCreate(&idata->sound, sndOutput, rate, type, width, sndCallback, play, 0, false) == errNone) {
          if (SndStreamStart(idata->sound) == errNone) {
            r = 0;
          } else {
            SndStreamDelete(idata->sound);
            idata->sound = 0;
            VFSFileClose(fileRef);
            xfree(play);
          }
        } else {
          VFSFileClose(fileRef);
          xfree(play);
        }
      } else {
        VFSFileClose(fileRef);
      }
    }
  } else {
    if (idata->sound) {
      SndStreamDelete(idata->sound);
      idata->sound = 0;
    }
  }

  if (name) xfree(name);

  return r;
}

static int command_script_crash(int pe) {
  script_int_t fatal;

  if (script_get_integer(pe, 0, &fatal) == 0) {
    pumpkin_test_exception(fatal);
  }

  return 0;
}

static int command_script_setio(int pe) {
  command_internal_data_t *idata = command_get_data();
  Boolean hasInput, hasOutput;
  char *iname = NULL;
  char *oname = NULL;
  Err err;

  hasInput  = script_opt_string(pe, 0, &iname) == 0 && iname && iname[0];
  hasOutput = script_opt_string(pe, 1, &oname) == 0 && oname && oname[0];

  if (idata->in) {
    debug(DEBUG_TRACE, "Command", "setio: closing old input");
    VFSFileClose(idata->in);
    idata->in = NULL;
  }
  if (idata->out) {
    debug(DEBUG_TRACE, "Command", "setio: closing old output");
    VFSFileClose(idata->out);
    idata->out = NULL;
  }

  if (hasInput) {
    debug(DEBUG_TRACE, "Command", "setio: input \"%s\"", iname);
    err = VFSFileOpen(idata->volume, iname, vfsModeRead, &idata->in);
    if (err != errNone) {
      debug(DEBUG_TRACE, "Command", "setio: create input \"%s\"", iname);
      VFSFileCreate(idata->volume, iname);
      err = VFSFileOpen(idata->volume, iname, vfsModeRead, &idata->in);
    }
  } else {
    debug(DEBUG_TRACE, "Command", "setio: no input");
  }
  plibc_setfd(0, idata->in);

  if (hasOutput) {
    debug(DEBUG_TRACE, "Command", "setio: output \"%s\"", oname);
    err = VFSFileOpen(idata->volume, oname, vfsModeWrite, &idata->out);
    if (err != errNone) {
      debug(DEBUG_TRACE, "Command", "setio: create output \"%s\"", oname);
      VFSFileCreate(idata->volume, oname);
      err = VFSFileOpen(idata->volume, oname, vfsModeWrite, &idata->out);
    } else {
      debug(DEBUG_TRACE, "Command", "setio: recreate output \"%s\"", oname);
      VFSFileClose(idata->out);
      VFSFileDelete(idata->volume, oname);
      VFSFileCreate(idata->volume, oname);
      err = VFSFileOpen(idata->volume, oname, vfsModeWrite, &idata->out);
    }
  } else {
    debug(DEBUG_TRACE, "Command", "setio: no output");
  }
  plibc_setfd(1, idata->out);

  if (iname) xfree(iname);
  if (oname) xfree(oname);

  return 0;
}

static int command_pterm_draw(uint8_t col, uint8_t row, uint8_t code, uint32_t fg, uint32_t bg, uint8_t attr, void *_data) {
  command_internal_data_t *idata = command_get_data();
  RGBColorType text, back, oldt, oldf, oldb;
  uint32_t aux, x, y;
  FontID old;

  if (col < idata->ncols && row < idata->nrows) {
    x = col * idata->fwidth;
    y = row * idata->fheight;
    old = FntSetFont(idata->font);
    if (attr & ATTR_INVERSE) {
      aux = fg;
      fg = bg;
      bg = aux;
    }
    LongToRGB(fg, &text);
    LongToRGB(bg, &back);
    WinSetTextColorRGB(&text, &oldt);
    WinSetBackColorRGB(&back, &oldb);
    WinPaintChar(code, x, y);
    if (attr & ATTR_UNDERSCORE) {
      WinSetForeColorRGB(&text, &oldf);
      WinPaintLine(x, y + idata->fheight - 1, x + idata->fwidth - 1, y + idata->fheight - 1);
      WinSetForeColorRGB(&oldf, NULL);
    }
    WinSetTextColorRGB(&oldt, NULL);
    WinSetBackColorRGB(&oldb, NULL);
    FntSetFont(old);
  }

  return 0;
}

static int command_pterm_erase(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, uint32_t bg, uint8_t attr, void *_data) {
  command_internal_data_t *idata = command_get_data();
  RGBColorType back, oldb;
  RectangleType rect;

  if (col1 < idata->ncols && col2 <= idata->ncols && row1 < idata->nrows && row2 <= idata->nrows && col2 >= col1 && row2 >= row1) {
    //if (attr & ATTR_BRIGHT) bg += 8;
    LongToRGB(bg, &back);
    WinSetBackColorRGB(&back, &oldb);
    RctSetRectangle(&rect, col1 * idata->fwidth, row1 * idata->fheight, (col2 - col1) * idata->fwidth, (row2 - row1) * idata->fheight);
    WinEraseRectangle(&rect, 0);
    WinSetBackColorRGB(&oldb, NULL);
  }

  return 0;
}

static int command_pterm_scroll(uint8_t row1, uint8_t row2, int16_t dir, void *_data) {
  command_internal_data_t *idata = command_get_data();
  RectangleType rect, vacated;

  if (row1 < idata->nrows && row2 <= idata->nrows && row2 >= row1) {
    RctSetRectangle(&rect, 0, row1 * idata->fheight, idata->ncols * idata->fwidth, (row2 - row1) * idata->fheight);
    WinScrollRectangle(&rect, dir < 0 ? winUp : winDown, idata->fheight, &vacated);
  }

  return 0;
}

static int command_function_cmain(int pe, void *data) {
  command_ext_t *cmd = (command_ext_t *)data;
  DmOpenRef dbRef;
  Boolean firstLoad;
  int (*cmain)(int argc, char *argv[]);
  char *argv[MAX_ARGC], *arg;
  void *lib;
  int argc, r, i;

  if (cmd) {
    argv[0] = cmd->name;
    for (argc = 1; argc < MAX_ARGC; argc++) {
      if (script_opt_string(pe, argc-1, &arg) != 0) break;
      argv[argc] = arg;
    }

    dbRef = NULL;
    lib = NULL;
    cmain = NULL;

    if (cmd->cmain == NULL) {
      if ((dbRef = DmOpenDatabase(0, cmd->dbID, dmModeReadOnly)) != NULL) {
        if ((lib = DmResourceLoadLib(dbRef, sysRsrcTypeDlib, &firstLoad)) != NULL) {
          debug(DEBUG_TRACE, "Command", "unsafe command '%s' loaded", cmd->name);
          cmain = sys_lib_defsymbol(lib, "CommandMain", 1);
        }
      }
    } else {
      debug(DEBUG_TRACE, "Command", "safe command '%s' already loaded", cmd->name);
      cmain = cmd->cmain;
    }

    if (cmain) {
      if (lib == NULL || mutex_lock(cmd->mutex) == 0) {
        r = cmain(argc, argv);
        debug(DEBUG_TRACE, "Command", "command '%s' returned %d", cmd->name, r);
        if (lib != NULL) mutex_unlock(cmd->mutex);
      }
    }

    if (lib) {
      debug(DEBUG_TRACE, "Command", "unsafe command '%s' unloaded", cmd->name);
      sys_lib_close(lib);
    }

    if (dbRef) {
      DmCloseDatabase(dbRef);
    }

    for (i = 1; i < argc; i++) {
      if (argv[i]) xfree(argv[i]);
    }
  }

  return 0;
}

static void command_load_external_commands(command_internal_data_t *idata) {
  DmSearchStateType stateInfo;
  LocalID dbID;
  DmOpenRef dbRef;
  MemHandle h;
  Boolean safe, newSearch, firstLoad;
  char name[dmDBNameLength], *s;
  int (*commandMain)(int argc, char *argv[]);
  int i;
  void *lib;

  for (newSearch = true, i = 0; i < MAX_EXTERNAL_CMDS; newSearch = false) {
    if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, commandExternalType, 0, false, NULL, &dbID) != errNone) break;

    if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == errNone) {
      if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
        if ((lib = DmResourceLoadLib(dbRef, sysRsrcTypeDlib, &firstLoad)) != NULL) {
          commandMain = sys_lib_defsymbol(lib, "CommandMain", 1);
          safe = false;
          if (commandMain) {
            StrNCopy(idata->ext_commands[i].name, name, dmDBNameLength);
            if ((h = DmGet1Resource(commandNameType, 1000)) != NULL) {
              if ((s = MemHandleLock(h)) != NULL) {
                StrNCopy(idata->ext_commands[i].name, s, dmDBNameLength);
                MemHandleUnlock(h);
              }
              DmReleaseResource(h);
            }
            if ((h = DmGet1Resource(commandSafeType, 1000)) != NULL) {
              if ((s = MemHandleLock(h)) != NULL) {
                safe = s[0] != 0; 
                MemHandleUnlock(h);
              }
              DmReleaseResource(h);
            }
            idata->ext_commands[i].dbID = dbID; 
            if (safe) {
              debug(DEBUG_INFO, "Command", "safe command '%s' loaded", idata->ext_commands[i].name);
              idata->ext_commands[i].cmain = commandMain;
              idata->ext_commands[i].mutex = NULL;
              idata->ext_commands[i].lib = lib;
            } else {
              debug(DEBUG_INFO, "Command", "unsafe command '%s' will be loaded on demand", idata->ext_commands[i].name);
              idata->ext_commands[i].cmain = NULL; 
              idata->ext_commands[i].mutex = mutex_create(idata->ext_commands[i].name); 
              idata->ext_commands[i].lib = NULL;
            }
            pumpkin_script_global_function_data(idata->pe, idata->ext_commands[i].name, command_function_cmain, &idata->ext_commands[i]);
            i++;

          } else {
            debug(DEBUG_ERROR, "Command", "CommandMain not found in dlib");
          }

          if (!safe) {
            sys_lib_close(lib);
          }
        }
        DmCloseDatabase(dbRef);
      }
    }
  }
}

static Err StartApplication(void *param) {
  command_data_t *data;
  command_internal_data_t *idata;
  UInt32 iterator, swidth, sheight, density;
  UInt16 prefsSize;
  FontID font, old;
  uint32_t color;
  int i;

  FrmCenterDialogs(true);

  data = xcalloc(1, sizeof(command_data_t));
  idata = xcalloc(1, sizeof(command_internal_data_t));
  data->idata = idata;
  pumpkin_set_data(data);
  plibc_init();
  plibc_dup(2); // reserve file desriptor 3 (2 is duplicated to 3)

  WinScreenGetAttribute(winScreenDensity, &density);

  prefsSize = sizeof(command_prefs_t);
  if (PrefGetAppPreferences(AppID, 1, &idata->prefs, &prefsSize, true) == noPreferenceFound) {
    idata->prefs.font = density == kDensityLow ? font6x10Idl : font8x14Id;
    idata->prefs.foreground = defaultForeground;
    idata->prefs.background = defaultBackground;
    idata->prefs.highlight  = defaultHighlight;
    PrefSetAppPreferences(AppID, 1, 1, &idata->prefs, sizeof(command_prefs_t), true);
  }

  iterator = vfsIteratorStart;
  VFSVolumeEnumerate(&idata->volume, &iterator);

  idata->wait = SysTicksPerSecond() / 4;
  VFSChangeDir(idata->volume, "/");
  VFSCurrentDir(idata->volume, idata->cwd, MAXCMD);

  font = idata->prefs.font - ((density == kDensityLow) ? 9100 : 9000);
  idata->font = font;
  old = FntSetFont(font);

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  idata->fwidth = FntCharWidth('A');
  idata->fheight = FntCharHeight();
  FntSetFont(old);
  idata->ncols = swidth  / idata->fwidth;
  idata->nrows = sheight / idata->fheight;
  idata->t = pterm_init(idata->ncols, idata->nrows, 1);
  idata->cb.draw = command_pterm_draw;
  idata->cb.erase = command_pterm_erase;
  idata->cb.scroll = command_pterm_scroll;
  idata->cb.data = data;
  pterm_callback(idata->t, &idata->cb);

  color = RGBToLong(&idata->prefs.foreground);
  pterm_setfg(idata->t, color);
  color = RGBToLong(&idata->prefs.background);
  pterm_setbg(idata->t, color);

  if ((idata->pe = pumpkin_script_create()) > 0) {
    pumpkin_script_init_env(idata->pe);
    script_loadlib(idata->pe, "libshell");
    script_loadlib(idata->pe, "liboshell");

    for (i = 0; builtinCommands[i].name; i++) {
      pumpkin_script_global_function(idata->pe, builtinCommands[i].name, builtinCommands[i].function);
    }

    command_load_external_commands(idata);
  }

  pumpkin_setio(command_getchar, command_haschar, command_putchar, command_setcolor, idata);

  FrmGotoForm(MainForm);

  return errNone;
}

static void check_telnet(void) {
  command_internal_data_t *idata = command_get_data();
  conn_filter_t *conn, *telnet;
  uint8_t b;
  int r;

  if (idata->telnet != NULL) {
    telnet = idata->telnet;
    for (;;) {
      r = telnet->peek(telnet, 0);
      if (r == 0) break;
      if (r == -1 || telnet->read(telnet, &b) == -1) {
        idata->telnet = NULL;
        conn = telnet->next;
        telnet_close(telnet);
        conn_close(conn);
        idata->wait = SysTicksPerSecond() / 4;
        break;
      }
      command_putc(idata, b);
    }
  }
}

static void EventLoop() {
  command_internal_data_t *idata = command_get_data();
  EventType event;
  Err err;

  do {
    check_telnet();
    EvtGetEvent(&event, idata->wait);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);

  } while (event.eType != appStopEvent);
}

static void StopApplication(void) {
  command_data_t *data = pumpkin_get_data();
  command_internal_data_t *idata = data->idata;
  int i;

  FrmCloseAllForms();
  PrefSetAppPreferences(AppID, 1, 1, &idata->prefs, sizeof(command_prefs_t), true);

  plibc_close(3);
  plibc_finish();
  pumpkin_script_finish_env();
  pumpkin_script_destroy(idata->pe);

  for (i = 0; i < MAX_EXTERNAL_CMDS; i++) {
    if (idata->ext_commands[i].lib) {
      debug(DEBUG_INFO, "Command", "external command '%s' unloaded", idata->ext_commands[i].name);
      sys_lib_close(idata->ext_commands[i].lib);
    }
    if (idata->ext_commands[i].mutex) {
      mutex_destroy(idata->ext_commands[i].mutex);
    }
  }

  xfree(idata);
  xfree(data);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      break;
    default:
      debug(DEBUG_INFO, "Command", "launch code %d ignored", cmd);
      return 0;
  }

  if (StartApplication((void *)cmdPBP) == errNone) {
    EventLoop();
    StopApplication();
  }

  return 0;
}
