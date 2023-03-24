#include <PalmOS.h>

#include "gui.h"
#include "gps.h"
#include "app.h"
#include "main.h"
#include "ddb.h"
#include "misc.h"
#include "error.h"
#include "pumpkin.h"

static Err StartApplication(void *);
static void EventLoop(void);
static void StopApplication(void);

static Boolean ApplicationHandleEvent(EventPtr);
static void RetrieveVersion(Char *);

static AppPrefs prefs, prefs_aux;

static UInt16 ROMVerMajor;
static UInt16 ROMVerMinor;
static UInt16 ROMVerFix;
static Int16 ROMNumber;
static char appVersion[8];
static char romVersion[8];
static Boolean hasBacklight;

static Int16 wait;
static UInt16 ticksPerSecond;

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    if (StartApplication((void *)cmdPBP) != 0) {
      return 0;
    }
    EventLoop();
    StopApplication();
  }
    
  return 0;
}

static Err StartApplication(void *param) {
  UInt16 len, prefs_version;
  UInt32 dw;
  Err err;

  FrmCenterDialogs(true);

  RetrieveVersion(appVersion);
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dw);
  ROMVerMajor = sysGetROMVerMajor(dw);
  ROMVerMinor = sysGetROMVerMinor(dw);
  ROMVerFix = sysGetROMVerFix(dw);
  ROMNumber = ROMVerMajor*10 + ROMVerMinor;

  if (ROMVerFix)
    StrPrintF(romVersion, "%d.%d.%d", ROMVerMajor, ROMVerMinor, ROMVerFix);
  else
    StrPrintF(romVersion, "%d.%d", ROMVerMajor, ROMVerMinor);

  if ((10*ROMVerMajor + ROMVerMinor) < 35) {
    FrmCustomAlert(ErrorAlert, "Navegador version", appVersion, "requires at least PalmOS 3.5");
    return -1;
  }

  err = FtrGet(sysFtrCreator, sysFtrNumBacklight, &dw);
  hasBacklight = !err && (dw & 1);

  wait = evtWaitForever;
  ticksPerSecond = SysTicksPerSecond();

  len = 0;
  prefs_version = PrefGetAppPreferences(AppID, 1, NULL, &len, true);
  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (prefs_version == PREFS_VERSION) {
    PrefGetAppPreferences(AppID, 1, &prefs, &len, true);
  } else {
    InitPrefs(&prefs);
  }

  PrefSetAppPreferences(AppID, 1, PREFS_VERSION, &prefs, len, true);
  LoadPrefs();

  if (AppInit(param) != 0) {
    return -1;
  }

  FrmGotoForm(prefs.display);

  return 0;
}

static void EventLoop() {
  EventType event;
  Err err;
 
  do {
    // Get the next available event.
    EvtGetEvent(&event, wait);
 
    if (InterceptEvent(&event))
      continue;

    // Give the system a chance to handle the event.
    if (SysHandleEvent(&event))
      continue;

    // Give the menu bar a chance to update and handle the event.
    if (MenuHandleEvent(NULL, &event, &err))
      continue;

    // Give the application a chance to handle the event.
    if (ApplicationHandleEvent(&event))
      continue;
 
    FrmDispatchEvent(&event);

  } while (event.eType != appStopEvent);
}

static void StopApplication(void) {
  FrmCloseAllForms();
  AppFinish();
  PrefSetAppPreferences(AppID, 1, PREFS_VERSION, &prefs, sizeof(prefs), true);
  WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
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
      SetEventHandler(frm, form);
      handled = true;
      break;
    case frmCloseEvent:
      break;
    default:
      break;
  }

  return handled;
}

void SavePrefs(void) {
  MemMove(&prefs, &prefs_aux, sizeof(prefs));
}

AppPrefs *LoadPrefs(void) {
  MemMove(&prefs_aux, &prefs, sizeof(prefs));
  return &prefs_aux;
}

AppPrefs *GetPrefs(void) {
  return &prefs;
}

static void RetrieveVersion(Char *appVersion) {
  MemHandle h;
  char *s;

  if ((h = DmGetResource(verRsc, appVersionID)) != NULL) {
    if ((s = MemHandleLock(h)) != NULL) {
      StrCopy(appVersion, s);
      MemHandleUnlock(h);
    }
    else
      StrCopy(appVersion, "?.?");
    DmReleaseResource(h);
  } 
  else
    StrCopy(appVersion, "?.?");
}

char *GetAppVersion(void) {
  return appVersion;
}

char *GetRomVersion(void) {
  return romVersion;
}

Int16 GetRomVersionNumber(void) {
  return ROMNumber;
}

void SetWait(Int16 w) {
  wait = w < 0 ? evtWaitForever : ticksPerSecond / w;
}

void ToggleBacklight(void) {
  if (hasBacklight)
    EvtEnqueueKey(vchrBacklight, vchrBacklight, commandKeyMask);
}

void SetEventHandler(FormPtr frm, Int16 form) {
  switch(form) {
    case MainForm:
      FrmSetEventHandler(frm, MainFormHandleEvent);
      break;
    case SatForm:
      FrmSetEventHandler(frm, SatFormHandleEvent);
      break;
    case MapForm:
      FrmSetEventHandler(frm, MapFormHandleEvent);
      break;
    case GpsForm:
      FrmSetEventHandler(frm, GpsFormHandleEvent);
      break;
    case NetworkForm:
      FrmSetEventHandler(frm, NetworkFormHandleEvent);
      break;
    case UnitsForm:
      FrmSetEventHandler(frm, UnitsFormHandleEvent);
      break;
    case ButtonsForm:
      FrmSetEventHandler(frm, ButtonsFormHandleEvent);
      break;
    case FindForm:
      FrmSetEventHandler(frm, FindFormHandleEvent);
      break;
    case MapFindForm:
      FrmSetEventHandler(frm, MapFindFormHandleEvent);
      break;
    case ProgressForm:
      FrmSetEventHandler(frm, ProgressFormHandleEvent);
      break;
    case WaypointsForm:
      FrmSetEventHandler(frm, ObjectListHandleEvent);
      break;
    case RoutesForm:
      FrmSetEventHandler(frm, ObjectListHandleEvent);
      break;
    case TracksForm:
      FrmSetEventHandler(frm, TracksListHandleEvent);
      break;
    case EditPointForm:
    case NewPointForm:
    case AutoPointForm:
    case SavePointForm:
      FrmSetEventHandler(frm, EditPointFormHandleEvent);
      break;
    case CompassForm:
    case SeekPointForm:
    case FollowRouteForm:
      FrmSetEventHandler(frm, CompassFormHandleEvent);
      break;
    case EditRouteForm:
    case NewRouteForm:
      FrmSetEventHandler(frm, EditRouteFormHandleEvent);
      break;
    case SelectPointForm:
      FrmSetEventHandler(frm, SelectPointFormHandleEvent);
      break;
    case TripForm:
      FrmSetEventHandler(frm, TripFormHandleEvent);
      break;
    case AstroForm:
      FrmSetEventHandler(frm, AstroFormHandleEvent);
      break;
    case SymbolForm:
      FrmSetEventHandler(frm, SymbolFormHandleEvent);
      break;
    case LoadMapForm:
      FrmSetEventHandler(frm, LoadMapFormHandleEvent);
      break;
    case MapColorForm:
      FrmSetEventHandler(frm, MapColorFormHandleEvent);
  }
}
