#include <PalmOS.h>

// m68k-palmos-gcc -O2 -Wall -Wno-switch -palmos5 -c SysLibLoad.c
// m68k-palmos-objdump -s SysLibLoad.o

extern Boolean SysLibNewRefNum(UInt32 type, UInt32 creator, UInt16 *refNum) SYS_TRAP(0xA503);
extern Err SysLibRegister(UInt16 refNum, LocalID dbID, void *code, UInt32 size, UInt16 *dispatchTblP, UInt8 *globalsP) SYS_TRAP(0xA504);
extern void SysLibCancelRefNum(UInt16 refNum) SYS_TRAP(0xA505);

Err SysLibLoad(UInt32 libType, UInt32 libCreator, UInt16 *refNumP) {
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  DmOpenRef dbRef;
  LocalID dbID;
  MemHandle h;
  SysLibTblEntryType sysLibTbl;
  SysLibEntryProcPtr sysLibEntry;
  Err err = sysErrLibNotFound;

  if (refNumP) *refNumP = 0xffff;

  if (DmGetNextDatabaseByTypeCreator(true, &stateInfo, libType, libCreator, false, &cardNo, &dbID) == errNone) {
    if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadOnly)) != NULL) {
      if ((h = DmGet1Resource(sysResTLibrary, 0)) != NULL) {
        if ((sysLibEntry = MemHandleLock(h)) != NULL) {
          if (SysLibNewRefNum(libType, libCreator, refNumP)) {
            err = errNone;
          } else {
            MemSet(&sysLibTbl, sizeof(SysLibTblEntryType), 0);
            err = sysLibEntry(*refNumP, &sysLibTbl);
            if (err == errNone) {
              err = SysLibRegister(*refNumP, dbID, sysLibEntry, MemHandleSize(h), (UInt16 *)sysLibTbl.dispatchTblP, (UInt8 *)sysLibTbl.globalsP);
            }
            if (err != errNone) {
              SysLibCancelRefNum(*refNumP);
            }
          }
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      DmCloseDatabase(dbRef);
    }
  }

  return err;
}
