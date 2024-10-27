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

void FntWordWrapReverseNLines(Char const * const chars, UInt16 maxWidth, UInt16 *linesToScrollP, UInt16 *scrollPosP) {
  pumpkin_system_call_p(0, sysTrapFntWordWrapReverseNLines, 0, NULL, NULL, chars, maxWidth, linesToScrollP, scrollPosP);
}
