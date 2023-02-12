#include <PalmOS.h>
#include <BtLib.h>
#include <CPMLib.h>
#include <DLServer.h>
#include <ExpansionMgr.h>
#include <FSLib.h>
#include <INetMgr.h>
#include <PceNativeCall.h>
#include <PdiLib.h>
#include <SerialMgrOld.h>
#include <SerialSdrv.h>
#include <SerialVdrv.h>
#include <SlotDrvrLib.h>
#include <SslLib.h>

#include "debug.h"

Err	IrOpen(UInt16 refnum, UInt32 options) {
  debug(DEBUG_ERROR, "PALMOS", "IrOpen not implemented");
  return 0;
}

Err	IrClose(UInt16 refnum) {
  debug(DEBUG_ERROR, "PALMOS", "IrClose not implemented");
  return 0;
}

IrStatus IrBind(UInt16 refNum,IrConnect* con, IrCallBack callBack) {
  debug(DEBUG_ERROR, "PALMOS", "IrBind not implemented");
  return 0;
}

IrStatus IrUnbind(UInt16 refNum,IrConnect* con) {
  debug(DEBUG_ERROR, "PALMOS", "IrUnbind not implemented");
  return 0;
}

IrStatus IrDiscoverReq(UInt16 refNum,IrConnect* con) {
  debug(DEBUG_ERROR, "PALMOS", "IrDiscoverReq not implemented");
  return 0;
}

IrStatus IrConnectIrLap(UInt16 refNum,IrDeviceAddr deviceAddr) {
  debug(DEBUG_ERROR, "PALMOS", "IrConnectIrLap not implemented");
  return 0;
}

IrStatus IrDisconnectIrLap(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "IrDisconnectIrLap not implemented");
  return 0;
}

IrStatus IrConnectReq(UInt16 refNum,IrConnect* con, IrPacket* packet, UInt8 credit) {
  debug(DEBUG_ERROR, "PALMOS", "IrConnectReq not implemented");
  return 0;
}

IrStatus IrConnectRsp(UInt16 refNum,IrConnect* con,IrPacket* packet, UInt8 credit) {
  debug(DEBUG_ERROR, "PALMOS", "IrConnectRsp not implemented");
  return 0;
}

IrStatus IrDataReq(UInt16 refNum,IrConnect* con, IrPacket* packet) {
  debug(DEBUG_ERROR, "PALMOS", "IrDataReq not implemented");
  return 0;
}

void IrLocalBusy(UInt16 refNum,BOOL flag) {
  debug(DEBUG_ERROR, "PALMOS", "IrLocalBusy not implemented");
}

UInt16 IrMaxTxSize(UInt16 refNum,IrConnect* con) {
  debug(DEBUG_ERROR, "PALMOS", "IrMaxTxSize not implemented");
  return 0;
}

UInt16 IrMaxRxSize(UInt16 refNum,IrConnect* con) {
  debug(DEBUG_ERROR, "PALMOS", "IrMaxRxSize not implemented");
  return 0;
}

IrStatus IrSetDeviceInfo(UInt16 refNum,UInt8 *info, UInt8 len) {
  debug(DEBUG_ERROR, "PALMOS", "IrSetDeviceInfo not implemented");
  return 0;
}

BOOL IrIsNoProgress(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "IrIsNoProgress not implemented");
  return 0;
}

BOOL IrIsRemoteBusy(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "IrIsRemoteBusy not implemented");
  return 0;
}

BOOL IrIsMediaBusy(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "IrIsMediaBusy not implemented");
  return 0;
}

BOOL IrIsIrLapConnected(UInt16 refNum) {
  debug(DEBUG_ERROR, "PALMOS", "IrIsIrLapConnected not implemented");
  return 0;
}

IrStatus IrTestReq(UInt16 refNum,IrDeviceAddr devAddr, IrConnect* con, IrPacket* packet) {
  debug(DEBUG_ERROR, "PALMOS", "IrTestReq not implemented");
  return 0;
}

Boolean IrHandleEvent(UInt16 refnum) {
  debug(DEBUG_ERROR, "PALMOS", "IrHandleEvent not implemented");
  return 0;
}

Err IrWaitForEvent(UInt16 libRefnum,Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "IrWaitForEvent not implemented");
  return 0;
}

