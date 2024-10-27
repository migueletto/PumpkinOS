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

UInt32 SerSend(UInt16 refNum, const void *bufP, UInt32 count, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(SER_LIB, sysLibTrapCustom+18, 0, &iret, NULL, refNum, bufP, count, errP);
  return (UInt32)iret;
}
