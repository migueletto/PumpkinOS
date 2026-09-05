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

#define MAX_SOCKETS 16

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
  MemHandle sockets[MAX_SOCKETS];
  UInt32 numSockets;
} INetLibData;

typedef struct {
  MemHandle inetH;
  UInt8 *indexUrlP;
  UInt8 *urlP;
  UInt8 *cacheIndexURLP;
  Int32 timeout;
  UInt16 flags;
  INetURLType url;
  UInt32 dataOffset;
  MemHandle dataH;
  DmOpenRef dbRef;
  UInt16 index;
  UInt8 contentType;
  UInt8 compressionType;
  UInt8 contentFlags;
  UInt32 uncompDataSize;
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
  INetLibData *data;
  UInt32 numSockets;
  Err err = inetErrParamsInvalid;

  if (inetH) {
    numSockets = 0;
    if ((data = MemHandleLock(inetH)) != NULL) {
      numSockets = data->numSockets;
      MemHandleUnlock(inetH);
    }

    if (numSockets == 0) {
      MemHandleFree(inetH);
      err = errNone;
    } else {
      debug(DEBUG_ERROR, "INetMgr", "INetLibClose there are still %u open sockets", numSockets);
    }
  }

  return err;
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

static Err INetLibSettingGetStr(void *buf, UInt16 *bufLenP, char *value) {
  UInt32 len;
  Err err = inetErrSettingSizeInvalid;

  if (bufLenP && value) {
    len = StrLen(value) + 1;
    if (buf) {
      if (len > *bufLenP) {
        len = *bufLenP;
      }
      sys_memcpy(buf, value, len);
    }
    *bufLenP = len;
    err = errNone;
  }

  if (err) {
    debug(DEBUG_ERROR, "INetMgr", "INetLibSettingGetStr invalid parameters");
  }

  return err;
}

static Err INetLibSettingGetPtr(void *buf, UInt16 *bufLenP, void *value) {
  UInt8 *ram;
  UInt32 d;
  Err err = inetErrSettingSizeInvalid;

  if (buf && bufLenP) {
    if (pumpkin_is_m68k()) {
      if (*bufLenP == sizeof(UInt32)) {
        ram = pumpkin_heap_base();
        d = (uint8_t *)value - ram;
        put4b(d, (uint8_t *)buf, 0);
        debug(DEBUG_INFO, "INetMgr", "INetLibSettingGetPtr 0x%08X", d);
        err = errNone;
      }
    } else { 
      if (*bufLenP == sizeof(void *)) {
        sys_memcpy(buf, &value, sizeof(void *));
        err = errNone;
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
          break;
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

#if 0
status:
  inetStatusNew                 // just opened
  inetStatusResolvingName       // looking up host address
  inetStatusNameResolved        // found host address
  inetStatusConnecting          // connecting to host
  inetStatusConnected           // connected to host
  inetStatusSendingRequest      // sending request
  inetStatusWaitingForResponse  // waiting for response
  inetStatusReceivingResponse   // receiving response
  inetStatusResponseReceived    // response received
  inetStatusClosingConnection   // closing connection
  inetStatusClosed              // closed
  inetStatusAcquiringNetwork    // network temporarily unreachable; socket on hold
#endif

static void addSockStatusChangedEvent(MemHandle sock, UInt16 status) {
  INetEventType event;

  MemSet(&event, sizeof(INetEventType), 0);
  event.eType = inetSockStatusChangeEvent;
  event.data.inetSockStatusChange.sockH = sock;
  event.data.inetSockStatusChange.context = 0; // not used
  event.data.inetSockStatusChange.status = status;
  event.data.inetSockStatusChange.sockErr = 0;

  EvtAddEventToQueue((EventType *)&event);
}

void INetLibGetEvent(UInt16 libRefnum, MemHandle inetH, INetEventType *eventP, Int32 timeout) {
  INetLibData *data;
  INetLibSocketData *sockData;
  UInt32 i;

  if (inetH == NULL) {
    debug(DEBUG_INFO, "INetMgr", "INetLibGetEvent inetH is null, calling EvtGetEvent directly");
    EvtGetEvent((EventType *)eventP, timeout);

  } else {
    debug(DEBUG_INFO, "INetMgr", "INetLibGetEvent inetH is not null");
    eventP->eType = nilEvent;
    eventP->penDown = false;
    eventP->screenX = 0;
    eventP->screenY = 0;

    if ((data = MemHandleLock(inetH)) != NULL) {
      if (data->numSockets > 0) {
        for (i = 0; i < MAX_SOCKETS; i++) {
          if (data->sockets[i]) {
            if ((sockData = MemHandleLock(data->sockets[i])) != NULL) {
              switch (sockData->url.schemeEnum) {
                case inetSchemeFile:
                  if (sockData->dataH) {
                    if (sockData->dataOffset < MemHandleSize(sockData->dataH)) {
                      eventP->eType = inetSockReadyEvent;
                      eventP->data.inetSockReady.sockH = data->sockets[i];
                      eventP->data.inetSockReady.context = 0; // not used
                      eventP->data.inetSockReady.inputReady = true;
                      eventP->data.inetSockReady.outputReady = false;
                    }
                  }
                  break;
                default:
                  debug(DEBUG_ERROR, "INetMgr", "INetLibGetEvent invalid scheme %u", sockData->url.schemeEnum);
                  break;
              }
              MemHandleUnlock(data->sockets[i]);
            }
            break;
          }
        }
      }

      MemHandleUnlock(inetH);
    }

    if (eventP->eType == nilEvent) {
      debug(DEBUG_INFO, "INetMgr", "INetLibGetEvent no socket event, calling EvtGetEvent");
      EvtGetEvent((EventType *)eventP, timeout);
    }
  }
}

Err INetLibURLOpen(UInt16 libRefnum, MemHandle inetH, UInt8 *urlP, UInt8 *cacheIndexURLP, MemHandle *sockHP, Int32 timeout, UInt16 flags) {
  MemHandle h, sockHandle = NULL;
  INetURLType url;
  INetLibData *data;
  INetLibSocketData *sockData;
  LocalID dbID;
  UInt32 type, creator, oldLength, newLength, i;
  uint32_t urlOffset, dataOffset;
  uint16_t urlLength, dataLength;
  uint8_t *rec, *p;
  char *path, *s;
  Err err = inetErrParamsInvalid;

  if (inetH && urlP && sockHP) {
    *sockHP = NULL;
    if ((data = MemHandleLock(inetH)) != NULL) {
      if (data->numSockets < MAX_SOCKETS) {
        MemSet(&url, sizeof(INetURLType), 0);
        if (INetLibURLCrack(libRefnum, urlP, &url) == errNone) {
          if ((sockHandle = MemHandleNew(sizeof(INetLibSocketData))) != NULL) {
            if ((sockData = MemHandleLock(sockHandle)) != NULL) {
              sockData->inetH = inetH;
              sockData->indexUrlP = (UInt8 *)StrDup((char *)urlP);
              sockData->urlP = (UInt8 *)StrDup((char *)urlP);
              sockData->cacheIndexURLP = cacheIndexURLP ? (UInt8 *)StrDup((char *)cacheIndexURLP) : NULL;
              sockData->timeout = timeout;
              sockData->flags = flags;
              MemMove(&sockData->url, &url, sizeof(INetURLType));
              for (i = 0; i < MAX_SOCKETS; i++) {
                if (data->sockets[i] == NULL) {
                  data->sockets[i] = sockHandle;
                  debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen socket %p slot %u scheme %u", sockHandle, i, url.schemeEnum);

                  switch (url.schemeEnum) {
                     case inetSchemeFile:
                       if ((path = MemPtrNew(url.pathLen + 1)) != NULL) {
                         StrNCopy(path, (char *)url.pathP + 1, url.pathLen - 1);
                         if ((dbID = DmFindDatabase(0, path)) != 0) {
                           if (DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator) == errNone) {
                             if (type == sysFileTpqa && creator == sysFileCClipper) {
                               if ((sockData->dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
                                 debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen file \"%s\" dbRef %p", path, sockData->dbRef);

                                 if ((h = DmGetRecord(sockData->dbRef, sockData->index)) != NULL) {
                                   if ((rec = MemHandleLock(h)) != NULL) {
                                     get4b(&urlOffset, rec, 0);
                                     get2b(&urlLength, rec, 4);
                                     get4b(&dataOffset, rec, 6);
                                     get2b(&dataLength, rec, 10);
                                     sockData->contentType = rec[12];
                                     sockData->compressionType = rec[13];
                                     get4b(&sockData->uncompDataSize, rec, 14);
                                     sockData->contentFlags = rec[18];
                                     debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen urlOffset=%u, urlLength=%u, dataOffset=%u, dataLength=%u",
                                       urlOffset, urlLength, dataOffset, dataLength);
                                     debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen contentType=%u, compressionType=%u, uncompDataSize=%u, flags=0x%02X",
                                       sockData->contentType, sockData->compressionType, sockData->uncompDataSize, sockData->contentFlags);
                                     if (urlLength > 0) {
                                       debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen url=\"%.*s\"", urlLength, (char *)rec + urlOffset);
                                       oldLength = StrLen((char *)sockData->urlP);
                                       newLength = oldLength + 1 + urlLength + 1;
                                       if ((s = MemPtrNew(newLength)) != NULL) {
                                         StrCopy(s, (char *)sockData->urlP);
                                         s[oldLength] = '/';
                                         StrNCopy(s + oldLength + 1, (char *)rec + urlOffset, urlLength);
                                         s[newLength - 1] = 0;
                                         MemPtrFree(sockData->urlP);
                                         sockData->urlP = (UInt8 *)s;
                                       }
                                       debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen complete url=\"%s\"", (char *)sockData->urlP);
                                     }
                                     if (dataLength > 0) {
                                       debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen data (%u bytes):", dataLength);
                                       debug_bytes(DEBUG_INFO, "INetMgr", rec + dataOffset, dataLength);
                                     }
                                     if ((sockData->dataH = MemHandleNew(dataLength)) != NULL) {
                                       if ((p = MemHandleLock(sockData->dataH)) != NULL) {
                                         MemMove(p, rec + dataOffset, dataLength);
                                         MemHandleUnlock(sockData->dataH);
                                         debug(DEBUG_INFO, "INetMgr", "INetLibURLOpen read %u bytes from record %d into data handle", dataLength, sockData->index);
                                       }
                                     }
                                     MemHandleUnlock(h);
                                   }
                                   DmReleaseRecord(sockData->dbRef, sockData->index, false);
                                 }

                                 *sockHP = sockHandle;
                                 data->numSockets++;
                                 err = errNone;
                               }
                             }
                           }
                         }
                         MemPtrFree(path);
                       }
                       break;
                     default:
                       debug(DEBUG_ERROR, "INetMgr", "INetLibURLOpen invalid scheme %u", url.schemeEnum);
                       break;
                  }
                  break;
                }
              }
              MemHandleUnlock(sockHandle);
            } else {
              MemHandleFree(sockHandle);
            }
          }
        }
        MemHandleUnlock(inetH);
      }
    }
  }

  if (err) {
    if (sockHandle) {
      if ((sockData = MemHandleLock(sockHandle)) != NULL) {
        if (sockData->indexUrlP) MemPtrFree(sockData->indexUrlP);
        if (sockData->urlP) MemPtrFree(sockData->urlP);
        if (sockData->cacheIndexURLP) MemPtrFree(sockData->cacheIndexURLP);
        if (sockData->dataH) MemHandleFree(sockData->dataH);
        if (sockData->dbRef) DmCloseDatabase(sockData->dbRef);
        MemHandleUnlock(sockHandle);
      }
      MemHandleFree(sockHandle);
    }
  } else {
    addSockStatusChangedEvent(*sockHP, inetStatusNew);
  }

  return err;
}

