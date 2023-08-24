#include <PalmOS.h>
#include <VFSMgr.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "thread.h"
#include "pumpkin.h"
#include "endianness.h"
#include "util.h"
#include "sys.h"
#include "debug.h"
#include "resource.h"

#include "doomkeys.h"
#include "m_argv.h"
#include "i_video.h"
#include "m_config.h"
#include "game.h"
#include "host.h"

#define ROOT "/PALM/Programs/"

struct dg_file_t {
  FileRef fr;
};

struct dg_dir_t {
  FileRef fr;
  UInt32 iterator;
};

#define MAX_ITEMS 10

static UInt16 volref;
static uint64_t t0;
static UInt32 screenWidth, screenHeight;
static int scale, width, height;
static uint32_t modMask, keyMask;
static uint64_t extKeyMask[2];
static char *title;
static int gameIndex, extraIndex;
static int gameWads, extraWads;
static char *gameItems[MAX_ITEMS];
static int gameVariantIndex[MAX_ITEMS];
static char *gameExtraItems[MAX_ITEMS];
static Boolean ready;

#define KEYQUEUE_SIZE 16

static uint16_t KeyQueue[KEYQUEUE_SIZE];
static uint32_t KeyQueueWriteIndex;
static uint32_t KeyQueueReadIndex;

#define Y0 30

static uint16_t *DG_ScreenBuffer;

extern void D_DoomMain(void);

uint16_t DG_color16(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t c16 = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, r, g, b, 0xFF);
  return sys_htobe16(c16);
}

static void DG_Init(void) {
  t0 = sys_get_clock();
  modMask = 0;
  keyMask = 0;
  extKeyMask[0] = 0;
  extKeyMask[1] = 0;
  KeyQueueWriteIndex = 0;
  KeyQueueReadIndex = 0;
  DG_ScreenBuffer = sys_malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 2);
}

static void DG_Finish(void) {
  sys_free(DG_ScreenBuffer);
}

static uint8_t convertToDoomKey(int key) {
  if (key >= 'A' && key <= 'Z') {
    key += 32;
  }

  return key;
}

