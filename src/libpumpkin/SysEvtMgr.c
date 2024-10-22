#include <PalmOS.h>

#include "debug.h"

Err EvtSysInit(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtSysInit not implemented");
  return 0;
}

void EvtGetSysEvent(SysEventType *eventP, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "EvtGetSysEvent not implemented");
}

Err EvtProcessSoftKeyStroke(PointType *startPtP, PointType *endPtP) {
  debug(DEBUG_ERROR, "PALMOS", "EvtProcessSoftKeyStroke not implemented");
  return errNone;
}

Err EvtSetPenQueuePtr(MemPtr penQueueP, UInt32 size) {
  debug(DEBUG_ERROR, "PALMOS", "EvtSetPenQueuePtr not implemented");
  return errNone;
}

UInt32 EvtPenQueueSize(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtPenQueueSize not implemented");
  return 0;
}

Err EvtEnqueuePenPoint(PointType *ptP) {
  debug(DEBUG_ERROR, "PALMOS", "EvtEnqueuePenPoint not implemented");
  return errNone;
}

Err EvtDequeuePenStrokeInfo(PointType *startPtP, PointType *endPtP) {
  debug(DEBUG_ERROR, "PALMOS", "EvtDequeuePenStrokeInfo not implemented");
  return errNone;
}

Err EvtDequeuePenPoint(PointType *retP) {
  debug(DEBUG_ERROR, "PALMOS", "EvtDequeuePenPoint not implemented");
  return errNone;
}

Err EvtFlushNextPenStroke(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtFlushNextPenStroke not implemented");
  return errNone;
}

Err EvtSetKeyQueuePtr(MemPtr keyQueueP, UInt32 size) {
  debug(DEBUG_ERROR, "PALMOS", "EvtSetKeyQueuePtr not implemented");
  return errNone;
}

UInt32 EvtKeyQueueSize(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtKeyQueueSize not implemented");
  return 0;
}

Err EvtFlushKeyQueue(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtFlushKeyQueue not implemented");
  return errNone;
}

/* in Event.c
Err EvtEnqueueKey(WChar ascii, UInt16 keycode, UInt16 modifiers) {
  debug(DEBUG_ERROR, "PALMOS", "EvtEnqueueKey not implemented");
  return 0;
}
*/

Boolean EvtKeyQueueEmpty(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtKeyQueueEmpty not implemented");
  return true;
}

Err EvtDequeueKeyEvent(SysEventType *eventP, UInt16 peek) {
  debug(DEBUG_ERROR, "PALMOS", "EvtDequeueKeyEvent not implemented");
  return errNone;
}

Err EvtWakeup(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtWakeup not implemented");
  return errNone;
}

Err EvtWakeupWithoutNilEvent(void) {
  debug(DEBUG_ERROR, "PALMOS", "EvtWakeupWithoutNilEvent not implemented");
  return errNone;
}

Err EvtResetAutoOffTimer(void) {
  return errNone;
}

Err EvtSetAutoOffTimer(EvtSetAutoOffCmd cmd, UInt16 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "EvtSetAutoOffTimer not implemented");
  return errNone;
}

void EvtEnableGraffiti(Boolean enable) {
  debug(DEBUG_ERROR, "PALMOS", "EvtEnableGraffiti not implemented");
}

/* in Event.c
Boolean EvtSetNullEventTick(UInt32 tick) {
}
*/

const SilkscreenAreaType *EvtGetSilkscreenAreaList(UInt16* numAreas) {
  *numAreas = 0;
  return NULL;
}

const PenBtnInfoType *EvtGetPenBtnList(UInt16* numButtons) {
  *numButtons = 0;
  return NULL;
}
