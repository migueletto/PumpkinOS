#include <PalmOS.h>
#include <INetMgr.h>

#include "debug.h"

Err INetLibOpen(UInt16 libRefnum, UInt16 config, UInt32 flags, DmOpenRef cacheRef, UInt32 cacheSize, MemHandle *inetHP) {
  return inetErrConfigNotFound;
}

Err INetLibClose(UInt16 libRefnum, MemHandle inetH) {
  return inetErrConfigNotFound;
}

Err INetLibSleep(UInt16 libRefnum) {
  return inetErrConfigNotFound;
}

Err INetLibWake(UInt16 libRefnum) {
  return inetErrConfigNotFound;
}

Err INetLibSettingGet(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSettingEnum */ setting, void *bufP, UInt16 *bufLenP) {
  return inetErrConfigNotFound;
}

Err INetLibSettingSet(UInt16 libRefnum, MemHandle inetH, UInt16 /*INetSettingEnum*/ setting, void *bufP, UInt16 bufLen) {
  return inetErrConfigNotFound;
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
