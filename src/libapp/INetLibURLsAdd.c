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

Err INetLibURLsAdd(UInt16 libRefnum, Char *baseURLStr, Char *embeddedURLStr, Char *resultURLStr, UInt16 *resultLenP) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapURLsAdd, 0, &iret, NULL, libRefnum, baseURLStr, embeddedURLStr, resultURLStr, resultLenP);
  return (Err)iret;
}
