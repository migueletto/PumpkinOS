#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define LABEL   "Volume"

#define MEDIA_TYPE  expMediaType_PoserHost
#define FS_MOUNT    vfsMountClass_POSE
#define FS_CREATOR  BOOT_CREATOR

#if SYS_OS == 2
#define FS_TYPE     vfsFilesystemType_NTFS
#else
#define FS_TYPE     vfsFilesystemType_EXT2
#endif

#define MAX_CARD  32
#define MAX_PATH  256

#define PALMOS_MODULE "VFSMgr"
#define NUM_VOLUMES   4

#define return_err(err) \
  pumpkin_set_lasterr(err); \
  if (err && err != vfsErrFileEOF && err != vfsErrFileNotFound && err != expErrEnumerationEmpty) \
    debug(DEBUG_ERROR, "VFS", "%s: error 0x%04X (%s)", __FUNCTION__, err, pumpkin_error_msg(err)); \
  return err

#define checkvol(module, volRefNum) \
  if (volRefNum < 1 || volRefNum > NUM_VOLUMES || module->volume[volRefNum - 1][0] == 0) { \
    return_err(vfsErrVolumeBadRef); \
  }

typedef struct {
  vfs_session_t *session[NUM_VOLUMES];
  char volume[NUM_VOLUMES][MAX_CARD];
  char path[MAX_PATH];
  char path2[MAX_PATH];
  char tmpname[MAX_PATH];
} vfs_module_t;

static void buildpath(vfs_module_t *module, UInt16 volRefNum, char *dst, char *src) {
  char *s;

  MemSet(dst, MAX_PATH, 0);

  if (src[0] == '/') {
    StrNCopy(dst, module->volume[volRefNum - 1], MAX_PATH - 1);
    src++;
  } else {
    s = vfs_cwd(module->session[volRefNum - 1]);
    StrNCopy(dst, s, MAX_PATH - 1);
  }
  s = &dst[StrLen(dst)];
  StrNCat(dst, src, MAX_PATH - 1);

  // Some programs (Cellar Door) uses "/Palm", but pumpkin uses "/PALM" (and some filesystems are case sensitive)
  if (StrNCompare(src, "Palm", 4) == 0) {
    s[1] = 'A';
    s[2] = 'L';
    s[3] = 'M';
  }
}

int VFSInitModule(void) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);

  if (module == NULL) {
    if ((module = xcalloc(1, sizeof(vfs_module_t))) == NULL) {
      return -1;
    }

    pumpkin_set_local_storage(vfs_key, module);
  }

  return 0;
}

Boolean VFSVolumeExists(char *volume) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  vfs_session_t *session;
  Boolean r = false;

  if (module && volume && volume[0]) {
    session = vfs_open_session();
    r = vfs_chdir(session, volume) == 0;
    vfs_close_session(session);
  }

  return r;
}

int VFSAddVolume(char *volume) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  int i, volRefNum = -1;

  if (module && volume && volume[0]) {
    for (i = 0; i < NUM_VOLUMES; i++) {
      if (!StrCompare(volume, module->volume[i])) {
        volRefNum = i + 1;
        debug(DEBUG_INFO, PALMOS_MODULE, "volume \"%s\" already added as volRefNum %d", volume, volRefNum);
        return volRefNum;
      }
    }

    for (i = 0; i < NUM_VOLUMES; i++) {
      if (module->volume[i][0] == 0) break;
    }

    if (i < NUM_VOLUMES) {
      StrNCopy(module->volume[i], volume, MAX_CARD-1);
      volRefNum = i + 1;
      module->session[i] = vfs_open_session();
      if (vfs_chdir(module->session[i], module->volume[i]) == -1) {
        vfs_mkdir(module->session[i], module->volume[i]);
        vfs_chdir(module->session[i], module->volume[i]);
      }
      debug(DEBUG_TRACE, PALMOS_MODULE, "volume \"%s\" added as volRefNum %d", volume, volRefNum);
    } else {
      debug(DEBUG_ERROR, PALMOS_MODULE, "no room for volume \"%s\"", volume);
    }
  }

  return volRefNum;
}

