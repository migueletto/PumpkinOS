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

FieldType *FldNewField(void * *formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height, FontID font, UInt32 maxChars, Boolean editable, Boolean underlined, Boolean singleLine, Boolean dynamicSize, JustificationType justification, Boolean autoShift, Boolean hasScrollBar, Boolean numeric) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapFldNewField, 0, NULL, &pret, formPP, id, x, y, width, height, font, maxChars, editable, underlined, singleLine, dynamicSize, justification, autoShift, hasScrollBar, numeric);
  return (FieldType *)pret;
}