Err INetLibSockClose(UInt16 libRefnum, MemHandle sockHandle) {
  MemHandle inetH;
  INetLibSocketData *sockData;
  INetLibData *data;
  UInt32 i;
  Err err = inetErrParamsInvalid;

  if (sockHandle) {
    if ((sockData = MemHandleLock(sockHandle)) != NULL) {
      inetH = sockData->inetH;

      if ((data = MemHandleLock(inetH)) != NULL) {
        if (data->numSockets > 0) {
          for (i = 0; i < MAX_SOCKETS; i++) {
            if (data->sockets[i] == sockHandle) {
              debug(DEBUG_INFO, "INetMgr", "INetLibSockClose socket %p slot %u", sockHandle, i);
              switch (sockData->url.schemeEnum) {
                case inetSchemeFile:
                  break;
                default:
                  debug(DEBUG_ERROR, "INetMgr", "INetLibSockClose invalid scheme %u", sockData->url.schemeEnum);
                  break;
              }
              if (sockData->indexUrlP) MemPtrFree(sockData->indexUrlP);
              if (sockData->urlP) MemPtrFree(sockData->urlP);
              if (sockData->cacheIndexURLP) MemPtrFree(sockData->cacheIndexURLP);
              if (sockData->dataH) MemHandleFree(sockData->dataH);
              if (sockData->dbRef) DmCloseDatabase(sockData->dbRef);
              MemHandleUnlock(sockHandle);
              MemHandleFree(sockHandle);
              data->sockets[i] = NULL;
              data->numSockets--;
              err = errNone;
              break;
            }
          }
        } else {
          MemHandleUnlock(sockHandle);
        }
        MemHandleUnlock(inetH);
      } else {
        MemHandleUnlock(sockHandle);
      }
    }
  }

  return err;
}

