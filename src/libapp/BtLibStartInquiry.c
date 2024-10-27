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

Err BtLibStartInquiry(UInt16 btLibRefNum, UInt8 timeOut, UInt8 maxResp) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapStartInquiry, 0, &iret, NULL, btLibRefNum, timeOut, maxResp);
  return (Err)iret;
}
