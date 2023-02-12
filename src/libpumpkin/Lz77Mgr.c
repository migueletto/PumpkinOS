#include <PalmOS.h>

#include "debug.h"

Err Lz77LibOpen(UInt16 libRefnum, MemHandle *lz77HandleP, Boolean compressFlag, UInt32 sourceSize, MemHandle *destHP,
  UInt32 *destSizeP, UInt16 useVerNum, UInt8 *primerP, UInt32 primerL, UInt32 processedPrimerL) {
  debug(DEBUG_ERROR, "PALMOS", "Lz77LibOpen not implemented");
  return 0;
}

Err Lz77LibClose(UInt16 libRefnum, MemHandle lz77Handle, UInt32 *ResultingSizeP) {
  debug(DEBUG_ERROR, "PALMOS", "Lz77LibClose not implemented");
  return 0;
}

Err Lz77LibSleep(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "Lz77LibSleep not implemented");
  return 0;
}

Err Lz77LibWake(UInt16 libRefnum) {
  debug(DEBUG_ERROR, "PALMOS", "Lz77LibWake not implemented");
  return 0;
}
