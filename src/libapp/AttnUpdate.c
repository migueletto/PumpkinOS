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

Boolean AttnUpdate(UInt16 cardNo, LocalID dbID, UInt32 userData, AttnCallbackProc *callbackFnP, AttnFlagsType *flagsP, UInt16 *nagRateInSecondsP, UInt16 *nagRepeatLimitP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapAttnUpdate, 0, &iret, NULL, cardNo, dbID, userData, callbackFnP, flagsP, nagRateInSecondsP, nagRepeatLimitP);
  return (Boolean)iret;
}
