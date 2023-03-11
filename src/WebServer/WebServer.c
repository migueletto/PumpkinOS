#include <PalmOS.h>

#include "sys.h"
#include "script.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

#include "resource.h"

#define MAX_WORKER  20
#define MAX_ROOT    40

typedef struct {
  UInt16 port;
  char worker[MAX_WORKER];
  char root[MAX_ROOT];
} webserver_prefs_t;

typedef struct {
  int start, stop, running;
  Int32 wait;
  webserver_prefs_t prefs;
} webserver_data_t;

static void setField(FormType *frm, UInt16 fieldId, char *s, Boolean focus) {
  FieldType *fld;
  UInt16 objIndex, len;
  FieldAttrType attr;
  Boolean old;

  objIndex = FrmGetObjectIndex(frm, fieldId);
  fld = (FieldPtr)FrmGetObjectPtr(frm, objIndex);

  FldGetAttributes(fld, &attr);
  old = attr.editable;
  attr.editable = true;
  FldSetAttributes(fld, &attr);

  len = FldGetTextLength(fld);
  if (len) FldDelete(fld, 0, len);
  FldInsert(fld, s, StrLen(s));

  attr.editable = old;
  FldSetAttributes(fld, &attr);

  if (focus) fld->attr.hasFocus = true;
}

static void setFieldNum(FormType *frm, UInt16 fieldId, UInt32 value, Boolean focus) {
  char buf[8];

  if (value != 0xffffffff) {
    sys_snprintf(buf, sizeof(buf)-1, "%d", value);
    setField(frm, fieldId, buf, focus);
  }
}

static Boolean getField(FormType *frm, UInt16 fieldId, char *buf, int size) {
  FieldType *fld;
  UInt16 objIndex;
  char *s;
  Boolean r = false;

  objIndex = FrmGetObjectIndex(frm, fieldId);
  if ((fld = (FieldType *)FrmGetObjectPtr(frm, objIndex)) != NULL) {
    s = FldGetTextPtr(fld);
    if (s && s[0]) {
      StrNCopy(buf, s, size-1);
      r = true;
    }
  }

  return r;
}

static UInt32 getFieldNum(FormType *frm, UInt16 fieldId) {
  char buf[8];
  UInt32 value = 0xffffffff;

  if (getField(frm, fieldId, buf, sizeof(buf))) {
    value = sys_atoi(buf);
  }

  return value;
}

static void paintBitmap(UInt16 id, Coord x, Coord y) {
  BitmapType *bmp;
  MemHandle h;

  if ((h = DmGet1Resource(bitmapRsc, id)) != NULL) {
    if ((bmp = MemHandleLock(h)) != NULL) {
      WinPaintBitmap(bmp, x, y);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }
}

static void httpdStatus(int status) {
  FormType *frm;
  RectangleType rect;
  UInt16 index;
  Coord x, y;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, statusFld);
  FrmGetObjectBounds(frm, index, &rect);
  x = rect.topLeft.x + rect.extent.x + 2;
  y = rect.topLeft.y + 1;

  if (status) {
    paintBitmap(okBmp, x, y);
    setField(frm, statusFld, "Running", false);
  } else {
    setField(frm, statusFld, "Stopped", false);
    paintBitmap(errorBmp, x, y);
  }
}

static Boolean ApplicationHandleEvent(EventPtr event);

static Boolean idleCallback(void *_data) {
  webserver_data_t *data = (webserver_data_t *)_data;
  EventType event;
  Err err;

  for (;;) {
    EvtGetEvent(&event, data->wait);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
    if (event.eType == nilEvent) break;
    if (event.eType == appStopEvent) return true;
    if (data->stop) return true;
  }

  return false;
}

static Boolean MainFormHandleEvent(EventPtr event) {
  webserver_data_t *data = (webserver_data_t *)pumpkin_get_data();
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      setFieldNum(frm, portFld, data->prefs.port, false);
      setField(frm, workerFld, data->prefs.worker, false);
      setField(frm, rootFld, data->prefs.root, false);
      FrmDrawForm(frm);
      httpdStatus(0);
      handled = true;
      break;
    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case startCtl:
          if (!data->running) {
            frm = FrmGetActiveForm();
            data->prefs.port = getFieldNum(frm, portFld);
            getField(frm, workerFld, data->prefs.worker, MAX_WORKER);
            getField(frm, rootFld, data->prefs.root, MAX_ROOT);
            data->start = 1;
          }
          break;
        case stopCtl:
          if (data->running) {
            data->stop = 1;
          }
          break;
      }
      handled = true;
      break;
    case menuEvent:
      switch (event->data.menu.itemID) {
        case aboutCmd:
          AbtShowAbout(pumpkin_get_app_creator());
          break;
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);
      FrmSetEventHandler(frm, MainFormHandleEvent);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void EventLoop(webserver_data_t *data) {
  pumpkin_httpd_t *webServer;

  for (;;) {
    if (idleCallback(data) && !data->stop) break;

    if (data->start) {
      data->start = 0;
      data->stop = 0;
      httpdStatus(1);
      data->running = 1;
      debug(DEBUG_INFO, "WebServer", "starting httpd");
      webServer = pumpkin_httpd_create(data->prefs.port, 1, "worker", "/www", data, idleCallback);
      debug(DEBUG_INFO, "WebServer", "httpd exited");
      data->running = 0;
      httpdStatus(0);
      pumpkin_httpd_destroy(webServer);
      data->stop = 0;
    }
  }
}

static void StartApplication(webserver_data_t *data) {
  UInt32 creator;
  UInt16 prefsSize;

  creator = pumpkin_get_app_creator();
  prefsSize = sizeof(webserver_prefs_t);

  if (PrefGetAppPreferences(creator, 1, &data->prefs, &prefsSize, true) == noPreferenceFound) {
    data->prefs.port = 8000;
    StrNCopy(data->prefs.worker, "worker", MAX_WORKER-1);
    StrNCopy(data->prefs.root, "/www", MAX_ROOT-1);
    PrefSetAppPreferences(creator, 1, 1, &data->prefs, sizeof(webserver_prefs_t), true);
  }

  FrmGotoForm(MainForm);
}

static void StopApplication(webserver_data_t *data) {
  UInt32 creator;

  FrmCloseAllForms();

  creator = pumpkin_get_app_creator();
  PrefSetAppPreferences(creator, 1, 1, &data->prefs, sizeof(webserver_prefs_t), true);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  webserver_data_t *data;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    data = xcalloc(1, sizeof(webserver_data_t));
    data->wait = 10;
    pumpkin_set_data(data);

    StartApplication(data);
    EventLoop(data);
    StopApplication(data);

    pumpkin_set_data(NULL);
    xfree(data);
  }

  return 0;
}
