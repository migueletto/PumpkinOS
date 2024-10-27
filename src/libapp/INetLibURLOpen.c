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

Err INetLibURLOpen(UInt16 libRefnum, MemHandle inetH, UInt8 *urlP, UInt8 *cacheIndexURLP, MemHandle *sockHP, Int32 timeout, UInt16 flags) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapURLOpen, 0, &iret, NULL, libRefnum, inetH, urlP, cacheIndexURLP, sockHP, timeout, flags);
  return (Err)iret;
}
