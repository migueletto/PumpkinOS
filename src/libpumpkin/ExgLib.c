#include <PalmOS.h>

#include "debug.h"

Err ExgLibOpen(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibOpen not implemented");
  return sysErrParamErr;
}

Err ExgLibClose(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibClose not implemented");
  return sysErrParamErr;
}

Err ExgLibSleep(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibSleep not implemented");
  return sysErrParamErr;
}

Err ExgLibWake(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibWake not implemented");
  return sysErrParamErr;
}

Boolean ExgLibHandleEvent(UInt16 libRefnum, void *eventP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibHandleEvent not implemented");
  return false;
}

Err ExgLibConnect(UInt16 libRefNum, ExgSocketType *exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibConnect not implemented");
  return sysErrParamErr;
}

Err ExgLibAccept(UInt16 libRefnum, ExgSocketType *exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibAccept not implemented");
  return sysErrParamErr;
}

Err ExgLibDisconnect(UInt16 libRefnum, ExgSocketType *exgSocketP, Err error) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibDisconnect not implemented");
  return sysErrParamErr;
}

Err ExgLibPut(UInt16 libRefnum, ExgSocketType *exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibPut not implemented");
  return sysErrParamErr;
}

Err ExgLibGet(UInt16 libRefNum, ExgSocketType *exgSocketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibGet not implemented");
  return sysErrParamErr;
}

UInt32 ExgLibSend(UInt16 libRefNum, ExgSocketType *exgSocketP, const void *bufP, UInt32 bufLen, Err *errP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibSend not implemented");
  return 0;
}

UInt32 ExgLibReceive(UInt16 libRefNum, ExgSocketType *exgSocketP, void *bufP, UInt32 bufSize, Err *errP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibReceive not implemented");
  return 0;
}

Err ExgLibControl(UInt16 libRefNum, UInt16 op, void *valueP, UInt16 *valueLenP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibControl not implemented");
  return sysErrParamErr;
}

Err ExgLibRequest(UInt16 libRefNum, ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgLibRequest not implemented");
  return sysErrParamErr;
}
