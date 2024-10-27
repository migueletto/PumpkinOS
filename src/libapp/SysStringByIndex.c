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

Char *SysStringByIndex(UInt16 resID, UInt16 index, Char *strP, UInt16 maxLen) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapSysStringByIndex, 0, NULL, &pret, resID, index, strP, maxLen);
  return (Char *)pret;
}
