#include <PalmOS.h>

#include "debug.h"

Err ExgInit(void) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgInit not implemented");
  return sysErrParamErr;
}

Err ExgConnect(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgConnect not implemented");
  return sysErrParamErr;
}

Err ExgPut(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgPut not implemented");
  return sysErrParamErr;
}

Err ExgGet(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgGet not implemented");
  return sysErrParamErr;
}

Err ExgAccept(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgAccept not implemented");
  return sysErrParamErr;
}

Err ExgDisconnect(ExgSocketType *socketP, Err error) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgDisconnect not implemented");
  return sysErrParamErr;
}

UInt32 ExgSend(ExgSocketType *socketP, const void *bufP, UInt32 bufLen, Err *err) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgSend not implemented");
  return 0;
}

UInt32 ExgReceive(ExgSocketType *socketP, void *bufP, UInt32 bufLen, Err *err) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgReceive not implemented");
  return 0;
}

Err ExgControl(ExgSocketType *socketP, UInt16 op, void *valueP, UInt16 *valueLenP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgControl not implemented");
  return sysErrParamErr;
}

Err ExgRegisterData(UInt32 creatorID, UInt16 id, const Char *dataTypesP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgRegisterData not implemented");
  return sysErrParamErr;
}

Err ExgRegisterDatatype(UInt32 creatorID, UInt16 id, const Char *dataTypesP, const Char *descriptionsP, UInt16 flags) {
  char screator[8];

  pumpkin_id2s(creatorID, screator);
  if (dataTypesP) {
    debug(DEBUG_ERROR, PUMPKINOS, "ExgRegisterDatatype register creator '%s' id 0x%04X dataTypes \"%s\" descriptions \"%s\" flags 0x%04X not implemented",
      screator, id, dataTypesP, descriptionsP, flags);
  } else {
    debug(DEBUG_ERROR, PUMPKINOS, "ExgRegisterDatatype unregister creator '%s' id 0x%04X not implemented", screator, id);
  }

  return exgMemError;
}

Err ExgNotifyReceiveV35(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgNotifyReceiveV35 not implemented");
  return sysErrParamErr;
}

Err ExgNotifyReceive(ExgSocketType *socketP, UInt16 flags) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgNotifyReceive not implemented");
  return sysErrParamErr;
}

Err ExgNotifyGoto(ExgSocketType *socketP, UInt16 flags) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgNotifyGoto not implemented");
  return sysErrParamErr;
}

Err ExgDBRead(ExgDBReadProcPtr readProcP, ExgDBDeleteProcPtr deleteProcP, void *userDataP, LocalID *dbIDP, UInt16 cardNo, Boolean *needResetP, Boolean keepDates) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgDBRead not implemented");
  return sysErrParamErr;
}

Err ExgDBWrite(ExgDBWriteProcPtr writeProcP, void *userDataP, const char *nameP, LocalID dbID, UInt16 cardNo) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgDBWrite not implemented");
  return sysErrParamErr;
}

Boolean ExgDoDialog(ExgSocketType *socketP, ExgDialogInfoType *infoP, Err *errP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgDoDialog not implemented");
  return false;
}

Err ExgRequest(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgRequest not implemented");
  return sysErrParamErr;
}

Err ExgSetDefaultApplication(UInt32 creatorID, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgSetDefaultApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetDefaultApplication(UInt32 *creatorIDP, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgGetDefaultApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetTargetApplication(ExgSocketType *socketP, Boolean unwrap, UInt32 *creatorIDP, Char *descriptionP, UInt32 descriptionSize) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgGetTargetApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetRegisteredApplications(UInt32 **creatorIDsP, UInt32 *numAppsP, Char **namesP, Char **descriptionsP, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgGetRegisteredApplications not implemented");
  return sysErrParamErr;
}

Err ExgGetRegisteredTypes(Char **dataTypesP, UInt32 *sizeP, UInt16 id) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgGetRegisteredTypes not implemented");
  return sysErrParamErr;
}

Err ExgNotifyPreview(ExgPreviewInfoType *infoP) {
  debug(DEBUG_ERROR, PUMPKINOS, "ExgNotifyPreview not implemented");
  return sysErrParamErr;
}
