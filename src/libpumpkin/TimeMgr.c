#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "debug.h"

Err TimInit(void) {
  debug(DEBUG_ERROR, "PALMOS", "TimInit not implemented");
  return 0;
}

// Return the current date and time of the device in seconds since 12:00 A.M. on January 1, 1904.
UInt32 TimGetSeconds(void) {
  return sys_time() + pumpkin_dt();
}

void TimSetSeconds(UInt32 seconds) {
  debug(DEBUG_ERROR, "PALMOS", "TimSetSeconds not implemented");
}
