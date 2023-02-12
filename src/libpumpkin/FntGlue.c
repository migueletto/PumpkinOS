#include <PalmOS.h>
#include <FntGlue.h>
  
#include <time.h>
#include <sys/time.h>

#include "thread.h"
#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

FontID FntGlueGetDefaultFontID(FontDefaultType inFontType) {
  FontID fontID;

  switch (inFontType) {
    case defaultSmallFont:  fontID = stdFont;   break;
    case defaultLargeFont:  fontID = largeFont; break;
    case defaultBoldFont:   fontID = boldFont;  break;
    case defaultSystemFont: fontID = stdFont;   break;
    default: fontID = stdFont; break;
  }

  return fontID;
}

Boolean FntGlueTruncateString(char* iDstString, const char* iSrcString, FontID iFont, Coord iMaxWidth, Boolean iAddEllipsis) {
  debug(DEBUG_ERROR, "PALMOS", "FntGlueTruncateString not implemented");
  return 0;
}

Int16 FntGlueWCharWidth(WChar iChar) {
  debug(DEBUG_ERROR, "PALMOS", "FntGlueWCharWidth not implemented");
  return 0;
}

Int16 FntGlueWidthToOffset(const Char* charsP, UInt16 length, Int16 pixelWidth, Boolean* leadingEdge, Int16* truncWidth) {
  return FntWidthToOffset(charsP, length, pixelWidth, leadingEdge, truncWidth);
}
