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

void FntCharsInWidth(Char const *string, Int16 *stringWidthP, Int16 *stringLengthP, Boolean *fitWithinWidth) {
  pumpkin_system_call_p(0, sysTrapFntCharsInWidth, 0, NULL, NULL, string, stringWidthP, stringLengthP, fitWithinWidth);
}
