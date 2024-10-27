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

Err BtLibSocketGetInfo(UInt16 btLibRefNum, BtLibSocketRef socket, BtLibSocketInfoEnum infoType, void *valueP, UInt32 valueSize) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapSocketGetInfo, 0, &iret, NULL, btLibRefNum, socket, infoType, valueP, valueSize);
  return (Err)iret;
}
