#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <GPDLib.h>
#include <GPSLib.h>
#include <CPMLib.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define PALMOS_MODULE "System"

typedef struct {
  launch_request_t request;
  char *serialNumber;
  uint64_t t0;
  UInt32 last;
} sys_module_t;

int SysInitModule(void) {
  sys_module_t *module;

  if ((module = xcalloc(1, sizeof(sys_module_t))) == NULL) {
    return -1;
  }

  if ((module->serialNumber = MemPtrNew(8)) != NULL) {
    StrCopy(module->serialNumber, "12345");
    module->t0 = sys_get_clock();
  }
  pumpkin_set_local_storage(sys_key, module);

  return 0;
}

int SysFinishModule(void) {
  sys_module_t *module = (sys_module_t *)pumpkin_get_local_storage(sys_key);

  if (module) {
    if (module->serialNumber) MemPtrFree(module->serialNumber);
    xfree(module);
  }

  return 0;
}

Err SysAppStartup(SysAppInfoPtr *appInfoPP, MemPtr *prevGlobalsP, MemPtr *globalsPtrP) {
  return errNone;
}

Err SysAppExit(SysAppInfoPtr appInfoP, MemPtr prevGlobalsP, MemPtr globalsP) {
  return errNone;
}

void SysUnimplemented(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysUnimplemented not implemented");
}

void SysInit(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysInit not implemented");
}

void SysReset(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysReset not implemented");
}

void SysDoze(Boolean onlyNMI) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysDoze not implemented");
}

Err SysSetPerformance(UInt32 *sysClockP, UInt16 *cpuDutyP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSetPerformance not implemented");
  return 0;
}

void SysSleep(Boolean untilReset, Boolean emergency) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSleep not implemented");
}

UInt16 SysSetAutoOffTime(UInt16 seconds) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSetAutoOffTime not implemented");
  return 0;
}

UInt16 SysTicksPerSecond(void) {
  return 100;
}

Err SysLaunchConsole(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysLaunchConsole not implemented");
  return 0;
}

Boolean SysHandleEvent(EventPtr eventP) {
  EventType appStop;
  Boolean handled = false;

  switch (eventP->eType) {
    case keyDownEvent:
      if (eventP->data.keyDown.chr == vchrLaunch && (eventP->data.keyDown.modifiers & commandKeyMask)) {
        debug(DEBUG_INFO, PALMOS_MODULE, "SysHandleEvent vchrLaunch -> appStopEvent");
        MemSet(&appStop, sizeof(EventType), 0);
        appStop.eType = appStopEvent;
        EvtAddEventToQueue(&appStop);
        handled = true;
      }
      break;
    default:
      break;
  }

  return handled;
}

void SysUILaunch(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysUILaunch not implemented");
}

Err SysUIAppSwitch(UInt16 cardNo, LocalID dbID, UInt16 cmd, MemPtr cmdPBP) {
  sys_module_t *module = (sys_module_t *)pumpkin_get_local_storage(sys_key);
  char name[dmDBNameLength];
  EventType event;
  UInt32 result;
  UInt16 flags;

  if (DmDatabaseInfo(cardNo, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == errNone) {
    if (pumpkin_get_mode() != 0) {
      debug(DEBUG_INFO, PALMOS_MODULE, "SysUIAppSwitch calling \"%s\" cmd %d", name, cmd);
      flags = sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp;
      SysAppLaunch(0, dbID, flags, cmd, cmdPBP, &result);
      return errNone;
    }

    debug(DEBUG_INFO, PALMOS_MODULE, "SysUIAppSwitch registering switch to \"%s\" cmd %d param %p", name, cmd, cmdPBP);
    MemSet(&module->request, sizeof(launch_request_t), 0);
    StrNCopy(module->request.name, name, dmDBNameLength);
    module->request.code = cmd;
    if (cmdPBP) {
      module->request.hasParam = true;
      MemMove(&module->request.param, cmdPBP, sizeof(launch_union_t));
    }
  }

  MemSet(&event, sizeof(EventType), 0);
  event.eType = appStopEvent;
  EvtAddEventToQueue(&event);

  return errNone;
}

int SysUIAppSwitchCont(launch_request_t *request) {
  sys_module_t *module = (sys_module_t *)pumpkin_get_local_storage(sys_key);
  int r = 0;

  if (module->request.name[0]) {
    if (request) MemMove(request, &module->request, sizeof(launch_request_t));
    MemSet(&module->request, sizeof(launch_request_t), 0);
    r = 1;
  }

  return r;
}

Err SysCurAppDatabase(UInt16 *cardNoP, LocalID *dbIDP) {
  if (cardNoP) *cardNoP = 0;
  if (dbIDP) *dbIDP = pumpkin_get_app_localid();
  return errNone;
}

Err SysBroadcastActionCode(UInt16 cmd, MemPtr cmdPBP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysBroadcastActionCode not implemented");
  return 0;
}

UInt16 SysNewOwnerID(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysNewOwnerID not implemented");
  return 0;
}

UInt32 SysSetA5(UInt32 newValue) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSetA5 not implemented");
  return 0;
}

