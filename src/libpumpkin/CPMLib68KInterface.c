#include <PalmOS.h>
#include <CPMLib.h>

#include "pumpkin.h"
#include "CryptoPlugin.h"
#include "debug.h"

#define MAX_PROVIDERS 4

typedef struct {
  UInt32 count;
  UInt32 type[MAX_PROVIDERS];
  UInt32 id[MAX_PROVIDERS];
  pluginMainF main[MAX_PROVIDERS];
} provider_id_t;

static void callback(pumpkin_plugin_t *plugin, void *data) {
  provider_id_t *id = (provider_id_t *)data;

  if (id->count < MAX_PROVIDERS) {
    id->type[id->count] = plugin->type;
    id->id[id->count] = plugin->id;
    id->main[id->count++] = plugin->pluginMain;
  }
}

Err CPMLibOpen(UInt16 refnum, UInt16 *numProviders) {
  provider_id_t ids;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && numProviders) {
    ids.count = 0;
    pumpkin_enum_plugins(cryptoPluginType, callback, &ids);
    *numProviders = ids.count;
    debug(DEBUG_TRACE, "CPM", "CPMLibOpen found %d provider(s)", *numProviders);
    err = errNone;
  }

  return err;
}

Err CPMLibClose(UInt16 refnum) {
  return refnum == CpmLibRefNum ? errNone : cpmErrParamErr;
}

Err CPMLibSleep(UInt16 refnum) {
  return refnum == CpmLibRefNum ? errNone : cpmErrParamErr;
}

Err CPMLibWake(UInt16 refnum) {
  return refnum == CpmLibRefNum ? errNone : cpmErrParamErr;
}

Err CPMLibGetInfo(UInt16 refnum, CPMInfoType *infoP) {
  provider_id_t ids;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && infoP) {
    ids.count = 0;
    pumpkin_enum_plugins(cryptoPluginType, callback, &ids);

    infoP->numInstances = 1;
    infoP->numProviders = ids.count;
    infoP->defaultProviderPresent = true;
    err = errNone;
  }

  return err;
}

Err CPMLibSetDebugLevel(UInt16 refnum, UInt8 debugLevel) {
  return errNone;
}

Err CPMLibEnumerateProviders(UInt16 refnum, UInt32 providerIDs[], UInt16 *numProviders) {
  provider_id_t ids;
  UInt32 i;
  char buf[8];
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && providerIDs && numProviders) {
    ids.count = 0;
    pumpkin_enum_plugins(cryptoPluginType, callback, &ids);

    for (i = 0; i < ids.count; i++) {
      providerIDs[i] = ids.id[i];
      pumpkin_id2s(providerIDs[i], buf);
      debug(DEBUG_TRACE, "CPM", "CPMLibEnumerateProviders found provider '%s'", buf);
    }
    *numProviders = ids.count;

    err = errNone;
  }

  return err;
}

Err CPMLibGetProviderInfo(UInt16 refnum, UInt32 providerID, APProviderInfoType *providerInfoP) {
  provider_id_t ids;
  UInt32 i;
  char buf[8];
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && providerInfoP) {
    ids.count = 0;
    pumpkin_enum_plugins(cryptoPluginType, callback, &ids);

    for (i = 0; i < ids.count; i++) {
      if (ids.id[i] == providerID) {
        pumpkin_id2s(providerID, providerInfoP->name);
        providerInfoP->name[4] = 0;
        providerInfoP->other[0] = 0;
        providerInfoP->flags = APF_MP | APF_HASH;
        pumpkin_id2s(providerID, buf);

        switch (ids.type[i]) {
          case cryptoPluginType:
            providerInfoP->flags |= APF_HASH;
            debug(DEBUG_TRACE, "CPM", "CPMLibGetProviderInfo hash provider '%s'", buf);
            break;
        }
        providerInfoP->numAlgorithms = 1; // XXX what number must be set here ?
        providerInfoP->bHardware = false;
        err = errNone;
        break;
      }
    }

    if (i == ids.count) {
      err = cpmErrProviderNotFound;
    }
  }

  return err;
}

Err CPMLibSetDefaultProvider(UInt16 refnum, UInt32 providerID) {
  debug(DEBUG_ERROR, "CPM", "CPMLibSetDefaultProvider not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibGenerateRandomBytes(UInt16 refnum, UInt8 *bufferP, UInt32 *bufLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibGenerateRandomBytes not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibAddRandomSeed(UInt16 refnum, UInt8 *seedDataP, UInt32 dataLen) {
  debug(DEBUG_ERROR, "CPM", "CPMLibAddRandomSeed not implemented");
  return cpmErrUnimplemented;
}

static UInt32 getProvider(UInt32 type, UInt32 providerID, provider_id_t *ids) {
  UInt32 i;

  ids->count = 0;
  pumpkin_enum_plugins(type, callback, ids);

  for (i = 0; i < ids->count; i++) {
    if (ids->id[i] == providerID) {
      break;
    }
  }

  return i < ids->count ? i : -1;
}

Err CPMLibGenerateKey(UInt16 refnum, UInt8 *keyDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP) {
    if ((i = getProvider(cryptoPluginType, keyInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->key_generate) {
        keyInfoP->providerContext.localContext = cryptoPlugin->key_generate(keyInfoP, NULL);
        if (keyInfoP->providerContext.localContext) {
          err = errNone;
        }
      }
    }
  }

  return err;
}

Err CPMLibImportKeyInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP && importDataP && dataLen && encoding == IMPORT_EXPORT_TYPE_RAW) {
    if ((i = getProvider(cryptoPluginType, keyInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->key_generate) {
        keyInfoP->length = dataLen;
        keyInfoP->providerContext.localContext = cryptoPlugin->key_generate(keyInfoP, importDataP);
        if (keyInfoP->providerContext.localContext) {
          err = errNone;
        }
      }
    }
  }

  return err;
}

