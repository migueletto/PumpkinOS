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

Err INetLibURLCrack(UInt16 libRefnum, UInt8 *urlTextP, INetURLType *urlP) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapURLCrack, 0, &iret, NULL, libRefnum, urlTextP, urlP);
  return (Err)iret;
}
