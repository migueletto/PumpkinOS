#include <PalmOS.h>

#include "ddb.h"

#include "debug.h"

DmOpenRef DbOpen(LocalID dbID, UInt16 mode, Err *err)
{
  DmOpenRef dbRef;

  if (!(dbRef = DmOpenDatabase(0, dbID, mode))) {
    *err = DmGetLastErr();
    return 0;
  }

  *err = 0;
  return dbRef;
}

DmOpenRef DbOpenByName(char *name, UInt16 mode, Err *err)
{
  LocalID dbID;

  if ((dbID = DmFindDatabase(0, name)) == 0) {
    *err = dmErrCantFind;
    return 0;
  }

  return DbOpen(dbID, mode, err);
}

DmOpenRef DbOpenCreateByName(char *name, UInt32 type, UInt32 creator, UInt16 mode, Err *err)
{
  DmOpenRef dbRef;

  if ((dbRef = DbOpenByName(name, mode, err)) != 0)
    return dbRef;

  if ((*err = DbCreate(name, type, creator)) != 0)
    return 0;

  return DbOpenByName(name, mode, err);
}

Err DbClose(DmOpenRef dbRef)
{
  return dbRef ? DmCloseDatabase(dbRef) : 0;
}

Err DbRename(char *oldname, char *newname)
{
  LocalID dbID;

  if (!oldname || !newname)
    return dmErrCantFind;

  if ((dbID = DmFindDatabase(0, oldname)) == 0)
    return dmErrCantFind;

  return DmSetDatabaseInfo(0, dbID, newname,
           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

Err DbDelete(char *name)
{
  LocalID dbID;

  if ((dbID = DmFindDatabase(0, name)) == 0)
    return dmErrCantFind;

  return DmDeleteDatabase(0, dbID);
}

Err DbCreate(char *name, UInt32 type, UInt32 creator)
{
  return DmCreateDatabase(0, name, creator, type, false);
}

Err DbGetTypeCreator(char *name, UInt32 *type, UInt32 *creator)
{
  LocalID dbID;

  if ((dbID = DmFindDatabase(0, name)) == 0)
    return dmErrCantFind;

  return DmDatabaseInfo(0, dbID, 
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            type, creator);
}

Err DbGetAttributes(DmOpenRef dbRef, UInt16 *attr)
{
  LocalID dbID;
  Err err;

  *attr = 0;

  if ((err = DmOpenDatabaseInfo(dbRef, &dbID, NULL, NULL, NULL, NULL)) != 0)
    return err;

  return DmDatabaseInfo(0, dbID, NULL, attr,
           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

Err DbSetAttributes(DmOpenRef dbRef, UInt16 attr)
{
  LocalID dbID;
  Err err;

  if ((err = DmOpenDatabaseInfo(dbRef, &dbID, NULL, NULL, NULL, NULL)) != 0)
    return err;

  return DmSetDatabaseInfo(0, dbID, NULL, &attr,
           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

char *DbOpenRec(DmOpenRef dbRef, UInt16 index, Err *err)
{
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
  return DbUpdateCloseRec(dbRef, index, rec, false);
}

Err DbUpdateCloseRec(DmOpenRef dbRef, UInt16 index, char *rec, Boolean update) {
  Err err1, err2;

  if (!dbRef || !rec)
    return -1; // XXX

  err1 = MemPtrUnlock(rec);
  err2 = DmReleaseRecord(dbRef, index, update);

  return err1 ? err1 : err2;
}

Err DbCreateRec(DmOpenRef dbRef, UInt16 *index, UInt16 size, UInt16 category)
{
  MemHandle recH;
  char *rec;
  Err err;

  if (!dbRef)
    return -1; // XXX

  *index = DmNumRecords(dbRef);

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

Err DbDeleteRec(DmOpenRef dbRef, UInt16 index)
{
  if (!dbRef)
    return -1; // XXX

  return DmRemoveRecord(dbRef, index);
}

Err DbResizeRec(DmOpenRef dbRef, UInt16 index, UInt32 size)
{
  if (!dbRef)
    return -1; // XXX

  if (DmResizeRecord(dbRef, index, size) == NULL)
    return DmGetLastErr();

  return 0;
}
