#include <PalmOS.h>

#include "pumpkin.h"
#include "resource.h"
#include "debug.h"

typedef struct {
  UInt32 widget1, widget2;
} taskbar_data_t;

static Boolean MainFormHandleEvent(EventType *event) {
  taskbar_data_t *data;
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;
    case appWidgetEvent:
      data = pumpkin_get_data();
      if (event->data.widget.id == data->widget1) {
        pumpkin_taskbar_ui(1);
        FrmAlert(10018);
        pumpkin_taskbar_ui(0);
      } else if (event->data.widget.id == data->widget2) {
        pumpkin_taskbar_ui(1);
        FrmAlert(10019);
        pumpkin_taskbar_ui(0);
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormPtr frm;
  UInt16 formID;
  Boolean handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      formID = event->data.frmLoad.formID;
      frm = FrmInitForm(formID);
      FrmSetActiveForm(frm);
      switch (formID) {
        case MainForm:
          FrmSetEventHandler(frm, MainFormHandleEvent);
          break;
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void EventLoop(void) {
  EventType event;
  Err err;

  do {
    EvtGetEvent(&event, evtWaitForever);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent);
}

static Err LauncherNotificationHandler(SysNotifyParamType *notifyParamsP) {
  SysNotifyAppLaunchOrQuitType *launch;
  char name[dmDBNameLength];
  char screator[8];
  UInt32 creator;

  switch (notifyParamsP->notifyType) {
    case sysNotifyAppLaunchingEvent:
      launch = (SysNotifyAppLaunchOrQuitType *)notifyParamsP->notifyDetailsP;
      if (launch) {
        DmDatabaseInfo(0, launch->dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator);
        pumpkin_id2s(creator, screator);
        debug(DEBUG_INFO, "taskbar", "sysNotifyAppLaunchingEvent '%s' \"%s\"", screator, name);
        pumpkin_taskbar_add(launch->dbID, creator, name);
      }
      break;
    case sysNotifyAppQuittingEvent:
      launch = (SysNotifyAppLaunchOrQuitType *)notifyParamsP->notifyDetailsP;
      if (launch) {
        DmDatabaseInfo(0, launch->dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &creator);
        pumpkin_id2s(creator, screator);
        debug(DEBUG_INFO, "taskbar", "sysNotifyAppQuittingEvent '%s' \"%s\"", screator, name);
        pumpkin_taskbar_remove(launch->dbID);
      }
      break;
  }

  return 0;
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  taskbar_data_t *data;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    data = sys_calloc(1, sizeof(taskbar_data_t));
    pumpkin_set_data(data);

    SysNotifyRegister(0, pumpkin_get_app_localid(), sysNotifyAppQuittingEvent, LauncherNotificationHandler, sysNotifyNormalPriority, NULL);
    SysNotifyRegister(0, pumpkin_get_app_localid(), sysNotifyTimeChangeEvent,  LauncherNotificationHandler, sysNotifyNormalPriority, NULL);

    pumpkin_taskbar_create();
    data->widget1 = pumpkin_taskbar_add_widget(9998);
    data->widget2 = pumpkin_taskbar_add_widget(9999);

    FrmGotoForm(MainForm);
    debug(DEBUG_INFO, "taskbar", "starting");
    EventLoop();
    debug(DEBUG_INFO, "taskbar", "finishing");
    FrmCloseAllForms();

    pumpkin_taskbar_remove_widget(data->widget1);
    pumpkin_taskbar_remove_widget(data->widget2);
    pumpkin_taskbar_destroy();

    SysNotifyUnregister(0, pumpkin_get_app_localid(), sysNotifyAppQuittingEvent, sysNotifyNormalPriority);
    SysNotifyUnregister(0, pumpkin_get_app_localid(), sysNotifyTimeChangeEvent,  sysNotifyNormalPriority);

    pumpkin_set_data(NULL);
    sys_free(data);
  }

  return 0;
}
