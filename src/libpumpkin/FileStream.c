#include <PalmOS.h>

#include "sys.h"
#include "mutex.h"
#include "pwindow.h"
#include "vfs.h"
#include "mem.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define PALMOS_MODULE "File"

typedef struct {
  LocalID dbID;
  DmOpenRef dbRef;
  UInt32 openMode;
} FileType;

FileHand FileOpen(UInt16 cardNo, const Char *nameP, UInt32 type, UInt32 creator, UInt32 openMode, Err *errP) {
  UInt16 attr, excl;
  UInt32 creatorf, typef;
  char s1[8], s2[8];
  DmOpenRef dbRef;
  LocalID dbID;
  Boolean exists;
  FileType *f = NULL;

  /*
  openMode: Mode in which to open the file stream. You
  must specify only one primary mode selector.
  Additionally, you can use the | operator
  (bitwise inclusive OR) to append one or more
  secondary mode selectors to the primary mode
  selector.
  */

  // Primary Open Mode Constants:
  // fileModeReadOnly   Open for read-only access
  // fileModeReadWrite  Open/create for read/write, discarding any previous version of stream
  // fileModeUpdate     Open/create for read/write, preserving previous version of stream if it exists
  // fileModeAppend     Open/create for read/write, always writing to the end of the stream

  if ((dbID = DmFindDatabase(cardNo, nameP)) != 0) {
    if (DmDatabaseInfo(cardNo, dbID, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &typef, &creatorf) != errNone) {
      if (errP) *errP = fileErrNotFound;
      return NULL;
    }
    if (!(attr & dmHdrAttrStream)) {
      debug(DEBUG_ERROR, PALMOS_MODULE, "FileOpen(\"%s\"): not a stream database", nameP);
      if (errP) *errP = fileErrNotStream;
      return NULL;
    }
    if (!(openMode & fileModeAnyTypeCreator)) {
      if (creator != creatorf) {
        pumpkin_id2s(creator, s1);
        pumpkin_id2s(creatorf, s2);
        debug(DEBUG_ERROR, PALMOS_MODULE, "FileOpen(\"%s\"): parameter creator '%s' does not match file creator '%s'", nameP, s1, s2);
        if (errP) *errP = fileErrTypeCreatorMismatch;
        return NULL;
      }
      if (type != typef) {
        pumpkin_id2s(type, s1);
        pumpkin_id2s(typef, s2);
        debug(DEBUG_ERROR, PALMOS_MODULE, "FileOpen(\"%s\"): parameter type '%s' does not match file type '%s'", nameP, s1, s2);
        if (errP) *errP = fileErrTypeCreatorMismatch;
        return NULL;
      }
    }
    exists = true;
  } else {
    exists = false;
  }

  if (errP) *errP = 0;
  excl = (openMode & fileModeExclusive) ? dmModeExclusive : 0;
  dbRef = NULL;

  if ((openMode & fileModeLeaveOpen)) {
    debug(DEBUG_ERROR, PALMOS_MODULE, "FileOpen mode fileModeLeaveOpen not supported");
  }

  if ((openMode & fileModeUpdate)) {
    if (!dbID) {
      if (DmCreateDatabaseEx(nameP, creator, type, dmHdrAttrStream, 0, true) == errNone) {
        dbID = DmFindDatabase(cardNo, nameP);
      } else {
        if (errP) *errP = fileErrCreateError;
      }
    }
    if (dbID) {
      if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadWrite | excl)) == NULL) {
        if (errP) *errP = fileErrCreateError;
      }
    }
  } else if ((openMode & fileModeReadOnly)) {
    if (dbID) {
      if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadOnly | excl)) == NULL) {
        if (errP) *errP = fileErrNotFound;
      }
    } else {
      if (errP) *errP = fileErrNotFound;
    }
  } else if ((openMode & fileModeReadWrite)) {
    if ((openMode & fileModeDontOverwrite) && exists) {
      if (errP) *errP = fileErrReplaceError;
    } else {
      if (DmCreateDatabaseEx(nameP, creator, type, dmHdrAttrStream, 0, true) == errNone) {
        dbID = DmFindDatabase(cardNo, nameP);
        if (dbID) {
          if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadWrite | excl)) == NULL) {
            if (errP) *errP = fileErrCreateError;
          }
        } else {
          if (errP) *errP = fileErrCreateError;
        }
      } else {
        if (errP) *errP = fileErrCreateError;
      }
    }
  } else if ((openMode & fileModeAppend)) {
    if (!dbID) {
      if (DmCreateDatabaseEx(nameP, creator, type, dmHdrAttrStream, 0, true) == errNone) {
        dbID = DmFindDatabase(cardNo, nameP);
      } else {
        if (errP) *errP = fileErrCreateError;
      }
      if (dbID) {
        if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadWrite | excl)) == NULL) {
          if (errP) *errP = fileErrCreateError;
        }
        StoFileSeek(dbRef, 0, 1);
      }
    }
  } else {
    if (errP) *errP = fileErrInvalidParam;
  }

  if (dbRef) {
    if ((f = pumpkin_heap_alloc(sizeof(FileType), "FileType")) != NULL) {
      f->dbID = dbID;
      f->dbRef = dbRef;
      f->openMode = openMode;
    }
  }

  return f;
}

