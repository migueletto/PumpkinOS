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

SliderControlType *CtlGlueNewSliderControl(void **formPP, UInt16 ID, ControlStyleType style, DmResID thumbID, DmResID backgroundID, Coord x, Coord y, Coord width, Coord height, UInt16 minValue, UInt16 maxValue, UInt16 pageSize, UInt16 value) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapAccessorDispatch, 9, NULL, &pret, formPP, ID, style, thumbID, backgroundID, x, y, width, height, minValue, maxValue, pageSize, value);
  return (SliderControlType *)pret;
}
