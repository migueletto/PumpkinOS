#include <PalmOS.h>

#include "debug.h"

ProgressPtr	PrgStartDialogV31(const Char *title, PrgCallbackFunc textCallback) {
  return PrgStartDialog(title, textCallback, NULL);
}

ProgressPtr	PrgStartDialog(const Char *title, PrgCallbackFunc textCallback, void *userDataP) {
  debug(DEBUG_ERROR, "PALMOS", "PrgStartDialog not implemented");
  return 0;
}

void PrgStopDialog(ProgressPtr prgP, Boolean force) {
  debug(DEBUG_ERROR, "PALMOS", "PrgStopDialog not implemented");
}

Boolean	PrgHandleEvent(ProgressPtr prgGP, EventType *eventP) {
  debug(DEBUG_ERROR, "PALMOS", "PrgHandleEvent not implemented");
  return 0;
}

void PrgUpdateDialog(ProgressPtr prgGP, UInt16 err, UInt16 stage, const Char *messageP, Boolean updateNow) {
  debug(DEBUG_ERROR, "PALMOS", "PrgUpdateDialog not implemented");
}
