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

Err INetLibSockOpen(UInt16 libRefnum, MemHandle inetH, UInt16 scheme, MemHandle *sockHP) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapSockOpen, 0, &iret, NULL, libRefnum, inetH, scheme, sockHP);
  return (Err)iret;
}