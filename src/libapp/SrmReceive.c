#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <VFSMgr.h>
#include <ExpansionMgr.h>
#include <DLServer.h>
#include <SerialMgrOld.h>
#include <UDAMgr.h>
#include <PceNativeCall.h>
#include <FixedMath.h>
#include <CPMLib.h>
#include <GPSLib68K.h>
#include <GPDLib.h>
#include <PdiLib.h>
#include <BtLib.h>
#include <FSLib.h>
#include <SslLib.h>
#include <INetMgr.h>
#include <SlotDrvrLib.h>

UInt32 SrmReceive(UInt16 portId, void *rcvBufP, UInt32 count, Int32 timeout, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSerialDispatch, sysSerialReceive, &iret, NULL, portId, rcvBufP, count, timeout, errP);
  return (UInt32)iret;
}
