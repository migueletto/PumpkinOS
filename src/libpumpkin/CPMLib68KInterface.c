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
  UInt32 providerIDs[MAX_PROVIDERS];
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && numProviders) {
    err = CPMLibEnumerateProviders(refnum, providerIDs, numProviders);
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
  CryptoPluginType *cryptoPlugin;
  UInt32 i, count;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && providerIDs && numProviders) {
    ids.count = 0;
    pumpkin_enum_plugins(cryptoPluginType, callback, &ids);

    for (i = 0, count = 0; i < ids.count && count < MAX_PROVIDERS; i++) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->info) {
        providerIDs[count++] = ids.id[i];
      }
    }
    *numProviders = count;
    err = errNone;
  }

  return err;
}

Err CPMLibGetProviderInfo(UInt16 refnum, UInt32 providerID, APProviderInfoType *providerInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i, flags;
  const char *name;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && providerInfoP) {
    ids.count = 0;
    pumpkin_enum_plugins(cryptoPluginType, callback, &ids);

    for (i = 0; i < ids.count; i++) {
      if (ids.id[i] == providerID) {
        if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->info) {
          name = cryptoPlugin->info(&flags);
          StrNCopy(providerInfoP->name, name, 32);
          providerInfoP->other[0] = 0;
          providerInfoP->flags = flags;
          providerInfoP->numAlgorithms = 1; // XXX what number must be set here ?
          providerInfoP->bHardware = false;
          err = errNone;
        }
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

static UInt32 getProvider(UInt32 type, UInt32 providerID, provider_id_t *ids) {
  UInt32 i;

  ids->count = 0;
  pumpkin_enum_plugins(type, callback, ids);

  // if providerID is zero, return the first provider (if any)
  if (providerID == 0) return ids->count > 0 ? 0 : -1;

  for (i = 0; i < ids->count; i++) {
    if (ids->id[i] == providerID) {
      break;
    }
  }

  return i < ids->count ? i : -1;
}

Err CPMLibGenerateRandomBytes(UInt16 refnum, UInt8 *bufferP, UInt32 *bufLenP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && bufferP && bufLenP) {
    // XXX use the first provider
    if ((i = getProvider(cryptoPluginType, 0, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->random_get) {
        if (cryptoPlugin->random_get(bufferP, *bufLenP)) {
          err = errNone;
        }
      }
    }
  }

  return err;
}

Err CPMLibAddRandomSeed(UInt16 refnum, UInt8 *seedDataP, UInt32 dataLen) {
  debug(DEBUG_ERROR, "CPM", "CPMLibAddRandomSeed not cpmErrUnsupported");
  return cpmErrUnsupported;
}

Err CPMLibGenerateKey(UInt16 refnum, UInt8 *keyDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  provider_id_t ids;
  CryptoPluginType *cryptoPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && keyInfoP) {
    if ((i = getProvider(cryptoPluginType, keyInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->key_generate) {
        if (cryptoPlugin->key_generate(keyInfoP, NULL)) {
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

  if (refnum == CpmLibRefNum && keyInfoP && importDataP) {
    if ((i = getProvider(cryptoPluginType, keyInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->key_import) {
        if (cryptoPlugin->key_import(keyInfoP, encoding, importDataP, dataLen)) {
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

  if (refnum == CpmLibRefNum && keyInfoP && dataLenP && keyInfoP->exportable) {
    if ((i = getProvider(cryptoPluginType, keyInfoP->providerContext.providerID, &ids)) != -1) {
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->key_export) {
        if (cryptoPlugin->key_export(keyInfoP, encoding, exportDataP, dataLenP)) {
          err = errNone;
        }
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
        cryptoPlugin->key_release(keyInfoP);
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
      if ((cryptoPlugin = (CryptoPluginType *)ids.main[i](NULL)) != NULL && cryptoPlugin->hash_init) {
        if (cryptoPlugin->hash_init(hashInfoP)) {
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
        cryptoPlugin->hash_update(hashInfoP, bufIn, bufInLen);
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
            cryptoPlugin->hash_update(hashInfoP, bufIn, bufInLen);
          }
          cryptoPlugin->hash_finalize(hashInfoP, bufOut);
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
        cryptoPlugin->hash_free(hashInfoP);
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
        if (cryptoPlugin->cipher_init(keyInfoP, cipherInfoP, encrypt)) {
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
        if (cryptoPlugin->cipher_update(cipherInfoP->providerContext.localContext, keyInfoP, bufIn, bufInLen, bufOut, bufOutLenP)) {
          err = errNone;
        }
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
        if (cryptoPlugin->cipher_update(cipherInfoP->providerContext.localContext, keyInfoP, bufIn, bufInLen, bufOut, bufOutLenP)) {
          err = errNone;
        }
      }
    }
  }

  return err;
}

Err CPMLibEncrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  Err err;

  if ((err = CPMLibEncryptInit(refnum, keyInfoP, cipherInfoP)) == errNone) {
    err = CPMLibEncryptFinal(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
    CPMLibReleaseCipherInfo(refnum, cipherInfoP);
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
  Err err;

  if ((err = CPMLibDecryptInit(refnum, keyInfoP, cipherInfoP)) == errNone) {
    err = CPMLibDecryptFinal(refnum, keyInfoP, cipherInfoP, bufIn, bufInLen, bufOut, bufOutLenP);
    CPMLibReleaseCipherInfo(refnum, cipherInfoP);
  }

  return err;
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

#define KEY_SIZE   32   // 256 bits
#define PLAIN_SIZE 64

void CPMLibTest(void) {
  UInt16 refnum, numProviders;
  APProviderInfoType providerInfo;
  APKeyInfoType keyInfo;
  APCipherInfoType cipherInfo;
  UInt32 providerIDs[MAX_PROVIDERS];
  char buf[8];
  char plain[PLAIN_SIZE];
  char decrypted[PLAIN_SIZE];
  UInt8 encrypted[PLAIN_SIZE];
  UInt32 i, len, providerID;

  if (SysLibFind(CpmLibName, &refnum) != errNone) return;
  if (SysLibLoad(sysFileTLibrary, cpmCreator, &refnum) != errNone) return;
  if (CPMLibOpen(refnum, &numProviders) != errNone) return;

  providerID = 0;

  if (CPMLibEnumerateProviders(refnum, providerIDs, &numProviders) == errNone) {
    debug(DEBUG_INFO, "CPM", "found %d provider(s)", numProviders);
    for (i = 0; i < numProviders; i++) {
      if (CPMLibGetProviderInfo(refnum, providerIDs[i], &providerInfo) == errNone) {
        pumpkin_id2s(providerIDs[i], buf);
        debug(DEBUG_INFO, "CPM", "provider %d: id '%s', name \"%s\", flags 0x%08X", i+1, buf, providerInfo.name, providerInfo.flags);
        if (providerID == 0 && (providerInfo.flags & APF_CIPHER)) {
          debug(DEBUG_INFO, "CPM", "will use provider id '%s' for encryption", buf);
          providerID = providerIDs[i];
        }
      }
    }
  }

  if (providerID == 0) {
    debug(DEBUG_ERROR, "CPM", "no provider available for encryption");
    return;
  }

  MemSet(&keyInfo, sizeof(APKeyInfoType), 0);
  keyInfo.providerContext.providerID = providerID;
  keyInfo.length = KEY_SIZE;

  if (CPMLibGenerateKey(refnum, NULL, 0, &keyInfo) == errNone) {
    MemSet(&cipherInfo, sizeof(APCipherInfoType), 0);
    cipherInfo.providerContext.providerID = providerID;
    cipherInfo.type = apSymmetricTypeRijndael;

    MemSet(plain, PLAIN_SIZE, 0);
    MemSet(encrypted, PLAIN_SIZE, 0);
    MemSet(decrypted, PLAIN_SIZE, 0);
    StrCopy(plain, "Some plain text to be encrypted for testing.");
    len = PLAIN_SIZE;
    debug(DEBUG_INFO, "CPM", "plain text: %d bytes, length %d bytes [%s]", len, StrLen(plain), plain);

    if (CPMLibEncrypt(refnum, &keyInfo, &cipherInfo, (UInt8 *)plain, len, encrypted, &len) == errNone) {
      debug(DEBUG_INFO, "CPM", "encrypted: %d bytes", len);
      debug_bytes(DEBUG_INFO, "CPM", encrypted, len);

      if (CPMLibDecrypt(refnum, &keyInfo, &cipherInfo, encrypted, len, (UInt8 *)decrypted, &len) == errNone) {
        debug(DEBUG_INFO, "CPM", "decrypted: %d bytes, length %d bytes [%s]", len, StrLen(decrypted), decrypted);
        if (!StrCompare(plain, decrypted)) {
          debug(DEBUG_INFO, "CPM", "decrypted text matches plain text");
        } else {
          debug(DEBUG_ERROR, "CPM", "decrypted text does not match plain text");
        }
      } else {
        debug(DEBUG_ERROR, "CPM", "encryption failed");
      }
    } else {
      debug(DEBUG_ERROR, "CPM", "encryption failed");
    }

    CPMLibReleaseKeyInfo(refnum, &keyInfo);
  } else {
    debug(DEBUG_ERROR, "CPM", "inport key failed");
  }

  CPMLibClose(refnum);
}
