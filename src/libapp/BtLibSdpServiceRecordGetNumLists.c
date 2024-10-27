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

Err BtLibSdpServiceRecordGetNumLists(UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, BtLibSdpAttributeIdType attributeID, UInt16 *numLists) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapSdpServiceRecordGetNumLists, 0, &iret, NULL, btLibRefNum, recordH, attributeID, numLists);
  return (Err)iret;
}
