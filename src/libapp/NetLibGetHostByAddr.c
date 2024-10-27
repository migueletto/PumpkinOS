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

NetHostInfoPtr NetLibGetHostByAddr(UInt16 libRefNum, UInt8 *addrP, UInt16 len, UInt16 type, NetHostInfoBufPtr bufP, Int32 timeout, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapGetHostByAddr, 0, &iret, NULL, libRefNum, addrP, len, type, bufP, timeout, errP);
  return (NetHostInfoPtr)iret;
}
