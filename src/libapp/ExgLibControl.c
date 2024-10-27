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

Err ExgLibControl(UInt16 libRefNum, UInt16 op, void *valueP, UInt16 *valueLenP) {
  uint64_t iret;
  pumpkin_system_call_p(EXG_LIB, exgLibTrapControl, 0, &iret, NULL, libRefNum, op, valueP, valueLenP);
  return (Err)iret;
}
