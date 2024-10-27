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

Err SerSetWakeupHandler(UInt16 refNum, SerWakeupHandler procP, UInt32 refCon) {
  uint64_t iret;
  pumpkin_system_call_p(SER_LIB, sysLibTrapCustom+15, 0, &iret, NULL, refNum, procP, refCon);
  return (Err)iret;
}