Err INetLibCTPSend(UInt16 libRefnum, MemHandle inetH, MemHandle *sockHP, UInt8 *writeP, UInt32 writelen, Int32 timeout, UInt16 ctpCommand) {
  return inetErrConfigNotFound;
}

Err INetLibSockRead(UInt16 libRefnum, MemHandle sockHandle, void *bufP, UInt32 reqBytes, UInt32 *actBytesP, Int32 timeout) {
  INetLibSocketData *sockData;
  UInt32 len;
  UInt8 *p;
  Err err = inetErrParamsInvalid;

  if (sockHandle && bufP && actBytesP) {
    if ((sockData = MemHandleLock(sockHandle)) != NULL) {
      switch (sockData->url.schemeEnum) {
        case inetSchemeFile:
          if (sockData->dbRef) {
            len = MemHandleSize(sockData->dataH);
            debug(DEBUG_INFO, "INetMgr", "INetLibSockRead dbRef %p requested %u bytes from %u, offset is %u", sockData->dbRef, reqBytes, len, sockData->dataOffset);
            if (reqBytes > 0) {
              if (sockData->dataOffset + reqBytes > len) {
                reqBytes = len - sockData->dataOffset;
              }
              if (reqBytes > 0) {
                if ((p = MemHandleLock(sockData->dataH)) != NULL) {
                  MemMove(bufP, p, reqBytes);
                  MemHandleUnlock(sockData->dataH);
                  sockData->dataOffset += reqBytes;
                  debug(DEBUG_INFO, "INetMgr", "INetLibSockRead read %u bytes from %u, offset is now %u", reqBytes, len, sockData->dataOffset);
                }
              }
              *actBytesP = reqBytes;
              err = errNone;
            } else {
              *actBytesP = reqBytes;
              err = errNone;
            }
          }
          break;
        default:
          debug(DEBUG_ERROR, "INetMgr", "INetLibSockRead invalid scheme %u", sockData->url.schemeEnum);
          break;
      }
      MemHandleUnlock(sockHandle);
    }
  }

  return err;
}

