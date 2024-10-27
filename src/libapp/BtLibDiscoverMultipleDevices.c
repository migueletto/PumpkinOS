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

Err BtLibDiscoverMultipleDevices(UInt16 btLibRefNum, Char *instructionTxt, Char *buttonTxt, BtLibClassOfDeviceType *deviceFilterList, UInt8 deviceFilterListLen, UInt8 *numDevicesSelected, Boolean addressAsName, Boolean showLastList) {
  uint64_t iret;
  pumpkin_system_call_p(BT_LIB, btLibTrapDiscoverMultipleDevices, 0, &iret, NULL, btLibRefNum, instructionTxt, buttonTxt, deviceFilterList, deviceFilterListLen, numDevicesSelected, addressAsName, showLastList);
  return (Err)iret;
}
