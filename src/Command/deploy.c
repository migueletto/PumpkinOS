#include <PalmOS.h>

#include "sys.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "pumpkin.h"
#include "storage.h"
#include "file.h"
#include "deploy.h"
#include "debug.h"
#include "resource.h"

#define MAX_SCRIPT 32768

Err command_app_deploy(char *name, UInt32 creator, char *script) {
  LocalID dbID;
  DmOpenRef dbRef;
  MemHandle icon, form, menu, about;
  SysNotifyParamType notify;
  SysNotifyDBCreatedType dbCreated;
  UInt32 type;
  Int32 len, iconLen, formLen, menuLen, aboutLen;
  void *iconBuf, *formBuf, *menuBuf, *aboutBuf;
  char *buf;
  Err err = dmErrInvalidParam;

  icon = DmGet1Resource('Icon', 1);
  iconLen = MemHandleSize(icon);
  iconBuf = MemHandleLock(icon);

  form = DmGet1Resource(sysRsrcTypeScript, frmScrpID);
  formLen = MemHandleSize(form);
  formBuf = MemHandleLock(form);

  menu = DmGet1Resource(MenuRscType, frmScrpMenu);
  menuLen = MemHandleSize(menu);
  menuBuf = MemHandleLockEx(menu, false);

  about = DmGet1Resource(formRscType, aboutDialog+2);
  aboutLen = MemHandleSize(about);
  aboutBuf = MemHandleLockEx(about, false);

  if (name) {
    if ((buf = MemPtrNew(MAX_SCRIPT)) != NULL) {
      if ((len = read_file(script, buf, MAX_SCRIPT)) > 0) {
        if ((dbID = DmFindDatabase(0, name)) != 0) {
          DmDeleteDatabase(0, dbID);
        }
        if ((err = DmCreateDatabase(0, name, creator, 'temp', true)) == errNone) {
          if ((dbID = DmFindDatabase(0, name)) != 0) {
            if ((dbRef = DmOpenDatabase(0, dbID, dmModeWrite)) != NULL) {
              if (DmNewResourceEx(dbRef, sysRsrcTypeScript, 2, len, buf) != NULL) {
                DmNewResourceEx(dbRef, sysRsrcTypeScript, 1, formLen, formBuf);
                DmNewResourceEx(dbRef, iconType, 1000, iconLen, iconBuf);
                DmNewResourceEx(dbRef, MenuRscType, 1001, menuLen, menuBuf);
                DmNewResourceEx(dbRef, formRscType, aboutDialog+2, aboutLen, aboutBuf);
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

  MemHandleUnlock(about);
  DmReleaseResource(about);

  MemHandleUnlock(menu);
  DmReleaseResource(menu);

  MemHandleUnlock(form);
  DmReleaseResource(form);

  MemHandleUnlock(icon);
  DmReleaseResource(icon);

  return err;
}
