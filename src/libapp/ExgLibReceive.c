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

UInt32 ExgLibReceive(UInt16 libRefNum, ExgSocketType *exgSocketP, void *bufP, UInt32 bufSize, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(EXG_LIB, exgLibTrapReceive, 0, &iret, NULL, libRefNum, exgSocketP, bufP, bufSize, errP);
  return (UInt32)iret;
}
