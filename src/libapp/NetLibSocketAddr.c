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

Int16 NetLibSocketAddr(UInt16 libRefnum, NetSocketRef socketRef, NetSocketAddrType *locAddrP, Int16 *locAddrLenP, NetSocketAddrType *remAddrP, Int16 *remAddrLenP, Int32 timeout, Err *errP) {
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapSocketAddr, 0, &iret, NULL, libRefnum, socketRef, locAddrP, locAddrLenP, remAddrP, remAddrLenP, timeout, errP);
  return (Int16)iret;
}
