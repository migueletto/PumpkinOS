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

Err INetLibConfigAliasGet(UInt16 refNum, UInt16 aliasIndex, UInt16 *indexP, Boolean *isAnotherAliasP) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibConfigAliasGet, 0, &iret, NULL, refNum, aliasIndex, indexP, isAnotherAliasP);
  return (Err)iret;
}
