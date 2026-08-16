#include <PalmOS.h>
#include <SonyCLIE.h>

#include "pumpkin.h"
#include "syslibs.h"
#include "debug.h"

typedef struct {
  Boolean hrmode;
} sony_lib_t;

Err HROpen(UInt16 refNum) {
  sony_lib_t *module;
  Err err = hrErrNoFeature;

  if (refNum == SonyHRLibRefNum) {
    if (pumpkin_get_density() == kDensityDouble) {
      if ((module = sys_calloc(1, sizeof(sony_lib_t))) != NULL) {
        pumpkin_set_local_storage(sonylib_key, module);
        err = errNone;
      }
    } else {
      debug(DEBUG_ERROR, "SonyHR", "screen density is not double");
    }
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u", refNum);
  }

  return err;
}

Err HRClose(UInt16 refNum) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  Err err = hrErrNotOpen;

  if (refNum == SonyHRLibRefNum && module) {
    pumpkin_set_local_storage(sonylib_key, NULL);
    sys_free(module);
    err = errNone;
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }

  return err;
}

Err HRGetAPIVersion(UInt16 refNum, UInt16 *versionP) {
  if (versionP) *versionP = HR_VERSION_SUPPORT_FNTSIZE;
  return errNone;
}

void HRWinClipRectangle(UInt16 refNum, RectangleType *rP) {
}

void HRWinCopyRectangle(UInt16 refNum, WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  UInt32 oldScalingMode;
  UInt16 prevCoordSys;

  if (refNum == SonyHRLibRefNum && module) {
    if (module->hrmode) {
      prevCoordSys = WinSetCoordinateSystem(kDensityDouble);
      oldScalingMode = WinSetScalingMode(kBitmapScalingOff);
      WinCopyRectangle(srcWin, dstWin, srcRect, destX, destY, mode);
      WinSetScalingMode(oldScalingMode);
      WinSetCoordinateSystem(prevCoordSys);
    } else {
      WinCopyRectangle(srcWin, dstWin, srcRect, destX, destY, mode);
    }
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }
}

WinHandle HRWinCreateBitmapWindow(UInt16 refNum, BitmapType *bitmapP, UInt16 *error) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  WinHandle wh = NULL;

  if (refNum == SonyHRLibRefNum && module) {
    wh = WinCreateBitmapWindow(bitmapP, error);
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }

  return wh;
}

WinHandle HRWinCreateOffscreenWindow(UInt16 refNum, Coord width, Coord height, WindowFormatType format, UInt16 *error) {
  return NULL;
}

WinHandle HRWinCreateWindow(UInt16 refNum, RectangleType *bounds, FrameType frame, Boolean modal, Boolean focusable, UInt16 *error) {
  return NULL;
}

void HRWinDisplayToWindowPt(UInt16 refNum, Coord *extentX, Coord *extentY) {
}

void HRWinDrawBitmap(UInt16 refNum, BitmapType *bitmapP, Coord x, Coord y) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  UInt32 oldScalingMode;
  UInt16 prevCoordSys;

  if (refNum == SonyHRLibRefNum && module) {
    if (module->hrmode) {
      prevCoordSys = WinSetCoordinateSystem(kDensityDouble);
      oldScalingMode = WinSetScalingMode(kBitmapScalingOff);
      WinDrawBitmap(bitmapP, x, y);
      WinSetScalingMode(oldScalingMode);
      WinSetCoordinateSystem(prevCoordSys);
    } else {
      WinDrawBitmap(bitmapP, x, y);
    }
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }
}

void HRWinDrawChar(UInt16 refNum, WChar theChar, Coord x, Coord Y) {
}

void HRWinDrawChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y) {
}

void HRWinDrawGrayLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2) {
}

void HRWinDrawGrayRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP) {
}

void HRWinDrawInvertedChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y) {
}

void HRWinDrawLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  UInt16 prevCoordSys;

  if (refNum == SonyHRLibRefNum && module) {
    if (module->hrmode) {
      prevCoordSys = WinSetCoordinateSystem(kDensityDouble);
      WinDrawLine(x1, y1, x2, y2);
      WinSetCoordinateSystem(prevCoordSys);
    } else {
      WinDrawLine(x1, y1, x2, y2);
    }
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }
}

