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

Boolean INetLibWirelessIndicatorCmd(UInt16 refNum, MemHandle inetH, UInt16 cmd, int enableOrX, int y) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapWirelessIndicatorCmd, 0, &iret, NULL, refNum, inetH, cmd, enableOrX, y);
  return (Boolean)iret;
}
