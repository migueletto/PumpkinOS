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

UDAReaderType *UDAMemoryReaderNew(const UInt8 *bufferP, UDABufferSize bufferSizeInBytes) {
  void *pret;
  pumpkin_system_call_p(0, sysTrapUdaMgrDispatch, sysUdaMemoryReaderNew, NULL, &pret, bufferP, bufferSizeInBytes);
  return (UDAReaderType *)pret;
}
