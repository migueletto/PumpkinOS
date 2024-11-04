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

UInt32 ExgSend(ExgSocketType *socketP, const void *bufP, UInt32 bufLen, Err *err) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapExgSend, 0, &iret, NULL, socketP, bufP, bufLen, err);
  return (UInt32)iret;
}