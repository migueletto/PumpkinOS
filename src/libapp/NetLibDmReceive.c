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

Int16 NetLibDmReceive(UInt16 libRefNum, NetSocketRef socket, void *recordP, UInt32 recordOffset, UInt16 rcvLen, UInt16 flags, void *fromAddrP, UInt16 *fromLenP, Int32 timeout, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapDmReceive, 0, &iret, NULL, libRefNum, socket, recordP, recordOffset, rcvLen, flags, fromAddrP, fromLenP, timeout, errP);
  return (Int16)iret;
}
