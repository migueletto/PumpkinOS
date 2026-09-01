#include <PalmOS.h>
#include <INetMgr.h>

#include "bytes.h"
#include "debug.h"

typedef struct {
  UInt16 config;
  UInt32 flags;
  DmOpenRef cacheRef;
  UInt32 cacheSize;
} INetLibData;

Err INetLibOpen(UInt16 libRefnum, UInt16 config, UInt32 flags, DmOpenRef cacheRef, UInt32 cacheSize, MemHandle *inetHP) {
  INetLibData *data;
  MemHandle h;
  Err err = inetErrTooManyClients;

  if ((h = MemHandleNew(sizeof(INetLibData))) != NULL) {
    if ((data = MemHandleLock(h)) != NULL) {
      data->config = config;
      data->flags = flags;
      data->cacheRef = cacheRef;
      data->cacheSize = cacheSize;
      *inetHP = h;
      MemHandleUnlock(h);
      err = errNone;
    }
  }

  return err;
}

Err INetLibClose(UInt16 libRefnum, MemHandle inetH) {
  if (inetH) {
    MemHandleFree(inetH);
  }

  return errNone;
}

Err INetLibSleep(UInt16 libRefnum) {
  return errNone;
}

Err INetLibWake(UInt16 libRefnum) {
  return errNone;
}

static Err INetLibSettingGetUInt32(void *buf, UInt16 *bufLenP, UInt32 value) {
  Err err = inetErrSettingSizeInvalid;

  if (buf && bufLenP && *bufLenP == sizeof(UInt32)) {
    if (pumpkin_is_m68k()) {
      put4b(value, (uint8_t *)buf, 0);
    } else {
      put4l(value, (uint8_t *)buf, 0);
    }
    err = errNone;
  }

  return err;
}

static Err INetLibSettingGetPtr(void *buf, UInt16 *bufLenP, void *value) {
  UInt8 *ram;
  Err err = inetErrSettingSizeInvalid;

  if (buf && bufLenP) {
    if (pumpkin_is_m68k()) {
      if (*bufLenP == sizeof(UInt32)) {
        ram = pumpkin_heap_base();
        put4b((uint8_t *)value - ram, (uint8_t *)buf, 0);
        err = errNone;
      }
    } else { 
      if (*bufLenP == sizeof(void *)) {
        sys_memcpy(buf, &value, sizeof(void *));
      }
    }
  }
    
  return err;
}

Err INetLibSettingGet(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSettingEnum */ setting, void *bufP, UInt16 *bufLenP) {
  INetLibData *data;
  Err err = inetErrParamsInvalid;

  if (inetH && bufP && bufLenP) {
    if ((data = MemHandleLock(inetH)) != NULL) {
      switch (setting) {
        case inetSettingProxyType:           // (RW) UInt32, INetProxyEnum
          err = INetLibSettingGetUInt32(bufP, bufLenP, inetProxyNone);
          break;
        case inetSettingProxyName:           // (RW) Char[], name of proxy
          err = INetLibSettingGetUInt32(bufP, bufLenP, 0);
          break;
        case inetSettingProxyPort:           // (RW) UInt32,  TCP port # of proxy
          err = INetLibSettingGetUInt32(bufP, bufLenP, 0);
          break;
        case inetSettingProxySocketType:     // (RW) UInt32, which type of socket to use netSocketTypeXXX
          break;
        case inetSettingCacheSize:           // (RW) UInt32, max size of cache
          err = INetLibSettingGetUInt32(bufP, bufLenP, data->cacheSize);
          break;
        case inetSettingCacheRef:            // (R) DmOpenRef, ref of cache DB
          err = INetLibSettingGetPtr(bufP, bufLenP, data->cacheRef);
          break;
        case inetSettingNetLibConfig:        // (RW) UInt32, Which NetLib config to use.
          err = INetLibSettingGetUInt32(bufP, bufLenP, data->config);
          break;
        case inetSettingRadioID:             // (R)  UInt32[2], the 64-bit radio ID
        case inetSettingBaseStationID:       // (R)  UInt32, the radio base station ID
        case inetSettingMaxRspSize:          // (W) UInt32 (in bytes)
        case inetSettingConvAlgorithm:       // (W) UInt32 (CTPConvEnum)
        case inetSettingContentWidth:        // (W) UInt32 (in pixels)
        case inetSettingContentVersion:      // (W) UInt32 Content version (encoder version)
        case inetSettingNoPersonalInfo:      // (RW) UInt32 send no deviceID/zipcode
        case inetSettingUserName:
        case inetSettingGraphicsSel:         // (W) UInt8 (User Graphics selection)
        case inetSettingTransportType:       // (RW) UInt32, INetTransportEnum
        case inetSettingServerBits1:         // (RW) UInt32, bits sent by the server over ctp
        case inetSettingSendRawLocationInfo: // (W) Boolean, make the handheld send its Raw Location information.
        case inetSettingEnableCookies:       // (RW) Boolean
        case inetSettingMaxCookieJarSize:    // (RW) UInt32, maximum cookie jar size in in kilobytes
        case inetSettingLocRawInfo:          // (R) void* Allocated memory buffer - must be free by caller
        case inetSettingProxyNameDefault:    // Default Name for this config
        case inetSettingProxyPortDefault:    // Default Port for this config
        case inetSettingProxyNameEditable:   // Is the proxy name editable?
        case inetSettingProxyPortEditable:   // Is the proxy port editable?
        case inetSettingPalmUserID:          // // The palm.net user id
          break;
      }
      MemHandleUnlock(inetH);
    }
  }

  return err;
}

