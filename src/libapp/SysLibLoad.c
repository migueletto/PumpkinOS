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

Err SysLibLoad(UInt32 libType, UInt32 libCreator, UInt16 *refNumP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSysLibLoad, 0, &iret, NULL, libType, libCreator, refNumP);
  return (Err)iret;
}
