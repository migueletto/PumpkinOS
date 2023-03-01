#include <PalmOS.h>
#include <DLServer.h>

#include "sys.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"

Err DlkStartServer(DlkServerParamPtr paramP) {
  debug(DEBUG_ERROR, "PALMOS", "DlkStartServer not implemented");
  return sysErrParamErr;
}

void DlkSetLogEntry(const Char *textP, Int16 textLen, Boolean append) {
  debug(DEBUG_ERROR, "PALMOS", "DlkSetLogEntry not implemented");
}

Err DlkDispatchRequest(DlkServerSessionPtr sessP) {
  debug(DEBUG_ERROR, "PALMOS", "DlkDispatchRequest not implemented");
  return sysErrParamErr;
}

Err DlkControl(DlkCtlEnum op, void *param1P, void *param2P) {
  debug(DEBUG_ERROR, "PALMOS", "DlkControl not implemented");
  return 0;
}

Err DlkGetSyncInfo(UInt32 *succSyncDateP, UInt32 *lastSyncDateP, DlkSyncStateType *syncStateP, Char *nameBufP, Char *logBufP, Int32 *logLenP) {
  if (succSyncDateP) *succSyncDateP = 0;
  if (lastSyncDateP) *lastSyncDateP = 0;
  if (syncStateP) *syncStateP = dlkSyncStateNeverSynced;
  if (nameBufP) StrNCopy(nameBufP, SYSTEM_NAME, dlkUserNameBufSize);
  if (logBufP) *logBufP = 0;
  if (logLenP) *logLenP = 0;

  return errNone;
}
