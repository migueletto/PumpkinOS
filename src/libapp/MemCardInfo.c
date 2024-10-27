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

Err MemCardInfo(UInt16 cardNo, Char *cardNameP, Char *manufNameP, UInt16 *versionP, UInt32 *crDateP, UInt32 *romSizeP, UInt32 *ramSizeP, UInt32 *freeBytesP) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapMemCardInfo, 0, &iret, NULL, cardNo, cardNameP, manufNameP, versionP, crDateP, romSizeP, ramSizeP, freeBytesP);
  return (Err)iret;
}
