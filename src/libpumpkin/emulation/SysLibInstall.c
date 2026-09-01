#include <PalmOS.h>
#include "SysLibAux.h"

// m68k-palmos-gcc -O2 -Wall -Wno-switch -c SysLibInstall.c
// m68k-palmos-objdump -s SysLibInstall.o

Err SysLibInstall(SysLibEntryProcPtr sysLibEntry, UInt16 *refNumP) {
  SysLibTblEntryType sysLibTbl;
  Err err = sysErrLibNotFound;

  if (refNumP) *refNumP = 0xffff;

  if (SysLibNewRefNum(sysResTLibrary, 0, refNumP)) {
    err = errNone;
  } else {
    MemSet(&sysLibTbl, sizeof(SysLibTblEntryType), 0);
    err = sysLibEntry(*refNumP, &sysLibTbl);
    if (err == errNone) {
      err = SysLibRegister(*refNumP, 0, sysLibEntry, MemPtrSize(sysLibEntry), (UInt16 *)sysLibTbl.dispatchTblP, (UInt8 *)sysLibTbl.globalsP);
    }
    if (err != errNone) {
      SysLibCancelRefNum(*refNumP);
    }
  }

  return err;
}
