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

Err INetLibCTPSend(UInt16 libRefnum, MemHandle inetH, MemHandle *sockHP, UInt8 *writeP, UInt32 writelen, Int32 timeout, UInt16 ctpCommand) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapCTPSend, 0, &iret, NULL, libRefnum, inetH, sockHP, writeP, writelen, timeout, ctpCommand);
  return (Err)iret;
}
