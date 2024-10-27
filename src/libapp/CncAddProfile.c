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

Err CncAddProfile(Char *name, UInt32 port, UInt32 baud, UInt16 volume, UInt16 handShake, const Char *initString, const Char *resetString, Boolean isModem, Boolean isPulse) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapCncAddProfile, 0, &iret, NULL, name, port, baud, volume, handShake, initString, resetString, isModem, isPulse);
  return (Err)iret;
}
