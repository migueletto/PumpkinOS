#include <PalmOS.h>
#include <CPMLib.h>

#include "pumpkin.h"
#include "HashPlugin.h"
#include "md5.h"
#include "sha1.h"
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
    pumpkin_enum_plugins(hashPluginType, callback, &ids);
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
    pumpkin_enum_plugins(hashPluginType, callback, &ids);

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
    pumpkin_enum_plugins(hashPluginType, callback, &ids);

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
    pumpkin_enum_plugins(hashPluginType, callback, &ids);

    for (i = 0; i < ids.count; i++) {
      if (ids.id[i] == providerID) {
        pumpkin_id2s(providerID, providerInfoP->name);
        providerInfoP->name[4] = 0;
        providerInfoP->other[0] = 0;
        providerInfoP->flags = APF_MP | APF_HASH;
        pumpkin_id2s(providerID, buf);

        switch (ids.type[i]) {
          case hashPluginType:
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

Err CPMLibGenerateKey(UInt16 refnum, UInt8 *keyDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibGenerateKey not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibImportKeyInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APKeyInfoType *keyInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibImportKeyInfo not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibExportKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibExportKeyInfo not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibReleaseKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibReleaseKeyInfo not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibHashInit(UInt16 refnum, APHashInfoType *hashInfoP) {
  provider_id_t ids;
  HashPluginType *hashPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP) {
    ids.count = 0;
    pumpkin_enum_plugins(hashPluginType, callback, &ids);
    for (i = 0; i < ids.count; i++) {
      if (ids.id[i] == hashInfoP->providerContext.providerID) {
        break;
      }
    }

    if (i < ids.count) {
      if ((hashPlugin = (HashPluginType *)ids.main[i](NULL)) != NULL) {
        hashInfoP->providerContext.localContext = hashPlugin->init(hashInfoP->type);
        if (hashInfoP->providerContext.localContext) {
          hashInfoP->length = hashPlugin->size(hashInfoP->providerContext.localContext);
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
  HashPluginType *hashPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP && bufIn) {
    ids.count = 0;
    pumpkin_enum_plugins(hashPluginType, callback, &ids);
    for (i = 0; i < ids.count; i++) {
      if (ids.id[i] == hashInfoP->providerContext.providerID) {
        break;
      }
    }

    if (i < ids.count) {
      if ((hashPlugin = (HashPluginType *)ids.main[i](NULL)) != NULL) {
        hashPlugin->update(hashInfoP->providerContext.localContext, bufIn, bufInLen);
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
  HashPluginType *hashPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP && bufOut && bufOutLenP) {
    ids.count = 0;
    pumpkin_enum_plugins(hashPluginType, callback, &ids);
    for (i = 0; i < ids.count; i++) {
      if (ids.id[i] == hashInfoP->providerContext.providerID) {
        break;
      }
    }

    if (i < ids.count) {
      if ((hashPlugin = (HashPluginType *)ids.main[i](NULL)) != NULL) {
        if (*bufOutLenP < hashInfoP->length) {
          *bufOutLenP = hashInfoP->length;
          err = cpmErrBufTooSmall;
        } else {
          if (bufIn) {
            hashPlugin->update(hashInfoP->providerContext.localContext, bufIn, bufInLen);
          }
          hashPlugin->finalize(hashInfoP->providerContext.localContext, bufOut);
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
  HashPluginType *hashPlugin;
  UInt32 i;
  Err err = cpmErrParamErr;

  if (refnum == CpmLibRefNum && hashInfoP) {
    ids.count = 0;
    pumpkin_enum_plugins(hashPluginType, callback, &ids);
    for (i = 0; i < ids.count; i++) {
      if (ids.id[i] == hashInfoP->providerContext.providerID) {
        break;
      }
    }

    if (i < ids.count) {
      if ((hashPlugin = (HashPluginType *)ids.main[i](NULL)) != NULL) {
        hashPlugin->free(hashInfoP->providerContext.localContext);
        hashInfoP->providerContext.localContext = NULL;
        err = errNone;
      }
    } else {
      err = cpmErrProviderNotFound;
    }
  }

  return err;
}

Err CPMLibEncryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibEncryptInit not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibEncryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibEncryptUpdate not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibEncryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibEncryptFinal not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibEncrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibEncrypt not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibDecryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibDecryptInit not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibDecryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibDecryptUpdate not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibDecryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibDecryptFinal not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibDecrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibDecrypt not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibImportCipherInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibImportCipherInfo not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibExportCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibExportCipherInfo not implemented");
  return cpmErrUnimplemented;
}

Err CPMLibReleaseCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP) {
  debug(DEBUG_ERROR, "CPM", "CPMLibReleaseCipherInfo not implemented");
  return cpmErrUnimplemented;
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
