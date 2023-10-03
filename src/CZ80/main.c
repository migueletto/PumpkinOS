#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "vfs.h"
#include "ptr.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "pumpkin.h"
#include "cz80.h"
#include "coco.h"
#include "spectrum.h"
#include "zx81.h"
#include "trs80.h"
#include "apple2.h"
#include "debug.h"
#include "resource.h"

#define ROOT  "/PALM/Programs/CZ80"
#define QUEUE_SIZE  16

typedef struct {
  computer_t *computer;
  surface_t *surface;
  vfs_session_t *session;
  UInt16 volref;
  int width, height;

  uint32_t modMask, keyMask;
  uint64_t extKeyMask[2];
  uint16_t kbdQueue[QUEUE_SIZE];
  uint32_t writeIndex;
  uint32_t readIndex;
} app_data_t;

static void putKey(app_data_t *data, int down, int key){
  data->kbdQueue[data->writeIndex] = (down << 8) | key;
  data->writeIndex++;
  data->writeIndex %= QUEUE_SIZE;
}

static void processNormalKeys(app_data_t *data, uint32_t oldMask, uint32_t newMask) {
  uint32_t diff;

  diff = oldMask ^ newMask;
  if (diff) {
    if (diff & keyBitPageDown) {
      putKey(data, newMask & keyBitPageDown ? 1 : 0, WINDOW_KEY_DOWN);
    }
    if (diff & keyBitPageUp) {
      putKey(data, newMask & keyBitPageUp ? 1 : 0, WINDOW_KEY_UP);
    }
    if (diff & keyBitLeft) {
      putKey(data, newMask & keyBitLeft ? 1 : 0, WINDOW_KEY_LEFT);
    }
    if (diff & keyBitRight) {
      putKey(data, newMask & keyBitRight ? 1 : 0, WINDOW_KEY_RIGHT);
    }
  }
}

static void processModKeys(app_data_t *data, uint32_t oldMask, uint32_t newMask) {
  uint32_t diff;

  diff = oldMask ^ newMask;
  if (diff) {
    if (diff & WINDOW_MOD_SHIFT) {
      putKey(data, newMask & WINDOW_MOD_SHIFT ? 1 : 0, WINDOW_KEY_SHIFT);
    }
    if (diff & WINDOW_MOD_CTRL) {
      putKey(data, newMask & WINDOW_MOD_CTRL ? 1 : 0, WINDOW_KEY_CTRL);
    }
    if (diff & WINDOW_MOD_LALT) {
      putKey(data, newMask & WINDOW_MOD_LALT ? 1 : 0, WINDOW_KEY_LALT);
    }
  }
}

static void processExtKeys(app_data_t *data, uint64_t oldMask, uint64_t newMask, int offset) {
  int i, key, down;
  uint64_t diff;

  diff = oldMask ^ newMask;
  if (diff) {
    for (i = 0; i < 64; i++) {
      if (diff & 1) {
        key = offset + i;
        down = newMask & 1;
        putKey(data, down, key);
      }
      newMask >>= 1;
      diff >>= 1;
    }
  }
}

static void processKeys(app_data_t *data) {
  uint32_t _keyMask, _modMask;
  uint64_t _extKeyMask[2];

  pumpkin_status(NULL, NULL, &_keyMask, &_modMask, NULL, _extKeyMask);

  processNormalKeys(data, data->keyMask, _keyMask);
  data->keyMask = _keyMask;

  processModKeys(data, data->modMask, _modMask);
  data->modMask = _modMask;

  processExtKeys(data, data->extKeyMask[0], _extKeyMask[0], 0);
  data->extKeyMask[0] = _extKeyMask[0];

  processExtKeys(data, data->extKeyMask[1], _extKeyMask[1], 64);
  data->extKeyMask[1] = _extKeyMask[1];
}

static void drawFrame(app_data_t *data) {
  uint32_t height;
  uint16_t *src;
  int y, len;

  src = (uint16_t *)data->surface->getbuffer(data->surface->data, &len);
  WinScreenGetAttribute(winScreenHeight, &height);
  height -= 30;
  y = 30 + (height - data->height) / 2;
  pumpkin_screen_copy(src, y, y + data->height);
}

static Boolean validWindow(void) {
  FormType *frm = FrmGetActiveForm();
  return frm && FrmGetWindowHandle(frm) == WinGetActiveWindow();
}

static void menuEventHandler(UInt16 id) {
  switch (id) {
    case menuAbout:
      AbtShowAboutPumpkin(pumpkin_get_app_creator());
      break;
  }
}

static Boolean mainFormHandleEvent(EventType *event) {
  app_data_t *data = pumpkin_get_data();
  UInt32 sw, sh;
  FormPtr frm;
  WinHandle wh;
  RectangleType rect;
  IndexedColorType old;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      WinScreenMode(winScreenModeGet, &sw, &sh, NULL, NULL);
      wh = FrmGetWindowHandle(frm);
      RctSetRectangle(&rect, 0, 0, sw, sh);
      WinSetBounds(wh, &rect);
      FrmDrawForm(frm);
      rect.topLeft.y = 15;
      rect.extent.y -= 15;
      old = WinSetBackColor(255); // black
      WinEraseRectangle(&rect, 0);
      WinSetBackColor(old);
      handled = true;
      break;
    case menuEvent:
      menuEventHandler(event->data.menu.itemID);
      handled = true;
      break;
    case nilEvent:
      processKeys(data);
      break;
    default:
      break;
  }

  return handled;
}

