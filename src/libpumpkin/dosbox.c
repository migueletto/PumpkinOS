#include <PalmOS.h>
#include <VFSMgr.h>

#include "pumpkin.h"
#include "dosbox.h"
#include "debug.h"

#define DOSBOX_DRIVEC DOSBOX_HOME "/C"
#define DOSBOX_RUN    "run.bat"
#define DOSBOX_BAT    "dosbox.bat"
#define DOSBOX_APP    "DOSBox"
#define DOSBOX_NAME   'dosN'

UInt32 DOSBoxMain(void) {
  LocalID dbID;
  MemHandle h;
  FileRef fileRef;
  UInt32 result;
  char *s, name[16], buf[256];
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
      debug(DEBUG_ERROR, DOSBOX_APP, "name resource not found");
    }

    if (name[0]) {
      StrNPrintF(buf, sizeof(buf)-1, "%s/%s", DOSBOX_DRIVEC, name);
      ok = false;

      if (VFSFileOpen(1, buf, vfsModeRead, &fileRef) == errNone) {
        VFSFileClose(fileRef);
        StrNPrintF(buf, sizeof(buf)-1, "%s/%s/%s", DOSBOX_DRIVEC, name, DOSBOX_BAT);

        if (VFSFileOpen(1, buf, vfsModeRead, &fileRef) == errNone) {
          VFSFileClose(fileRef);
          StrNPrintF(buf, sizeof(buf)-1, "%s/%s", DOSBOX_DRIVEC, DOSBOX_RUN);
          VFSFileDelete(1, buf);

          if (VFSFileOpen(1, buf, vfsModeWrite, &fileRef) == errNone) {
            StrNPrintF(buf, sizeof(buf)-1, "cd %s\r\n%s\r\n", name, DOSBOX_BAT);

            if (VFSFileWrite(fileRef, StrLen(buf), buf, NULL) == errNone) {
              ok = true;
            } else {
              debug(DEBUG_ERROR, DOSBOX_APP, "error writing bat file");
            }
            VFSFileClose(fileRef);
          } else {
            debug(DEBUG_ERROR, DOSBOX_APP, "error opening %s for write", buf);
          }
        } else {
          debug(DEBUG_ERROR, DOSBOX_APP, "%s now found", buf);
        }
      } else {
        debug(DEBUG_ERROR, DOSBOX_APP, "%s now found", buf);
      }

      if (ok) {
        SysAppLaunch(0, dbID, 0, sysAppLaunchCmdNormalLaunch, NULL, &result);
      }
    }
  } else {
    debug(DEBUG_ERROR, DOSBOX_APP, "%s now found", DOSBOX_APP);
  }

  return 0;
}
