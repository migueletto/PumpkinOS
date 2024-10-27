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

UInt32 HostExgLibSend(UInt16 libRefNum, void *exgSocketP, const void * const bufP, const UInt32 bufLen, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapHostControl, hostSelectorExgLibSend, &iret, NULL, libRefNum, exgSocketP, bufP, bufLen, errP);
  return (UInt32)iret;
}