static void setEventHandler(FormType *frm, Int16 form) {
  switch (form) {
    case MainForm:
      FrmSetEventHandler(frm, mainFormHandleEvent);
      break;
  }
}

static Boolean applicationHandleEvent(EventType *event) {
  FormPtr frm;
  UInt16 form;
  Boolean handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);
      setEventHandler(frm, form);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void mainLoop(app_data_t *data) {
  EventType event;
  uint64_t t1, t2;
  uint32_t dt, us;
  Err err;

  for (;;) {
    t1 = sys_get_clock();
    if (data->computer->run(data->computer, 2000) != 0) break;
    us = data->computer->delay ? (data->computer->delay(data->computer) ? 200 : 0) : 200;

    EvtGetEventUs(&event, us);
    if (!SysHandleEvent(&event)) {
      if (!MenuHandleEvent(NULL, &event, &err)) {
        if (!applicationHandleEvent(&event)) {
          FrmDispatchEvent(&event);
        }
      }
    }
    if (event.eType == appStopEvent) break;
    t2 = sys_get_clock();
    dt = t2 - t1;

    if (dt < 2000) {
      //sys_usleep(2000 - dt);
    }
  }
}

static int computerSurfaceEvent(void *_data, uint32_t us, int *arg1, int *arg2) {
  app_data_t *data = (app_data_t *)_data;
  uint16_t key;
  int down, ev = 0;

  if (data->readIndex != data->writeIndex) {
    key = data->kbdQueue[data->readIndex];
    data->readIndex++;
    data->readIndex %= QUEUE_SIZE;

    down = key >> 8;
    if (down) {
      ev = WINDOW_KEYDOWN;
    } else {
      ev = WINDOW_KEYUP;
    }
    *arg1 = key & 0xFF;
  }

  return ev;
}

static void computerSurfaceUpdate(void *_data) {
  app_data_t *data = (app_data_t *)_data;

  if (validWindow()) {
    drawFrame(data);
  }
}

static void startApp(app_data_t *data) {
  UInt32 iterator;
  FileRef fr;

  iterator = vfsIteratorStart;
  VFSVolumeEnumerate(&data->volref, &iterator);

  if (VFSFileOpen(data->volref, ROOT, vfsModeRead, &fr) == errNone) {
    VFSFileClose(fr);
  } else {
    VFSDirCreate(data->volref, ROOT);
  }

  data->session = vfs_open_session();
  data->width = 640;
  data->height = 400;

  data->computer = cz80_init(data->session);
  data->computer->option(data->computer, "font", "8");
  data->computer->disk(data->computer, 0, 0, 77, 1, 26, 128, 0, "/app_card/PALM/Programs/CZ80/disk0.dsk");
  data->computer->disk(data->computer, 1, 0, 77, 1, 26, 128, 0, "/app_card/PALM/Programs/CZ80/disk1.dsk");

  //data->computer = spectrum_init(data->session);
  //data->computer->rom(data->computer, 0, 0x4000, "/app_card/PALM/Programs/CZ80/48.rom");

  //data->computer = zx81_init(data->session);
  //data->computer->rom(data->computer, 0, 0x2000, "/app_card/PALM/Programs/CZ80/tk85.rom");

  //data->computer = coco_init(data->session);
  //data->computer->rom(data->computer, 0, 0x10000, "/app_card/PALM/Programs/CZ80/coco2.bmp");
  //data->computer->rom(data->computer, 1, 0x2000,  "/app_card/PALM/Programs/CZ80/bas13.rom");
  //data->computer->rom(data->computer, 2, 0x2000,  "/app_card/PALM/Programs/CZ80/extbas11.rom");

  //data->computer = trs80_init(data->session);
  //data->computer->rom(data->computer, 0, 0x0400, "/app_card/PALM/Programs/CZ80/trs80m1.chr");
  //data->computer->rom(data->computer, 1, 0x3800, "/app_card/PALM/Programs/CZ80/model3.rom");
  //data->computer->disk(data->computer, 0, 0, 0, 0, 0, 0, 0, "/app_card/PALM/Programs/CZ80/tdos13a.disk");

  //data->computer = apple2_init(data->session);
  //data->computer->rom(data->computer, 0, 0x0300, "/app_card/PALM/Programs/CZ80/apple2.chr");
  //data->computer->rom(data->computer, 1, 0x3000, "/app_card/PALM/Programs/CZ80/apple2p.rom");
  //data->computer->rom(data->computer, 2, 0x0100, "/app_card/PALM/Programs/CZ80/diskII.rom");
  //data->computer->disk(data->computer, 0, 0, 35, 1, 16, 256, 1, "/app_card/PALM/Programs/CZ80/master.nib");

  data->surface = surface_create(data->width, data->height, SURFACE_ENCODING_RGB565);
  data->surface->event = computerSurfaceEvent;
  data->surface->update = computerSurfaceUpdate;
  data->surface->udata = data;
  data->computer->set_surface(data->computer, 0, data->surface);

  FrmCenterDialogs(true);
  FrmGotoForm(MainForm);
}

static void stopApp(app_data_t *data) {
  data->computer->close(data->computer);
  surface_destroy(data->surface);
  vfs_close_session(data->session);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  app_data_t *data;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    data = MemPtrNew(sizeof(app_data_t));
    pumpkin_set_data(data);
    startApp(data);
    mainLoop(data);
    stopApp(data);
    pumpkin_set_data(NULL);
    MemPtrFree(data);
  }

  return 0;
}
