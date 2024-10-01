#include <PalmOS.h>
#include <VFSMgr.h>

#include "junzip.h"
#include "debug.h"

typedef struct {
  JZFile zip;
  Int32 size;
  Int32 pointer;
  UInt8 *p;
  char *dir;
} mem_zip_t;

typedef struct {
  JZFile zip;
  Int32 size;
  Int32 pointer;
  FileRef f;
  char *dir;
} file_zip_t;

static sys_size_t mem_read(JZFile *file, void *buf, sys_size_t size) {
  mem_zip_t *zip = (mem_zip_t *)file;
  Int32 sz = (Int32)size;

  if (buf) {
    if ((zip->pointer + sz) > zip->size) size = zip->size - zip->pointer;
    MemMove(buf, &zip->p[zip->pointer], sz);
    zip->pointer += sz;
  } else {
    sz = 0;
  }

  return sz;
}

static sys_size_t mem_tell(JZFile *file) {
  mem_zip_t *zip = (mem_zip_t *)file;

  return zip->pointer;
}

static int mem_seek(JZFile *file, sys_size_t offset, int whence) {
  mem_zip_t *zip = (mem_zip_t *)file;
  Int32 offs = (Int32)offset;

  switch (whence) {
    case SYS_SEEK_SET:
      if (offs < 0) zip->pointer = 0;
      else zip->pointer = offs <= zip->size ? offs : zip->size;
      break;
    case SYS_SEEK_CUR:
      if (zip->pointer + offs < 0) zip->pointer = 0;
      else zip->pointer = zip->pointer + offs <= zip->size ? zip->pointer + offs : zip->size;
      break;
    case SYS_SEEK_END:
      if (zip->pointer + zip->size + offs < 0) zip->pointer = 0;
      else zip->pointer = zip->pointer + zip->size + offs <= zip->size ? zip->pointer + zip->size + offs : zip->size;
      break;
  }

  return 0;
}

static int mem_error(JZFile *file) {
  return 0;
}

static void mem_close(JZFile *file) {
}

static sys_size_t file_read(JZFile *file, void *buf, sys_size_t size) {
  file_zip_t *zip = (file_zip_t *)file;
  Int32 sz = (Int32)size;
  UInt32 numBytesRead;
  Err err;

  if (buf) {
    err = VFSFileRead(zip->f, sz, buf, &numBytesRead);
    sz = err == errNone ? numBytesRead : 0;
  } else {
    sz = 0;
  }

  return sz;
}

static sys_size_t file_tell(JZFile *file) {
  file_zip_t *zip = (file_zip_t *)file;
  UInt32 filePos;
  Err err;

  err = VFSFileTell(zip->f, &filePos);

  return err == errNone ? filePos : 0;
}

static int file_seek(JZFile *file, sys_size_t offset, int whence) {
  file_zip_t *zip = (file_zip_t *)file;
  Int32 offs = (Int32)offset;
  FileOrigin origin;
  Err err;

  switch (whence) {
    case SYS_SEEK_SET:
      origin = vfsOriginBeginning;
      break;
    case SYS_SEEK_CUR:
      origin = vfsOriginCurrent;
      break;
    case SYS_SEEK_END:
      origin = vfsOriginEnd;
      break;
    default:
      return -1;
  }

  err = VFSFileSeek(zip->f, origin, offs);

  return err == errNone ? 0 : -1;
}

static int file_error(JZFile *file) {
  return 0;
}

static void file_close(JZFile *file) {
  file_zip_t *zip = (file_zip_t *)file;

  VFSFileClose(zip->f);
}

static void writeFile(char *filename, void *data, sys_size_t bytes) {
  FileRef f;
  UInt32 numBytesWritten;
  int i;

  // simplistic directory creation support
  for (i = 0; filename[i]; i++) {
    if (filename[i] != '/') continue;

    filename[i] = '\0'; // terminate string at this point

    if (VFSFileOpen(1, filename, vfsModeRead, &f) != errNone) {
      debug(DEBUG_TRACE, "unzip", "create directory \"%s\"", filename);
      if (VFSDirCreate(1, filename) != errNone) {
        debug(DEBUG_ERROR, "unzip", "couldn't create directory \"%s\"", filename);
        return;
      }
    } else {
      VFSFileClose(f);
    }

    filename[i] = '/'; // Put the separator back
  }

  if (!i || filename[i-1] == '/')
    return; // empty filename or directory entry

  if (VFSFileOpen(1, filename, vfsModeRead, &f) == errNone) {
    debug(DEBUG_TRACE, "unzip", "delete existing file \"%s\"", filename);
    VFSFileClose(f);
    VFSFileDelete(1, filename);
  }

  debug(DEBUG_TRACE, "unzip", "create file \"%s\"", filename);
  if (VFSFileCreate(1, filename) == errNone) {
    if (bytes > 0) {
      if (VFSFileOpen(1, filename, vfsModeWrite, &f) == errNone && f != NULL) {
        VFSFileWrite(f, bytes, data, &numBytesWritten);
        VFSFileClose(f);
      } else {
        debug(DEBUG_ERROR, "unzip", "couldn't open \"%s\" for writing", filename);
      }
    }
  } else {
    debug(DEBUG_ERROR, "unzip", "couldn't create file \"%s\"", filename);
  }
}

