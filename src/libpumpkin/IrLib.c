#include <PalmOS.h>
#include <IrLib.h>

#include "debug.h"

Err IrOpen(UInt16 refnum, UInt32 options) {
  debug(DEBUG_INFO, "IrLib", "IrOpen options 0x%08X", options);
  return errNone;
}

Err IrClose(UInt16 refnum) {
  return errNone;
}

IrStatus IrBind(UInt16 refNum, IrConnect *con, IrCallBack callBack) {
  return IR_STATUS_SUCCESS;
}

IrStatus IrUnbind(UInt16 refNum, IrConnect *con) {
  return IR_STATUS_SUCCESS;
}

IrStatus IrDiscoverReq(UInt16 refNum, IrConnect *con) {
  debug(DEBUG_ERROR, "IrLib", "IrDiscoverReq not implemented");
  return 0;
}

IrStatus IrConnectIrLap(UInt16 refNum, IrDeviceAddr deviceAddr) {
  debug(DEBUG_ERROR, "IrLib", "IrConnectIrLap not implemented");
  return 0;
}

IrStatus IrDisconnectIrLap(UInt16 refNum) {
  debug(DEBUG_ERROR, "IrLib", "IrDisconnectIrLap not implemented");
  return 0;
}

IrStatus IrConnectReq(UInt16 refNum, IrConnect *con, IrPacket *ppacket, UInt8 credit) {
  debug(DEBUG_ERROR, "IrLib", "IrConnectReq not implemented");
  return 0;
}

IrStatus IrConnectRsp(UInt16 refNum, IrConnect *con, IrPacket *ppacket, UInt8 credit) {
  debug(DEBUG_ERROR, "IrLib", "IrConnectRsp not implemented");
  return 0;
}

IrStatus IrDataReq(UInt16 refNum, IrConnect *con, IrPacket *ppacket) {
  debug(DEBUG_ERROR, "IrLib", "IrDataReq not implemented");
  return 0;
}

void IrLocalBusy(UInt16 refNum,BOOL flag) {
  debug(DEBUG_ERROR, "IrLib", "IrLocalBusy not implemented");
}

UInt16 IrMaxTxSize(UInt16 refNum, IrConnect *con) {
  debug(DEBUG_ERROR, "IrLib", "IrMaxTxSize not implemented");
  return 0;
}

UInt16 IrMaxRxSize(UInt16 refNum, IrConnect *con) {
  debug(DEBUG_ERROR, "IrLib", "IrMaxRxSize not implemented");
  return 0;
}

IrStatus IrSetDeviceInfo(UInt16 refNum, UInt8 *info, UInt8 len) {
  debug(DEBUG_INFO, "IrLib", "IrSetDeviceInfo %u bytes", len);
  debug_bytes(DEBUG_INFO, "IrLib", info, len);
  return IR_STATUS_SUCCESS;
}

IrStatus IrIAS_SetDeviceName(UInt16 refNum, UInt8 *name, UInt8 len) {
  debug(DEBUG_INFO, "IrLib", "IrIAS_SetDeviceName %u bytes", len);
  debug_bytes(DEBUG_INFO, "IrLib", name, len);
  return IR_STATUS_SUCCESS;
}

IrStatus IrIAS_Add(UInt16 refNum,IrIasObject* obj) {
  debug(DEBUG_INFO, "IrLib", "IrIAS_Add");
  return IR_STATUS_SUCCESS;
}

BOOL IrIsNoProgress(UInt16 refNum) {
  debug(DEBUG_ERROR, "IrLib", "IrIsNoProgress not implemented");
  return 0;
}

BOOL IrIsRemoteBusy(UInt16 refNum) {
  debug(DEBUG_ERROR, "IrLib", "IrIsRemoteBusy not implemented");
  return 0;
}

BOOL IrIsMediaBusy(UInt16 refNum) {
  debug(DEBUG_ERROR, "IrLib", "IrIsMediaBusy not implemented");
  return 0;
}

BOOL IrIsIrLapConnected(UInt16 refNum) {
  return 0;
}

IrStatus IrTestReq(UInt16 refNum, IrDeviceAddr devAddr, IrConnect *con, IrPacket *ppacket) {
  debug(DEBUG_ERROR, "IrLib", "IrTestReq not implemented");
  return 0;
}

Boolean IrHandleEvent(UInt16 refnum) {
  debug(DEBUG_ERROR, "IrLib", "IrHandleEvent not implemented");
  return 0;
}

Err IrWaitForEvent(UInt16 libRefnum, Int32 timeout) {
  debug(DEBUG_ERROR, "IrLib", "IrWaitForEvent not implemented");
  return 0;
}

