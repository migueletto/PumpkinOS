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

FormBitmapType *FrmNewBitmap(FormType * *formPP, UInt16 ID, UInt16 rscID, Coord x, Coord y) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapFrmNewBitmap, 0, NULL, &pret, formPP, ID, rscID, x, y);
  return (FormBitmapType *)pret;
}
