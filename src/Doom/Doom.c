#include <PalmOS.h>
#include <VFSMgr.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#include "script.h"
#include "thread.h"
#include "ptr.h"
#include "media.h"
#include "pwindow.h"
#include "pumpkin.h"
#include "endianness.h"
#include "sys.h"
#include "debug.h"
#include "xalloc.h"
#include "resource.h"

#include "doomgeneric.h"
#include "doomkeys.h"
#include "m_argv.h"
#include "m_menu.h"
#include "g_game.h"
#include "p_saveg.h"

#define FONT_STATUS 0
#define FONT_MENU   3
#define FONT_MSG    0

#define MAX_MENU_ITEMS 10
#define MAX_FACES 42

#define DOOM_ROOT "/PALM/Programs/Doom"

typedef struct {
  int pe;
  script_ref_t ref;
  int ptr;
  char *wad;
} doom_t;

typedef struct {
  char *msg;
  int x, y;
} msg_t;

struct dg_file_t {
  FileRef fr;
};

void D_DoomMain(void);
void M_FindResponseFile(void);
void dg_Create(void);

static uint32_t modMask, keyMask;
static uint64_t extKeyMask[2];

static UInt16 volref;
static Boolean quit;
static uint64_t t0;
static int width, height, cols, rows;
static int current_face;
static msg_t *msg;
static char status_top[256];
static int saveSlot;

#define KEYQUEUE_SIZE 16

static uint16_t KeyQueue[KEYQUEUE_SIZE];
static uint32_t KeyQueueWriteIndex;
static uint32_t KeyQueueReadIndex;

static const RGBColorType white  = {0, 0xFF, 0xFF, 0xFF};
static const RGBColorType yellow = {0, 0xFF, 0xFF, 0x00};
static const RGBColorType black  = {0, 0x00, 0x00, 0x00};

#define Y0 30

void DG_Init(void) {
  t0 = sys_get_clock();
  KeyQueueWriteIndex = 0;
  KeyQueueReadIndex = 0;
  xmemset(status_top, 0, sizeof(status_top));
}

static uint8_t convertToDoomKey(int key) {
  switch (key) {
    case 13:
      key = KEY_ENTER;
      break;
    case 27:
      key = KEY_ESCAPE;
      break;
    case 32:
      key = KEY_USE;
      break;
    default:
      if (key >= 'A' && key <= 'Z') {
        key += 32;
      }
      break;
    }

  return key;
}

static void addKeyToQueue(int down, int key){
  uint16_t keyData = (down << 8) | key;
  KeyQueue[KeyQueueWriteIndex] = keyData;
  KeyQueueWriteIndex++;
  KeyQueueWriteIndex %= KEYQUEUE_SIZE;
  //debug(1, "XXX", "key %d %s", key, down ? "down" : "up");
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
      addKeyToQueue(newMask & WINDOW_MOD_CTRL ? 1 : 0, KEY_FIRE);
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
  Boolean valid = FrmGetWindowHandle(FrmGetActiveForm()) == WinGetActiveWindow();
  M_Active(!valid);
  return valid;
}

