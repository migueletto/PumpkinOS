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

UInt16 SerGetStatus(UInt16 refNum, Boolean *ctsOnP, Boolean *dsrOnP) {
  uint64_t iret;
  pumpkin_system_call_p(SER_LIB, sysLibTrapCustom+2, 0, &iret, NULL, refNum, ctsOnP, dsrOnP);
  return (UInt16)iret;
}
