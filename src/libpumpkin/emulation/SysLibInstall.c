#include <PalmOS.h>

// m68k-palmos-gcc -O2 -Wall -Wno-switch -palmos5 -c SysLibInstall.c
// m68k-palmos-objdump -s SysLibLoad.o

extern Boolean SysLibNewRefNum(UInt32 type, UInt32 creator, UInt16 *refNum) SYS_TRAP(0xA503);
extern Err SysLibRegister(UInt16 refNum, LocalID dbID, void *code, UInt32 size, UInt16 *dispatchTblP, UInt8 *globalsP) SYS_TRAP(0xA504);
extern void SysLibCancelRefNum(UInt16 refNum) SYS_TRAP(0xA505);

Err SysLibInstall(SysLibEntryProcPtr sysLibEntry, UInt16 *refNumP) {
  SysLibTblEntryType sysLibTbl;
  Err err = sysErrLibNotFound;

  if (refNumP) *refNumP = 0xffff;

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

  return err;
}
