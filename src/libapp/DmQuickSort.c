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

Err DmQuickSort(DmOpenRef dbP, DmComparF *compar, Int16 other) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapDmQuickSort, 0, &iret, NULL, dbP, compar, other);
  return (Err)iret;
}