Err CPMLibExportKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP && dataLenP && encoding == IMPORT_EXPORT_TYPE_RAW) {
    if ((i = getProvider(cryptoPluginType, keyInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->key_export) {
        cryptoPlugin->key_export(keyInfoP->providerContext.localContext, exportDataP, dataLenP);
        err = errNone;
      }
    }
  }

  return err;
}

Err CPMLibReleaseKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP) {
    if ((i = getProvider(cryptoPluginType, keyInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->key_release) {
        cryptoPlugin->key_release(keyInfoP->providerContext.localContext);
        err = errNone;
      }
    }
  }

  return err;
}

Err CPMLibHashInit(UInt16 refnum, APHashInfoType *hashInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP) {
    if ((i = getProvider(cryptoPluginType, hashInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->hash_init && cryptoPlugin->hash_size) {
        hashInfoP->providerContext.localContext = cryptoPlugin->hash_init(hashInfoP->type);
        if (hashInfoP->providerContext.localContext) {
          hashInfoP->length = cryptoPlugin->hash_size(hashInfoP->providerContext.localContext);
          err = errNone;
        }
      }
    } else {
      err = cpmErrProviderNotFound;
    }
  }

  return err;
}

Err CPMLibHashUpdate(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP && bufIn) {
    if ((i = getProvider(cryptoPluginType, hashInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->hash_update) {
        cryptoPlugin->hash_update(hashInfoP->providerContext.localContext, bufIn, bufInLen);
        err = errNone;
      }
    } else {
      err = cpmErrProviderNotFound;
    }
  }

  return err;
}

Err CPMLibHashFinal(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP && bufOut && bufOutLenP) {
    if ((i = getProvider(cryptoPluginType, hashInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->hash_update && cryptoPlugin->hash_finalize) {
        if (*bufOutLenP < hashInfoP->length) {
          *bufOutLenP = hashInfoP->length;
          err = cpmErrBufTooSmall;
        } else {
          if (bufIn) {
            cryptoPlugin->hash_update(hashInfoP->providerContext.localContext, bufIn, bufInLen);
          }
          cryptoPlugin->hash_finalize(hashInfoP->providerContext.localContext, bufOut);
          *bufOutLenP = hashInfoP->length;
          err = errNone;
        }
      }
    } else {
      err = cpmErrProviderNotFound;
    }
  }

  return err;
}

Err CPMLibHash(UInt16 refnum, APHashEnum type, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  Err err = cpmErrParamErr;

  if (hashInfoP && bufIn && bufOut) {
    hashInfoP->type = type;
    if ((err = CPMLibHashInit(refnum, hashInfoP)) == errNone) {
      err = CPMLibHashFinal(refnum, hashInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
      CPMLibReleaseHashInfo(refnum, hashInfoP);
    }
  }

  return err;
}

Err CPMLibImportHashInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APHashInfoType *hashInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibImportHashInfo not supported");
  return cpmErrUnsupported;
}

Err CPMLibExportHashInfo(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibExportHashInfo not supported");
  return cpmErrUnsupported;
}

Err CPMLibReleaseHashInfo(UInt16 refnum, APHashInfoType *hashInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP) {
    if ((i = getProvider(cryptoPluginType, hashInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->hash_free) {
        cryptoPlugin->hash_free(hashInfoP->providerContext.localContext);
        hashInfoP->providerContext.localContext = NULL;
        err = errNone;
      }
    } else {
      err = cpmErrProviderNotFound;
    }
  }

  return err;
}

static Err CPMLibEncryptDecryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, Boolean encrypt) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP && cipherInfoP) {
    if ((i = getProvider(cryptoPluginType, cipherInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->cipher_init) {
        cipherInfoP->providerContext.localContext = cryptoPlugin->cipher_init(keyInfoP, cipherInfoP, encrypt);
        if (cipherInfoP->providerContext.localContext) {
          err = errNone;
        }
      }
    }
  }

  return err;
}

Err CPMLibEncryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP) {
  return CPMLibEncryptDecryptInit(refnum, keyInfoP, cipherInfoP, true);
}