Err INetLibSettingSet(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSettingEnum*/ setting, void *bufP, UInt16 bufLen) {
  INetLibData *data;
  Err err = inetErrParamsInvalid;

  if (inetH && bufP) {
    if ((data = MemHandleLock(inetH)) != NULL) {
      switch (setting) {
        default:
          break;
      }
      MemHandleUnlock(inetH);
    }
  }

  return err;
}

void INetLibGetEvent(UInt16 libRefnum, MemHandle inetH, INetEventType *eventP, Int32 timeout) {
}

Err INetLibURLOpen(UInt16 libRefnum, MemHandle inetH, UInt8 *urlP, UInt8 *cacheIndexURLP, MemHandle *sockHP, Int32 timeout, UInt16 flags) {
  return inetErrConfigNotFound;
}

Err INetLibCTPSend(UInt16 libRefnum, MemHandle inetH, MemHandle *sockHP, UInt8 *writeP, UInt32 writelen, Int32 timeout, UInt16 ctpCommand) {
  return inetErrConfigNotFound;
}

Err INetLibSockClose(UInt16 libRefnum, MemHandle socketH) {
  return inetErrConfigNotFound;
}

Err INetLibSockRead(UInt16 libRefnum, MemHandle sockH, void *bufP, UInt32 reqBytes, UInt32 *actBytesP, Int32 timeout) {
  return inetErrConfigNotFound;
}

Err INetLibSockWrite(UInt16 libRefnum, MemHandle sockH, void *bufP, UInt32 reqBytes, UInt32 *actBytesP, Int32 timeout) {
  return inetErrConfigNotFound;
}

Err INetLibSockOpen(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSchemEnum*/ scheme, MemHandle *sockHP) {
  return inetErrConfigNotFound;
}

Err INetLibSockStatus(UInt16 libRefnum, MemHandle socketH, UInt16 *statusP, Err* sockErrP, Boolean* inputReadyP, Boolean* outputReadyP) {
  return inetErrConfigNotFound;
}

Err INetLibSockSettingGet(UInt16 libRefnum, MemHandle socketH, UInt16 /*INetSockSettingEnum*/ setting, void *bufP, UInt16 *bufLenP) {
  return inetErrConfigNotFound;
}

Err INetLibSockSettingSet(UInt16 libRefnum, MemHandle socketH, UInt16 /*INetSockSettingEnum*/ setting, void *bufP, UInt16 bufLen) {
  return inetErrConfigNotFound;
}

Err INetLibSockConnect(UInt16 libRefnum, MemHandle sockH, UInt8 *hostnameP, UInt16 port, Int32 timeou) {
  return inetErrConfigNotFound;
}

Err INetLibSockHTTPReqCreate(UInt16 libRefnum, MemHandle sockH, UInt8 *verbP, UInt8 *resNameP, UInt8 *refererP) {
  return inetErrConfigNotFound;
}

Err INetLibSockHTTPAttrSet(UInt16 libRefnum, MemHandle sockH, UInt16 /*inetHTTPAttrEnum*/ attr, UInt16 attrIndex, UInt8 *bufP, UInt16 bufLen, UInt16 flags) {
  return inetErrConfigNotFound;
}

Err INetLibSockHTTPReqSend(UInt16 libRefnum, MemHandle sockH, void *writeP, UInt32 writeLen, Int32 timeout) {
  return inetErrConfigNotFound;
}

