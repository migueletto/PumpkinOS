#include <PalmOS.h>

#include "debug.h"

static void AbtShowAboutEx(UInt32 creator, UInt16 formID) {
  FormType *frm, *previous;
  MemHandle h;
  DmSearchStateType stateInfo;
  UInt16 cardNo;
  LocalID dbID;
  char name[dmDBNameLength], buf[64], *s;

  if ((frm = FrmInitForm(formID)) != NULL) {
    MemSet(name, sizeof(name), 0);
    if (DmGetNextDatabaseByTypeCreator(true, &stateInfo, sysFileTApplication, creator, false, &cardNo, &dbID) == errNone) {
      DmDatabaseInfo(cardNo, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }
    FrmCopyLabel(frm, 11001, name);

    MemSet(buf, sizeof(buf), 0);
    if ((h = DmGet1Resource(verRsc, 1)) != NULL) {
      if ((s = MemHandleLock(h)) != NULL) {
        if (StrLen(s) < 16) {
          StrPrintF(buf, "v. %s", s);
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
    FrmCopyLabel(frm, 11002, buf);

    StrPrintF(buf, "About %s", name);
    FrmSetTitle(frm, buf);

    previous = FrmGetActiveForm();
    FrmDoDialog(frm);
    FrmSetTitle(frm, NULL);
    FrmDeleteForm(frm);
    FrmSetActiveForm(previous);
  }
}

// Marked as system use only
void AbtShowAbout(UInt32 creator) {
  AbtShowAboutEx(creator, aboutDialog);
}

void AbtShowAboutPumpkin(UInt32 creator) {
  AbtShowAboutEx(creator, aboutDialog+1);
}
