#include <PalmOS.h>

#include "debug.h"

Int32 DbgInit(MemPtr spaceP, MemPtr dispatchTableP[], Boolean openComm) {
  debug(DEBUG_ERROR, "PALMOS", "DbgInit not implemented");
  return 0;
}

void DbgSrcBreak(void) {
  debug(DEBUG_ERROR, "PALMOS", "DbgSrcBreak not implemented");
}

void DbgSrcMessage(const Char *debugStr) {
  debug(DEBUG_ERROR, "PALMOS", "DbgSrcMessage not implemented");
}

void DbgBreak(void) {
  debug(DEBUG_ERROR, "PALMOS", "DbgBreak not implemented");
}

void DbgMessage(const Char *aStr) {
  debug(DEBUG_ERROR, "PALMOS", "DbgMessage not implemented");
}

Char *DbgGetMessage(UInt8 *bufferP, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "DbgGetMessage not implemented");
  return 0;
}

Err DbgCommSettings(UInt32 *baudP, UInt32 *flagsP) {
  debug(DEBUG_ERROR, "PALMOS", "DbgCommSettings not implemented");
  return 0;
}