int VFSFinishModule(void) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  int i;

  if (module) {
    for (i = 0; i < NUM_VOLUMES; i++) {
      if (module->session[i]) vfs_close_session(module->session[i]);
    }
    xfree(module);
  }

  return 0;
}

Err VFSInit(void) {
  return_err(errNone);
}

char *VFSGetMount(UInt16 volRefNum) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  char *m;

  if (volRefNum < 1 || volRefNum > NUM_VOLUMES || module->volume[volRefNum - 1][0] == 0) {
    m = NULL;
  } else {
    m = vfs_getmount(module->session[volRefNum-1], "/");
  }

  return m;
}

Err VFSFileCreate(UInt16 volRefNum, const Char *pathNameP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  vfs_file_t *f;
  Err err = vfsErrBadName;

  checkvol(module, volRefNum);

  if (pathNameP && pathNameP[0]) {
    buildpath(module, volRefNum, module->path, (char *)pathNameP);
    if ((f = vfs_open(module->session[volRefNum-1], module->path, VFS_WRITE | VFS_TRUNC)) != NULL) {
      vfs_close(f);
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileOpen(UInt16 volRefNum, const Char *pathNameP, UInt16 openMode, FileRef *fileRefP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  vfs_dir_t *d;
  vfs_file_t *f;
  int type, mode;
  Err err = vfsErrBadName;

  checkvol(module, volRefNum);

  if (pathNameP && pathNameP[0]) {
    buildpath(module, volRefNum, module->path, (char *)pathNameP);
    debug(DEBUG_TRACE, PALMOS_MODULE, "VFSFileOpen %d \"%s\" -> \"%s\"", volRefNum, pathNameP, module->path);
    type = vfs_checktype(module->session[volRefNum-1], module->path);

    if (type == -1) {
      err = vfsErrFileNotFound;
    } else if (type == VFS_DIR) {
      if ((d = vfs_opendir(module->session[volRefNum-1], module->path)) != NULL) {
        if (fileRefP) {
          *fileRefP = d;
          err = errNone;
        }
      }
    } else if (type == VFS_FILE) {
      mode = 0;
      if (openMode & vfsModeRead)  mode |= VFS_READ;
      if (openMode & vfsModeWrite) mode |= VFS_WRITE;
      if ((f = vfs_open(module->session[volRefNum-1], module->path, mode)) != NULL) {
        if (fileRefP) {
          *fileRefP = f;
          err = errNone;
        }
      }
    }
  }

  return_err(err);
}

Err VFSFileClose(FileRef fileRef) {
  vfs_dir_t *d;
  vfs_file_t *f;
  int type;
  Err err = vfsErrFileBadRef;

  if (fileRef) {
    type = vfs_type(fileRef);
    if (type == VFS_FILE) {
      f = (vfs_file_t *)fileRef;
      if (vfs_close(f) == 0) {
        err = errNone;
      }
    } else if (type == VFS_DIR) {
      d = (vfs_dir_t *)fileRef;
      if (vfs_closedir(d) == 0) {
        err = errNone;
      }
    }
  }

  return_err(err);
}

char *VFSFileGets(FileRef fileRef, UInt32 numBytes, char *bufP) {
  vfs_file_t *f;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    if (bufP) {
      bufP = vfs_gets(f, bufP, numBytes);
    }
  }

  return bufP;
}

Err VFSFileRead(FileRef fileRef, UInt32 numBytes, void *bufP, UInt32 *numBytesReadP) {
  vfs_file_t *f;
  int nread;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    if (bufP) {
      nread = numBytes ? vfs_read(f, (uint8_t *)bufP, numBytes) : 0;
      if (nread >= 0) {
        if (numBytesReadP) *numBytesReadP = nread;
        err = errNone;
      }
    }
  }

  return_err(err);
}

Err VFSFileReadData(FileRef fileRef, UInt32 numBytes, void *bufBaseP, UInt32 offset, UInt32 *numBytesReadP) {
  uint8_t *buf = (uint8_t *)bufBaseP;
  return VFSFileRead(fileRef, numBytes, buf ? &buf[offset] : NULL, numBytesReadP);
}

Err VFSFileWrite(FileRef fileRef, UInt32 numBytes, const void *dataP, UInt32 *numBytesWrittenP) {
  vfs_file_t *f;
  int nwritten;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    if (dataP) {
      nwritten = numBytes ? vfs_write(f, (uint8_t *)dataP, numBytes) : 0;
      if (nwritten >= 0) {
        if (numBytesWrittenP) *numBytesWrittenP = nwritten;
        err = errNone;
      }
    }
  }

  return_err(err);
}

