#include <PalmOS.h>
#include <SslLib.h>

#include "debug.h"

Err SslLibName(UInt16 refnum) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibName not implemented");
  return 0;
}

Err SslLibOpen(UInt16 refnum) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibOpen not implemented");
  return 0;
}

Err SslLibClose(UInt16 refnum) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibClose not implemented");
  return 0;
}

Err SslLibSleep(UInt16 refnum) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibSleep not implemented");
  return 0;
}

Err SslLibWake(UInt16 refnum) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibWake not implemented");
  return 0;
}

Err SslLibCreate(UInt16 refnum, SslLib **lib) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibCreate not implemented");
  return 0;
}

Err SslLibDestroy(UInt16 refnum, SslLib *lib) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibDestroy not implemented");
  return 0;
}

Err SslLibSetLong(UInt16 refnum, SslLib *lib, SslAttribute attr, Int32 value) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibSetLong not implemented");
  return 0;
}

Err SslLibSetPtr(UInt16 refnum, SslLib *lib, SslAttribute attr,void *value) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibSetPtr not implemented");
  return 0;
}

Int32 SslLibGetLong(UInt16 refnum, SslLib *lib, SslAttribute attr) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibGetLong not implemented");
  return 0;
}

Err SslLibGetPtr(UInt16 refnum, SslLib *lib, SslAttribute attr,void **value) {
  debug(DEBUG_ERROR, "PALMOS", "SslLibGetPtr not implemented");
  return 0;
}

Err SslContextCreate(UInt16 refnum, SslLib *lib, SslContext **ctx) {
  debug(DEBUG_ERROR, "PALMOS", "SslContextCreate not implemented");
  return 0;
}

Err SslContextDestroy(UInt16 refnum, SslContext *ctx) {
  debug(DEBUG_ERROR, "PALMOS", "SslContextDestroy not implemented");
  return 0;
}

Err SslContextSetLong(UInt16 refnum, SslContext *lib, SslAttribute attr, Int32 v) {
  debug(DEBUG_ERROR, "PALMOS", "SslContextSetLong not implemented");
  return 0;
}

Err SslContextSetPtr(UInt16 refnum, SslContext *lib, SslAttribute attr,void *v) {
  debug(DEBUG_ERROR, "PALMOS", "SslContextSetPtr not implemented");
  return 0;
}

Int32 SslContextGetLong(UInt16 refnum, SslContext *lib, SslAttribute attr) {
  debug(DEBUG_ERROR, "PALMOS", "SslContextGetLong not implemented");
  return 0;
}

Err SslContextGetPtr(UInt16 refnum, SslContext *lib, SslAttribute attr,void **v) {
  debug(DEBUG_ERROR, "PALMOS", "SslContextGetPtr not implemented");
  return 0;
}

Err SslOpen(UInt16 refnum, SslContext *ctx, UInt16 mode, UInt32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SslOpen not implemented");
  return 0;
}

Err SslClose(UInt16 refnum, SslContext *ctx, UInt16 mode, UInt32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SslClose not implemented");
  return 0;
}

void SslConsume(UInt16 refnum, SslContext *ctx, Int32 number) {
  debug(DEBUG_ERROR, "PALMOS", "SslConsume not implemented");
}

Err SslFlush(UInt16 refnum, SslContext *ctx, Int32 *outstanding) {
  debug(DEBUG_ERROR, "PALMOS", "SslFlush not implemented");
  return 0;
}