void HRWinDrawPixel(UInt16 refNum, Coord x, Coord y) {
}

void HRWinDrawRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  UInt16 prevCoordSys;

  if (refNum == SonyHRLibRefNum && module) {
    if (module->hrmode) {
      prevCoordSys = WinSetCoordinateSystem(kDensityDouble);
      WinDrawRectangle(rP, cornerDiam);
      WinSetCoordinateSystem(prevCoordSys);
    } else {
      WinDrawRectangle(rP, cornerDiam);
    }
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }
}

void HRWinDrawRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP) {
}

void HRWinDrawTruncChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth) {
}

void HRWinEraseChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y) {
}

void HRWinEraseLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2) {
}

void HRWinErasePixel(UInt16 refNum, Coord x, Coord y) {
}

void HRWinEraseRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam) {
}

void HRWinEraseRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP) {
}

void HRWinFillLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2) {
}

void HRWinFillRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam) {
}

void HRWinGetClip(UInt16 refNum, RectangleType *rP) {
}

void HRWinGetDisplayExtent(UInt16 refNum, Coord *extentX, Coord *extentY) {
}

void HRWinGetFramesRectangle(UInt16 refNum, FrameType frame, RectangleType *rP, RectangleType *obscuredRectP) {
}

IndexedColorType HRWinGetPixel(UInt16 refNum, Coord x, Coord y) {
  return 0;
}

void HRWinGetWindowBounds(UInt16 refNum, RectangleType *rP) {
}

void HRWinGetWindowExtent(UInt16 refNum, Coord *extentX, Coord *extentY) {
}

void HRWinGetWindowFrameRect(UInt16 refNum, WinHandle winHandle, RectangleType *rP) {
}

void HRWinInvertChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y) {
}

void HRWinInvertLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2) {
}

void HRWinInvertPixel(UInt16 refNum, Coord x, Coord y) {
}

void HRWinInvertRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam) {
}

void HRWinInvertRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP) {
}

void HRWinPaintBitmap(UInt16 refNum, BitmapType *bitmapP, Coord x, Coord y) {
}

void HRWinPaintChar(UInt16 refNum, WChar theChar, Coord x, Coord y) {
}

void HRWinPaintChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y) {
}

void HRWinPaintLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2) {
}

void HRWinPaintLines(UInt16 refNum, UInt16 numLines, WinLineType lines[]) {
}

void HRWinPaintPixel(UInt16 refNum, Coord x, Coord y) {
}

void HRWinPaintPixels(UInt16 refNum, UInt16 numPoints, PointType pts[]) {
}

void HRWinPaintRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam) {
}

void HRWinPaintRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP) {
}

void HRWinRestoreBits(UInt16 refNum, WinHandle winHandle, Coord destX, Coord destY) {
}

WinHandle HRWinSaveBits(UInt16 refNum, RectangleType *sourceP, UInt16 *error) {
  return NULL;
}

Err HRWinScreenMode(UInt16 refNum, WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  Err err = sysErrParamErr;

  if (refNum == SonyHRLibRefNum && module) {
    switch (operation) {
      case winScreenModeGet:
        err = WinScreenMode(operation, widthP, heightP, depthP, enableColorP);
        if (err == errNone) {
          if (module->hrmode) {
            if (widthP) *widthP = hrWidth;
            if (heightP) *heightP = hrHeight;
            debug(DEBUG_INFO, "SonyHR", "winScreenModeGet in high-resolution mode");
          } else {
            if (widthP) *widthP = stdWidth;
            if (heightP) *heightP = stdHeight;
            debug(DEBUG_INFO, "SonyHR", "winScreenModeGet in compatibility mode");
          }
        }
        break;
      case winScreenModeSet:
        if (widthP && heightP && *widthP == hrWidth && *heightP == hrHeight) {
          if (!module->hrmode) {
            debug(DEBUG_INFO, "SonyHR", "winScreenModeSet switching to high-resolution mode");
            module->hrmode = true;
          }
        } else if (widthP && heightP && *widthP == stdWidth && *heightP == stdHeight) {
          if (module->hrmode) {
            debug(DEBUG_INFO, "SonyHR", "winScreenModeSet switching to compatibility mode");
            module->hrmode = false;
          }
        }
        err = WinScreenMode(operation, widthP, heightP, depthP, enableColorP);
        break;
      case winScreenModeSetToDefaults:
        if (module->hrmode) {
          debug(DEBUG_INFO, "SonyHR", "winScreenModeSetToDefaults switching to compatibility mode");
          module->hrmode = false;
        }
        err = WinScreenMode(operation, widthP, heightP, depthP, enableColorP);
        break;
      default:
        err = WinScreenMode(operation, widthP, heightP, depthP, enableColorP);
        break;
    }
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }

  return err;
}