static int processFile(JZFile *zip) {
  mem_zip_t *z = (mem_zip_t *)zip;
  JZFileHeader header;
  char *name, filename[1024];
  Int32 len;
  UInt8 *data;

  MemSet(filename, sizeof(filename), 0);
  StrNCopy(filename, z->dir, sizeof(filename) - 2);
  len = StrLen(filename);
  if (len > 0 && filename[len-1] != '/') {
    filename[len-1] = '/';
    filename[len] = 0;
    len++;
  }
  name = filename + len;

  if (jzReadLocalFileHeader(zip, &header, name, sizeof(filename) - len - 1)) {
    debug(DEBUG_ERROR, "unzip", "couldn't read local file header");
    return -1;
  }

  if ((data = (UInt8 *)sys_malloc(header.uncompressedSize)) == NULL) {
    debug(DEBUG_ERROR, "unzip", "couldn't allocate memory");
    return -1;
  }

  debug(DEBUG_TRACE, "unzip", "entry \"%s\", %d (%d) bytes", filename, header.compressedSize, header.uncompressedSize);

  if (jzReadData(zip, &header, data) != Z_OK) {
    debug(DEBUG_ERROR, "unzip", "couldn't read file data");
    sys_free(data);
    return -1;
  }

  writeFile(filename, data, header.uncompressedSize);
  sys_free(data);

  return 0;
}

static int recordCallback(JZFile *zip, int idx, JZFileHeader *header, char *filename, void *user_data) {
  sys_size_t offset;

  offset = zip->tell(zip); // store current position

  if (zip->seek(zip, header->offset, SYS_SEEK_SET)) {
    return 0; // abort
  }

  processFile(zip); // alters file offset
  zip->seek(zip, offset, SYS_SEEK_SET); // return to position

  return 1; // continue
}

int pumpkin_unzip_memory(UInt8 *p, UInt32 size, char *dir) {
  JZEndRecord endRecord;
  mem_zip_t *zip;
  int r = -1;

  if (p && dir) {
    zip = sys_malloc(sizeof(mem_zip_t));
    zip->zip.read = mem_read;
    zip->zip.tell = mem_tell;
    zip->zip.seek = mem_seek;
    zip->zip.error = mem_error;
    zip->zip.close = mem_close;
    zip->size = size;
    zip->p = p;
    zip->dir = dir;

    if (jzReadEndRecord((JZFile *)zip, &endRecord) == 0) {
      if (jzReadCentralDirectory((JZFile *)zip, &endRecord, recordCallback, NULL) == 0) {
        r = 0;
      } else {
        debug(DEBUG_ERROR, "unzip", "jzReadCentralDirectory failed");
      }
    } else {
      debug(DEBUG_ERROR, "unzip", "jzReadEndRecord failed");
    }
    sys_free(zip);
  }

  return r;
}

int pumpkin_unzip_resource(UInt32 type, UInt16 id, char *dir) {
  MemHandle h;
  UInt8 *p;
  int r = -1;

  if (dir && (h = DmGetResource(type, id)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      r = pumpkin_unzip_memory(p, MemHandleSize(h), dir);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  return r;
}

int pumpkin_unzip_file(FileRef f, char *dir) {
  JZEndRecord endRecord;
  file_zip_t *zip;
  int r = -1;

  if (f && dir) {
    zip = sys_malloc(sizeof(file_zip_t));
    zip->zip.read = file_read;
    zip->zip.tell = file_tell;
    zip->zip.seek = file_seek;
    zip->zip.error = file_error;
    zip->zip.close = file_close;
    zip->f = f;
    zip->dir = dir;

    if (jzReadEndRecord((JZFile *)zip, &endRecord) == 0) {
      if (jzReadCentralDirectory((JZFile *)zip, &endRecord, recordCallback, NULL) == 0) {
        r = 0;
      } else {
        debug(DEBUG_ERROR, "unzip", "jzReadCentralDirectory failed");
      }
    } else {
      debug(DEBUG_ERROR, "unzip", "jzReadEndRecord failed");
    }
    sys_free(zip);
  }

  return r;
}

int pumpkin_unzip_filename(char *filename, char *dir) {
  FileRef f;
  int r = -1;

  if (filename && dir && VFSFileOpen(1, filename, vfsModeRead, &f) == errNone) {
    r = pumpkin_unzip_file(f, dir);
    VFSFileClose(f);
  }

  return r;
}
