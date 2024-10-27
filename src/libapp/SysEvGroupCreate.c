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

Err SysEvGroupCreate(UInt32 *evIDP, UInt32 *tagP, UInt32 init) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapSysEvGroupCreate, 0, &iret, NULL, evIDP, tagP, init);
  return (Err)iret;
}
