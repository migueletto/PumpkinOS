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

GraphicControlType *CtlNewGraphicControl(void * *formPP, UInt16 ID, ControlStyleType style, DmResID bitmapID, DmResID selectedBitmapID, Coord x, Coord y, Coord width, Coord height, UInt8 group, Boolean leftAnchor) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapCtlNewGraphicControl, 0, NULL, &pret, formPP, ID, style, bitmapID, selectedBitmapID, x, y, width, height, group, leftAnchor);
  return (GraphicControlType *)pret;
}
