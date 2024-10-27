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

UInt16 SysBatteryInfoV20(Boolean set, UInt16 *warnThresholdP, UInt16 *criticalThresholdP, Int16 *maxTicksP, SysBatteryKind *kindP, Boolean *pluggedIn) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSysBatteryInfoV20, 0, &iret, NULL, set, warnThresholdP, criticalThresholdP, maxTicksP, kindP, pluggedIn);
  return (UInt16)iret;
}
