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

Err AttnGetAttention(UInt16 cardNo, LocalID dbID, UInt32 userData, AttnCallbackProc *callbackFnP, AttnLevelType level, AttnFlagsType flags, UInt16 nagRateInSeconds, UInt16 nagRepeatLimit) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapAttnGetAttention, 0, &iret, NULL, cardNo, dbID, userData, callbackFnP, level, flags, nagRateInSeconds, nagRepeatLimit);
  return (Err)iret;
}
