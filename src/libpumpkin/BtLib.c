#include <PalmOS.h>
#include <BtLib.h>

#include "debug.h"

Err BtLibOpen(UInt16 btLibRefNum, Boolean allowStackToFail) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibOpen not implemented");
  return 0;
}

Err BtLibClose(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibClose not implemented");
  return 0;
}

Err BtLibSleep(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSleep not implemented");
  return 0;
}

Err BtLibWake(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibWake not implemented");
  return 0;
}

Err BtLibHandleEvent (UInt16 btLibRefNum, void *eventP) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibHandleEvent not implemented");
  return 0;
}

void BtLibHandleTransportEvent(UInt16 btLibRefNum, void *transportEventP) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibHandleTransportEvent not implemented");
}

Err BtLibRegisterManagementNotification(UInt16 btLibRefNum, BtLibManagementProcPtr callbackP, UInt32 refCon) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibRegisterManagementNotification not implemented");
  return 0;
}

Err BtLibUnregisterManagementNotification(UInt16 btLibRefNum, BtLibManagementProcPtr callbackP) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibUnregisterManagementNotification not implemented");
  return 0;
}

Err BtLibStartInquiry(UInt16 btLibRefNum,  UInt8 timeOut, UInt8 maxResp) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibStartInquiry not implemented");
  return 0;
}

Err BtLibCancelInquiry(UInt16 btLibRefNum ) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibCancelInquiry not implemented");
  return 0;
}

Err BtLibGetSelectedDevices(UInt16 btLibRefNum, BtLibDeviceAddressType* selectedDeviceArray, UInt8 arraySize, UInt8 *numDevicesReturned) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibGetSelectedDevices not implemented");
  return 0;
}

Err BtLibLinkConnect(UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibLinkConnect not implemented");
  return 0;
}

Err BtLibLinkDisconnect(UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibLinkDisconnect not implemented");
  return 0;
}

Err BtLibPiconetCreate(UInt16 btLibRefNum, Boolean unlockInbound, Boolean discoverable) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibPiconetCreate not implemented");
  return 0;
}

Err BtLibPiconetDestroy(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibPiconetDestroy not implemented");
  return 0;
}

Err BtLibPiconetUnlockInbound(UInt16 btLibRefNum, Boolean discoverable) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibPiconetUnlockInbound not implemented");
  return 0;
}

Err BtLibPiconetLockInbound(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibPiconetLockInbound not implemented");
  return 0;
}

Err BtLibLinkSetState(UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP, BtLibLinkPrefsEnum pref, void* linkState, UInt16 linkStateSize) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibLinkSetState not implemented");
  return 0;
}

Err BtLibLinkGetState(UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP, BtLibLinkPrefsEnum pref, void* linkState, UInt16 linkStateSize) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibLinkGetState not implemented");
  return 0;
}

Err BtLibSetGeneralPreference(UInt16 btLibRefNum, BtLibGeneralPrefEnum pref, void* prefValue, UInt16 prefValueSize) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSetGeneralPreference not implemented");
  return 0;
}

Err BtLibGetGeneralPreference(UInt16 btLibRefNum, BtLibGeneralPrefEnum pref, void* prefValue, UInt16 prefValueSize) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibGetGeneralPreference not implemented");
  return 0;
}

Err BtLibSocketClose(UInt16 btLibRefNum,  BtLibSocketRef socket) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSocketClose not implemented");
  return 0;
}

Err BtLibSocketListen(UInt16 btLibRefNum,  BtLibSocketRef socket, BtLibSocketListenInfoType *listenInfo) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSocketListen not implemented");
  return 0;
}

Err BtLibSocketConnect(UInt16 btLibRefNum,  BtLibSocketRef socket, BtLibSocketConnectInfoType* connectInfo) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSocketConnect not implemented");
  return 0;
}

Err BtLibSocketRespondToConnection(UInt16 btLibRefNum,  BtLibSocketRef socket, Boolean accept) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSocketRespondToConnection not implemented");
  return 0;
}

Err BtLibSocketSend(UInt16 btLibRefNum, BtLibSocketRef socket, UInt8 *data, UInt32 dataLen) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSocketSend not implemented");
  return 0;
}

Err BtLibSocketAdvanceCredit(UInt16 btLibRefNum, BtLibSocketRef socket, UInt8 credit) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSocketAdvanceCredit not implemented");
  return 0;
}

Err BtLibSdpServiceRecordCreate(UInt16 btLibRefNum, BtLibSdpRecordHandle* recordH ) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSdpServiceRecordCreate not implemented");
  return 0;
}

Err BtLibSdpServiceRecordDestroy(UInt16 btLibRefNum, BtLibSdpRecordHandle recordH) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSdpServiceRecordDestroy not implemented");
  return 0;
}

Err BtLibSdpServiceRecordStartAdvertising(UInt16 btLibRefNum, BtLibSdpRecordHandle recordH) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSdpServiceRecordStartAdvertising not implemented");
  return 0;
}

Err BtLibSdpServiceRecordStopAdvertising(UInt16 btLibRefNum, BtLibSdpRecordHandle recordH) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSdpServiceRecordStopAdvertising not implemented");
  return 0;
}

Err BtLibSdpCompareUuids(UInt16 btLibRefNum, BtLibSdpUuidType *uuid1, BtLibSdpUuidType *uuid2) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSdpCompareUuids not implemented");
  return 0;
}

Err BtLibSecurityFindTrustedDeviceRecord(UInt16 btLibRefNum, BtLibDeviceAddressTypePtr addrP, UInt16 *index) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSecurityFindTrustedDeviceRecord not implemented");
  return 0;
}

Err BtLibSecurityRemoveTrustedDeviceRecord(UInt16 btLibRefNum, UInt16 index) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSecurityRemoveTrustedDeviceRecord not implemented");
  return 0;
}

UInt16 BtLibSecurityNumTrustedDeviceRecords(UInt16 btLibRefNum, Boolean persistentOnly) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibSecurityNumTrustedDeviceRecords not implemented");
  return 0;
}

Err BtLibAddrAToBtd(UInt16 libRefNum, const Char *a, BtLibDeviceAddressType *btDevP) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibAddrAToBtd not implemented");
  return 0;
}

Err BtLibServiceOpen(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibServiceOpen not implemented");
  return 0;
}

Err BtLibServiceClose(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibServiceClose not implemented");
  return 0;
}

Err BtLibServiceIndicateSessionStart(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibServiceIndicateSessionStart not implemented");
  return 0;
}

Err BtLibServicePlaySound(UInt16 btLibRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "BtLibServicePlaySound not implemented");
  return 0;
}
