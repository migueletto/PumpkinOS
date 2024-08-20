#include <PalmOS.h>

#include "md5.h"
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
  MD5Context ctx;
  Err err =  sysErrParamErr;

  if (strP && digestP) {
    md5Init(&ctx);
    md5Update(&ctx, strP, strLen);
    md5Finalize(&ctx);
    MemMove(digestP, ctx.digest, 16);
    debug(DEBUG_TRACE, "MD5", "EncDigestMD5 \"%.*s\"", strLen, strP);
    debug_bytes(DEBUG_TRACE, "MD5", digestP, 16);
    err = errNone;
  }

  return err;
}
