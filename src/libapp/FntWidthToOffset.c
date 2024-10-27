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

Int16 FntWidthToOffset(Char const *pChars, UInt16 length, Int16 pixelWidth, Boolean *leadingEdge, Int16 *truncWidth) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapFntWidthToOffset, 0, &iret, NULL, pChars, length, pixelWidth, leadingEdge, truncWidth);
  return (Int16)iret;
}
