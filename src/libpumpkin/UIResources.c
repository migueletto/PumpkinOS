#include <PalmOS.h>

#include "debug.h"

void *ResLoadForm(UInt16 rscID) {
  debug(DEBUG_ERROR, "PALMOS", "ResLoadForm not implemented");
  return NULL;
}

void *ResLoadMenu(UInt16 rscID) {
  debug(DEBUG_ERROR, "PALMOS", "ResLoadMenu not implemented");
  return NULL;
}

// system use only
Char *ResLoadString(UInt16 rscID) {
  debug(DEBUG_ERROR, "PALMOS", "ResLoadString not implemented");
  return NULL;
}

UInt32 ResLoadConstant(UInt16 rscID) {
  MemHandle h;
  UInt32 *p, r = 0;

  if ((h = DmGetResource(constantRscType, rscID)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      r = *p;
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  return r;
}
