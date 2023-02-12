#include <PalmOS.h>

#include "debug.h"

Err ExgInit(void) {
  debug(DEBUG_ERROR, "PALMOS", "ExgInit not implemented");
  return sysErrParamErr;
}

Err ExgConnect(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgConnect not implemented");
  return sysErrParamErr;
}

Err ExgPut(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgPut not implemented");
  return sysErrParamErr;
}

Err ExgGet(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgGet not implemented");
  return sysErrParamErr;
}

Err ExgAccept(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgAccept not implemented");
  return sysErrParamErr;
}

Err ExgDisconnect(ExgSocketType *socketP, Err error) {
  debug(DEBUG_ERROR, "PALMOS", "ExgDisconnect not implemented");
  return sysErrParamErr;
}

UInt32 ExgSend(ExgSocketType *socketP, const void *bufP, UInt32 bufLen, Err *err) {
  debug(DEBUG_ERROR, "PALMOS", "ExgSend not implemented");
  return 0;
}

UInt32 ExgReceive(ExgSocketType *socketP, void *bufP, UInt32 bufLen, Err *err) {
  debug(DEBUG_ERROR, "PALMOS", "ExgReceive not implemented");
  return 0;
}

Err ExgControl(ExgSocketType *socketP, UInt16 op, void *valueP, UInt16 *valueLenP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgControl not implemented");
  return sysErrParamErr;
}

Err ExgRegisterData(UInt32 creatorID, UInt16 id, const Char *dataTypesP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgRegisterData not implemented");
  return sysErrParamErr;
}

Err ExgRegisterDatatype(UInt32 creatorID, UInt16 id, const Char *dataTypesP, const Char *descriptionsP, UInt16 flags) {
  debug(DEBUG_ERROR, "PALMOS", "ExgRegisterDatatype not implemented");
  return sysErrParamErr;
}

Err ExgNotifyReceiveV35(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgNotifyReceiveV35 not implemented");
  return sysErrParamErr;
}

Err ExgNotifyReceive(ExgSocketType *socketP, UInt16 flags) {
  debug(DEBUG_ERROR, "PALMOS", "ExgNotifyReceive not implemented");
  return sysErrParamErr;
}

Err ExgNotifyGoto(ExgSocketType *socketP, UInt16 flags) {
  debug(DEBUG_ERROR, "PALMOS", "ExgNotifyGoto not implemented");
  return sysErrParamErr;
}

Err ExgDBRead(ExgDBReadProcPtr readProcP, ExgDBDeleteProcPtr deleteProcP, void *userDataP, LocalID *dbIDP, UInt16 cardNo, Boolean *needResetP, Boolean keepDates) {
  debug(DEBUG_ERROR, "PALMOS", "ExgDBRead not implemented");
  return sysErrParamErr;
}

Err ExgDBWrite(ExgDBWriteProcPtr writeProcP, void *userDataP, const char *nameP, LocalID dbID, UInt16 cardNo) {
  debug(DEBUG_ERROR, "PALMOS", "ExgDBWrite not implemented");
  return sysErrParamErr;
}

Boolean ExgDoDialog(ExgSocketType *socketP, ExgDialogInfoType *infoP, Err *errP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgDoDialog not implemented");
  return false;
}

Err ExgRequest(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgRequest not implemented");
  return sysErrParamErr;
}

Err ExgSetDefaultApplication(UInt32 creatorID, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgSetDefaultApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetDefaultApplication(UInt32 *creatorIDP, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgGetDefaultApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetTargetApplication(ExgSocketType *socketP, Boolean unwrap, UInt32 *creatorIDP, Char *descriptionP, UInt32 descriptionSize) {
  debug(DEBUG_ERROR, "PALMOS", "ExgGetTargetApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetRegisteredApplications(UInt32 **creatorIDsP, UInt32 *numAppsP, Char **namesP, Char **descriptionsP, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgGetRegisteredApplications not implemented");
  return sysErrParamErr;
}

Err ExgGetRegisteredTypes(Char **dataTypesP, UInt32 *sizeP, UInt16 id) {
  debug(DEBUG_ERROR, "PALMOS", "ExgGetRegisteredTypes not implemented");
  return sysErrParamErr;
}

Err ExgNotifyPreview(ExgPreviewInfoType *infoP) {
  debug(DEBUG_ERROR, "PALMOS", "ExgNotifyPreview not implemented");
  return sysErrParamErr;
}
