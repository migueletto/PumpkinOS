    case cpmLibTrapAddRandomSeed: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 *seedDataP = sys_va_arg(ap, UInt8 *);
      UInt32 dataLen = sys_va_arg(ap, UInt32);
      Err ret = CPMLibAddRandomSeed(refnum, seedDataP, dataLen);
      *iret = ret;
      }
      break;

    case cpmLibTrapDecrypt: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibDecrypt(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapDecryptFinal: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibDecryptFinal(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapDecryptInit: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      Err ret = CPMLibDecryptInit(refnum, keyInfoP, cipherInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapDecryptUpdate: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibDecryptUpdate(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapEncrypt: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibEncrypt(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapEncryptFinal: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibEncryptFinal(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapEncryptInit: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      Err ret = CPMLibEncryptInit(refnum, keyInfoP, cipherInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapEncryptUpdate: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibEncryptUpdate(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapEnumerateProviders: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt32 *providerIDs = sys_va_arg(ap, UInt32 *);
      UInt16 *numProviders = sys_va_arg(ap, UInt16 *);
      Err ret = CPMLibEnumerateProviders(refnum, providerIDs, numProviders);
      *iret = ret;
      }
      break;

    case cpmLibTrapExportCipherInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *exportDataP = sys_va_arg(ap, UInt8 *);
      UInt32 *dataLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibExportCipherInfo(refnum, cipherInfoP, encoding, exportDataP, dataLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapExportHashInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APHashInfoType *hashInfoP = sys_va_arg(ap, APHashInfoType *);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *exportDataP = sys_va_arg(ap, UInt8 *);
      UInt32 *dataLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibExportHashInfo(refnum, hashInfoP, encoding, exportDataP, dataLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapExportKeyInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *exportDataP = sys_va_arg(ap, UInt8 *);
      UInt32 *dataLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibExportKeyInfo(refnum, keyInfoP, encoding, exportDataP, dataLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapExportVerifyInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APVerifyInfoType *verifyInfoP = sys_va_arg(ap, APVerifyInfoType *);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *exportDataP = sys_va_arg(ap, UInt8 *);
      UInt32 *dataLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibExportVerifyInfo(refnum, verifyInfoP, encoding, exportDataP, dataLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapGenerateKey: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 *keyDataP = sys_va_arg(ap, UInt8 *);
      UInt32 dataLen = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      Err ret = CPMLibGenerateKey(refnum, keyDataP, dataLen, keyInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapGenerateRandomBytes: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 *bufferP = sys_va_arg(ap, UInt8 *);
      UInt32 *bufLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibGenerateRandomBytes(refnum, bufferP, bufLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapGetInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      CPMInfoType *infoP = sys_va_arg(ap, CPMInfoType *);
      Err ret = CPMLibGetInfo(refnum, infoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapGetProviderInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt32 providerID = sys_va_arg(ap, UInt32);
      APProviderInfoType *providerInfoP = sys_va_arg(ap, APProviderInfoType *);
      Err ret = CPMLibGetProviderInfo(refnum, providerID, providerInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapHash: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APHashEnum type = sys_va_arg(ap, APHashEnum);
      APHashInfoType *hashInfoP = sys_va_arg(ap, APHashInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibHash(refnum, type, hashInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapHashFinal: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APHashInfoType *hashInfoP = sys_va_arg(ap, APHashInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      Err ret = CPMLibHashFinal(refnum, hashInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      *iret = ret;
      }
      break;

    case cpmLibTrapHashInit: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APHashInfoType *hashInfoP = sys_va_arg(ap, APHashInfoType *);
      Err ret = CPMLibHashInit(refnum, hashInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapHashUpdate: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APHashInfoType *hashInfoP = sys_va_arg(ap, APHashInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      Err ret = CPMLibHashUpdate(refnum, hashInfoP, bufIn, bufInLen);
      *iret = ret;
      }
      break;

    case cpmLibTrapImportCipherInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *importDataP = sys_va_arg(ap, UInt8 *);
      UInt32 dataLen = sys_va_arg(ap, UInt32);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      Err ret = CPMLibImportCipherInfo(refnum, encoding, importDataP, dataLen, cipherInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapImportHashInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *importDataP = sys_va_arg(ap, UInt8 *);
      UInt32 dataLen = sys_va_arg(ap, UInt32);
      APHashInfoType *hashInfoP = sys_va_arg(ap, APHashInfoType *);
      Err ret = CPMLibImportHashInfo(refnum, encoding, importDataP, dataLen, hashInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapImportKeyInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *importDataP = sys_va_arg(ap, UInt8 *);
      UInt32 dataLen = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      Err ret = CPMLibImportKeyInfo(refnum, encoding, importDataP, dataLen, keyInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapImportVerifyInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 encoding = sys_va_arg(ap, UInt32);
      UInt8 *importDataP = sys_va_arg(ap, UInt8 *);
      UInt32 dataLen = sys_va_arg(ap, UInt32);
      APVerifyInfoType *verifyInfoP = sys_va_arg(ap, APVerifyInfoType *);
      Err ret = CPMLibImportVerifyInfo(refnum, encoding, importDataP, dataLen, verifyInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapReleaseCipherInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APCipherInfoType *cipherInfoP = sys_va_arg(ap, APCipherInfoType *);
      Err ret = CPMLibReleaseCipherInfo(refnum, cipherInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapReleaseHashInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APHashInfoType *hashInfoP = sys_va_arg(ap, APHashInfoType *);
      Err ret = CPMLibReleaseHashInfo(refnum, hashInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapReleaseKeyInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      Err ret = CPMLibReleaseKeyInfo(refnum, keyInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapReleaseVerifyInfo: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APVerifyInfoType *verifyInfoP = sys_va_arg(ap, APVerifyInfoType *);
      Err ret = CPMLibReleaseVerifyInfo(refnum, verifyInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapSetDebugLevel: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt8 debugLevel = sys_va_arg(ap, UInt32);
      Err ret = CPMLibSetDebugLevel(refnum, debugLevel);
      *iret = ret;
      }
      break;

    case cpmLibTrapSetDefaultProvider: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt32 providerID = sys_va_arg(ap, UInt32);
      Err ret = CPMLibSetDefaultProvider(refnum, providerID);
      *iret = ret;
      }
      break;

    case cpmLibTrapVerify: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APVerifyInfoType *verifyInfoP = sys_va_arg(ap, APVerifyInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      UInt8 *signature = sys_va_arg(ap, UInt8 *);
      UInt32 signatureLen = sys_va_arg(ap, UInt32);
      VerifyResultType *verifyResultP = sys_va_arg(ap, VerifyResultType *);
      Err ret = CPMLibVerify(refnum, keyInfoP, verifyInfoP, bufIn, bufInLen, bufOut, bufOutLenP, signature, signatureLen, verifyResultP);
      *iret = ret;
      }
      break;

    case cpmLibTrapVerifyFinal: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APVerifyInfoType *verifyInfoP = sys_va_arg(ap, APVerifyInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      UInt8 *bufOut = sys_va_arg(ap, UInt8 *);
      UInt32 *bufOutLenP = sys_va_arg(ap, UInt32 *);
      UInt8 *signature = sys_va_arg(ap, UInt8 *);
      UInt32 signatureLen = sys_va_arg(ap, UInt32);
      VerifyResultType *verifyResultP = sys_va_arg(ap, VerifyResultType *);
      Err ret = CPMLibVerifyFinal(refnum, keyInfoP, verifyInfoP, bufIn, bufInLen, bufOut, bufOutLenP, signature, signatureLen, verifyResultP);
      *iret = ret;
      }
      break;

    case cpmLibTrapVerifyInit: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APVerifyInfoType *verifyInfoP = sys_va_arg(ap, APVerifyInfoType *);
      Err ret = CPMLibVerifyInit(refnum, keyInfoP, verifyInfoP);
      *iret = ret;
      }
      break;

    case cpmLibTrapVerifyUpdate: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      APKeyInfoType *keyInfoP = sys_va_arg(ap, APKeyInfoType *);
      APVerifyInfoType *verifyInfoP = sys_va_arg(ap, APVerifyInfoType *);
      UInt8 *bufIn = sys_va_arg(ap, UInt8 *);
      UInt32 bufInLen = sys_va_arg(ap, UInt32);
      Err ret = CPMLibVerifyUpdate(refnum, keyInfoP, verifyInfoP, bufIn, bufInLen);
      *iret = ret;
      }
      break;

    case sysLibTrapClose: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      Err ret = CPMLibClose(refnum);
      *iret = ret;
      }
      break;

    case sysLibTrapOpen: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      UInt16 *numProviders = sys_va_arg(ap, UInt16 *);
      Err ret = CPMLibOpen(refnum, numProviders);
      *iret = ret;
      }
      break;

    case sysLibTrapSleep: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      Err ret = CPMLibSleep(refnum);
      *iret = ret;
      }
      break;

    case sysLibTrapWake: {
      UInt16 refnum = sys_va_arg(ap, UInt32);
      Err ret = CPMLibWake(refnum);
      *iret = ret;
      }
      break;

