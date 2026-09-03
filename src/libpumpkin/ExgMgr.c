#include <PalmOS.h>

#include "debug.h"

Err ExgInit(void) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgInit not implemented");
  return sysErrParamErr;
}

Err ExgConnect(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgConnect not implemented");
  return sysErrParamErr;
}

static void printSocket(ExgSocketType *socketP) {
  debug(DEBUG_INFO, "ExgMgr", "ExgPut libraryRef %u", socketP->libraryRef);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut socketRef %u", socketP->socketRef);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut target %u", socketP->target);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut count %u", socketP->count);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut length %u", socketP->length);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut time %u", socketP->time);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut appData %u", socketP->appData);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut goToCreator %u", socketP->goToCreator);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut goToParams.dbCardNo %u", socketP->goToParams.dbCardNo);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut goToParams.dbID 0x%08X", socketP->goToParams.dbID);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut goToParams.recordNum %u", socketP->goToParams.recordNum);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut goToParams.uniqueID 0x%08X", socketP->goToParams.uniqueID);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut goToParams.matchCustom %u", socketP->goToParams.matchCustom);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut localMode %d", socketP->localMode);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut packetMode %d", socketP->packetMode);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut noGoTo %d", socketP->noGoTo);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut noStatus %d", socketP->noStatus);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut preview %d", socketP->preview);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut description \"%s\"", socketP->description);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut type \"%s\"", socketP->type);
  debug(DEBUG_INFO, "ExgMgr", "ExgPut name \"%s\"", socketP->name);
}

// ExgPut: initiates the transfer of data to the destination device.
// Specify either a value for the libraryRef field or a URL in
// the name field. libraryRef should be 0 if the name field
// contains a URL. The structure should also contain a value for
// the target, type, or name field.

Err ExgPut(ExgSocketType *socketP) {
  debug(DEBUG_INFO, "ExgMgr", "ExgPut %p", socketP);

  if (socketP) {
    printSocket(socketP);

    if (socketP->name) {
    } else if (socketP->libraryRef) {
    }
  }

  return errNone;
}

Err ExgGet(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgGet not implemented");
  return sysErrParamErr;
}

Err ExgAccept(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgAccept not implemented");
  return sysErrParamErr;
}

Err ExgDisconnect(ExgSocketType *socketP, Err error) {
  debug(DEBUG_INFO, "ExgMgr", "ExgDisconnect %p 0x%04X", socketP, error);

  if (socketP) {
     printSocket(socketP);
  }

  return errNone;
}

UInt32 ExgSend(ExgSocketType *socketP, const void *bufP, UInt32 bufLen, Err *err) {
  UInt32 r = 0;

  debug(DEBUG_INFO, "ExgMgr", "ExgSend %p %p %u %p", socketP, bufP, bufLen, err);
  if (err) *err = sysErrParamErr;

  if (socketP) {
     printSocket(socketP);
     debug_bytes(DEBUG_INFO, "ExgMgr", (uint8_t *)bufP, bufLen);
     r = bufLen;
     if (err) *err = errNone;
  }

  return r;
}

UInt32 ExgReceive(ExgSocketType *socketP, void *bufP, UInt32 bufLen, Err *err) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgReceive not implemented");
  return 0;
}

Err ExgControl(ExgSocketType *socketP, UInt16 op, void *valueP, UInt16 *valueLenP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgControl not implemented");
  return sysErrParamErr;
}

Err ExgRegisterData(UInt32 creatorID, UInt16 id, const Char *dataTypesP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgRegisterData not implemented");
  return sysErrParamErr;
}

Err ExgRegisterDatatype(UInt32 creatorID, UInt16 id, const Char *dataTypesP, const Char *descriptionsP, UInt16 flags) {
  char screator[8];

  pumpkin_id2s(creatorID, screator);
  if (dataTypesP) {
    debug(DEBUG_ERROR, "ExgMgr", "ExgRegisterDatatype register creator '%s' id 0x%04X dataTypes \"%s\" descriptions \"%s\" flags 0x%04X not implemented",
      screator, id, dataTypesP, descriptionsP, flags);
  } else {
    debug(DEBUG_ERROR, "ExgMgr", "ExgRegisterDatatype unregister creator '%s' id 0x%04X not implemented", screator, id);
  }

  return exgMemError;
}

Err ExgNotifyReceiveV35(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgNotifyReceiveV35 not implemented");
  return sysErrParamErr;
}

Err ExgNotifyReceive(ExgSocketType *socketP, UInt16 flags) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgNotifyReceive not implemented");
  return sysErrParamErr;
}

Err ExgNotifyGoto(ExgSocketType *socketP, UInt16 flags) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgNotifyGoto not implemented");
  return sysErrParamErr;
}

// Converts a Palm OS database from its internal format and writes it to storage RAM
Err ExgDBRead(ExgDBReadProcPtr readProcP, ExgDBDeleteProcPtr deleteProcP, void *userDataP, LocalID *dbIDP, UInt16 cardNo, Boolean *needResetP, Boolean keepDates) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgDBRead not implemented");
  return sysErrParamErr;
}

Err ExgDBWrite(ExgDBWriteProcPtr writeProcP, void *userDataP, const char *nameP, LocalID dbID, UInt16 cardNo) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgDBWrite not implemented");
  return sysErrParamErr;
}

Boolean ExgDoDialog(ExgSocketType *socketP, ExgDialogInfoType *infoP, Err *errP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgDoDialog not implemented");
  return false;
}

Err ExgRequest(ExgSocketType *socketP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgRequest not implemented");
  return sysErrParamErr;
}

Err ExgSetDefaultApplication(UInt32 creatorID, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgSetDefaultApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetDefaultApplication(UInt32 *creatorIDP, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgGetDefaultApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetTargetApplication(ExgSocketType *socketP, Boolean unwrap, UInt32 *creatorIDP, Char *descriptionP, UInt32 descriptionSize) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgGetTargetApplication not implemented");
  return sysErrParamErr;
}

Err ExgGetRegisteredApplications(UInt32 **creatorIDsP, UInt32 *numAppsP, Char **namesP, Char **descriptionsP, UInt16 id, const Char *dataTypeP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgGetRegisteredApplications not implemented");
  return sysErrParamErr;
}

Err ExgGetRegisteredTypes(Char **dataTypesP, UInt32 *sizeP, UInt16 id) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgGetRegisteredTypes not implemented");
  return sysErrParamErr;
}

Err ExgNotifyPreview(ExgPreviewInfoType *infoP) {
  debug(DEBUG_ERROR, "ExgMgr", "ExgNotifyPreview not implemented");
  return sysErrParamErr;
}
