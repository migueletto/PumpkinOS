#include <PalmOS.h>

#include "debug.h"

#define PALMOS_MODULE "Category"

// Marked as system use only
Err AddrDBSort(UInt16 libRefnum, DmOpenRef dbR, Int16 other) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "AddrDBSort not implemented");
  return sysErrParamErr;
}

// Marked as system use only
Err AddrJDBSort(UInt16 libRefnum, DmOpenRef dbR, Int16 other) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "AddrJDBSort not implemented");
  return sysErrParamErr;
}