void DG_DrawFrame(void) {
  WinHandle wh, old;
  BitmapType *bmp;
  RectangleType rect;
  uint32_t *src;
  uint16_t *dst, aux;
  int i, j, k, w, fh;
  int red, green, blue, alpha;

  if (!validwindow()) return;
  process_keys();

  WinSetCoordinateSystem(kCoordinatesDouble);
  wh = WinGetDisplayWindow();
  old = WinSetDrawWindow(wh);
  FntSetFont(font6x10);
  fh = FntCharHeight();
  WinSetTextColorRGB(&black, NULL);
  WinSetBackColorRGB(&white, NULL);

  bmp = WinGetBitmap(wh);
  dst = BmpGetBits(bmp);
  dst += Y0 * bmp->width;
  src = (uint32_t *)DG_ScreenBuffer;

  for (i = 0, k = 0; i < height; i++) {
    for (j = 0; j < width; j++, k++) {
      surface_rgb_color(SURFACE_ENCODING_ARGB, NULL, 0, src[k], &red, &green, &blue, &alpha);
      aux = surface_color_rgb(SURFACE_ENCODING_RGB565, NULL, 0, red, green, blue, 0xFF);
      dst[k] = htobe16(aux);
    }
  }

  if (status_top[0]) {
    WinPaintChars(status_top, StrLen(status_top), 0, Y0 + height);
    w = FntCharsWidth(status_top, StrLen(status_top));
  } else {
    w = 0;
  }
  RctSetRectangle(&rect, w, Y0 + height, 320 - w, fh);
  WinEraseRectangle(&rect, 0);

  WinSetTextColorRGB(&yellow, NULL);
  WinSetBackColorRGB(&black, NULL);

  for (i = 0; i < rows; i++) {
    if (msg[i].msg && msg[i].msg[0]) {
      WinPaintChars(msg[i].msg, StrLen(msg[i].msg), msg[i].x, Y0 + msg[i].y);
    }
  }

  WinSetDrawWindow(old);
  WinSetCoordinateSystem(kCoordinatesStandard);

  pumpkin_screen_dirty(wh, 0, 0, bmp->width, bmp->height);
}

void DG_StatusTop(char *s) {
  xmemset(status_top, 0, sizeof(status_top));
  if (s) strncpy(status_top, s, sizeof(status_top)-1);
}

void DG_Status(int health, int armor, int ammo0, int maxammo0, int ammo1, int maxammo1, int ammo2, int maxammo2, int ammo3, int maxammo3, int faceindex) {
  MemHandle h;
  BitmapType *bmp;
  RectangleType rect;
  char buf[64];
  int fh, w, x, y;

  if (!validwindow()) return;

  WinSetCoordinateSystem(kCoordinatesDouble);
  FntSetFont(font8x14);
  fh = FntCharHeight();
  WinSetTextColorRGB(&black, NULL);
  WinSetBackColorRGB(&white, NULL);

  snprintf(buf, sizeof(buf)-1, "Health: %3d%%", health);
  WinPaintChars(buf, StrLen(buf), 0, 320 - 2*fh);

  snprintf(buf, sizeof(buf)-1, "Armor : %3d%%", armor);
  WinPaintChars(buf, StrLen(buf), 0, 320 - fh);

  snprintf(buf, sizeof(buf)-1, "Ammo 1: %3d/%3d", ammo0, maxammo0);
  w = FntCharsWidth(buf, StrLen(buf));
  WinPaintChars(buf, StrLen(buf), 320 - w, 320 - 4*fh);
  snprintf(buf, sizeof(buf)-1, "Ammo 2: %3d/%3d", ammo1, maxammo1);
  WinPaintChars(buf, StrLen(buf), 320 - w, 320 - 3*fh);
  snprintf(buf, sizeof(buf)-1, "Ammo 3: %3d/%3d", ammo2, maxammo2);
  WinPaintChars(buf, StrLen(buf), 320 - w, 320 - 2*fh);
  snprintf(buf, sizeof(buf)-1, "Ammo 4: %3d/%3d", ammo3, maxammo3);
  WinPaintChars(buf, StrLen(buf), 320 - w, 320 - fh);

  if (faceindex >= 0 && faceindex < MAX_FACES && faceindex != current_face) {
    if ((h = DmGet1Resource(bitmapRsc, faceBmp + faceindex)) != NULL) {
      bmp = MemHandleLock(h);
      x = (320 - 32) / 2;
      y = 320 - 32;
      RctSetRectangle(&rect, x, y, 32, 32);
      WinEraseRectangle(&rect, 0);
      WinPaintBitmap(bmp, x, y);
      MemHandleUnlock(h);
      DmReleaseResource(h);
      current_face = faceindex;
    }
  }

  WinSetCoordinateSystem(kCoordinatesStandard);
}