static void addKeyToQueue(int down, int key){
  uint16_t keyData = (down << 8) | key;
  KeyQueue[KeyQueueWriteIndex] = keyData;
  KeyQueueWriteIndex++;
  KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

static void process_normal_keys(uint32_t oldMask, uint32_t newMask) {
  uint32_t diff;

  diff = oldMask ^ newMask;
  if (diff) {
    if (diff & keyBitPageDown) {
      addKeyToQueue(newMask & keyBitPageDown ? 1 : 0, KEY_DOWNARROW);
    }
    if (diff & keyBitPageUp) {
      addKeyToQueue(newMask & keyBitPageUp ? 1 : 0, KEY_UPARROW);
    }
    if (diff & keyBitLeft) {
      addKeyToQueue(newMask & keyBitLeft ? 1 : 0, KEY_LEFTARROW);
    }
    if (diff & keyBitRight) {
      addKeyToQueue(newMask & keyBitRight ? 1 : 0, KEY_RIGHTARROW);
    }
  }
}

static void process_mod_keys(uint32_t oldMask, uint32_t newMask) {
  uint32_t diff;

  diff = oldMask ^ newMask;
  if (diff) {
    if (diff & WINDOW_MOD_CTRL) {
      addKeyToQueue(newMask & WINDOW_MOD_CTRL ? 1 : 0, KEY_RCTRL);
    }
    if (diff & WINDOW_MOD_SHIFT) {
      addKeyToQueue(newMask & WINDOW_MOD_SHIFT ? 1 : 0, KEY_RSHIFT);
    }
  }
}

static void process_ext_keys(uint64_t oldMask, uint64_t newMask, int offset) {
  uint64_t diff;
  int i, key;

  diff = oldMask ^ newMask;
  if (diff) {
    for (i = 0; i < 64; i++) {
      if (diff & 1) {
        key = convertToDoomKey(offset + i);
        addKeyToQueue(newMask & 1, key);
      }
      newMask >>= 1;
      diff >>= 1;
    }
  }
}

static void process_keys(void) {
  uint32_t _keyMask, _modMask;
  uint64_t _extKeyMask[2];

  pumpkin_status(NULL, NULL, &_keyMask, &_modMask, NULL, _extKeyMask);

  process_normal_keys(keyMask, _keyMask);
  keyMask = _keyMask;

  process_mod_keys(modMask, _modMask);
  modMask = _modMask;

  process_ext_keys(extKeyMask[0], _extKeyMask[0], 0);
  extKeyMask[0] = _extKeyMask[0];

  process_ext_keys(extKeyMask[1], _extKeyMask[1], 64);
  extKeyMask[1] = _extKeyMask[1];
}

static Boolean validwindow(void) {
  FormType *frm = FrmGetActiveForm();
  return frm && FrmGetWindowHandle(frm) == WinGetActiveWindow();
}

uint16_t *DG_GetScreenBuffer(void) {
  return DG_ScreenBuffer;
}

void DG_DrawFrame(void) {
  WinHandle wh;
  BitmapType *bmp;
  uint16_t *src, *dst, c16;
  int i, j, x0, x, y;

  if (!validwindow() || !ready) return;
  process_keys();

  wh = WinGetDisplayWindow();
  bmp = WinGetBitmap(wh);
  dst = BmpGetBits(bmp);
  dst += Y0 * screenWidth;
  src = DG_ScreenBuffer;
  x0 = 0;

  if (screenWidth == SCREENWIDTH) {
    MemMove(dst, src, SCREENWIDTH * SCREENHEIGHT * 2);

  } else if (scale == 1) {
    x0 = (screenWidth - SCREENWIDTH) / 2;
    dst += x0;
    for (y = 0; y < SCREENHEIGHT; y++) {
      MemMove(dst, src, SCREENWIDTH * 2);
      src += SCREENWIDTH;
      dst += screenWidth;
    }

  } else {
    x0 = screenWidth / 2 - SCREENWIDTH;
    dst += x0;
    for (y = 0, i = 0, j = 0; y < SCREENHEIGHT; y++) {
      for (x = 0; x < SCREENWIDTH; x++) {
        c16 = src[i++];
        dst[j] = c16;
        dst[j + 1] = c16;
        dst[screenWidth + j] = c16;
        dst[screenWidth + j + 1] = c16;
        j += 2;
      }
      j += screenWidth;
    }
  }

  pumpkin_screen_dirty(wh, x0, Y0, width, height);
}

uint32_t DG_GetTicksMs(void) {
  return (sys_get_clock() - t0) / 1000;
}

static void setGroupControl(FormType *frm, UInt16 id, UInt16 i, int value) {
  UInt16 index;
  ControlType *ctl;

  index = FrmGetObjectIndex(frm, id + i);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, value);
}

static int getGroupControl(FormType *frm, UInt16 id, UInt16 num) {
  UInt16 index, i;
  ControlType *ctl;

  for (i = 0; i < num; i++) {
    index = FrmGetObjectIndex(frm, id + i);
    ctl = FrmGetObjectPtr(frm, index);
    if (CtlGetValue(ctl)) return i;
  }

  return 0;
}

static void setControl(FormType *frm, UInt16 id, int value) {
  setGroupControl(frm, id, 0, value);
}

static int getControl(FormType *frm, UInt16 id) {
  UInt16 index;
  ControlType *ctl;

  index = FrmGetObjectIndex(frm, id);
  ctl = FrmGetObjectPtr(frm, index);
  return CtlGetValue(ctl);
}

static void setIntVariable(char *name, int n) {
  char value[16];
  sys_snprintf(value, sizeof(value), "%d", n);
  M_SetVariable(name, value);
}

