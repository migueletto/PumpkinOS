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

NetIPAddr NetLibAddrAToIN(UInt16 libRefnum, const Char *a) {
  uint64_t iret;
  pumpkin_system_call_p(NET_LIB, netLibTrapAddrAToIN, 0, &iret, NULL, libRefnum, a);
  return (NetIPAddr)iret;
}
