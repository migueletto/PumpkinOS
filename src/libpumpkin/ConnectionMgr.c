#include <PalmOS.h>

#include "debug.h"

Err CncProfileSettingGet(CncProfileID profileId, UInt16 paramId, void* paramBufferP, UInt16 * ioParamSizeP) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileSettingGet not implemented");
  return sysErrParamErr;
}

Err CncProfileSettingSet(CncProfileID iProfileId, UInt16 paramId, const void* paramBufferP, UInt16 paramSize) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileSettingSet not implemented");
  return sysErrParamErr;
}

Err CncProfileSetCurrent(CncProfileID profileId) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileSetCurrent not implemented");
  return sysErrParamErr;
}

Err CncProfileGetCurrent(CncProfileID * profileIdP) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileGetCurrent not implemented");
  return sysErrParamErr;
}

Err CncProfileGetIDFromName(const Char* profileNameP, CncProfileID * profileIdP) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileGetIDFromName not implemented");
  return sysErrParamErr;
}

Err CncProfileCreate(CncProfileID * profileIdP) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileCreate not implemented");
  return sysErrParamErr;
}

Err CncProfileDelete(CncProfileID  profileId) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileDelete not implemented");
  return sysErrParamErr;
}

Err CncProfileGetIDFromIndex(UInt16 index, CncProfileID* profileIdP) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileGetIDFromIndex not implemented");
  return sysErrParamErr;
}

Err CncProfileGetIndex(CncProfileID profileId, UInt16* indexP) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileGetIndex not implemented");
  return sysErrParamErr;
}

Err CncProfileCount(UInt16* profilesCountP) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileCount not implemented");
  return sysErrParamErr;
}

Err CncProfileOpenDB(void) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileOpenDB not implemented");
  return sysErrParamErr;
}

Err CncProfileCloseDB(void) {
  debug(DEBUG_ERROR, "PALMOS", "CncProfileCloseDB not implemented");
  return sysErrParamErr;
}

Err CncGetProfileList(Char ***nameListPPP, UInt16 *countP) {
  debug(DEBUG_ERROR, "PALMOS", "CncGetProfileList not implemented");
  return sysErrParamErr;
}

Err CncGetProfileInfo(Char *name, UInt32 *port, UInt32 *baud, UInt16 *volume, UInt16 *handShake, Char *initString, Char *resetString, Boolean *isModem, Boolean *isPulse) {
  debug(DEBUG_ERROR, "PALMOS", "CncGetProfileInfo not implemented");
  return sysErrParamErr;
}

Err CncAddProfile(Char *name, UInt32 port, UInt32 baud, UInt16 volume, UInt16 handShake, const Char *initString, const Char *resetString, Boolean isModem, Boolean isPulse) {
  debug(DEBUG_ERROR, "PALMOS", "CncAddProfile not implemented");
  return sysErrParamErr;
}

Err CncDeleteProfile(const Char *name) {
  debug(DEBUG_ERROR, "PALMOS", "CncDeleteProfile not implemented");
  return sysErrParamErr;
}
