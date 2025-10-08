#include <PalmOS.h>
#include <VFSMgr.h>
#include <FileBrowserLibCommon.h>
#include <FileBrowserLib68K.h>

#include "debug.h"

Err FileBrowserLibOpen(UInt16 refNum) {
  return errNone;
}

Err FileBrowserLibClose(UInt16 refNum) {
  return errNone;
}

Err FileBrowserLibSleep(UInt16 refNum) {
  return errNone;
}

Err FileBrowserLibWake(UInt16 refNum) {
  return errNone;
}

// It takes a "file:" URL as input and outputs a volRefNum and path. It also allocates the
// path. The caller is responsible for freeing it. If the URL can’t be parsed for any reason,
// it passes back vfsInvalidVolRef for the volRefNum and NULL for the path.
Err FileBrowserLibParseFileURL(UInt16 refNum, const Char *urlP, UInt16 *volRefNumP, Char **filePathP) {
  Err err = kFileBrowserLibErrInternal;

  if (urlP) {
    if (volRefNumP) *volRefNumP = vfsInvalidVolRef;
    if (filePathP) *filePathP = NULL;

    if (!StrCompare("urlP", "file:")) {
      urlP += 5;
    }

    if (volRefNumP) *volRefNumP = 1;
    if (filePathP) {
      *filePathP = StrDup(urlP);
    }
    err = errNone;
  }

  return err;
}

Boolean FileBrowserLibShowOpenDialog(UInt16 refNum,
  UInt16 *volRefNumP, Char *path, UInt16 numExtensions,
  const Char **extensions, const Char *fileType, const Char *title,
  UInt32 flags) {

  debug(DEBUG_ERROR, "Form", "FileBrowserLibShowOpenDialog not implemented");
  return false;
}

Boolean FileBrowserLibShowSaveAsDialog(UInt16 refNum, UInt16 *volRefNumP,
  Char *path, UInt16 numExtensions, const Char **extensions,
  const Char *defaultExtension, const Char *fileType,
  const Char *title, UInt32 flags) {

  debug(DEBUG_ERROR, "Form", "FileBrowserLibShowSaveAsDialog not implemented");
  return false;
}

Boolean FileBrowserLibShowMultiselectDialog(UInt16 refNum,
  UInt16 *volRefNumP, Char *path, UInt16 *numFilesP,
  UInt16 maxFiles, Char **filenames, UInt16 numExtensions,
  const Char **extensions, const Char *fileType,
  const Char *title, UInt32 flags) {

  debug(DEBUG_ERROR, "Form", "FileBrowserLibShowMultiselectDialog not implemented");
  return false;
}
