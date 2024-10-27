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

Err BtLibSocketCreate(UInt16 btLibRefNum, BtLibSocketRef *socketRefP, BtLibSocketProcPtr callbackP, UInt32 refCon, BtLibProtocolEnum socketProtocol) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapSocketCreate, 0, &iret, NULL, btLibRefNum, socketRefP, callbackP, refCon, socketProtocol);
  return (Err)iret;
}
