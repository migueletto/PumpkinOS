#include <PalmOS.h>

#include <time.h>
#include <sys/time.h>

#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "sys.h"
#include "debug.h"
#include "xalloc.h"

Err TimInit(void) {
  debug(DEBUG_ERROR, "PALMOS", "TimInit not implemented");
  return 0;
}

// Return the tick count since the last reset.
UInt32 TimGetTicks(void) {
  return ((uint64_t)sys_get_clock()) / 10000; // XXX 100 ticks per second
}

// Return the current date and time of the device in seconds since 12:00 A.M. on January 1, 1904.
UInt32 TimGetSeconds(void) {
  return sys_time() + pumpkin_dt();
}

void TimSetSeconds(UInt32 seconds) {
  debug(DEBUG_ERROR, "PALMOS", "TimSetSeconds not implemented");
}