Err INetLibSockHTTPAttrGet(UInt16 libRefnum, MemHandle sockH, UInt16 /*inetHTTPAttrEnum*/ attr, UInt16 attrIndex, void *bufP, UInt32 *bufLenP) {
  return inetErrConfigNotFound;
}

Err INetLibURLCrack(UInt16 libRefnum, UInt8 *urlTextP, INetURLType* urlP) {
  return inetErrConfigNotFound;
}

Err INetLibURLsAdd(UInt16 libRefnum, Char *baseURLStr, Char *embeddedURLStr, Char *resultURLStr, UInt16 *resultLenP) {
  return inetErrConfigNotFound;
}

Int16 INetLibURLsCompare(UInt16 libRefnum, Char *URLStr1, Char *URLStr2) {
  return -1;
}

Err INetLibURLGetInfo(UInt16 libRefnum, MemHandle inetH, UInt8 *urlTextP, INetURLInfoType* urlInfoP) {
  return inetErrConfigNotFound;
}

Boolean INetLibWiCmd(UInt16 refNum, UInt16 /*WiCmdEnum*/ cmd, int enableOrX, int y) {
  return false;
}

Boolean INetLibWirelessIndicatorCmd(UInt16 refNum, MemHandle inetH, UInt16 /*WiCmdEnum*/ cmd, int enableOrX, int y) {
  return false;
}

Err INetLibCheckAntennaState(UInt16 refNum) {
  return inetErrConfigNotFound;
}

Err INetLibCacheList(UInt16 libRefnum, MemHandle inetH, UInt8 *cacheIndexURLP, UInt16 *indexP, UInt32 *uidP, INetCacheEntryP cacheP) {
  return inetErrConfigNotFound;
}

Err INetLibCacheGetObject(UInt16 libRefnum, MemHandle clientParamH, UInt8 *urlTextP, UInt32 uniqueID, INetCacheInfoPtr cacheInfoP ) {
  return inetErrConfigNotFound;
}

Err INetLibCachePurge(UInt16 libRefnum, MemHandle clientParamH, UInt8 *urlTextP, UInt32 uniqueID) {
  return inetErrConfigNotFound;
}

Err INetLibCacheGetObjectV2(UInt16 libRefnum, MemHandle clientParamH, UInt8 *urlTextP, UInt32 uniqueID, UInt16 rcIndex, INetCacheInfoPtr cacheInfoP, INetCacheEntryP cacheEntryP ) {
  return inetErrConfigNotFound;
}

Err INetLibIndexedCacheFind(UInt16 libRefnum, DmOpenRef cacheDBRef, UInt8 *dataP, Int16 lookFor, UInt16 *indexP, Int16 order, UInt32 *cacheIdP) {
  return inetErrConfigNotFound;
}

Err INetLibPrepareCacheForHistory(UInt16 libRefnum, MemHandle clientParamH) {
  return inetErrConfigNotFound;
}

Err INetLibConfigMakeActive(UInt16 refNum, MemHandle inetH, UInt16 configIndex) {
  return inetErrConfigNotFound;
}

Err INetLibConfigList(UInt16 refNum, INetConfigNameType nameArray[], UInt16 *arrayEntriesP) {
  return inetErrConfigNotFound;
}

Err INetLibConfigIndexFromName(UInt16 refNum, INetConfigNamePtr nameP, UInt16 *indexP) {
  return inetErrConfigNotFound;
}

Err INetLibConfigDelete(UInt16 refNum, UInt16 index) {
  return inetErrConfigNotFound;
}

Err INetLibConfigSaveAs(UInt16 refNum, MemHandle inetH, INetConfigNamePtr nameP) {
  return inetErrConfigNotFound;
}

Err INetLibConfigRename(UInt16 refNum, UInt16 index, INetConfigNamePtr newNameP) {
  return inetErrConfigNotFound;
}

Err INetLibConfigAliasSet(UInt16 refNum, UInt16 configIndex, UInt16 aliasToIndex) {
  return inetErrConfigNotFound;
}

Err INetLibConfigAliasGet(UInt16 refNum, UInt16 aliasIndex, UInt16 *indexP, Boolean *isAnotherAliasP) {
  return inetErrConfigNotFound;
}

Err INetLibSockFileGetByIndex(UInt16 libRefnum, MemHandle sockH, UInt32 index, MemHandle *handleP, UInt32 *offsetP, UInt32 *lengthP) {
  return inetErrConfigNotFound;
}