UInt16 SysUIBusy(Boolean set, Boolean value) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysUIBusy not implemented");
  return 0;
}

UInt8 SysLCDContrast(Boolean set, UInt8 newContrastLevel) {
  // silently ignored
  return 0;
}

UInt8 SysLCDBrightness(Boolean set, UInt8 newBrightnessLevel) {
  // silently ignored
  return 0;
}

void SysBatteryDialog(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysBatteryDialog not implemented");
}

Err SysSetTrapAddress(UInt16 trapNum, void *procP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSetTrapAddress not implemented");
  return 0;
}

void *SysGetTrapAddress(UInt16 trapNum) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysGetTrapAddress not implemented");
  return NULL;
}

UInt16 SysDisableInts(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysDisableInts not implemented");
  return 0;
}

void SysRestoreStatus(UInt16 status) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysRestoreStatus not implemented");
}

// Returns a string such as “v. 3.0.”
// You must free the returned string using the MemPtrFree function.
Char *SysGetOSVersionString(void) {
  UInt16 ROMVerMajor, ROMVerMinor, ROMVerFix;
  UInt32 dw;
  char *s;

  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dw);
  ROMVerMajor = sysGetROMVerMajor(dw);
  ROMVerMinor = sysGetROMVerMinor(dw);
  ROMVerFix = sysGetROMVerFix(dw);
  if ((s = MemPtrNew(16)) != NULL) {
    StrPrintF(s, "v. %d.%d", ROMVerMajor, ROMVerMinor, ROMVerFix);
  }

  return s;
}

Err SysGetROMToken(UInt16 cardNo, UInt32 token, UInt8 **dataP, UInt16 *sizeP) {
  sys_module_t *module = (sys_module_t *)pumpkin_get_local_storage(sys_key);
  Err err = -1;

  if (token == sysROMTokenSnum) {
    *dataP = (UInt8 *)module->serialNumber;
    *sizeP = StrLen(module->serialNumber);
    err = errNone;
  }

  return err;
}

Err HwrGetROMToken(UInt16 cardNo, UInt32 token, UInt8 **dataP, UInt16 *sizeP) {
  return SysGetROMToken(cardNo, token, dataP, sizeP);
}

Err SysLibInstall(SysLibEntryProcPtr libraryP, UInt16 *refNumP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysLibInstall not implemented");
  return 0;
}

Err SysLibLoad(UInt32 libType, UInt32 libCreator, UInt16 *refNumP) {
  // XXX always success
  if (libType == netLibType && libCreator == netCreator) {
    *refNumP = NetLibRefNum;
  } else if (libType == sysFileTLibrary && libCreator == GPD_LIB_CREATOR) {
    *refNumP = GPDLibRefNum;
  } else if (libType == sysFileTLibrary && libCreator == gpsLibCreator) {
    *refNumP = GPSLibRefNum;
  } else if (libType == sysFileTLibrary && libCreator == cpmCreator) {
    *refNumP = CpmLibRefNum;
  } else {
    *refNumP = 0;
  }
  return errNone;
}

Err SysLibRemove(UInt16 refNum) {
  // XXX always success
  return errNone;
}

Err SysLibFind(const Char *nameP, UInt16 *refNumP) {
  // XXX always success
  if (nameP && !StrCompare(nameP, NetLibName)) {
    *refNumP = NetLibRefNum;
  } else if (nameP && !StrCompare(nameP, GPDLibName)) {
    *refNumP = GPDLibRefNum;
  } else if (nameP && !StrCompare(nameP, gpsLibName)) {
    *refNumP = GPSLibRefNum;
  } else if (nameP && !StrCompare(nameP, CpmLibName)) {
    *refNumP = CpmLibRefNum;
  } else {
    *refNumP = 0;
  }
  return errNone;
}

