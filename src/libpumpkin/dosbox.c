#include <PalmOS.h>
#include <VFSMgr.h>

#include "pumpkin.h"
#include "dosbox.h"
#include "unzip.h"
#include "debug.h"

#define DOSBOX_DRIVEC DOSBOX_HOME "/C"
#define DOSBOX_RUN    "run.bat"
#define DOSBOX_BAT    "dosbox.bat"
#define DOSBOX_APP    "DOSBox"
#define DOSBOX_NAME   'dosN'

UInt32 DOSBoxMain(Boolean sameTask) {
  LocalID dbID;
  MemHandle h;
  FileRef fileRef;
  UInt32 result;
  char *s, name[16], buf[256];
  Err err;
  Boolean ok;

  if ((dbID = DmFindDatabase(0, DOSBOX_APP)) != 0) {
    MemSet(name, sizeof(name), 0);
    if ((h = DmGet1Resource(DOSBOX_NAME, 1)) != NULL) {
      if ((s = MemHandleLock(h)) != NULL) {
        StrNCopy(name, s, 8);
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    } else {
      FrmCustomAlert(1000, "Name resource not found.", "", "");
    }

    if (name[0]) {
      StrNPrintF(buf, sizeof(buf)-1, "%s/%s/", DOSBOX_DRIVEC, name);
      ok = false;

      if ((err = VFSFileOpen(1, buf, vfsModeRead, &fileRef)) != errNone) {
        if ((err = VFSDirCreate(1, buf)) == errNone) {
          if (pumpkin_unzip_resource(zipRsc, 1, 1, buf) == 0) {
            err = VFSFileOpen(1, buf, vfsModeRead, &fileRef);
          } else {
            FrmCustomAlert(1000, "Error unziping resource.", "", "");
            err = -1;
          }
        } else {
          FrmCustomAlert(1000, "Error creaing dir: ", name, "");
        }
      }

      if (err == errNone) {
        VFSFileClose(fileRef);
        StrNPrintF(buf, sizeof(buf)-1, "%s/%s/%s", DOSBOX_DRIVEC, name, DOSBOX_BAT);

        if (VFSFileOpen(1, buf, vfsModeRead, &fileRef) == errNone) {
          VFSFileClose(fileRef);
          StrNPrintF(buf, sizeof(buf)-1, "%s/%s", DOSBOX_DRIVEC, DOSBOX_RUN);

          if (VFSFileOpen(1, buf, vfsModeWrite, &fileRef) == errNone) {
            VFSFileClose(fileRef);
            VFSFileDelete(1, buf);
          }
          VFSFileCreate(1, buf);

          if (VFSFileOpen(1, buf, vfsModeWrite, &fileRef) == errNone) {
            StrNPrintF(buf, sizeof(buf)-1, "cd %s\r\n%s\r\n", name, DOSBOX_BAT, DOSBOX_RUN);

            if (VFSFileWrite(fileRef, StrLen(buf), buf, NULL) == errNone) {
              ok = true;
            } else {
              FrmCustomAlert(1000, "Error writing file: ", DOSBOX_BAT, "");
            }
            VFSFileClose(fileRef);
          } else {
            FrmCustomAlert(1000, "Error opening file for write: ", DOSBOX_RUN, "");
          }
        } else {
          FrmCustomAlert(1000, "File not found: ", DOSBOX_BAT, "");
        }
      }

      if (ok) {
        SysAppLaunch(0, dbID, 0, sameTask ? dosboxLaunchCmd : sysAppLaunchCmdNormalLaunch, NULL, &result);
      }
    }
  } else {
    FrmCustomAlert(1000, DOSBOX_APP, " is not installed.", "");
  }

  return 0;
}
