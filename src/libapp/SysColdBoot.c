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

void SysColdBoot(void *card0P, UInt32 card0Size, void *card1P, UInt32 card1Size, UInt32 sysCardHeaderOffset) {
  pumpkin_system_call_p(0, sysTrapSysColdBoot, 0, NULL, NULL, card0P, card0Size, card1P, card1Size, sysCardHeaderOffset);
}