SysLibTblEntryPtr SysLibTblEntry(UInt16 refNum) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysLibTblEntry not implemented");
  return 0;
}

Err SysLibOpen(UInt16 refNum) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysLibOpen not implemented");
  return 0;
}

Err SysLibClose(UInt16 refNum) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysLibClose not implemented");
  return 0;
}

Err SysLibSleep(UInt16 refNum) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysLibSleep not implemented");
  return 0;
}

Err SysLibWake(UInt16 refNum) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysLibWake not implemented");
  return 0;
}

Err SysTranslateKernelErr(Err err) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTranslateKernelErr not implemented");
  return 0;
}

Err SysTaskCreate(UInt32 *taskIDP, UInt32 *creator, ProcPtr codeP, MemPtr stackP, UInt32 stackSize, UInt32 attr, UInt32 priority, UInt32 tSlice) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskCreate not implemented");
  return 0;
}

Err SysTaskDelete(UInt32 taskID, UInt32 priority) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskDelete not implemented");
  return 0;
}

Err SysTaskTrigger(UInt32 taskID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskTrigger not implemented");
  return 0;
}

UInt32 SysTaskID(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskID not implemented");
  return 0;
}

Err SysTaskDelay(Int32 delay) {
  // ticks -> ms
  return SysTaskDelayMs(delay * 10);
}

Err SysTaskDelayMs(Int32 delay) {
  UInt32 d, usec;
  int64_t t0, t;

  if (delay > 0) {
    if (pumpkin_get_mode() == 1) {
      EvtPumpEvents(0);
    }

    usec = delay * 1000;

    if (thread_needs_run()) {
      t0 = sys_get_clock();
      for (d = 0; d <= usec; d += 100) {
        t = sys_get_clock();
        if ((uint32_t)(t - t0) >= usec) break;
        thread_yield(0);
        if (thread_must_end()) break;
        sys_usleep(usec < 100 ? usec : 100);
      }
    } else {
      sys_usleep(usec);
    }
  }

  return errNone;
}

Err SysTaskSetTermProc(UInt32 taskID, SysTermProcPtr termProcP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskSetTermProc not implemented");
  return 0;
}

Err SysTaskSwitching(Boolean enable) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskSwitching not implemented");
  return 0;
}

Err SysTaskWait(Int32 timeout) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskWait not implemented");
  return 0;
}

Err SysTaskWake(UInt32 taskID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskWake not implemented");
  return 0;
}

void SysTaskWaitClr(void) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskWaitClr not implemented");
}

Err SysTaskSuspend(UInt32 taskID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskSuspend not implemented");
  return 0;
}

Err SysTaskResume(UInt32 taskID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTaskResume not implemented");
  return 0;
}

Err SysSemaphoreCreate(UInt32 *smIDP, UInt32 *tagP, Int32 initValue) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSemaphoreCreate not implemented");
  return 0;
}

Err SysSemaphoreDelete(UInt32 smID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSemaphoreDelete not implemented");
  return 0;
}

Err SysSemaphoreWait(UInt32 smID, UInt32 priority, Int32 timeout) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSemaphoreWait not implemented");
  return 0;
}

Err SysSemaphoreSignal(UInt32 smID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSemaphoreSignal not implemented");
  return 0;
}

Err SysSemaphoreSet(UInt32 smID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysSemaphoreSet not implemented");
  return 0;
}

Err SysResSemaphoreCreate(UInt32 *smIDP, UInt32 *tagP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysResSemaphoreCreate not implemented");
  return 0;
}

Err SysResSemaphoreDelete(UInt32 smID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysResSemaphoreDelete not implemented");
  return 0;
}

Err SysResSemaphoreReserve(UInt32 smID, UInt32 priority, Int32 timeout) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysResSemaphoreReserve not implemented");
  return 0;
}

Err SysResSemaphoreRelease(UInt32 smID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysResSemaphoreRelease not implemented");
  return 0;
}

Err SysTimerCreate(UInt32 *timerIDP, UInt32 *tagP, SysTimerProcPtr timerProc, UInt32 periodicDelay, UInt32 param) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTimerCreate not implemented");
  return 0;
}

Err SysTimerDelete(UInt32 timerID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTimerDelete not implemented");
  return 0;
}

