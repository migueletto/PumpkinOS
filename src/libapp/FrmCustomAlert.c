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

UInt16 FrmCustomAlert(UInt16 alertId, const Char *s1, const Char *s2, const Char *s3) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapFrmCustomAlert, 0, &iret, NULL, alertId, s1, s2, s3);
  return (UInt16)iret;
}
