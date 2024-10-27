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

Err BtLibSdpServiceRecordSetAttributesForSocket(UInt16 btLibRefNum, BtLibSocketRef socket, BtLibSdpUuidType *serviceUUIDList, UInt8 uuidListLen, const Char *serviceName, UInt16 serviceNameLen, BtLibSdpRecordHandle recordH) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapSdpServiceRecordSetAttributesForSocket, 0, &iret, NULL, btLibRefNum, socket, serviceUUIDList, uuidListLen, serviceName, serviceNameLen, recordH);
  return (Err)iret;
}
