#include <PalmOS.h>
#include <CPMLib.h>

#include "debug.h"

Err CPMLibOpen(UInt16 refnum, UInt16 *numProviders) {
  if (numProviders) *numProviders = 0;
  return cpmErrNoProviders;
}

Err CPMLibClose(UInt16 refnum) {
  return errNone;
}

Err CPMLibSleep(UInt16 refnum) {
  return errNone;
}

Err CPMLibWake(UInt16 refnum) {
  return errNone;
}

Err CPMLibGetInfo(UInt16 refnum, CPMInfoType *infoP) {
  if (infoP) {
    infoP->numInstances = 0;
    infoP->numProviders = 0;
  }

  return errNone;
}

Err CPMLibSetDebugLevel(UInt16 refnum, UInt8 debugLevel) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibSetDebugLevel not implemented");
  return sysErrParamErr;
}

Err CPMLibEnumerateProviders(UInt16 refnum, UInt32 providerIDs[], UInt16 *numProviders) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibEnumerateProviders not implemented");
  return sysErrParamErr;
}

Err CPMLibGetProviderInfo(UInt16 refnum, UInt32 providerID, APProviderInfoType *providerInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibGetProviderInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibSetDefaultProvider(UInt16 refnum, UInt32 providerID) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibSetDefaultProvider not implemented");
  return sysErrParamErr;
}

Err CPMLibGenerateRandomBytes(UInt16 refnum, UInt8 *bufferP, UInt32 *bufLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibGenerateRandomBytes not implemented");
  return sysErrParamErr;
}

Err CPMLibAddRandomSeed(UInt16 refnum, UInt8 *seedDataP, UInt32 dataLen) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibAddRandomSeed not implemented");
  return sysErrParamErr;
}

Err CPMLibGenerateKey(UInt16 refnum, UInt8 *keyDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibGenerateKey not implemented");
  return sysErrParamErr;
}

Err CPMLibImportKeyInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibImportKeyInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibExportKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibExportKeyInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibReleaseKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibReleaseKeyInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibHashInit(UInt16 refnum, APHashInfoType *hashInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibHashInit not implemented");
  return sysErrParamErr;
}

Err CPMLibHashUpdate(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibHashUpdate not implemented");
  return sysErrParamErr;
}

Err CPMLibHashFinal(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibHashFinal not implemented");
  return sysErrParamErr;
}

Err CPMLibHash(UInt16 refnum, APHashEnum type, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibHash not implemented");
  return sysErrParamErr;
}

Err CPMLibImportHashInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APHashInfoType *hashInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibImportHashInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibExportHashInfo(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibExportHashInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibReleaseHashInfo(UInt16 refnum, APHashInfoType *hashInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibReleaseHashInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibEncryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibEncryptInit not implemented");
  return sysErrParamErr;
}

Err CPMLibEncryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibEncryptUpdate not implemented");
  return sysErrParamErr;
}

Err CPMLibEncryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibEncryptFinal not implemented");
  return sysErrParamErr;
}

Err CPMLibEncrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibEncrypt not implemented");
  return sysErrParamErr;
}

Err CPMLibDecryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibDecryptInit not implemented");
  return sysErrParamErr;
}

Err CPMLibDecryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibDecryptUpdate not implemented");
  return sysErrParamErr;
}

Err CPMLibDecryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibDecryptFinal not implemented");
  return sysErrParamErr;
}

Err CPMLibDecrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibDecrypt not implemented");
  return sysErrParamErr;
}

Err CPMLibImportCipherInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibImportCipherInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibExportCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibExportCipherInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibReleaseCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibReleaseCipherInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibVerifyInit(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibVerifyInit not implemented");
  return sysErrParamErr;
}

Err CPMLibVerifyUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibVerifyUpdate not implemented");
  return sysErrParamErr;
}

Err CPMLibVerifyFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP, UInt8 *signature, UInt32 signatureLen, VerifyResultType *verifyResultP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibVerifyFinal not implemented");
  return sysErrParamErr;
}

Err CPMLibVerify(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP, UInt8 *signature, UInt32 signatureLen, VerifyResultType *verifyResultP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibVerify not implemented");
  return sysErrParamErr;
}

Err CPMLibImportVerifyInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APVerifyInfoType *verifyInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibImportVerifyInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibExportVerifyInfo(UInt16 refnum, APVerifyInfoType *verifyInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibExportVerifyInfo not implemented");
  return sysErrParamErr;
}

Err CPMLibReleaseVerifyInfo(UInt16 refnum, APVerifyInfoType *verifyInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "CPMLibReleaseVerifyInfo not implemented");
  return sysErrParamErr;
}
