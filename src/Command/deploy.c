#include <PalmOS.h>

#include "sys.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "pumpkin.h"
#include "storage.h"
#include "file.h"
#include "deploy.h"
#include "debug.h"

#define MAX_SCRIPT 32768

Err command_app_deploy(char *name, UInt32 creator, char *script) {
  LocalID dbID;
  DmOpenRef dbRef;
  MemHandle icon;
  Int32 len, iconLen;
  char *buf;
  void *p;
  Err err = dmErrInvalidParam;

  icon = DmGet1Resource('Icon', 1);
  iconLen = MemHandleSize(icon);
  p = MemHandleLock(icon);

  if (name) {
    if ((buf = MemPtrNew(MAX_SCRIPT)) != NULL) {
      if ((len = read_file(script, buf, MAX_SCRIPT)) > 0) {
        if ((dbID = DmFindDatabase(0, name)) != 0) {
          DmDeleteDatabase(0, dbID);
        }
        if ((err = DmCreateDatabase(0, name, creator, sysFileTApplication, true)) == errNone) {
          if ((dbID = DmFindDatabase(0, name)) != 0) {
            if ((dbRef = DmOpenDatabase(0, dbID, dmModeWrite)) != NULL) {
              if (DmNewResourceEx(dbRef, sysRsrcTypeScript, 1, len, buf) != NULL) {
                DmNewResourceEx(dbRef, iconType, 1000, iconLen, p);
                err = errNone;
              }
              DmCloseDatabase(dbRef);
            }
          }
        }
      }
      MemPtrFree(buf);
    }
  }

  MemHandleUnlock(icon);
  DmReleaseResource(icon);

  return err;
}
