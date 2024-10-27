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

FormGadgetType *FrmNewGadget(FormType * *formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapFrmNewGadget, 0, NULL, &pret, formPP, id, x, y, width, height);
  return (FormGadgetType *)pret;
}
