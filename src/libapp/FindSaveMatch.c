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

Boolean FindSaveMatch(FindParamsPtr findParams, UInt16 recordNum, UInt16 pos, UInt16 fieldNum, UInt32 appCustom, UInt16 cardNo, LocalID dbID) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapFindSaveMatch, 0, &iret, NULL, findParams, recordNum, pos, fieldNum, appCustom, cardNo, dbID);
  return (Boolean)iret;
}