Err CPMLibEncryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP && cipherInfoP && bufIn && bufOut && bufOutLenP) {
    if ((i = getProvider(cryptoPluginType, cipherInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->cipher_update) {
        cryptoPlugin->cipher_update(cipherInfoP->providerContext.localContext, keyInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
        err = errNone;
      }
    }
  }

  return err;
}

Err CPMLibEncryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP && cipherInfoP && bufIn && bufOut && bufOutLenP) {
    if ((i = getProvider(cryptoPluginType, cipherInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->cipher_update) {
        cryptoPlugin->cipher_update(cipherInfoP->providerContext.localContext, keyInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
        err = errNone;
      }
    }
  }

  return err;
}

Err CPMLibEncrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  Err err;

  if ((err = CPMLibEncryptInit(refnum, keyInfoP, cipherInfoP)) == errNone) {
    err = CPMLibEncryptFinal(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
  }

  return err;
}

Err CPMLibDecryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP) {
  return CPMLibEncryptDecryptInit(refnum, keyInfoP, cipherInfoP, false);
}

Err CPMLibDecryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  return CPMLibEncryptUpdate(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
}

Err CPMLibDecryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  return CPMLibEncryptFinal(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
}

Err CPMLibDecrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  return CPMLibEncrypt(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
}

Err CPMLibImportCipherInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibImportCipherInfo not supported");
  return cpmErrUnsupported;
}

Err CPMLibExportCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibExportCipherInfo not supported");
  return cpmErrUnsupported;
}

Err CPMLibReleaseCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && cipherInfoP) {
    if ((i = getProvider(cryptoPluginType, cipherInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->cipher_free) {
        cryptoPlugin->cipher_free(cipherInfoP->providerContext.localContext);
        err = errNone;
      }
    }
  }

  return err;
}

Err CPMLibVerifyInit(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibVerifyInit not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibVerifyUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen) {
  debug(DEBUG_ERROR, "CPM", "CPMLibVerifyUpdate not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibVerifyFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP, UInt8 *signature, UInt32 signatureLen, VerifyResultType *verifyResultP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibVerifyFinal not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibVerify(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP, UInt8 *signature, UInt32 signatureLen, VerifyResultType *verifyResultP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibVerify not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibImportVerifyInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APVerifyInfoType *verifyInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibImportVerifyInfo not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibExportVerifyInfo(UInt16 refnum, APVerifyInfoType *verifyInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibExportVerifyInfo not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibReleaseVerifyInfo(UInt16 refnum, APVerifyInfoType *verifyInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibReleaseVerifyInfo not implemented");
  return cpmErrUnimplemented;
}

void CPMLibTest(void) {
  UInt16 refnum, numProviders;
  APKeyInfoType keyInfo;
  APCipherInfoType cipherInfo;
  UInt8 key[64];
  char plain[64];
  char decrypted[64];
  UInt8 encrypted[64];
  UInt32 len;

  if (SysLibFind(CpmLibName, &refnum) != errNone) return;
  if (SysLibLoad(sysFileTLibrary, cpmCreator, &refnum) != errNone) return;
  if (CPMLibOpen(refnum, &numProviders) != errNone) return;

  MemMove(key, "01234567890123456789012345678901", 32);
  MemSet(&keyInfo, sizeof(APKeyInfoType), 0);
  keyInfo.providerContext.providerID = 's2nC';
  if (CPMLibImportKeyInfo(refnum, IMPORT_EXPORT_TYPE_RAW, key, 32, &keyInfo) == errNone) {
    MemSet(&cipherInfo, sizeof(APCipherInfoType), 0);
    cipherInfo.providerContext.providerID = 's2nC';
    cipherInfo.type = apSymmetricTypeRijndael;

    if (CPMLibEncryptInit(refnum, &keyInfo, &cipherInfo) == errNone) {
      MemSet(plain, 64, 0);
      StrCopy(plain, "Some plain text to be encrypted for testing.");
      len = StrLen(plain);
      debug(DEBUG_INFO, "CPM", "plaintext: %d [%s]", len, plain);
      MemSet(encrypted, 64, 0);
      if (CPMLibEncryptFinal(refnum, &keyInfo, &cipherInfo, (UInt8 *)plain, len, encrypted, &len) == errNone) {
        debug(DEBUG_INFO, "CPM", "encrypted: %d", len);
        debug_bytes(DEBUG_INFO, "CPM", encrypted, len);
      }
      CPMLibReleaseCipherInfo(refnum, &cipherInfo);
    }

    MemSet(&cipherInfo, sizeof(APCipherInfoType), 0);
    cipherInfo.providerContext.providerID = 's2nC';
    cipherInfo.type = apSymmetricTypeRijndael;

    if (CPMLibDecryptInit(refnum, &keyInfo, &cipherInfo) == errNone) {
      MemSet(decrypted, 64, 0);
      if (CPMLibDecryptFinal(refnum, &keyInfo, &cipherInfo, encrypted, len, (UInt8 *)decrypted, &len) == errNone) {
        debug(DEBUG_INFO, "CPM", "decrypted: %d [%s]", len, decrypted);
      }
      CPMLibReleaseCipherInfo(refnum, &cipherInfo);
    }

    CPMLibReleaseKeyInfo(refnum, &keyInfo);
  }

  CPMLibClose(refnum);
}