Err INetLibSockWrite(UInt16 libRefnum, MemHandle sockH, void *bufP, UInt32 reqBytes, UInt32 *actBytesP, Int32 timeout) {
  return inetErrParamsInvalid;
}

Err INetLibSockOpen(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSchemEnum*/ scheme, MemHandle *sockHP) {
  return inetErrConfigNotFound;
}

Err INetLibSockStatus(UInt16 libRefnum, MemHandle socketH, UInt16 *statusP, Err* sockErrP, Boolean* inputReadyP, Boolean* outputReadyP) {
  return inetErrConfigNotFound;
}

Err INetLibSockSettingGet(UInt16 libRefnum, MemHandle socketH, UInt16 /*INetSockSettingEnum*/ setting, void *bufP, UInt16 *bufLenP) {
  INetLibSocketData *sockData;
  Err err = inetErrParamsInvalid;

  if (socketH && bufLenP) {
    if ((sockData = MemHandleLock(socketH)) != NULL) {
      switch (setting) {
        case inetSockSettingScheme:             // (R)  UInt32 INetSchemeEnum (0)
          err = INetLibSettingGetUInt32(bufP, bufLenP, sockData->url.schemeEnum);
          break;
        case inetSockSettingSockContext:        // (RW) UInt32 (1)
          break;
        case inetSockSettingCompressionType:    // (R)  Char[] (2)
          break;
        case inetSockSettingCompressionTypeID:  // (R)  UInt32 (INetCompressionTypeEnum) (3)
          err = INetLibSettingGetUInt32(bufP, bufLenP, sockData->compressionType);
          break;
        case inetSockSettingContentType:        // (R)  Char[] (4)
          break;
        case inetSockSettingContentTypeID:      // (R)  UInt32 (INetContentTypeEnum) (5)
          err = INetLibSettingGetUInt32(bufP, bufLenP, sockData->contentType);
          break;
        case inetSockSettingData:               // (R)  UInt32 pointer to data (6)
          break;
        case inetSockSettingDataHandle:         // (R)  UInt32 handle to data (7)
          err = INetLibSettingGetPtr(bufP, bufLenP, sockData->dataH);
          break;
        case inetSockSettingDataOffset:         // (R)  UInt32 offset to data from handle (8)
          err = INetLibSettingGetUInt32(bufP, bufLenP, sockData->dataOffset);
          break;
        case inetSockSettingTitle:              // (RW) Char[] (9)
          break;
        case inetSockSettingURL:                // (R)  Char[] (10)
          err = INetLibSettingGetStr(bufP, bufLenP, (char *)sockData->urlP);
          break;
        case inetSockSettingIndexURL:           // (RW) Char[] (11)
          err = INetLibSettingGetStr(bufP, bufLenP, (char *)sockData->indexUrlP);
          break;
        case inetSockSettingFlags:              // (W)  UInt16 one or more of inetOpenURLFlagXXX flags (12)
          break;
        case inetSockSettingReadTimeout:        // (RW) UInt32 Read timeout in ticks (13)
          break;
        case inetSockSettingContentVersion:     // (R)  UInt32 version number for content (14)
          err = INetLibSettingGetUInt32(bufP, bufLenP, 0x8001); // XXX returninch value expected by Clipper
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
  INetLibSocketData *sockData;
  Err err = inetErrParamsInvalid;

  if (socketH) {
    if ((sockData = MemHandleLock(socketH)) != NULL) {
      switch (setting) {
        case inetSockSettingTitle:              // (RW) Char[] (9)
          err = errNone;
          break;
        default:
          break;
      }
      MemHandleUnlock(socketH);
    }
  }

  return err;
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

Err INetLibSockHTTPAttrGet(UInt16 libRefnum, MemHandle socketH, UInt16 /*inetHTTPAttrEnum*/ attr, UInt16 attrIndex, void *bufP, UInt32 *bufLenP32) {
  INetLibSocketData *sockData;
  UInt16 bufLen;
  UInt16 *bufLenP;
  Err err = inetErrParamsInvalid;
  // attrIndex is not used

  if (socketH && bufP && bufLenP32) {
    if ((sockData = MemHandleLock(socketH)) != NULL) {
      bufLen = *bufLenP32;
      bufLenP = &bufLen;

      switch (attr) {
        // local error trying to communicate with server, if any
        case inetHTTPAttrCommErr:                   // ( 0) (R) UInt32, read-only

        // object attributes, defined at creation
        case inetHTTPAttrEntityURL:                 // ( 1) (-) Char[], which resource was requested

        // Request only attributes:

        case inetHTTPAttrReqAuthorization:          // ( 2) (-) Char[]           
        case inetHTTPAttrReqFrom:                   // ( 3) (-) Char[]
        case inetHTTPAttrReqIfModifiedSince:        // ( 4) (-) UInt32
        case inetHTTPAttrReqReferer:                // ( 5) (-) Char[]
        case inetHTTPAttrWhichPart:                 // ( 6) (W) UInt32 (0 -> N)
        case inetHTTPAttrIncHTTP:                   // ( 7) (W) UInt32 (Boolean) only applicable when inetHTTPAttrConvAlgorithm set to ctpConvNone
        case inetHTTPAttrCheckMailHi:               // ( 8) (W) UInt32
        case inetHTTPAttrCheckMailLo:               // ( 9) (W) UInt32
        case inetHTTPAttrReqContentVersion:         // (10) (W) UInt32 Desired content version. Represented as 2 low bytes. Lowest byte is minor version, next higher byte is major version. 

        // Response only attributes:

        // Server response info
        case inetHTTPAttrRspAll:                    // (11) (-) Char[] - entire HTTP response including data
        case inetHTTPAttrRspSize:                   // (12) (R) UInt32 - entire HTTP Response size including header and data
        case inetHTTPAttrRspVersion:                // (13) (-) Char[]
        case inetHTTPAttrResult:                    // (14) (R) UInt32 (ctpErrXXX when using CTP Proxy)
        case inetHTTPAttrErrDetail:                 // (15) (R) UInt32 (server/proxy err code when using CTP Proxy)
        case inetHTTPAttrReason:                    // (16) (R) Char[]
        case inetHTTPAttrDate:                      // (17) (-) UInt32
        case inetHTTPAttrNoCache:                   // (18) (-) UInt32
        case inetHTTPAttrPragma:                    // (19) (-) Char[]
        case inetHTTPAttrServer:                    // (20) (-) Char[]
        case inetHTTPAttrWWWAuthentication:         // (21) (-) Char[]

        // Returned entity attributes
        case inetHTTPAttrContentAllow:              // (22) (-) Char[]
        case inetHTTPAttrContentLength:             // (23) (R) UInt32
          break;
        case inetHTTPAttrContentLengthUncompressed: // (24) (R) UInt32 (in bytes)
          err = INetLibSettingGetUInt32(bufP, bufLenP, sockData->uncompDataSize);
          break;
        case inetHTTPAttrContentPtr:                // (25) (-) Char *
        case inetHTTPAttrContentExpires:            // (26) (-) UInt32
        case inetHTTPAttrContentLastModified:       // (27) (-) UInt32
        case inetHTTPAttrContentLocation:           // (28) (-) Char[]
        case inetHTTPAttrContentLengthUntruncated:  // (29) (R) UInt32
        case inetHTTPAttrContentVersion:            // (30) (R) UInt32, actual content version. Represented as 2 low bytes. Lowest byte is minor version, next higher byte is major version. 
        case inetHTTPAttrContentCacheID:            // (31) (R) UInt32, cacheID for this item
        case inetHTTPAttrReqSize:                   // (32) (R) UInt32 size of request sent
          break;
        default:
          break;
      }

      *bufLenP32 = bufLen;
      MemHandleUnlock(socketH);
    }
  }

  return err;
}

static UInt8 *checkScheme(UInt8 *p, char *scheme, UInt16 type, INetURLType* urlP) {
  UInt32 len;

  if (urlP->schemeEnum == (UInt16)inetSchemeDefault) {
    len = StrLen(scheme) - 1;  // do not include the ':'
    if (!StrNCompare((char *)p, scheme, len + 1)) {
      urlP->schemeEnum = type;
      if (urlP->schemeP) {
        MemMove(urlP->schemeP, p, urlP->schemeLen < len ? urlP->schemeLen : len);
      } else {
        urlP->schemeP = p;
      }
      urlP->schemeLen = len;
      p += urlP->schemeLen;
    }
  }

  return p;
}

Err INetLibURLCrack(UInt16 libRefnum, UInt8 *urlTextP, INetURLType* urlP) {
  UInt32 len;
  UInt8 *p, *s;
  Err err = inetErrParamsInvalid;

  if (urlTextP && urlP) {
    p = urlTextP;
    urlP->version = 0;
    urlP->schemeEnum = inetSchemeDefault;

    p = checkScheme(p, "http:",     inetSchemeHTTP, urlP);
    p = checkScheme(p, "https:",    inetSchemeHTTPS, urlP);
    p = checkScheme(p, "ftp:",      inetSchemeFTP, urlP);
    p = checkScheme(p, "gopher:",   inetSchemeGopher, urlP);
    p = checkScheme(p, "file:",     inetSchemeFile, urlP);
    p = checkScheme(p, "news:",     inetSchemeNews, urlP);
    p = checkScheme(p, "mailto:",   inetSchemeMailTo, urlP);
    p = checkScheme(p, "palm:",     inetSchemePalm, urlP);
    p = checkScheme(p, "palmcall:", inetSchemePalmCall, urlP);
    p = checkScheme(p, "mac:",      inetSchemeMac, urlP);

    urlP->usernameLen = 0;
    urlP->passwordLen = 0;
    urlP->paramLen = 0;
    urlP->queryLen = 0;
    urlP->fragLen = 0;
    urlP->port = 0;

    switch (urlP->schemeEnum) {
      case inetSchemeDefault:
      case inetSchemeFile:
        urlP->schemeEnum = inetSchemeFile;
        urlP->hostnameLen = 0;

        // XXX for some odd reason, the path component must include the ':' from the scheme
        len = StrLen((char *)p);
        if (urlP->pathP) {
          if (len) MemMove(urlP->pathP, p, urlP->pathLen < len ? urlP->pathLen : len);
        } else {
          urlP->pathP = p;
        }
        urlP->pathLen = len;

        debug(DEBUG_INFO, "INetMgr", "INetLibURLCrack file scheme \"%.*s\"", urlP->pathLen, urlP->pathP);
        err = errNone;
        break;

      case inetSchemeHTTP:
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

        urlP->port = inetPortHTTP;

        debug(DEBUG_INFO, "INetMgr", "INetLibURLCrack http scheme host \"%.*s\" path \"%.*s\" port %d",
          urlP->hostnameLen, urlP->hostnameP, urlP->pathLen, urlP->pathP, urlP->port);
        err = errNone;
        break;

      default:
        debug(DEBUG_ERROR, "INetMgr", "INetLibURLCrack invalid scheme %d for \"%s\"", urlP->schemeEnum, (char *)urlTextP);
        break;
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
      debug(DEBUG_INFO, "INetMgr", "INetLibWirelessIndicatorCmd wiCmdInit");
      break;
    case wiCmdClear:
      debug(DEBUG_INFO, "INetMgr", "INetLibWirelessIndicatorCmd wiCmdClear");
      break;
    case wiCmdSetEnabled:
      debug(DEBUG_INFO, "INetMgr", "INetLibWirelessIndicatorCmd wiCmdSetEnabled");
      break;
    case wiCmdDraw:
      debug(DEBUG_INFO, "INetMgr", "INetLibWirelessIndicatorCmd wiCmdDraw");
      break;
    case wiCmdEnabled:
      debug(DEBUG_INFO, "INetMgr", "INetLibWirelessIndicatorCmd wiCmdEnabled");
      break;
    case wiCmdSetLocation:
      debug(DEBUG_INFO, "INetMgr", "INetLibWirelessIndicatorCmd wiCmdSetLocation");
      break;
    case wiCmdErase:
      debug(DEBUG_INFO, "INetMgr", "INetLibWirelessIndicatorCmd wiCmdErase");
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

Err INetLibSockFileGetByIndex(UInt16 libRefnum, MemHandle sockHandle, UInt32 index, MemHandle *handleP, UInt32 *offsetP, UInt32 *lengthP) {
  MemHandle recH;
  INetLibSocketData *sockData;
  UInt32 dataOffset;
  UInt16 dataLength;
  UInt8 *rec, *p;
  Err err = inetErrParamsInvalid;

  if (sockHandle && handleP && offsetP && lengthP) {
    *handleP = 0;
    *offsetP = 0;
    *lengthP = 0;

    if ((sockData = MemHandleLock(sockHandle)) != NULL) {
      if (sockData->dbRef) {
        if (index < DmNumRecords(sockData->dbRef)) {
          if (index == sockData->index) {
            debug(DEBUG_INFO, "INetMgr", "INetLibSockFileGetByIndex current index=%u", index);
            *handleP = sockData->dataH;
            *offsetP = sockData->dataOffset;
            *lengthP = MemHandleSize(sockData->dataH);
            err = errNone;
          } else {
            debug(DEBUG_INFO, "INetMgr", "INetLibSockFileGetByIndex current index %u, new index=%u", sockData->index, index);
            if ((recH = DmGetRecord(sockData->dbRef, index)) != NULL) {
              if ((rec = MemHandleLock(recH)) != NULL) {
                get4b(&dataOffset, rec, 6);
                get2b(&dataLength, rec, 10);
                if (sockData->dataH) {
                  MemHandleResize(sockData->dataH, dataLength);
                } else {
                  sockData->dataH = MemHandleNew(dataLength);
                }
                sockData->index = index;
                if ((p = MemHandleLock(sockData->dataH)) != NULL) {
                  MemMove(p, rec + dataOffset, dataLength);
                  MemHandleUnlock(sockData->dataH);
                  *handleP = sockData->dataH;
                  *offsetP = 0;
                  *lengthP = dataLength;
                  debug(DEBUG_INFO, "INetMgr", "INetLibSockFileGetByIndex read %u bytes from record %d into data handle", dataLength, sockData->index);
                  err = errNone;
                }
                MemHandleUnlock(recH);
              }
              DmReleaseRecord(sockData->dbRef, index, false);
            }
          }
        }
      }
      MemHandleUnlock(sockHandle);
    }
  }

  return err;
}