static void configure(void) {
  FormType *frm, *prev;
  int msg = -1, gamma = -1;

  prev = FrmGetActiveForm();
  frm = FrmInitForm(ConfigForm);
  setControl(frm, msgCtl, M_GetIntVariable(gameMsgOn()) ? 1 : 0);
  setGroupControl(frm, gammaCtl, M_GetIntVariable("usegamma"), 1);

  if (FrmDoDialog(frm) == okBtn) {
    msg = getControl(frm, msgCtl);
    gamma = getGroupControl(frm, gammaCtl, 4);
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(prev);

  if (msg != -1 && gamma != -1) {
    setIntVariable(gameMsgOn(), msg);
    setIntVariable("usegamma", gamma);
    setPalette();
  }
}

void MenuEvent(UInt16 id) {
  switch (id) {
    case menuConfig:
      configure();
      break;
    case menuQuit:
      M_Quit();
      break;
  }
}

static void resize(FormType *frm) {
  //UInt16 objIndex, i;
  UInt32 sw, sh;
  WinHandle wh;
  RectangleType rect;

  WinScreenMode(winScreenModeGet, &sw, &sh, NULL, NULL);
  wh = FrmGetWindowHandle(frm);
  RctSetRectangle(&rect, 0, 0, sw, sh);
  WinSetBounds(wh, &rect);

/*
  for (i = 0; i < 8; i++) {
    if ((objIndex = FrmGetObjectIndex(frm, gameCtl + i)) == 0xffff) break;
    FrmGetObjectBounds(frm, objIndex, &rect);
    rect.topLeft.x = (sw - rect.extent.x) / 2;
    FrmSetObjectBounds(frm, objIndex, &rect);
  }
*/
}

static Boolean MainFormHandleEvent(EventType *event) {
  FormPtr frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resize(frm);
      FrmSetTitle(frm, title);
      FrmDrawForm(frm);
      ready = true;
      handled = true;
      break;
    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;
    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static char *getGameRoot(char *name) {
  FileRef fr;
  char *rootdir;
  int len;

  len = StrLen(ROOT) + StrLen(name) + 1;
  rootdir = MemPtrNew(len);
  sys_snprintf(rootdir, len, "%s%s", ROOT, name);

  if (VFSFileOpen(volref, rootdir, vfsModeRead, &fr) == errNone) {
    VFSFileClose(fr);
  } else {
    VFSDirCreate(volref, rootdir);
  }

  return rootdir;
}

static void gameStart(void) {
  FileRef fr;
  char *rootdir, *wad, *wadfile, *extrawad, *config, *savedir, *argv[16];
  UInt16 len, i;

  title = gameName();
  rootdir = getGameRoot(title);

  i = gameVariantIndex[gameIndex];
  wadfile = gameWad(i);
  len = StrLen(rootdir) + 1 + StrLen(wadfile) + 1;
  wad = MemPtrNew(len);
  sys_snprintf(wad, len, "%s/%s", rootdir, wadfile);

  len = StrLen(rootdir) + 1 + 6 + 1 + 4 + 1;
  config = MemPtrNew(len);
  sys_snprintf(config, len, "%s/config%d.cfg", rootdir, i);
  
  if (VFSFileOpen(volref, config, vfsModeRead, &fr) == errNone) {
    VFSFileClose(fr);
  } else {
    VFSFileCreate(volref, config);
  }

  len = StrLen(rootdir) + 1 + 7 + 1 + 1;
  savedir = MemPtrNew(len);
  sys_snprintf(savedir, len, "%s/savedir%d", rootdir, i);

  if (VFSFileOpen(volref, savedir, vfsModeRead, &fr) == errNone) {
    VFSFileClose(fr);
  } else {
    VFSDirCreate(volref, savedir);
  }

  myargc = 0;
  argv[myargc++] = title;
  argv[myargc++] = "-iwad";
  argv[myargc++] = wad;
  argv[myargc++] = "-config";
  argv[myargc++] = config;
  argv[myargc++] = "-savedir";
  argv[myargc++] = savedir;
  if (extraIndex > 0) {
    len = StrLen(rootdir) + 1 + StrLen(gameExtraItems[extraIndex]) + 1;
    extrawad = MemPtrNew(len);
    sys_snprintf(extrawad, len, "%s/%s", rootdir, gameExtraItems[extraIndex]);
    argv[myargc++] = "-file";
    argv[myargc++] = extrawad;
  } else {
    extrawad = NULL;
  }
  argv[myargc] = NULL;
  myargv = argv;

  M_FindResponseFile();
  M_SetExeDir();
  DG_Init();
  D_DoomMain();
  DG_Finish();

  for (i = 0; i < MAX_ITEMS; i++) {
    if (gameExtraItems[i]) MemPtrFree(gameExtraItems[i]);
  }

  if (extrawad) MemPtrFree(extrawad);
  MemPtrFree(savedir);
  MemPtrFree(config);
  MemPtrFree(wad);
  MemPtrFree(rootdir);
}

static void setList(FormPtr frm, UInt16 extraID, UInt16 listID, UInt16 controlID, UInt16 width, UInt16 num, char **items) {
  ListType *lst;
  ControlType *ctl;
  RectangleType rect;
  UInt16 index;

  index = FrmGetObjectIndex(frm, listID);
  lst = (ListType *)FrmGetObjectPtr(frm, index);
  if (num > 1) {
    LstSetHeight(lst, num);
    FrmGetObjectBounds(frm, index, &rect);
    rect.extent.x = width + 8;
    FrmSetObjectBounds(frm, index, &rect);
    LstSetListChoices(lst, items, num);
    LstSetSelection(lst, 0);
  }

  index = FrmGetObjectIndex(frm, controlID);
  if (num > 1) {
    ctl = (ControlType *)FrmGetObjectPtr(frm, index);
    CtlSetLabel(ctl, LstGetSelectionText(lst, 0));
  } else {
    FrmHideObject(frm, index);
    index = FrmGetObjectIndex(frm, extraID);
    FrmHideObject(frm, index);
  }
}

static Boolean ChooseFormHandleEvent(EventType *event) {
  FormPtr frm;
  ListType *lst;
  UInt16 index, width, maxWidth, i;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resize(frm);

      // build game variations list
      maxWidth = 0;
      for (i = 0; i < gameWads; i++) {
        width = FntCharsWidth(gameItems[i], StrLen(gameItems[i]));
        if (width > maxWidth) maxWidth = width;
      }
      setList(frm, gameLbl, gameList, gameCtl, maxWidth, gameWads, gameItems);

      // build extra WADs list
      maxWidth = 0;
      for (i = 0; i < extraWads; i++) {
        width = FntCharsWidth(gameExtraItems[i], StrLen(gameExtraItems[i]));
        if (width > maxWidth) maxWidth = width;
      }
      setList(frm, extraLbl, extraList, extraCtl, maxWidth, extraWads, gameExtraItems);

      FrmDrawForm(frm);
      ready = true;
      handled = true;
      break;

    case ctlSelectEvent:
      if (event->data.ctlSelect.controlID == goBtn) {
        frm = FrmGetActiveForm();
        index = FrmGetObjectIndex(frm, gameList);
        lst = (ListType *)FrmGetObjectPtr(frm, index);
        gameIndex = LstGetSelection(lst);
        if (gameExtraItems[1]) {
          index = FrmGetObjectIndex(frm, extraList);
          lst = (ListType *)FrmGetObjectPtr(frm, index);
          extraIndex = LstGetSelection(lst);
        }
        handled = true;
      }
      break;

    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}


static void SetEventHandler(FormType *frm, Int16 form) {
  switch (form) {
    case MainForm:
      FrmSetEventHandler(frm, MainFormHandleEvent);
      break;
    case ChooseForm:
      FrmSetEventHandler(frm, ChooseFormHandleEvent);
      break;
  }
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormPtr frm;
  UInt16 form;
  Boolean handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);
      SetEventHandler(frm, form);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static Boolean EventLoop(void) {
  EventType event;
  Err err;

  for (;;) {
    if (thread_must_end()) return false;

    EvtGetEvent(&event, 20);
    if (!SysHandleEvent(&event)) {
      if (!MenuHandleEvent(NULL, &event, &err)) {
        if (!ApplicationHandleEvent(&event)) {
          FrmDispatchEvent(&event);
        }
      }
    }

    if (event.eType == appStopEvent) return false;
    if (gameIndex >= 0) break;
  }

  return true;
}

int DG_SleepMs(uint32_t ms) {
  unsigned char *buf;
  unsigned int len;
  uint32_t wait;
  EventType event;
  Err err;

  for (;;) {
    if (thread_must_end()) return -1;

    EvtGetEventUs(&event, ms * 1000);
    if (!SysHandleEvent(&event)) {
      if (!MenuHandleEvent(NULL, &event, &err)) {
        if (!ApplicationHandleEvent(&event)) {
          FrmDispatchEvent(&event);
        }
      }
    }

    wait = (ms && event.eType != nilEvent) ? ms * 1000 : 0;
    if (thread_server_read_timeout(wait, &buf, &len) == -1) return -1;
    if (buf) sys_free(buf);

    if (event.eType == appStopEvent) return -1;
    if (validwindow()) break;
  }

  return 0;
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
  uint16_t keyData;

  if (KeyQueueReadIndex == KeyQueueWriteIndex) {
    return 0;
  }

  keyData = KeyQueue[KeyQueueReadIndex];
  KeyQueueReadIndex++;
  KeyQueueReadIndex %= KEYQUEUE_SIZE;

  *pressed = keyData >> 8;
  *doomKey = keyData & 0xFF;

  return 1;
}

void DG_SetWindowTitle(const char *_title) {
  FormType *frm;

  title = (char *)_title;
  if ((frm = FrmGetActiveForm()) != NULL) {
    FrmSetTitle(frm, title);
  }
}

void DG_Fatal(const char *filename, const char *function, int lineNo, char *msg) {
  ErrDisplayFileLineMsgEx(filename, function, lineNo, msg, 0);
}

int DG_create(char *name) {
  return VFSFileCreate(volref, name) == errNone ? 0 : -1;
}

dg_file_t *DG_open(char *name, int wr) {
  dg_file_t *f;
  Err err;

  if ((f = sys_calloc(1, sizeof(dg_file_t))) != NULL) {
    if (wr) {
      if ((err = VFSFileOpen(volref, name, vfsModeWrite, &f->fr)) != errNone) {
        VFSFileCreate(volref, name);
        err = VFSFileOpen(volref, name, vfsModeWrite, &f->fr);
      }
    } else {
      err = VFSFileOpen(volref, name, vfsModeRead, &f->fr);
    }
    if (err != errNone) {
      sys_free(f);
      f = NULL;
    }
  }

  return f;
}

void DG_close(dg_file_t *f) {
  if (f) {
    VFSFileClose(f->fr);
    sys_free(f);
  }
}

int DG_eof(dg_file_t *f) {
  int r = 1;

  if (f) {
    if (VFSFileEOF(f->fr) == errNone) {
      r = 0;
    }
  }

  return r;
}

int DG_isdir(char *name) {
  FileRef fr;
  UInt32 attr;
  int r = 0;

  if (VFSFileOpen(volref, name, vfsModeRead, &fr) == errNone) {
    if (VFSFileGetAttributes(fr, &attr) == errNone) {
      r = (attr & vfsFileAttrDirectory) ? 1 : 0;
    }
    VFSFileClose(fr);
  }

  return r;
}

int DG_read(dg_file_t *f, void *buf, int n) {
  UInt32 nread;
  int r = -1;

  if (f) {
    if (VFSFileRead(f->fr, n, buf, &nread) == errNone) {
      r = nread;
    }
  }

  return r;
}

int DG_write(dg_file_t *f, void *buf, int n) {
  UInt32 nwritten;
  int r = -1;

  if (f) {
    if (VFSFileWrite(f->fr, n, buf, &nwritten) == errNone) {
      r = nwritten;
    }
  }

  return r;
}

int DG_seek(dg_file_t *f, int offset) {
  int r = -1;

  if (f) {
    if (VFSFileSeek(f->fr, vfsOriginBeginning, offset) == errNone) {
      r = 0;
    }
  }

  return r;
}

int DG_tell(dg_file_t *f) {
  UInt32 pos;
  int r = 0;

  if (r) {
    if (VFSFileTell(f->fr, &pos) == errNone) {
      r = pos;
    }
  }

  return r;
}

int DG_filesize(dg_file_t *f) {
  UInt32 size;
  int r = 0;

  if (f) {
    if (VFSFileSize(f->fr, &size) == errNone) {
      r = size;
    }
  }

  return r;
}

int DG_mkdir(char *name) {
  FileRef fr;
  UInt32 attr;
  int r = 0;

  if (VFSFileOpen(volref, name, vfsModeRead, &fr) == errNone) {
    if (VFSFileGetAttributes(fr, &attr) == errNone) {
      r = (attr & vfsFileAttrDirectory) ? 0 : -1;
    }
    VFSFileClose(fr);
  } else {
    r = VFSDirCreate(volref, name) == errNone ? 0 : -1;
  }

  return r;
}

int DG_remove(char *name) {
  return VFSFileDelete(volref, name) == errNone ? 0 : -1;
}

int DG_rename(char *oldname, char *newname) {
  return VFSFileRename(volref, oldname, newname) == errNone ? 0 : -1;
}

int DG_printf(dg_file_t *f, const char *fmt, ...) {
  sys_va_list ap;
  char buf[256];
  int n = 0;

  if (f && fmt) {
    sys_va_start(ap, fmt);
    n = sys_vsnprintf(buf, sizeof(buf), fmt, ap);
    sys_va_end(ap);
    DG_write(f, buf, StrLen(buf));
  }

  return n;
}

int DG_vprintf(dg_file_t *f, const char *fmt, sys_va_list ap) {
  char buf[256];
  int n = 0;

  if (f && fmt) {
    n = sys_vsnprintf(buf, sizeof(buf), fmt, ap);
    DG_write(f, buf, StrLen(buf));
  }

  return n;
}

char *DG_fgets(dg_file_t *f, char *s, int size) {
  char *r = s;
  UInt32 nread;
  int i = 0;

  if (f && s && size > 0) {
    for (; i < size-1;) {
      if (VFSFileEOF(f->fr) != errNone) break;
      if (VFSFileRead(f->fr, 1, &s[i], &nread) != errNone || nread != 1) {
        r = NULL;
        break;
      }
      i++;
      if (s[i-1] == '\n') {
        s[i] = 0;
        break;
      }
    }
  }

  return i ? r : NULL;
}

dg_dir_t *DG_opendir(char *name) {
  dg_dir_t *f;
  UInt32 attr;

  if ((f = sys_calloc(1, sizeof(dg_dir_t))) != NULL) {
    if (VFSFileOpen(volref, name, vfsModeRead, &f->fr) != errNone) {
      sys_free(f);
      return NULL;
    }
    f->iterator = vfsIteratorStart;

    VFSFileGetAttributes(f->fr, &attr);
    if (!(attr & vfsFileAttrDirectory)) {
      VFSFileClose(f->fr);
      sys_free(f);
      return NULL;
    }
  }

  return f;
}

int DG_readdir(dg_dir_t *f, char *name, int len) {
  FileInfoType info;
  int r = -1;

  if (f && name && len > 0) {
    info.nameP = name;
    info.nameBufLen = len;
    if (VFSDirEntryEnumerate(f->fr, &f->iterator, &info) == errNone) {
      r = (info.attributes == vfsFileAttrDirectory) ? 1 : 0;
    }
  }

  return r;
}

void DG_closedir(dg_dir_t *f) {
  if (f) {
    VFSFileClose(f->fr);
    sys_free(f);
  }
}

void DG_debug_full(const char *file, const char *func, int line, int level, const char *fmt, ...) {
  sys_va_list ap;

  sys_va_start(ap, fmt);
  debugva_full(file, func, line, level, "DOOM", fmt, ap);
  sys_va_end(ap);
}

void DG_debugva_full(const char *file, const char *func, int line, int level, const char *fmt, sys_va_list ap) {
  debugva_full(file, func, line, level, "DOOM", fmt, ap);
}

static void gamePrepare(void) {
  FileRef fr;
  FileInfoType info;
  UInt32 iterator;
  UInt16 variants, i;
  Boolean present[MAX_ITEMS];
  char *rootdir, *ext, name[128];

  title = gameName();
  rootdir = getGameRoot(title);
  variants = gameVariants();

  MemSet(gameExtraItems, sizeof(gameExtraItems), 0);
  extraWads = 1;
  extraIndex = 0;
  gameWads = 0;
  for (i = 0; i < MAX_ITEMS; i++) {
    present[i] = false;
  }

  if (VFSFileOpen(volref, rootdir, vfsModeRead, &fr) == errNone) {
    iterator = vfsIteratorStart;
    for (;;) {
      info.nameP = name;
      info.nameBufLen = sizeof(name);
      if (VFSDirEntryEnumerate(fr, &iterator, &info) != errNone) break;
      if (info.attributes & vfsFileAttrDirectory) continue;
      ext = getext(name);
      if (!ext || StrCaselessCompare(ext, "wad")) continue;
      for (i = 0; i < variants; i++) {
        if (!StrCompare(name, gameWad(i))) break;
      }
      if (i < gameVariants()) {
        // game WAD
        if (i < MAX_ITEMS) {
          present[i] = true;
        }
      } else {
        // extra WAD
        if (extraWads < MAX_ITEMS) {
          gameExtraItems[extraWads] = MemPtrNew(StrLen(name) + 1);
          StrCopy(gameExtraItems[extraWads], name);
          extraWads++;
        }
      }
    }
    VFSFileClose(fr);

    for (i = 0; i < MAX_ITEMS; i++) {
      if (present[i]) {
        gameVariantIndex[gameWads] = i;
        gameItems[gameWads] = gameVariant(i);
        gameWads++;
      }
    }
  }

  MemPtrFree(rootdir);

  if (gameWads == 0) {
    gameIndex = -1;
    pumpkin_error_dialog("Required WAD(s) were not found");
    pumpkin_fatal_error(0);

  } else if (gameWads == 1 && extraWads == 1) {
    gameIndex = 0;
    FrmGotoForm(MainForm);
    gameStart();

  } else {
    gameExtraItems[0] = MemPtrNew(5);
    StrCopy(gameExtraItems[0], "none");
    FrmGotoForm(ChooseForm);
    gameIndex = -1;
    if (EventLoop()) {
      gameStart();
    }
  }
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  UInt32 iterator;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    iterator = vfsIteratorStart;
    VFSVolumeEnumerate(&volref, &iterator);

    WinScreenGetAttribute(winScreenWidth, &screenWidth);
    WinScreenGetAttribute(winScreenHeight, &screenHeight);
    if (screenWidth >= SCREENWIDTH * 2 && (screenHeight - Y0) >= SCREENHEIGHT * 2) {
      scale = 2;
    } else {
      scale = 1;
    }
    width = SCREENWIDTH * scale;
    height = SCREENHEIGHT * scale;

    FrmCenterDialogs(true);
    ready = false;
    gamePrepare();
  }

  return 0;
}
