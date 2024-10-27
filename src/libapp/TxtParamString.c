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

Char *TxtParamString(const Char *inTemplate, const Char *param0, const Char *param1, const Char *param2, const Char *param3) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapIntlDispatch, intlTxtParamString, NULL, &pret, inTemplate, param0, param1, param2, param3);
  return (Char *)pret;
}
