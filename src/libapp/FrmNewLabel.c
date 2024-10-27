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

FormLabelType *FrmNewLabel(FormType * *formPP, UInt16 ID, const Char *textP, Coord x, Coord y, FontID font) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapFrmNewLabel, 0, NULL, &pret, formPP, ID, textP, x, y, font);
  return (FormLabelType *)pret;
}
