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

Err TxtConvertEncoding(Boolean newConversion, TxtConvertStateType *ioStateP, const Char *srcTextP, UInt16 *ioSrcBytes, CharEncodingType srcEncoding, Char *dstTextP, UInt16 *ioDstBytes, CharEncodingType dstEncoding, const Char *substitutionStr, UInt16 substitutionLen) {
  uint64_t iret;
  pumpkin_system_call_p(0, sysTrapIntlDispatch, intlTxtConvertEncoding, &iret, NULL, newConversion, ioStateP, srcTextP, ioSrcBytes, srcEncoding, dstTextP, ioDstBytes, dstEncoding, substitutionStr, substitutionLen);
  return (Err)iret;
}
