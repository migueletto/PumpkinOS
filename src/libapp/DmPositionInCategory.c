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

UInt16 DmPositionInCategory(DmOpenRef dbP, UInt16 index, UInt16 category) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapDmPositionInCategory, 0, &iret, NULL, dbP, index, category);
  return (UInt16)iret;
}
