#include <PalmOS.h>

#include "debug.h"

Err EncDES(UInt8 *srcP, UInt8 *keyP, UInt8 *dstP, Boolean encrypt) {
  // not documented
  debug(DEBUG_ERROR, "PALMOS", "EncDES not implemented");
  return sysErrParamErr;
}

Err EncDigestMD4(UInt8 *strP, UInt16 strLen, UInt8 digestP[16]) {
  // not documented
  debug(DEBUG_ERROR, "PALMOS", "EncDigestMD4 not implemented");
  return sysErrParamErr;
}

Err EncDigestMD5(UInt8 *strP, UInt16 strLen, UInt8 digestP[16]) {
  // not documented
  debug(DEBUG_ERROR, "PALMOS", "EncDigestMD5 not implemented");
  return sysErrParamErr;
}
