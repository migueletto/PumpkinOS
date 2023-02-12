#include <PalmOS.h>

extern SysLibTblEntryType *SysLibGet(UInt16 refNum) SYS_TRAP(0xA506);

/*
#define sysLibTrapName    0xA800
#define sysLibTrapOpen    0xA801
#define sysLibTrapClose   0xA802
#define sysLibTrapSleep   0xA803
#define sysLibTrapWake    0xA804
#define sysLibTrapCustom  0xA805
*/

typedef struct {
  UInt16 numEntries;
  void *entries;
} syslib_dispatch_t;

Err SysLibCustom(UInt16 refNum, ...) {
  SysLibTblEntryType *sysLibTbl;
  syslib_dispatch_t *dispatchTable;

/*
  dc.w  .Lname-jmptable
  dc.w  PalmOsLibOpen-jmptable
  dc.w  PalmOsLibClose-jmptable
  dc.w  PalmOsLibSleep-jmptable
  dc.w  PalmOsLibWake-jmptable
  dc.w  PalmOsLibGet-jmptable
*/

  if ((sysLibTbl = SysLibGet(refNum)) != NULL) {
    if (sysLibTbl->dispatchTblP) {
      dispatchTable = (syslib_dispatch_t *)sysLibTbl->dispatchTblP;
      if (dispatchTable > 0) {
      }
    }
  }

  return 0;
}