Err VFSFileDelete(UInt16 volRefNum, const Char *pathNameP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  Err err = vfsErrBadName;

  checkvol(module, volRefNum);

  if (pathNameP && pathNameP[0]) {
    buildpath(module, volRefNum, module->path, (char *)pathNameP);
    if (vfs_unlink(module->session[volRefNum-1], module->path) == 0) {
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileRename(UInt16 volRefNum, const Char *pathNameP, const Char *newNameP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  Err err = vfsErrBadName;

  checkvol(module, volRefNum);

  if (pathNameP && pathNameP[0] && newNameP && newNameP[0]) {
    buildpath(module, volRefNum, module->path, (char *)pathNameP);
    buildpath(module, volRefNum, module->path2, (char *)newNameP);
    if (vfs_rename(module->session[volRefNum-1], module->path, module->path2) == 0) {
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileSeek(FileRef fileRef, FileOrigin origin, Int32 offset) {
  vfs_file_t *f;
  int fromend;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    switch (origin) {
      case vfsOriginBeginning: fromend =  0; break;
      case vfsOriginCurrent:   fromend = -1; break;
      case vfsOriginEnd:       fromend =  1; break;
      default: return_err(sysErrParamErr);
    }
    if (vfs_seek(f, offset, fromend) != -1) {
      err = VFSFileEOF(fileRef) ? vfsErrFileEOF : errNone;
    }
  }

  return_err(err);
}

Err VFSFileTruncate(FileRef fileRef, Int32 offset) {
  vfs_file_t *f;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    if (vfs_truncate(f, offset) != -1) {
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileEOF(FileRef fileRef) {
  vfs_file_t *f;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    switch (vfs_peek(f, 0)) {
      case 0: err = vfsErrFileEOF; break;
      case 1: err = errNone; break;
    }
  }

  return_err(err);
}

Err VFSFileTell(FileRef fileRef, UInt32 *filePosP) {
  vfs_file_t *f;
  uint32_t pos;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    pos = vfs_seek(f, 0, -1);
    if (pos != -1) {
      if (filePosP) *filePosP = pos;
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileSize(FileRef fileRef, UInt32 *fileSizeP) {
  vfs_file_t *f;
  uint32_t current, size;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    current = vfs_seek(f, 0, -1);
    size = vfs_seek(f, 0, 1);
    if (current != -1 && size != -1 && vfs_seek(f, current, 0) != -1) {
      if (fileSizeP) *fileSizeP = size;
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileResize(FileRef fileRef, UInt32 newSize) {
  vfs_file_t *f;
  Err err = vfsErrFileBadRef;

  if (fileRef && vfs_type(fileRef) == VFS_FILE) {
    f = (vfs_file_t *)fileRef;
    if (vfs_seek(f, newSize, 0) != -1) {
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileGetAttributes(FileRef fileRef, UInt32 *attributesP) {
  vfs_file_t *f;
  vfs_ent_t *ent;
  int type;
  Err err = vfsErrFileBadRef;

  if (fileRef) {
    type = vfs_type(fileRef);
    if (type == VFS_FILE) {
      f = (vfs_file_t *)fileRef;
      if ((ent = vfs_fstat(f)) != NULL) {
        if (attributesP) {
          *attributesP = 0;
        }
        err = errNone;
      }
    } else if (type == VFS_DIR) {
      if (attributesP) {
        *attributesP = vfsFileAttrDirectory;
      }
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileSetAttributes(FileRef fileRef, UInt32 attributes) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSFileSetAttributes not implemented");
  return_err(sysErrParamErr);
}

Err VFSFileGetDate(FileRef fileRef, UInt16 whichDate, UInt32 *dateP) {
  vfs_file_t *f;
  vfs_ent_t *ent;
  uint32_t dt;
  int type;
  Err err = vfsErrFileBadRef;

  if (fileRef) {
    type = vfs_type(fileRef);
    if (type == VFS_FILE) {
      f = (vfs_file_t *)fileRef;
      if ((ent = vfs_fstat(f)) != NULL) {
        if (dateP) {
          dt = pumpkin_dt();
          switch (whichDate) {
            case vfsFileDateCreated:  *dateP = dt + ent->ctime; break;
            case vfsFileDateModified: *dateP = dt + ent->mtime; break;
            case vfsFileDateAccessed: *dateP = dt + ent->atime; break;
            default: return_err(sysErrParamErr);
          }
        }
        err = errNone;
      }
    } else if (type == VFS_DIR) {
      if (dateP) {
        dt = pumpkin_dt();
        switch (whichDate) {
          case vfsFileDateCreated:  *dateP = dt; break; // XXX
          case vfsFileDateModified: *dateP = dt; break; // XXX
          case vfsFileDateAccessed: *dateP = dt; break; // XXX
          default: return_err(sysErrParamErr);
        }
      }
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSFileSetDate(FileRef fileRef, UInt16 whichDate, UInt32 date) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSFileSetDate not implemented");
  return_err(sysErrParamErr);
}

Err VFSDirCreate(UInt16 volRefNum, const Char *dirNameP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  Err err = vfsErrBadName;

  checkvol(module, volRefNum);

  if (dirNameP && dirNameP[0]) {
    buildpath(module, volRefNum, module->path, (char *)dirNameP);
    debug(DEBUG_TRACE, PALMOS_MODULE, "VFSDirCreate \"%s\" -> \"%s\"", dirNameP, module->path);
    if (vfs_mkdir(module->session[volRefNum-1], module->path) == 0) {
      err = errNone;
    }
  }

  return_err(err);
}

Err VFSDirEntryEnumerate(FileRef dirRef, UInt32 *dirEntryIteratorP, FileInfoType *infoP) {
  vfs_dir_t *d;
  vfs_ent_t *ent;
  Err err = vfsErrFileBadRef;

  if (dirRef) {
    if (vfs_type(dirRef) == VFS_DIR) {
      d = (vfs_dir_t *)dirRef;

      for (;;) {
        ent = vfs_readdir(d);
        if (ent == NULL) break;
        if (StrCompare(ent->name, ".") && StrCompare(ent->name, "..")) break;
      }

      if (ent != NULL) {
        if (dirEntryIteratorP) {
          if (*dirEntryIteratorP == vfsIteratorStart) {
            *dirEntryIteratorP = 0;
          } else {
            *dirEntryIteratorP = *dirEntryIteratorP + 1;
          }
        }
        if (infoP) {
          infoP->attributes = 0;
          if (!ent->wr) infoP->attributes |= vfsFileAttrReadOnly;
          if (ent->type == VFS_DIR) infoP->attributes |= vfsFileAttrDirectory;
          if (infoP->nameP && infoP->nameBufLen) {
            StrNCopy(infoP->nameP, ent->name, infoP->nameBufLen);
          }
          err = errNone;
        }
      } else {
        if (dirEntryIteratorP) *dirEntryIteratorP = vfsIteratorStop;
        err = expErrEnumerationEmpty;
      }
    } else {
      if (dirEntryIteratorP) *dirEntryIteratorP = vfsIteratorStop;
      err = vfsErrNotADirectory;
    }
  }

  return_err(err);
}

Err VFSGetDefaultDirectory(UInt16 volRefNum, const Char *fileTypeStr, Char *pathStr, UInt16 *bufLenP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  Err err = sysErrParamErr;

  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSGetDefaultDirectory not implemented");
  checkvol(module, volRefNum);

  if (fileTypeStr && pathStr && bufLenP) {
    StrNCopy(pathStr, "/", *bufLenP);
    *bufLenP = 1;
    err = errNone;
  }

  return_err(err);
}

Err VFSRegisterDefaultDirectory(const Char *fileTypeStr, UInt32 mediaType, const Char *pathStr) {
  char stype[8];

  if (fileTypeStr && pathStr) {
    pumpkin_id2s(mediaType, stype);
    debug(DEBUG_ERROR, PALMOS_MODULE, "VFSRegisterDefaultDirectory \"%s\" '%s' \"%s\" not implemented", fileTypeStr, stype, pathStr);
  }

  return_err(errNone);
}

Err VFSUnregisterDefaultDirectory(const Char *fileTypeStr, UInt32 mediaType) {
  char stype[8];

  if (fileTypeStr) {
    pumpkin_id2s(mediaType, stype);
    debug(DEBUG_ERROR, PALMOS_MODULE, "VFSUnregisterDefaultDirectory \"%s\" '%s' not implemented", fileTypeStr, stype);
  }

  return_err(errNone);
}

Err VFSVolumeFormat(UInt8 flags, UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP) {
  return_err(vfsErrNoFileSystem);
}

Err VFSVolumeMount(UInt8 flags, UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP) {
  return_err(errNone);
}

Err VFSVolumeUnmount(UInt16 volRefNum) {
  return_err(errNone);
}

Err VFSVolumeEnumerate(UInt16 *volRefNumP, UInt32 *volIteratorP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  UInt32 i, num;
  Err err = sysErrParamErr;

  if (volRefNumP && volIteratorP) {
    for (i = 0, num = 0; i < NUM_VOLUMES && module->volume[i][0]; i++) {
      num++;
    }

    switch (*volIteratorP) {
      case vfsIteratorStart:
      case 0x80000000: // XXX WinLauncher passes volIterator = 0x80000000
        if (num == 0) {
          *volIteratorP = vfsIteratorStop;
          err = expErrEnumerationEmpty;
        } else if (num == 1) {
          *volRefNumP = 1;
          *volIteratorP = vfsIteratorStop;
          err = errNone;
        } else {
          *volRefNumP = 1;
          *volIteratorP = 1;
          err = errNone;
        }
        break;
      case vfsIteratorStop:
        err = sysErrParamErr;
        break;
      default:
        if (num <= 1 || *volIteratorP >= num) {
          err = sysErrParamErr;
        } else if (*volIteratorP == num - 1) {
          *volRefNumP = num;
          *volIteratorP = vfsIteratorStop;
          err = errNone;
        } else {
          *volRefNumP = *volIteratorP + 1;
          *volIteratorP = *volIteratorP + 1;
          err = errNone;
        }
        break;
    }
  }

  return_err(err);
}

Err VFSVolumeInfo(UInt16 volRefNum, VolumeInfoType *volInfoP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);

  checkvol(module, volRefNum);

  if (volInfoP) {
    MemSet(volInfoP, sizeof(VolumeInfoType), 0);
    volInfoP->attributes = 0;
    volInfoP->fsType = FS_TYPE;
    volInfoP->fsCreator = FS_CREATOR;
    volInfoP->mountClass = FS_MOUNT;
    volInfoP->slotLibRefNum = 0;
    volInfoP->slotRefNum = 0;
    volInfoP->mediaType = MEDIA_TYPE;
  }

  return_err(errNone);
}

Err VFSVolumeGetLabel(UInt16 volRefNum, Char *labelP, UInt16 bufLen) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);

  checkvol(module, volRefNum);

  if (labelP && bufLen) {
    StrNPrintF(labelP, bufLen-1, "%s%d", LABEL, volRefNum);
  }

  return_err(errNone);
}

Err VFSVolumeSetLabel(UInt16 volRefNum, const Char *labelP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);

  checkvol(module, volRefNum);

  return_err(vfsErrBadName);
}

Err VFSVolumeSize(UInt16 volRefNum, UInt32 *volumeUsedP, UInt32 *volumeTotalP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  uint64_t total, free;
  Err err = vfsErrFileBadRef;

  checkvol(module, volRefNum);
  buildpath(module, volRefNum, module->path, "");

  if (vfs_statfs(module->session[volRefNum-1], module->path, &total, &free) == 0) {
    if (volumeTotalP) *volumeTotalP = total; // XXX overflow 64 -> 32 bits
    if (volumeUsedP) *volumeUsedP = total - free; // XXX overflow 64 -> 32 bits
    err = errNone;
  }

  return_err(err);
}

Err VFSInstallFSLib(UInt32 creator, UInt16 *fsLibRefNumP) {
  return_err(errNone);
}

Err VFSRemoveFSLib(UInt16 fsLibRefNum) {
  return_err(errNone);
}

Err VFSFileDBGetResource(FileRef ref, DmResType type, DmResID resID, MemHandle *resHP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSFileDBGetResource not implemented");
  return_err(sysErrParamErr);
}

Err VFSExportDatabaseToFileCustom(UInt16 volRefNum, const Char *pathNameP, UInt16 cardNo, LocalID dbID, VFSExportProcPtr exportProcP, void *userDataP) {
  return_err(vfsErrBadName);
}

Err VFSExportDatabaseToFile(UInt16 volRefNum, const Char *pathNameP, UInt16 cardNo, LocalID dbID) {
  return VFSExportDatabaseToFileCustom(volRefNum, pathNameP, cardNo, dbID, NULL, NULL);
}

Err VFSImportDatabaseFromFileCustom(UInt16 volRefNum, const Char *pathNameP, UInt16 *cardNoP, LocalID *dbIDP, VFSImportProcPtr importProcP, void *userDataP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSImportDatabaseFromFileCustom not implemented");
  return_err(sysErrParamErr);
}

Err VFSImportDatabaseFromFile(UInt16 volRefNum, const Char *pathNameP, UInt16 *cardNoP, LocalID *dbIDP) {
  return VFSImportDatabaseFromFileCustom(volRefNum, pathNameP, cardNoP, dbIDP, NULL, NULL);
}

Err VFSFileDBGetRecord(FileRef ref, UInt16 recIndex, MemHandle *recHP, UInt8 *recAttrP, UInt32 *uniqueIDP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSFileDBGetRecord not implemented");
  return_err(sysErrParamErr);
}

Err VFSFileDBInfo(FileRef ref, Char *nameP,
          UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
          UInt32 *modDateP, UInt32 *bckUpDateP,
          UInt32 *modNumP, MemHandle *appInfoHP,
          MemHandle *sortInfoHP, UInt32 *typeP,
          UInt32 *creatorP, UInt16 *numRecordsP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSFileDBInfo not implemented");
  return_err(vfsErrBadData);
}

Err VFSChangeDir(UInt16 volRefNum, char *path) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  char *old, *cwd;
  Err err = sysErrParamErr;

  checkvol(module, volRefNum);

  if (path) {
    if (path[0] == '/') {
      buildpath(module, volRefNum, module->path, path);
    } else {
      StrNCopy(module->path, path, MAX_PATH - 1);
    }
    old = xstrdup(vfs_cwd(module->session[volRefNum-1]));
    if (vfs_chdir(module->session[volRefNum-1], module->path) == 0) {
      cwd = vfs_cwd(module->session[volRefNum-1]);
      if (StrNCompare(module->volume[volRefNum-1], cwd, StrLen(module->volume[volRefNum-1])) == 0) {
        err = errNone;
      } else {
        debug(DEBUG_ERROR, PALMOS_MODULE, "attempt to escape root old=\"%s\" path=\"%s\" new=\"%s\"", old, path, cwd);
        vfs_chdir(module->session[volRefNum-1], old);
      }
    }
    xfree(old);
  }

  return_err(err);
}

Err VFSCurrentDir(UInt16 volRefNum, char *path, UInt16 max) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  char *cwd;
  Err err = sysErrParamErr;

  checkvol(module, volRefNum);

  if (path) {
    cwd = vfs_cwd(module->session[volRefNum-1]);
    cwd += StrLen(module->volume[volRefNum-1]) - 1;
    StrNCopy(path, cwd, max-1);
    err = errNone;
  }

  return_err(err);
}

Err VFSRealPath(UInt16 volRefNum, char *path, char *realPath, UInt16 max) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  char *cwd, *s;
  Int32 len;
  Err err = sysErrParamErr;

  checkvol(module, volRefNum);

  if (path && realPath) {
    buildpath(module, volRefNum, module->path, path);

    if ((cwd = vfs_cwd(module->session[volRefNum-1])) != NULL) {
      if ((s = vfs_abspath(cwd, module->path)) != NULL) {
        len = StrLen(module->volume[volRefNum-1]);
        if (StrNCompare(s, module->volume[volRefNum-1], len) == 0) {
          MemMove(realPath, s + len - 1, max - 1);
          err = errNone;
        }
        xfree(s);
      }
    }
  }

  return_err(err);
}

Int32 VFSFileVPrintF(FileRef fileRef, const char *format, sys_va_list ap) {
  UInt32 numBytesWritten;
  char buf[1024];
  Int32 r;

  r = sys_vsnprintf(buf, sizeof(buf), format, ap);
  VFSFileWrite(fileRef, StrLen(buf), buf, &numBytesWritten);

  return r;
}

Int32 VFSFilePrintF(FileRef fileRef, const char *format, ...) {
  sys_va_list ap;
  UInt32 numBytesWritten;
  char buf[1024];
  Int32 r;

  sys_va_start(ap, format);
  r = sys_vsnprintf(buf, sizeof(buf), format, ap);
  sys_va_end(ap);
  VFSFileWrite(fileRef, StrLen(buf), buf, &numBytesWritten);

  return r;
}

Err VFSGetAttributes(UInt16 volRefNum, const Char *pathNameP, UInt32 *attributesP) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  int type;
  Err err = vfsErrBadName;

  checkvol(module, volRefNum);

  if (pathNameP && pathNameP[0] && attributesP) {
    buildpath(module, volRefNum, module->path, (char *)pathNameP);
    debug(DEBUG_TRACE, PALMOS_MODULE, "VFSFileType \"%s\" -> \"%s\"", pathNameP, module->path);
    type = vfs_checktype(module->session[volRefNum-1], module->path);

    if (type == VFS_FILE) {
      *attributesP = 0;
      err = errNone;
    } else if (type == VFS_DIR) {
      *attributesP = vfsFileAttrDirectory;
      err = errNone;
    }
  }

  return_err(err);
}

char *VFSTmpName(void) {
  vfs_module_t *module = (vfs_module_t *)pumpkin_get_local_storage(vfs_key);
  return module->tmpname;
}

Err VFSCustomControl(UInt32 fsCreator, UInt32 apiCreator, UInt16 apiSelector, void *valueP, UInt16 *valueLenP) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "VFSCustomControl not implemented");
  return_err(sysErrParamErr);
}