void DG_ClearMsg(void) {
  int i;

  for (i = 0; i < rows; i++) {
    if (msg[i].msg) {
      xmemset(msg[i].msg, 0, cols);
    }
  }
}

void DG_PrintMsg(char *s) {
  int i, row, start, len, h, y;

  WinSetCoordinateSystem(kCoordinatesDouble);
  start = 0;
  row = 0;
  for (i = 0;; i++) {
    if (s[i] == '\n' || s[i] == 0) {
      if (msg[row].msg == NULL) {
        msg[row].msg = xcalloc(1, cols);
      } else {
        xmemset(msg[row].msg, 0, cols);
      }
      len = i - start;
      if (len > cols) len = cols;
      if (len > 0) strncpy(msg[row].msg, &s[start], len);
      msg[row].x = (width - FntCharsWidth(msg[row].msg, len)) / 2;
      start = i+1;
      row++;
    }
    if (s[i] == 0) break;
  }

  if (row > rows) row = rows;
  h = row * FntCharHeight();
  y = (height - h) / 2;

  for (i = 0; i < row; i++) {
    msg[i].y = y;
    y += FntCharHeight();
  }
  WinSetCoordinateSystem(kCoordinatesStandard);
}

uint32_t DG_GetTicksMs(void) {
  return (sys_get_clock() - t0) / 1000;
}

static void SetGroupControl(FormType *frm, UInt16 id, UInt16 i, UInt16 value) {
  UInt16 index;
  ControlType *ctl;

  index = FrmGetObjectIndex(frm, id + i);
  ctl = FrmGetObjectPtr(frm, index);
  CtlSetValue(ctl, value);
}

static UInt16 GetGroupControl(FormType *frm, UInt16 id, UInt16 max) {
  UInt16 index, i;
  ControlType *ctl;

  for (i = 0; i < max; i++) {
    index = FrmGetObjectIndex(frm, id + i);
    ctl = FrmGetObjectPtr(frm, index);
    if (CtlGetValue(ctl)) return i;
  }

  return 0;
}

