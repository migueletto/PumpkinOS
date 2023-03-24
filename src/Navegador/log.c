#include <PalmOS.h>

#include "log.h"
#include "ddb.h"

FileHand OpenLog(char *name, UInt32 creator, UInt32 type, UInt32 mode) {
  Err err;
  FileHand f;

  if ((f = FileOpen(0, name, type, creator, mode, &err)) == NULL)
    return NULL;

  SeekLog(f, 0);

  return f;
}

void CloseLog(FileHand f) {
  if (f) FileClose(f);
}

Err DeleteLog(char *name) {
  return FileDelete(0, name);
}

Err RenameLog(char *oldname, char *newname) {
  return DbRename(oldname, newname);
}

Err SeekLog(FileHand f, Int32 offset) {
  return FileSeek(f, offset, fileOriginEnd);
}

UInt32 LogSize(FileHand f) {
  UInt32 size;
  Err err;

  FileTell(f, (Int32 *)&size, &err);
  return err ? 0 : size;
}

Err WriteLog(FileHand f, void *buf, UInt32 n) {
  Err err = -1;
  if (f) FileWrite(f, buf, 1, n, &err);
  return err;
}

Err ReadLog(FileHand f, void *buf, UInt32 n) {
  Err err = -1;
  if (f) FileRead(f, buf, 1, n, &err);
  return err;
}
