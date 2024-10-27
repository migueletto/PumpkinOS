#include <PalmOS.h>

#include "bytes.h"
#include "debug.h"

// XXX not sure if any of these are correct
// XXX should dstBitOffsetP be incremented in these functions ?

void NetLibBitMove(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt8 *srcP, UInt32 *srcBitOffsetP, UInt32 numBits) {
  debug(DEBUG_ERROR, "PALMOS", "NetLibBitMove not implemented");
}

void NetLibBitPutUIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt32 value) {
  put4b(value, dstP, *dstBitOffsetP);
}

void NetLibBitPutIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, Int32 value) {
  put4b(value, dstP, *dstBitOffsetP);
}

UInt32 NetLibBitGetUIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP) {
  UInt32 value;
  get4b(&value, dstP, *dstBitOffsetP);
  return value;
}

Int32 NetLibBitGetIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP) {
  Int32 value;
  get4b((UInt32 *)&value, dstP, *dstBitOffsetP);
  return value;
}

void NetLibBitPutFixed(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt32 value, UInt16 numBits) {
  debug(DEBUG_ERROR, "PALMOS", "NetLibBitPutFixed not implemented");
}

UInt32 NetLibBitGetFixed(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, UInt16 numBits) {
  debug(DEBUG_ERROR, "PALMOS", "NetLibBitGetFixed not implemented");
  return 0;
}
