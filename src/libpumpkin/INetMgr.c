#include <PalmOS.h>
#include <INetMgr.h>

#include "bytes.h"
#include "debug.h"

// To launch the Viewer and display a web clipping page, use the
// launch code sysAppLaunchCmdOpenDB. Pass the database ID and
// card number for the .pqa that you want to display.
// To launch the Viewer and display a specific URL, use the launch
// code sysAppLaunchCmdGoToURL. Pass a pointer to the URL string
// as a parameter to this launch code.

typedef struct {
  UInt16 config;
  UInt32 flags;
  DmOpenRef cacheRef;
  UInt32 cacheSize;
  UInt32 maxRespSize;
  UInt32 convAlgorithm;
  UInt32 contentWidth;
  UInt8 enableCookies;
  UInt8 graphicsSel;
} INetLibData;

typedef struct {
  UInt8 *urlP;
  UInt8 *cacheIndexURLP;
  Int32 timeout;
  UInt16 flags;
  INetURLType url;
} INetLibSocketData;

Err INetLibOpen(UInt16 libRefnum, UInt16 config, UInt32 flags, DmOpenRef cacheRef, UInt32 cacheSize, MemHandle *inetHP) {
  INetLibData *data;
  MemHandle h;
  Err err = inetErrTooManyClients;
  // flags: not used by PalmOS

  if (inetHP && config <= 1) {
    if ((h = MemHandleNew(sizeof(INetLibData))) != NULL) {
      if ((data = MemHandleLock(h)) != NULL) {
        data->config = config;
        data->flags = flags;
        data->cacheRef = cacheRef;
        data->cacheSize = cacheSize;
        data->enableCookies = true;
        *inetHP = h;
        MemHandleUnlock(h);
        err = errNone;
      }
    } else {
      MemHandleFree(h);
    }
  } else {
    debug(DEBUG_ERROR, "INetMgr", "config %u not supported or null inetHP %p", config, inetHP);
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

  if (err) {
    debug(DEBUG_ERROR, "INetMgr", "INetLibSettingGetUInt32 invalid parameters buf=%p bufLenP=%p bufLen=%u",
      buf, bufLenP, bufLenP ? *bufLenP : 0);
  }

  return err;
}

static Err INetLibSettingGetBoolean(void *buf, UInt16 *bufLenP, Boolean value) {
  Err err = inetErrSettingSizeInvalid;

  if (buf && bufLenP && *bufLenP == 1) {
    put1(value, (uint8_t *)buf, value);
    err = errNone;
  }

  if (err) {
    debug(DEBUG_ERROR, "INetMgr", "INetLibSettingGetBoolean invalid parameters buf=%p bufLenP=%p bufLen=%u",
      buf, bufLenP, bufLenP ? *bufLenP : 0);
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

  if (err) {
    debug(DEBUG_ERROR, "INetMgr", "INetLibSettingGetPtr invalid parameters");
  }
    
  return err;
}

static Err INetLibSettingSetUInt32(void *buf, UInt16 bufLen, UInt32 *valueP) {
  Err err = inetErrSettingSizeInvalid;

  if (buf && bufLen == sizeof(UInt32) && valueP) {
    if (pumpkin_is_m68k()) {
      get4b(valueP, (uint8_t *)buf, 0);
    } else {
      get4l(valueP, (uint8_t *)buf, 0);
    }
    err = errNone;
  }

  if (err) {
    debug(DEBUG_ERROR, "INetMgr", "INetLibSettingSetUInt32 invalid parameters");
  }

  return err;
}

static Err INetLibSettingSetUInt8(void *buf, UInt16 bufLen, UInt8 *valueP) {
  Err err = inetErrSettingSizeInvalid;

  if (buf && bufLen == 1 && valueP) {
    *valueP = *((uint8_t *)buf);
    err = errNone;
  }

  if (err) {
    debug(DEBUG_ERROR, "INetMgr", "INetLibSettingSetBoolean invalid parameters");
  }

  return err;
}

Err INetLibSettingGet(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSettingEnum */ setting, void *bufP, UInt16 *bufLenP) {
  INetLibData *data;
  Err err = inetErrParamsInvalid;

  if (inetH && bufP && bufLenP) {
    if ((data = MemHandleLock(inetH)) != NULL) {
      switch (setting) {
        case inetSettingProxyType:           // (RW) UInt32, INetProxyEnum (0)
          err = INetLibSettingGetUInt32(bufP, bufLenP, inetProxyNone);
          break;
        case inetSettingProxyName:           // (RW) Char[], name of proxy (1)
          err = INetLibSettingGetUInt32(bufP, bufLenP, 0);
          break;
        case inetSettingProxyPort:           // (RW) UInt32,  TCP port # of proxy (2)
          err = INetLibSettingGetUInt32(bufP, bufLenP, 0);
          break;
        case inetSettingProxySocketType:     // (RW) UInt32, which type of socket to use netSocketTypeXXX (3)
          break;
        case inetSettingCacheSize:           // (RW) UInt32, max size of cache (4)
          err = INetLibSettingGetUInt32(bufP, bufLenP, data->cacheSize);
          break;
        case inetSettingCacheRef:            // (R) DmOpenRef, ref of cache DB (5)
          err = INetLibSettingGetPtr(bufP, bufLenP, data->cacheRef);
          break;
        case inetSettingNetLibConfig:        // (RW) UInt32, Which NetLib config to use. (6)
          err = INetLibSettingGetUInt32(bufP, bufLenP, data->config);
          break;
        case inetSettingRadioID:             // (R)  UInt32[2], the 64-bit radio ID (7)
        case inetSettingBaseStationID:       // (R)  UInt32, the radio base station ID (8)
        case inetSettingMaxRspSize:          // (W) UInt32 (in bytes) (9)
        case inetSettingConvAlgorithm:       // (W) UInt32 (CTPConvEnum) (10)
        case inetSettingContentWidth:        // (W) UInt32 (in pixels) (11)
        case inetSettingContentVersion:      // (W) UInt32 Content version (encoder version) (12)
        case inetSettingNoPersonalInfo:      // (RW) UInt32 send no deviceID/zipcode (13)
        case inetSettingUserName:            // (14)
        case inetSettingGraphicsSel:         // (W) UInt8 (User Graphics selection) (15)
          break;
        case inetSettingTransportType:       // (RW) UInt32, INetTransportEnum (16)
          err = INetLibSettingGetUInt32(bufP, bufLenP, inetTransportPPP);
          break;
        case inetSettingServerBits1:         // (RW) UInt32, bits sent by the server over ctp (17)
          err = INetLibSettingGetUInt32(bufP, bufLenP, 0);
          break;
        case inetSettingSendRawLocationInfo: // (W) Boolean, make the handheld send its Raw Location information. (18)
          break;
        case inetSettingEnableCookies:       // (RW) Boolean (19)
          err = INetLibSettingGetBoolean(bufP, bufLenP, true);
          break;
        case inetSettingMaxCookieJarSize:    // (RW) UInt32, maximum cookie jar size in in kilobytes (20)
        case inetSettingLocRawInfo:          // (R) void* Allocated memory buffer - must be free by caller (21)
        case inetSettingProxyNameDefault:    // Default Name for this config (22)
        case inetSettingProxyPortDefault:    // Default Port for this config (23)
        case inetSettingProxyNameEditable:   // Is the proxy name editable? (24)
        case inetSettingProxyPortEditable:   // Is the proxy port editable? (25)
        case inetSettingPalmUserID:          // // The palm.net user id (26)
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
        case inetSettingProxyType:           // (RW) UInt32, INetProxyEnum
        case inetSettingProxyName:           // (RW) Char[], name of proxy
        case inetSettingProxyPort:           // (RW) UInt32,  TCP port # of proxy
        case inetSettingProxySocketType:     // (RW) UInt32, which type of socket to use netSocketTypeXXX
          break;
        case inetSettingCacheSize:           // (RW) UInt32, max size of cache
          if ((err = INetLibSettingSetUInt32(bufP, bufLen, &data->cacheSize)) == errNone) {
            debug(DEBUG_INFO, "INetMgr", "INetLibSettingSet cacheSize=%u", data->cacheSize);
          }
          break;
        case inetSettingCacheRef:            // (R) DmOpenRef, ref of cache DB
        case inetSettingNetLibConfig:        // (RW) UInt32, Which NetLib config to use.
        case inetSettingRadioID:             // (R)  UInt32[2], the 64-bit radio ID
        case inetSettingBaseStationID:       // (R)  UInt32, the radio base station ID
          break;
        case inetSettingMaxRspSize:          // (W) UInt32 (in bytes)
          if ((err = INetLibSettingSetUInt32(bufP, bufLen, &data->maxRespSize)) == errNone) {
            debug(DEBUG_INFO, "INetMgr", "INetLibSettingSet maxRespSize=%u", data->maxRespSize);
          }
          break;
        case inetSettingConvAlgorithm:       // (W) UInt32 (CTPConvEnum)
          if ((err = INetLibSettingSetUInt32(bufP, bufLen, &data->convAlgorithm)) == errNone) {
            debug(DEBUG_INFO, "INetMgr", "INetLibSettingSet convAlgorithm=%u", data->convAlgorithm);
          }
          break;
        case inetSettingContentWidth:        // (W) UInt32 (in pixels)
          if ((err = INetLibSettingSetUInt32(bufP, bufLen, &data->contentWidth)) == errNone) {
            debug(DEBUG_INFO, "INetMgr", "INetLibSettingSet contentWidth=%u", data->contentWidth);
          }
          break;
        case inetSettingContentVersion:      // (W) UInt32 Content version (encoder version)
        case inetSettingNoPersonalInfo:      // (RW) UInt32 send no deviceID/zipcode
        case inetSettingUserName:
          break;
        case inetSettingGraphicsSel:         // (W) UInt8 (User Graphics selection)
          if ((err = INetLibSettingSetUInt8(bufP, bufLen, &data->graphicsSel)) == errNone) {
            debug(DEBUG_INFO, "INetMgr", "INetLibSettingSet graphicsSel=%u", data->graphicsSel);
          }
          break;
        case inetSettingTransportType:       // (RW) UInt32, INetTransportEnum
        case inetSettingServerBits1:         // (RW) UInt32, bits sent by the server over ctp
        case inetSettingSendRawLocationInfo: // (W) Boolean, make the handheld send its Raw Location information.
          break;
        case inetSettingEnableCookies:       // (RW) Boolean
          if ((err = INetLibSettingSetUInt8(bufP, bufLen, &data->enableCookies)) == errNone) {
            debug(DEBUG_INFO, "INetMgr", "INetLibSettingSet enableCookies=%u", data->enableCookies);
          }
          break;
        case inetSettingMaxCookieJarSize:    // (RW) UInt32, maximum cookie jar size in in kilobytes
        case inetSettingLocRawInfo:          // (R) void* Allocated memory buffer - must be free by caller
        case inetSettingProxyNameDefault:    // Default Name for this config
        case inetSettingProxyPortDefault:    // Default Port for this config
        case inetSettingProxyNameEditable:   // Is the proxy name editable?
        case inetSettingProxyPortEditable:   // Is the proxy port editable?
        case inetSettingPalmUserID:          // // The palm.net user id
        default:
          break;
      }
      MemHandleUnlock(inetH);
    }
  }

  return err;
}

void INetLibGetEvent(UInt16 libRefnum, MemHandle inetH, INetEventType *eventP, Int32 timeout) {
  if (inetH == NULL) {
    debug(DEBUG_INFO, "INetMgr", "INetLibGetEvent inetH is null, calling EvtGetEvent");
    EvtGetEvent((EventType *)eventP, timeout);
  } else {
    debug(DEBUG_INFO, "INetMgr", "INetLibGetEvent inetH is not null");
    EvtGetEvent((EventType *)eventP, timeout);
    //eventP->penDown = false;
    //eventP->screenX = 0;
    //eventP->screenY = 0;
    // inetSockReadyEvent
    // inetSockStatusChangeEvent
  }
}

Err INetLibURLOpen(UInt16 libRefnum, MemHandle inetH, UInt8 *urlP, UInt8 *cacheIndexURLP, MemHandle *sockHP, Int32 timeout, UInt16 flags) {
  MemHandle h;
  INetLibSocketData *data;
  Err err = inetErrParamsInvalid;

  if (inetH && urlP && sockHP) {
    if ((h = MemHandleNew(sizeof(INetLibSocketData))) != NULL) {
      if ((data = MemHandleLock(h)) != NULL) {
        data->urlP = (UInt8 *)StrDup((char *)urlP);
        data->cacheIndexURLP = cacheIndexURLP ? (UInt8 *)StrDup((char *)cacheIndexURLP) : NULL;
        data->timeout = timeout;
        data->flags = flags;
        INetLibURLCrack(libRefnum, urlP, &data->url);
        MemHandleUnlock(h);
        *sockHP = h;
        err = errNone;
      }
    } else {
      MemHandleFree(h);
    }
  }

  return err;
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
  INetLibSocketData *data;
  Err err = inetErrParamsInvalid;

  if (socketH && bufP && bufLenP) {
    if ((data = MemHandleLock(socketH)) != NULL) {
      switch (setting) {
        case inetSockSettingScheme:             // (R)  UInt32 INetSchemeEnum (0)
          err = INetLibSettingGetUInt32(bufP, bufLenP, data->url.schemeEnum);
          break;
        case inetSockSettingSockContext:        // (RW) UInt32 (1)
        case inetSockSettingCompressionType:    // (R)  Char[] (2)
        case inetSockSettingCompressionTypeID:  // (R)  UInt32 (INetCompressionTypeEnum) (3)
        case inetSockSettingContentType:        // (R)  Char[] (4)
        case inetSockSettingContentTypeID:      // (R)  UInt32 (INetContentTypeEnum) (5)
        case inetSockSettingData:               // (R)  UInt32 pointer to data (6)
        case inetSockSettingDataHandle:         // (R)  UInt32 handle to data (7)
        case inetSockSettingDataOffset:         // (R)  UInt32 offset to data from handle (8)
        case inetSockSettingTitle:              // (RW) Char[] (9)
        case inetSockSettingURL:                // (R)  Char[] (10)
        case inetSockSettingIndexURL:           // (RW) Char[] (11)
        case inetSockSettingFlags:              // (W)  UInt16 one or more of inetOpenURLFlagXXX flags (12)
        case inetSockSettingReadTimeout:        // (RW) UInt32 Read timeout in ticks (13)
        case inetSockSettingContentVersion:     // (R)  UInt32 version number for content (14)
          break;
        default:
          break;
      }
      MemHandleUnlock(socketH);
    }
  }

  return err;
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

#define checkScheme(p, scheme, type) \
   do { \
     if (urlP->schemeEnum == inetSchemeUnknown) { \
       UInt32 len = StrLen(scheme); \
       if (!StrNCompare((char *)p, scheme, len)) { \
         urlP->schemeEnum = type; \
         if (urlP->schemeP) { \
           MemMove(urlP->schemeP, p, urlP->schemeLen < len ? urlP->schemeLen : len); \
         } else { \
           urlP->schemeP = p; \
         }  \
         urlP->schemeLen = len; \
         p += urlP->schemeLen; \
       } \
     } \
   } while (0)

Err INetLibURLCrack(UInt16 libRefnum, UInt8 *urlTextP, INetURLType* urlP) {
   UInt32 len;
   UInt8 *p, *s;
   Err err = inetErrParamsInvalid;

   if (urlTextP && urlP) {
     p = urlTextP;
     urlP->version = 0;
     urlP->schemeEnum = inetSchemeUnknown;

     checkScheme(p, "http:",     inetSchemeHTTP);
     checkScheme(p, "https:",    inetSchemeHTTPS);
     checkScheme(p, "ftp:",      inetSchemeFTP);
     checkScheme(p, "gopher:",   inetSchemeGopher);
     checkScheme(p, "file:",     inetSchemeFile);
     checkScheme(p, "news:",     inetSchemeNews);
     checkScheme(p, "mailto:",   inetSchemeMailTo);
     checkScheme(p, "palm:",     inetSchemePalm);
     checkScheme(p, "palmcall:", inetSchemePalmCall);
     checkScheme(p, "mac:",      inetSchemeMac);

     if (urlP->schemeEnum != inetSchemeUnknown) {
       if ((s = (UInt8 *)StrChr((char *)p, '/')) != NULL) {
         len = s - p;
       } else {
         len = StrLen((char *)p);
       }

       if (urlP->hostnameP) {
         if (len) MemMove(urlP->hostnameP, p, urlP->hostnameLen < len ? urlP->hostnameLen : len);
       } else {
         urlP->hostnameP = p;
       }
       urlP->hostnameLen = len;

       p += len;
       len = StrLen((char *)p);
       if (urlP->pathP) {
         if (len) MemMove(urlP->pathP, p, urlP->pathLen < len ? urlP->pathLen : len);
       } else {
         urlP->pathP = p;
       }
       urlP->pathLen = len;

       urlP->port = 80;
       urlP->usernameLen = 0;
       urlP->passwordLen = 0;
       urlP->paramLen = 0;
       urlP->queryLen = 0;
       urlP->fragLen = 0;
       err = errNone;
     }
   }

  return err;
}

// Concatenates two URLs, resulting in one absolute URL.
// Used to append a URL fragment to a base URL, resulting in an
// absolute URL string that can be passed to INetLibURLOpen or
// other functions. This routine ensures that the resulting string
// conforms to the URL format.

Err INetLibURLsAdd(UInt16 libRefnum, Char *baseURLStr, Char *embeddedURLStr, Char *resultURLStr, UInt16 *resultLenP) {
  Err err = inetErrParamsInvalid;

  if (embeddedURLStr && resultLenP) {
    if (resultURLStr) {
      resultURLStr[0] = 0;
      if (baseURLStr) StrNCopy(resultURLStr, baseURLStr, *resultLenP);
      StrNCat(resultURLStr, embeddedURLStr, *resultLenP);
      *resultLenP = StrLen(resultURLStr) + 1;
      err = errNone;
    } else {
      *resultLenP = 0;
      if (baseURLStr) *resultLenP += StrLen(baseURLStr);
      *resultLenP += StrLen(embeddedURLStr) + 1;
      err = inetErrURLBufTooSmall;
    }
  }

  return err;
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
  switch (cmd) {
    case wiCmdInit:
    case wiCmdClear:
    case wiCmdSetEnabled:
    case wiCmdDraw:
    case wiCmdEnabled:
    case wiCmdSetLocation:
    case wiCmdErase:
      break;
  }

  return true;
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
  Err err = inetErrConfigNotFound;

  if (configIndex <= 0) {
    err = errNone;
  }

  return err;
}

Err INetLibConfigList(UInt16 refNum, INetConfigNameType nameArray[], UInt16 *arrayEntriesP) {
  Err err = inetErrConfigNotFound;

  if (nameArray && arrayEntriesP && *arrayEntriesP >= 2) {
    StrNCopy(nameArray[0].name, inetCfgNameDefault, inetConfigNameSize);
    StrNCopy(nameArray[1].name, inetCfgNameCTPDefault, inetConfigNameSize);
    *arrayEntriesP = 2;
    err = errNone;
  }

  return err;
}

Err INetLibConfigIndexFromName(UInt16 refNum, INetConfigNamePtr nameP, UInt16 *indexP) {
  Err err = inetErrConfigNotFound;

  if (nameP && indexP) {
    if (StrCompare(nameP->name, inetCfgNameDefault) == 0) {
      *indexP = 0;
      err = errNone;
    } else if (StrCompare(nameP->name, inetCfgNameCTPDefault) == 0) {
      *indexP = 1;
      err = errNone;
    }
  }

  return err;
}

Err INetLibConfigDelete(UInt16 refNum, UInt16 index) {
  return inetErrConfigCantDelete;
}

Err INetLibConfigSaveAs(UInt16 refNum, MemHandle inetH, INetConfigNamePtr nameP) {
  return inetErrConfigTooMany;
}

Err INetLibConfigRename(UInt16 refNum, UInt16 index, INetConfigNamePtr newNameP) {
  return inetErrConfigCantDelete;
}

Err INetLibConfigAliasSet(UInt16 refNum, UInt16 configIndex, UInt16 aliasToIndex) {
  return inetErrParamsInvalid;
}

Err INetLibConfigAliasGet(UInt16 refNum, UInt16 aliasIndex, UInt16 *indexP, Boolean *isAnotherAliasP) {
  return inetErrParamsInvalid;
}

Err INetLibSockFileGetByIndex(UInt16 libRefnum, MemHandle sockH, UInt32 index, MemHandle *handleP, UInt32 *offsetP, UInt32 *lengthP) {
  return inetErrConfigNotFound;
}