Err SysTimerWrite(UInt32 timerID, UInt32 value) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTimerWrite not implemented");
  return 0;
}

Err SysTimerRead(UInt32 timerID, UInt32 *valueP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysTimerRead not implemented");
  return 0;
}

Err SysKernelInfo(void *paramP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysKernelInfo not implemented");
  return 0;
}

Boolean SysCreatePanelList(UInt16 *panelCount, MemHandle *panelIDs) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysCreatePanelList not implemented");
  *panelCount = 0;
  *panelIDs = 0;
  return 0;
}

Boolean SysGetStackInfo(MemPtr *startPP, MemPtr *endPP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysGetStackInfo not implemented");
  return 0;
}

Err SysMailboxCreate(UInt32 *mbIDP, UInt32 *tagP, UInt32 depth) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysMailboxCreate not implemented");
  return 0;
}

Err SysMailboxDelete(UInt32 mbID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysMailboxDelete not implemented");
  return 0;
}

Err SysMailboxFlush(UInt32 mbID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysMailboxFlush not implemented");
  return 0;
}

Err SysMailboxSend(UInt32 mbID, void *msgP, UInt32 wAck) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysMailboxSend not implemented");
  return 0;
}

Err SysEvGroupCreate(UInt32 *evIDP, UInt32 *tagP, UInt32 init) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysEvGroupCreate not implemented");
  return 0;
}

Err SysEvGroupSignal(UInt32 evID, UInt32 mask, UInt32 value, Int32 type) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysEvGroupSignal not implemented");
  return 0;
}

Err SysEvGroupRead(UInt32 evID, UInt32 *valueP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysEvGroupRead not implemented");
  return 0;
}

UInt16 SysGetOrientation(void) {
  return sysOrientationPortrait;
}

Err SysSetOrientation(UInt16 orientation) {
  return sysErrNotAllowed;
}

UInt16 SysGetOrientationTriggerState(void) {
  return sysErrNotAllowed;
}

Err SysSetOrientationTriggerState(UInt16 triggerState) {
  return sysOrientationTriggerDisabled;
}

UInt16 SysBatteryInfo(Boolean set, UInt16 *warnThresholdP, UInt16 *criticalThresholdP, Int16 *maxTicksP, SysBatteryKind *kindP, Boolean *pluggedIn, UInt8 *percentP) {
  if (!set) {
    if (warnThresholdP) *warnThresholdP = 320;
    if (criticalThresholdP) *criticalThresholdP = 300;
    if (maxTicksP) *maxTicksP = 0; // XXX what is this ?
    if (kindP) *kindP = sysBatteryKindLiIon;
    if (pluggedIn) *pluggedIn = true;
    if (percentP) *percentP = pumpkin_get_battery();
  }

  return 370;
}

UInt16 SysBatteryInfoV20(Boolean set, UInt16 *warnThresholdP, UInt16 *criticalThresholdP, Int16 *maxTicksP, SysBatteryKind *kindP, Boolean *pluggedIn) {
  if (!set) {
    if (warnThresholdP) *warnThresholdP = 320;
    if (criticalThresholdP) *criticalThresholdP = 300;
    if (maxTicksP) *maxTicksP = 0; // XXX what is this ?
    if (kindP) *kindP = sysBatteryKindLiIon;
    if (pluggedIn) *pluggedIn = true;
  }

  return 370;
}

// Return the tick count since the last reset.
UInt32 TimGetTicks(void) {
  sys_module_t *module = (sys_module_t *)pumpkin_get_local_storage(sys_key);
  return ((uint64_t)sys_get_clock() - module->t0) / 10000; // XXX 100 ticks per second
}

UInt32 TimGetTicksMs(void) {
  sys_module_t *module = (sys_module_t *)pumpkin_get_local_storage(sys_key);
  return ((uint64_t)sys_get_clock() - module->t0) / 1000;
} 

Err SysMailboxWait(UInt32 mbID, void *msgP, UInt32 priority, Int32 timeout) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysMailboxWait not implemented");
  return 0;
}

Err SysEvGroupWait(UInt32 evID, UInt32 mask, UInt32 value, Int32 matchType, Int32 timeout) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysEvGroupWait not implemented");
  return 0;
}

void SysColdBoot(void *card0P, UInt32 card0Size, void *card1P, UInt32 card1Size, UInt32  sysCardHeaderOffset) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "SysColdBoot not implemented");
}
