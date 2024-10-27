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

Err BtLibLinkGetState(UInt16 btLibRefNum, BtLibDeviceAddressTypePtr remoteDeviceP, BtLibLinkPrefsEnum pref, void *linkState, UInt16 linkStateSize) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapLinkGetState, 0, &iret, NULL, btLibRefNum, remoteDeviceP, pref, linkState, linkStateSize);
  return (Err)iret;
}