void HRWinScrollRectangle(UInt16 refNum, RectangleType *rP, WinDirectionType direction, Coord distance, RectangleType *vacatedP) {
}

void HRWinSetClip(UInt16 refNum, RectangleType *rP) {
}

void HRWinSetWindowBounds(UInt16 refNum, WinHandle winHandle, RectangleType *rP) {
}

void HRWinWindowToDisplayPt(UInt16 refNum, Coord *extentX, Coord *extentY) {
}

Err HRWinGetPixelRGB(UInt16 refNum, Coord x, Coord y, RGBColorType *rgbP) {
  return errNone;
}

UInt32 HRBmpBitsSize(UInt16 refNum, BitmapType *bitmapP) {
  UInt32 bitsSize;
  BmpGetSizes(bitmapP, &bitsSize, NULL);
  return bitsSize;
}

UInt32 HRBmpSize(UInt16 refNum, BitmapType *bitmapP) {
  return 0;
}

BitmapType *HRBmpCreate(UInt16 refNum, Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, UInt16 *error) {
  sony_lib_t *module = (sony_lib_t *)pumpkin_get_local_storage(sonylib_key);
  BitmapType *bitmap = NULL;

  if (refNum == SonyHRLibRefNum && module) {
    bitmap = BmpCreate(width, height, depth, colortableP, error);
  } else {
    debug(DEBUG_ERROR, "SonyHR", "invalid refNum %u or module %p is null", refNum, module);
  }

  return bitmap;
}

HRFontID HRFntGetFont(UInt16 refNum) {
  return 0;
}

HRFontID HRFntSetFont(UInt16 refNum, HRFontID font) {
  return 0;
}

HRFontID HRFontSelect(UInt16 refNum, HRFontID font) {
  return 0;
}

Int16 HRFntBaseLine(UInt16 refNum) {
  return 0;
}

Int16 HRFntCharHeight(UInt16 refNum) {
  return 0;
}

Int16 HRFntLineHeight(UInt16 refNum) {
  return 0;
}

Int16 HRFntAverageCharWidth(UInt16 refNum) {
  return 0;
}

Int16 HRFntCharWidth(UInt16 refNum, Char ch) {
  return 0;
}

Int16 HRFntWCharWidth(UInt16 refNum, WChar iChar) {
  return 0;
}

Int16 HRFntCharsWidth(UInt16 refNum, Char const *chars, Int16 len) {
  return 0;
}

Int16 HRFntWidthToOffset(UInt16 refNum, Char const *pChars, UInt16 length, Int16 pixelWidth, Boolean *leadingEdge, Int16 *truncWidth) {
  return 0;
}

void HRFntCharsInWidth(UInt16 refNum, Char const *string, Int16 *stringWidthP, Int16 *stringLengthP, Boolean *fitWithinWidth) {
}

Int16 HRFntDescenderHeight(UInt16 refNum) {
  return 0;
}

Int16 HRFntLineWidth(UInt16 refNum, Char const *pChars, UInt16 length) {
  return 0;
}

UInt16 HRFntWordWrap(UInt16 refNum, Char const *chars, UInt16 maxWidth) {
  return 0;
}

void HRFntWordWrapReverseNLines(UInt16 refNum, Char const *const chars, UInt16 maxWidth, UInt16 *linesToScrollP, UInt16 *scrollPosP) {
}

void HRFntGetScrollValues(UInt16 refNum, Char const *chars, UInt16 width, UInt16 scrollPos, UInt16 *linesP, UInt16 *topLine) {
}