static void NewGame(void) {
  FormType *frm, *prev;
  int skill = -1, episode = -1;

  prev = FrmGetActiveForm();
  frm = FrmInitForm(NewGameForm);
  SetGroupControl(frm, skillCtl, 2, 1);
  SetGroupControl(frm, epiCtl, 0, 1);

  if (FrmDoDialog(frm) == okBtn) {
    skill = GetGroupControl(frm, skillCtl, 5);
    episode = GetGroupControl(frm, epiCtl, 3);
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(prev);

  if (skill != -1 && episode != -1) {
    G_DeferedInitNew(skill, episode+1, 1);
  }
}

static void SaveGame(void) {
  FormType *frm, *prev;
  int slot = -1;
  char *descr = NULL;

  prev = FrmGetActiveForm();
  frm = FrmInitForm(SaveGameForm);
  SetGroupControl(frm, slotCtl, 0, 1);

  if (FrmDoDialog(frm) == okBtn) {
    slot = GetGroupControl(frm, slotCtl, 5);
    descr = "SaveGame";
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(prev);

  if (slot != -1 && descr) {
    saveSlot = slot;
    G_SaveGame(saveSlot, descr);
  }
}

static void LoadGame(void) {
  FormType *frm, *prev;
  char name[256];
  int slot = -1;

  prev = FrmGetActiveForm();
  frm = FrmInitForm(LoadGameForm);
  SetGroupControl(frm, slotCtl, 0, 1);

  if (FrmDoDialog(frm) == okBtn) {
    slot = GetGroupControl(frm, slotCtl, 5);
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(prev);

  if (slot != -1) {
    saveSlot = slot;
    StrNCopy(name, P_SaveGameFile(saveSlot), sizeof(name)-1);
    G_LoadGame(name);
  }
}

void MenuEvent(UInt16 id) {
  switch (id) {
    case menuNew:
      NewGame();
      break;
    case menuSave:
      SaveGame();
      break;
    case menuLoad:
      LoadGame();
      break;
    case menuQuit:
      M_Quit();
      quit = true;
      break;
  }
}

static Boolean MainFormHandleEvent(EventType *event) {
  FormPtr frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
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

static void SetEventHandler(FormType *frm, Int16 form) {
  switch (form) {
    case MainForm:
      FrmSetEventHandler(frm, MainFormHandleEvent);
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

int DG_SleepMs(uint32_t ms) {
  unsigned char *buf;
  unsigned int len;
  uint32_t wait;
  EventType event;
  Err err;

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
  if (buf) xfree(buf);

  return (event.eType == appStopEvent) ? -1 : 0;
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

void DG_SetWindowTitle(const char *title) {
  FormType *frm;

  if (title && title[0]) {
    frm = FrmGetActiveForm();
    FrmSetTitle(frm, (char *)title);
  }
}

void DG_Fatal(char *msg) {
  ErrFatalDisplay(msg);
}

int DG_create(char *name) {
  return VFSFileCreate(volref, name) == errNone ? 0 : -1;
}

dg_file_t *DG_open(char *name, int wr) {
  dg_file_t *f;

  if ((f = xcalloc(1, sizeof(dg_file_t))) != NULL) {
    if (VFSFileOpen(volref, name, wr ? vfsModeWrite : vfsModeRead, &f->fr) != errNone) {
      xfree(f);
      f = NULL;
    }
  }

  return f;
}

void DG_close(dg_file_t *f) {
  if (f) {
    VFSFileClose(f->fr);
    xfree(f);
  }
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
  return VFSDirCreate(volref, name) == errNone ? 0 : -1;
}

int DG_remove(char *name) {
  return VFSFileDelete(volref, name) == errNone ? 0 : -1;
}

int DG_rename(char *oldname, char *newname) {
  return VFSFileRename(volref, oldname, newname) == errNone ? 0 : -1;
}

void DG_debug_full(const char *file, const char *func, int line, int level, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  debugva_full(file, func, line, level, "DOOM", fmt, ap);
  va_end(ap);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  UInt32 iterator;
  MemHandle fontH, mfontH;
  FontType *font, *mfont;
  FileRef fr;
  char *argv[4];

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    iterator = vfsIteratorStart;
    VFSVolumeEnumerate(&volref, &iterator);
    if (VFSFileOpen(volref, DOOM_ROOT, vfsModeRead, &fr) == errNone) {
      VFSFileClose(fr);
    } else {
      VFSDirCreate(volref, DOOM_ROOT);
    }

    mfontH = DmGetResource(fontExtRscType, font8x14ID);
    mfont = MemHandleLock(mfontH);
    FntDefineFont(font8x14, mfont);

    fontH = DmGetResource(fontExtRscType, font6x10ID);
    font = MemHandleLock(fontH);
    FntDefineFont(font6x10, font);

    saveSlot = 0;
    current_face = -1;
    width = DOOMGENERIC_RESX;
    height = DOOMGENERIC_RESY;
    cols = width / FntCharWidth('A');
    rows = height / FntCharHeight();
    msg = xcalloc(rows, sizeof(msg_t));
    modMask = 0;
    keyMask = 0;
    extKeyMask[0] = 0;
    extKeyMask[1] = 0;

    FrmGotoForm(MainForm);

    myargc = 0;
    argv[myargc++] = "doom";
    argv[myargc++] = "-droot";
    argv[myargc++] = DOOM_ROOT;
    argv[myargc] = NULL;
    myargv = argv;
    M_FindResponseFile();
    dg_Create();
    D_DoomMain();
/*
    for (quit = false; !quit;) {
      if (DG_SleepMs(1) == -1) break;
    }
*/

    xfree(msg);
    MemHandleUnlock(fontH);
    DmReleaseResource(fontH);
    MemHandleUnlock(mfontH);
    DmReleaseResource(mfontH);
  }

  return 0;
}
