#include <PalmOS.h>
#include <FSLib.h>

#include "debug.h"

Err FSLibOpen(UInt16 fsLibRefNum) {
  return errNone;
}

Err FSLibClose(UInt16 fsLibRefNum) {
  return errNone;
}

Err FSLibSleep(UInt16 fsLibRefNum) {
  return errNone;
}

Err FSLibWake(UInt16 fsLibRefNum) {
  return errNone;
}

UInt32 FSLibAPIVersion(UInt16 fsLibRefNum) {
  return fsLibAPIVersion;
}

Err FSFilesystemType(UInt16 fsLibRefNum, UInt32 *filesystemTypeP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFilesystemType not implemented");
  return sysErrParamErr;
}

Err FSFileCreate(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *pathNameP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileCreate not implemented");
  return sysErrParamErr;
}

Err FSFileClose(UInt16 fsLibRefNum, FileRef fileRef) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileClose not implemented");
  return sysErrParamErr;
}

Err FSFileDelete(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *pathNameP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileDelete not implemented");
  return sysErrParamErr;
}

Err FSFileRename(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *pathNameP, const Char *newNameP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileRename not implemented");
  return sysErrParamErr;
}

Err FSFileSeek(UInt16 fsLibRefNum, FileRef fileRef, FileOrigin origin, Int32 offset) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileSeek not implemented");
  return sysErrParamErr;
}

Err FSFileEOF(UInt16 fsLibRefNum, FileRef fileRef) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileEOF not implemented");
  return sysErrParamErr;
}

Err FSFileTell(UInt16 fsLibRefNum, FileRef fileRef, UInt32 *filePosP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileTell not implemented");
  return sysErrParamErr;
}

Err FSFileResize(UInt16 fsLibRefNum, FileRef fileRef, UInt32 newSize) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileResize not implemented");
  return sysErrParamErr;
}

Err FSFileGetAttributes(UInt16 fsLibRefNum, FileRef fileRef, UInt32 *attributesP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileGetAttributes not implemented");
  return sysErrParamErr;
}

Err FSFileSetAttributes(UInt16 fsLibRefNum, FileRef fileRef, UInt32 attributes) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileSetAttributes not implemented");
  return sysErrParamErr;
}

Err FSFileGetDate(UInt16 fsLibRefNum, FileRef fileRef, UInt16 whichDate, UInt32 *dateP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileGetDate not implemented");
  return sysErrParamErr;
}

Err FSFileSetDate(UInt16 fsLibRefNum, FileRef fileRef, UInt16 whichDate, UInt32 date) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileSetDate not implemented");
  return sysErrParamErr;
}

Err FSFileSize(UInt16 fsLibRefNum, FileRef fileRef, UInt32 *fileSizeP) {
  debug(DEBUG_ERROR, "PALMOS", "FSFileSize not implemented");
  return sysErrParamErr;
}

Err FSDirCreate(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *dirNameP) {
  debug(DEBUG_ERROR, "PALMOS", "FSDirCreate not implemented");
  return sysErrParamErr;
}

Err FSDirEntryEnumerate(UInt16 fsLibRefNum, FileRef dirRef, UInt32 *dirEntryIteratorP, FileInfoType *infoP) {
  debug(DEBUG_ERROR, "PALMOS", "FSDirEntryEnumerate not implemented");
  return sysErrParamErr;
}

Err FSVolumeFormat(UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP) {
  debug(DEBUG_ERROR, "PALMOS", "FSVolumeFormat not implemented");
  return sysErrParamErr;
}

Err FSVolumeMount(UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP) {
  debug(DEBUG_ERROR, "PALMOS", "FSVolumeMount not implemented");
  return sysErrParamErr;
}

Err FSVolumeUnmount(UInt16 fsLibRefNum, UInt16 volRefNum) {
  debug(DEBUG_ERROR, "PALMOS", "FSVolumeUnmount not implemented");
  return sysErrParamErr;
}

Err FSVolumeInfo(UInt16 fsLibRefNum, UInt16 volRefNum, VolumeInfoType *volInfoP) {
  debug(DEBUG_ERROR, "PALMOS", "FSVolumeInfo not implemented");
  return sysErrParamErr;
}

Err FSVolumeGetLabel(UInt16 fsLibRefNum, UInt16 volRefNum, Char *labelP, UInt16 bufLen) {
  debug(DEBUG_ERROR, "PALMOS", "FSVolumeGetLabel not implemented");
  return sysErrParamErr;
}

Err FSVolumeSetLabel(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *labelP) {
  debug(DEBUG_ERROR, "PALMOS", "FSVolumeSetLabel not implemented");
  return sysErrParamErr;
}

Err FSVolumeSize(UInt16 fsLibRefNum, UInt16 volRefNum, UInt32 *volumeUsedP, UInt32 *volumeTotalP) {
  debug(DEBUG_ERROR, "PALMOS", "FSVolumeSize not implemented");
  return sysErrParamErr;
}