Err FileClose(FileHand stream) {
  FileType *f;
  Err err = fileErrInvalidParam;

  if (stream) {
    f = (FileType *)stream;
    if (f->dbRef) DmCloseDatabase(f->dbRef);
    if (f->dbID && (f->openMode & fileModeTemporary)) {
      DmDeleteDatabase(0, f->dbID);
    }
    pumpkin_heap_free(f, "FileType");
    err = errNone;
  }

  return err;
}

Err FileDelete(UInt16 cardNo, const Char *nameP) {
  LocalID dbID;
  Err err = fileErrInvalidParam;

  if (nameP) {
    if ((dbID = DmFindDatabase(cardNo, nameP)) != 0) {
      err = DmDeleteDatabase(cardNo, dbID);
    } else {
      err = fileErrNotFound;
    }
  }

  return err;
}

Int32 FileReadLow(FileHand stream, void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize, Int32 numObj, Err *errP) {
  FileType *f;
  Int32 r = 0;

  *errP = fileErrIOError;

  if (stream) {
    f = (FileType *)stream;

    // XXX ignores offset e dataStoreBased
    r = StoFileRead(f->dbRef, baseP, objSize*numObj);

    if (r == -1) {
      *errP = DmGetLastErr();
      r = 0;
    } else {
      r = objSize ? r / objSize : 0;
      *errP = errNone;
    }
  }

  return r;
}

Int32 FileWrite(FileHand stream, const void *dataP, Int32 objSize, Int32 numObj, Err *errP) {
  FileType *f;
  Int32 r = 0;

  *errP = fileErrIOError;

  if (stream) {
    f = (FileType *)stream;

    r = StoFileWrite(f->dbRef, (uint8_t *)dataP, objSize*numObj);

    if (r == -1) {
      *errP = DmGetLastErr();
      r = 0;
    } else {
      r = objSize ? r / objSize : 0;
      *errP = errNone;
    }
  }

  return r;
}

Err FileSeek(FileHand stream, Int32 offset, FileOriginEnum origin) {
  FileType *f;
  int whence;
  Err err = fileErrIOError;

  if (stream) {
    f = (FileType *)stream;

    switch (origin) {
      case fileOriginBeginning: whence =  0; break;
      case fileOriginCurrent  : whence = -1; break;
      case fileOriginEnd      : whence =  1; break;
      default:
        return fileErrInvalidParam;
    }

    if (StoFileSeek(f->dbRef, offset, whence) == 0) {
      err = errNone;
    }
  }

  return err;
}

Int32 FileTell(FileHand stream, Int32 *fileSizeP, Err *errP) {
  FileType *f;
  Int32 r = -1;

  if (errP) *errP = fileErrIOError;

  if (stream) {
    f = (FileType *)stream;

    if ((r = StoFileSeek(f->dbRef, 0, -1)) != -1) {
      if (fileSizeP) {
        *fileSizeP = StoFileSeek(f->dbRef, 0, 1);
        StoFileSeek(f->dbRef, r, 0);
      }
      if (errP) *errP = errNone;
    }
  }

  return r;
}

Err FileTruncate(FileHand stream, Int32 newSize) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "FileTruncate not implemented");
  return 0;
}

Err FileControl(FileOpEnum op, FileHand stream, void *valueP, Int32 *valueLenP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "FileControl not implemented");
  return 0;
}
