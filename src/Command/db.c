#include <PalmOS.h>

#include "db.h"

DmOpenRef DbOpen(LocalID dbID, UInt16 mode, Err *err) {
  DmOpenRef dbRef;

  if (!(dbRef = DmOpenDatabase(0, dbID, mode))) {
    *err = DmGetLastErr();
    return NULL;
  }

  *err = 0;
  return dbRef;
}

DmOpenRef DbOpenByName(char *name, UInt16 mode, Err *err) {
  LocalID dbID;

  if ((dbID = DmFindDatabase(0, name)) == 0) {
    *err = dmErrCantFind;
    return NULL;
  }

  return DbOpen(dbID, mode, err);
}

Err DbClose(DmOpenRef dbRef) {
  return dbRef ? DmCloseDatabase(dbRef) : 0;
}

Err DbCreate(char *name, UInt32 type, UInt32 creator) {
  return DmCreateDatabase(0, name, creator, type, false);
}

Err DbResCreate(char *name, UInt32 type, UInt32 creator) {
  return DmCreateDatabase(0, name, creator, type, true);
}

Err DbGetTypeCreator(char *name, UInt32 *type, UInt32 *creator) {
  LocalID dbID;

  if ((dbID = DmFindDatabase(0, name)) == 0)
    return dmErrCantFind;

  return DmDatabaseInfo(0, dbID, 
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            type, creator);
}

Err DbGetAttributes(DmOpenRef dbRef, UInt16 *attr) {
  LocalID dbID;
  Err err;

  *attr = 0;

  if ((err = DmOpenDatabaseInfo(dbRef, &dbID, NULL, NULL, NULL, NULL)) != 0)
    return err;

  return DmDatabaseInfo(0, dbID, NULL, attr,
           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

Err DbSetAttributes(DmOpenRef dbRef, UInt16 attr) {
  LocalID dbID;
  Err err;

  if ((err = DmOpenDatabaseInfo(dbRef, &dbID, NULL, NULL, NULL, NULL)) != 0)
    return err;

  return DmSetDatabaseInfo(0, dbID, NULL, &attr,
           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

char *DbOpenRec(DmOpenRef dbRef, UInt16 index, Err *err) {
  MemHandle recH;
  char *rec;

  if (!dbRef) {
    *err = -1; // XXX
    return NULL;
  }

  if (!(recH = DmQueryRecord(dbRef, index))) {
    *err = DmGetLastErr();
    return NULL;
  }

  if (!(recH = DmGetRecord(dbRef, index))) {
    *err = DmGetLastErr();
    return NULL;
  }

  if ((rec = MemHandleLock(recH)) == NULL) {
    *err = DmGetLastErr();
    DmReleaseRecord(dbRef, index, false);
    return NULL;
  }

  *err = 0;
  return rec;
}

Err DbCloseRec(DmOpenRef dbRef, UInt16 index, char *rec) {
  Err err1, err2;

  if (!dbRef || !rec)
    return -1; // XXX

  err1 = MemPtrUnlock(rec);
  err2 = DmReleaseRecord(dbRef, index, false);

  return err1 ? err1 : err2;
}

Err DbCreateRec(DmOpenRef dbRef, UInt16 *index, UInt16 size, UInt16 category) {
  MemHandle recH;
  char *rec;
  Err err;

  *index = DbNumRecords(dbRef);

  if (!dbRef)
    return -1; // XXX

  if (!(recH = DmNewRecord(dbRef, index, size)))
    return DmGetLastErr();

  if ((rec = MemHandleLock(recH)) == NULL) {
    err = DmGetLastErr();
    DmReleaseRecord(dbRef, *index, false);
    DmRemoveRecord(dbRef, *index);
    return err;
  }

  if ((err = DmSet(rec, 0, size, 0)) != 0) {
    MemHandleUnlock(recH);
    DmReleaseRecord(dbRef, *index, false);
    DmRemoveRecord(dbRef, *index);
    return err;
  }

  MemHandleUnlock(recH);
  DmReleaseRecord(dbRef, *index, false);

  return 0;
}

Err DbDeleteRec(DmOpenRef dbRef, UInt16 index) {
  if (!dbRef)
    return -1; // XXX

  return DmDeleteRecord(dbRef, index);
}

Err DbRemoveRec(DmOpenRef dbRef, UInt16 index) {
  if (!dbRef)
    return -1; // XXX

  return DmRemoveRecord(dbRef, index);
}

Err DbResizeRec(DmOpenRef dbRef, UInt16 index, UInt32 size) {
  if (!dbRef)
    return -1; // XXX

  if (DmResizeRecord(dbRef, index, size) == NULL)
    return DmGetLastErr();

  return 0;
}

Err DbGetRecAttributes(DmOpenRef dbRef, UInt16 index, UInt16 *attr) {
  UInt32 uid;
  LocalID id;

  if (!dbRef)
    return -1; // XXX

  return DmRecordInfo(dbRef, index, attr, &uid, &id);
}

Err DbSetRecAttributes(DmOpenRef dbRef, UInt16 index, UInt16 attr) {
  Err err;
  UInt16 aux;
  UInt32 uid;
  LocalID id;

  if (!dbRef)
    return -1; // XXX

  if ((err = DmRecordInfo(dbRef, index, &aux, &uid, &id)) != 0)
    return err;

  return DmSetRecordInfo(dbRef, index, &attr, &uid);
}

Err DbGetRecID(DmOpenRef dbRef, UInt16 index, UInt32 *uid) {
  LocalID id;
  UInt16 attr;

  if (!dbRef)
    return -1; // XXX

  return DmRecordInfo(dbRef, index, &attr, uid, &id);
}

UInt16 DbNumRecords(DmOpenRef dbRef) {
  return DmNumRecords(dbRef);
}

char *DbOpenAppInfo(DmOpenRef dbRef) {
  LocalID id;

  if (!dbRef)
    return NULL;

  if ((id = DmGetAppInfoID(dbRef)) == 0)
    return NULL;

  return MemLocalIDToLockedPtr(id, 0);
}

void DbCloseAppInfo(char *info) {
  if (info)
    MemPtrUnlock(info);
}
