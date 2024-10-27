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

Int16 INetLibURLsCompare(UInt16 libRefnum, Char *URLStr1, Char *URLStr2) {
  uint64_t iret;
  pumpkin_system_call_p(INET_LIB, inetLibTrapURLsCompare, 0, &iret, NULL, libRefnum, URLStr1, URLStr2);
  return (Int16)iret;
}
