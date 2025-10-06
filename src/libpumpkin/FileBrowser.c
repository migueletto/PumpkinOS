#include <PalmOS.h>

#include "debug.h"

Err FileBrowserLibOpen(UInt16 refNum) {
  return errNone;
}

Err FileBrowserLibClose(UInt16 refNum) {
  return errNone;
}

Err FileBrowserLibParseFileURL(UInt16 refNum, const Char *urlP, UInt16 *volRefNumP, Char **filePathP) {
  debug(DEBUG_ERROR, "Form", "FileBrowserLibParseFileURL not implemented");
  return -1;
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
