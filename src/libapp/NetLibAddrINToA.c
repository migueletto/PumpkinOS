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

Char *NetLibAddrINToA(UInt16 libRefnum, NetIPAddr inet, Char *spaceP) {
  void *pret;
  pumpkin_system_call_p(NET_LIB, netLibTrapAddrINToA, 0, NULL, &pret, libRefnum, inet, spaceP);
  return (Char *)pret;
}
