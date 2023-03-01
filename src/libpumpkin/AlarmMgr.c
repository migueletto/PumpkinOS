#include <PalmOS.h>

#include "thread.h"
#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define PALMOS_MODULE "Alarm"

Err AlmInit(void) {
  return errNone;
}

Err AlmSetAlarm(UInt16 cardNo, LocalID dbID, UInt32 ref, UInt32 alarmSeconds, Boolean quiet) {
  Err err = almErrFull;

  if (pumpkin_alarm_set(dbID, alarmSeconds, ref) == 0) {
    err = errNone;
  }

  return err;
}

UInt32 AlmGetAlarm(UInt16 cardNo, LocalID dbID, UInt32 *refP) {
  UInt32 alarmSeconds = 0;

  pumpkin_alarm_get(dbID, &alarmSeconds, refP);

  return alarmSeconds;
}

void AlmEnableNotification(Boolean enable) {
  // system use only
  debug(DEBUG_ERROR, PALMOS_MODULE, "AlmEnableNotification not implemented");
}

Boolean AlmDisplayAlarm(Boolean okToDisplay) {
  // system use only
  debug(DEBUG_ERROR, PALMOS_MODULE, "AlmDisplayAlarm not implemented");
  return false;
}

void AlmCancelAll(void) {
  // system use only
  debug(DEBUG_ERROR, PALMOS_MODULE, "AlmCancelAll not implemented");
}

void AlmAlarmCallback(void) {
  // system use only
  debug(DEBUG_ERROR, PALMOS_MODULE, "AlmAlarmCallback not implemented");
}

void AlmTimeChange(void) {
  // system use only
  debug(DEBUG_ERROR, PALMOS_MODULE, "AlmTimeChange not implemented");
}
