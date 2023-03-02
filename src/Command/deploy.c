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
  SysNotifyParamType notify;
  SysNotifyDBCreatedType dbCreated;
  UInt32 type;
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
        if ((err = DmCreateDatabase(0, name, creator, 'temp', true)) == errNone) {
          if ((dbID = DmFindDatabase(0, name)) != 0) {
            if ((dbRef = DmOpenDatabase(0, dbID, dmModeWrite)) != NULL) {
              if (DmNewResourceEx(dbRef, sysRsrcTypeScript, 1, len, buf) != NULL) {
                DmNewResourceEx(dbRef, iconType, 1000, iconLen, p);
                err = errNone;
              }
              DmCloseDatabase(dbRef);

              if (err == errNone) {
                type = sysFileTApplication;
                DmSetDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, NULL);

                // trigger a sysNotifyDBCreatedEvent notification only after the database is
                // closed, otherwise Launcher could fail to open it.
                MemSet(&dbCreated, sizeof(dbCreated), 0);
                dbCreated.newDBID = dbID;
                dbCreated.creator = creator;
                dbCreated.type = type;
                dbCreated.resDB = dmHdrAttrResDB;
                StrNCopy(dbCreated.dbName, name, dmDBNameLength-1);

                MemSet(&notify, sizeof(notify), 0);
                notify.notifyType = sysNotifyDBCreatedEvent;
                notify.broadcaster = 0;
                notify.notifyDetailsP = &dbCreated;
                SysNotifyBroadcast(&notify);
              }
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
