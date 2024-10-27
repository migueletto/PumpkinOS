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

Err SrmCustomControl(UInt16 portId, UInt16 opCode, UInt32 creator, void *valueP, UInt16 *valueLenP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSerialDispatch, sysSerialCustomControl, &iret, NULL, portId, opCode, creator, valueP, valueLenP);
  return (Err)iret;
}
