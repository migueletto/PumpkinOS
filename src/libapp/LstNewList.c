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

Err LstNewList(void * *formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height, FontID font, Int16 visibleItems, Int16 triggerId) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapLstNewList, 0, &iret, NULL, formPP, id, x, y, width, height, font, visibleItems, triggerId);
  return (Err)iret;
}
